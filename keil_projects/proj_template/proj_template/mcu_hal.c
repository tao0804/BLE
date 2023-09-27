#include "mcu_hal.h"
#include "stack_svc_api.h"
#include "math.h"

#define MCU_AVDD_CFG 2.5
#define ARRAY_NUM(arr)	(sizeof(arr)/sizeof(arr[0]))

uint8 mcuAdcTableNum = 0;
MCU_ADC_TAB * p_mcuAdcTable = NULL;
volatile uint16 mcuAdcUserChIdx = 0;	// 当前采集点的Table索引信息，p_mcuAdcTable[mcuAdcUserChIdx]

const MCU_ADC_TAB adcTable[] = {
	{ADC_CH02, 100, MCU_AVDD_CFG/4096, P1, BIT2, &SYS->P1_MFP, SYS_MFP_P12_Msk, SYS_MFP_P12_ADC_CH2},
	{ADC_CH03, 100, MCU_AVDD_CFG/4096, P1, BIT3, &SYS->P1_MFP, SYS_MFP_P13_Msk, SYS_MFP_P13_ADC_CH3},
};

///////////////////////////////////////////////gpio_user///////////////////////////////////////////////
void mcu_gpio_user_init(void)
{//ldo拉高
	SYS->P1_MFP &= ~(SYS_MFP_P10_Msk);
	SYS->P1_MFP |= SYS_MFP_P10_GPIO;
	GPIO_InitOutput(P1, BIT0, GPIO_HIGH_LEVEL);
	GPIO_PullUp(P1, BIT0, GPIO_PULLUP_ENABLE);
	GPIO_ENABLE_DIGITAL_PATH(P1, BIT0);
	// GPIO_SetBits(P1, BIT0);
	GPIO_ClearBits(P1, BIT0);	// 拉低测低功耗电流
}
///////////////////////////////////////////////adc_driver///////////////////////////////////////////////

void mcu_adc_start_channel_convert(ADC_CHANNEL channel)
{
	ADC_Open(ADC, 0, 0, 0x01 << channel);
	ADC_START_CONV(ADC);
}

void mcu_adc_init(MCU_ADC_TAB *p_table, uint8 tableNum)
{
	int i;
	p_mcuAdcTable = p_table;
	mcuAdcTableNum = tableNum;
	
	for(i = 0; i < mcuAdcTableNum; i++)
	{
		GPIO_SetMode(adcTable[i].p_gpio, adcTable[i].pinMask, GPIO_MODE_INPUT);
		GPIO_PullUp(adcTable[i].p_gpio, adcTable[i].pinMask, GPIO_PULLUP_DISABLE);
		*adcTable[i].mfpReg &= ~adcTable[i].mfpMsk;
		*adcTable[i].mfpReg |= adcTable[i].mfpAdcCh;
		GPIO_DISABLE_DIGITAL_PATH(adcTable[i].p_gpio, adcTable[i].pinMask);
	}
	// 注册中断处理函数到协议栈，否则中断会卡死
	((interrupt_register_handler)SVC_interrupt_register)(ADC_IRQ, mcu_adc_isr);

	CLK_EnableModuleClock(ADC_MODULE);

	//Select ADC input range.1 means 0.4V~2.4V ;0 means 0.4V~1.4V.
	//0.4V~2.4V & 0.4V~1.4V both are theoretical value,the real range is determined by bandgap voltage.
	ADC_SelInputRange(ADC_INPUTRANGE_HIGH);
	// user can set this register to increase the sampling time to get a stable ADC input signal.
	ADC_SetExtraSampleTime(ADC, 0, ADC_SAMPLE_CLOCK_16);

	ADC_POWER_ON(ADC);

	// Enable ADC convert complete interrupt
	ADC_EnableInt(ADC, ADC_ADIF_INT);
	NVIC_EnableIRQ(ADC_IRQn);

	mcuAdcUserChIdx = 0;
	mcu_adc_start_channel_convert(p_mcuAdcTable[mcuAdcUserChIdx].adcChannel);
}

// 结束adc采样
void mcu_adc_deinit(void)
{
	CLK_DisableModuleClock(ADC_MODULE);
	ADC_POWER_DOWN(ADC);
	ADC_DisableInt(ADC, ADC_ADIF_INT);
	NVIC_DisableIRQ(ADC_IRQn);
}

// ADC中断服务程序
void mcu_adc_isr(void)
{
	uint16 adcCode = ADC_GET_CONVERSION_DATA(ADC, 0);
	uint16 adcFlag = ADC_GET_INT_FLAG(ADC, ADC_ADIF_INT);
	ADC_CLR_INT_FLAG(ADC, adcFlag);

	p_mcuAdcTable[mcuAdcUserChIdx].adcCode = adcCode;
	mcuAdcUserChIdx++;

	if (mcuAdcUserChIdx < mcuAdcTableNum) 
	{
		mcu_adc_start_channel_convert(p_mcuAdcTable[mcuAdcUserChIdx].adcChannel);
	}
}


void mcu_adc_main(void)
{
	uint8 i = 0;
	bool roll = false;
	if(mcuAdcUserChIdx >= mcuAdcTableNum)
	{
		for(i = 0; i < mcuAdcTableNum; i++)
			{
				p_mcuAdcTable[i].filterCnt++;
				p_mcuAdcTable[i].adcCodeSum += p_mcuAdcTable[i].adcCode;
				if(p_mcuAdcTable[i].filterCnt >= p_mcuAdcTable[i].filterLen)	// 采购目标次数filterLen
				{
					p_mcuAdcTable[i].voltage = p_mcuAdcTable[i].adcCodeSum / p_mcuAdcTable[i].filterCnt * p_mcuAdcTable[i].convRate;
					p_mcuAdcTable[i].rollCount++;	// 是否平均过
					p_mcuAdcTable[i].filterCnt = 0;
					p_mcuAdcTable[i].adcCodeSum = 0;
				}
			}
			mcuAdcUserChIdx = 0;	
			for(i = 0; i < mcuAdcTableNum; i++)	// 一周期采完adc结束
			{
				if(!p_mcuAdcTable[i].rollCount) roll = true;
				if(roll) mcu_adc_start_channel_convert(p_mcuAdcTable[mcuAdcUserChIdx].adcChannel);
				else mcu_adc_deinit();
			}
	}
}

float mcu_adc_get_voltage(uint8 index)
{
	if(NULL==p_mcuAdcTable || index>=mcuAdcTableNum)
		return 0;
	return p_mcuAdcTable[index].voltage;
}

void mcu_adc_user_init(void)
{
	mcu_adc_init(adcTable, ARRAY_NUM(adcTable));
}

float mcu_adc_get_temperature(uint8 index)
{
	//温度
	//Tsteinhart = 1/(A+B*log(Rth)+C*pow(log(Rth),3))-273.15;
	//Tbeta = 1/(1/(273.15+25)+1/Beta*log(Rth/R25))-273.15;
	//NCP15WF104F03RC 4250-4299 B特征值
	// tempreture_ntc1 = 1.0 / ( 1.0 / ( 273.15 + 25 ) + 1.0 / 4250 * log( NTC1_R / 100000.0 ) ) - 273.15;	
	float tempreture_ntc1;
	float NTC1_R; //某温度时ntc实际阻值
	uint16 B_character_ntc = 4250; //ntc B值
	float R_ntc_25 = 100000; //ntc在25度时标准电阻

	float voltage = mcu_adc_get_voltage(index);
	NTC1_R = ( ( ( 6.25 - voltage ) / ( 6.25 + voltage ) ) * 100000 );	// 运放分析采集的电压与ntc阻值关系
	tempreture_ntc1 = 1.0 / ( 1.0 / ( 273.15 + 25 ) + 1.0 / B_character_ntc * log( NTC1_R / ( R_ntc_25 ) ) ) - 273.15;

	p_mcuAdcTable[index].temperature = tempreture_ntc1;
	return p_mcuAdcTable[index].temperature;
}

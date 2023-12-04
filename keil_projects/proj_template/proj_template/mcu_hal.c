#include "mcu_hal.h"
#include "stack_svc_api.h"
#include "PN102Series.h"
#include "peripherals.h"


// #define MCU_AVDD_CFG 2.5
#define MCU_AVDD_CFG 2.4
#define ARRAY_NUM(arr)	(sizeof(arr)/sizeof(arr[0]))

uint8 mcuAdcTableNum = 0;
MCU_ADC_TAB * p_mcuAdcTable = NULL;
uint8 mcuGpioCnt = 0;
volatile uint16 mcuAdcUserChIdx = 0;	// 当前采集点的Table索引信息,p_mcuAdcTable[mcuAdcUserChIdx]

const MCU_ADC_TAB adcCfgTable[] = {
	{ADC_CH02, 10, MCU_AVDD_CFG/4096, P1, BIT2, &SYS->P1_MFP, SYS_MFP_P12_Msk, SYS_MFP_P12_ADC_CH2},
	{ADC_CH03, 10, MCU_AVDD_CFG/4096, P1, BIT3, &SYS->P1_MFP, SYS_MFP_P13_Msk, SYS_MFP_P13_ADC_CH3},
	{ADC_CH05, 10, MCU_AVDD_CFG/4096, P1, BIT5, &SYS->P1_MFP, SYS_MFP_P15_Msk, SYS_MFP_P15_ADC_CH5},
};

///////////////////////////////////////////////gpio_user///////////////////////////////////////////////
void mcu_gpio_user_init(void)
{//ldo拉高
	SYS->P1_MFP &= ~(SYS_MFP_P10_Msk);
	SYS->P1_MFP |= SYS_MFP_P10_GPIO;
	GPIO_InitOutput(P1, BIT0, GPIO_HIGH_LEVEL);
	GPIO_PullUp(P1, BIT0, GPIO_PULLUP_ENABLE);
	GPIO_ENABLE_DIGITAL_PATH(P1, BIT0);
	GPIO_SetBits(P1, BIT0);
	// GPIO_ClearBits(P1, BIT0);	// 拉低测电流
}

void mcu_gpio_led_init(void)
{// It's not appropriate to set led on/off
	SYS->P1_MFP &= ~(SYS_MFP_P14_Msk);
	SYS->P1_MFP |= SYS_MFP_P14_GPIO;
	// GPIO_InitOutput(P1, BIT4, GPIO_HIGH_LEVEL);
	// GPIO_PullUp(P1, BIT4, GPIO_PULLUP_ENABLE);
	GPIO_SetMode(P2, BIT4, GPIO_MODE_OUTPUT);
	GPIO_ENABLE_DIGITAL_PATH(P1, BIT4);
	// GPIO_SetBits(P1, BIT4);
}

void mcu_led_light(bool light)
{
	if(light)
	{
		GPIO_ClearBits(P1, BIT4);	// led on
	}
	else
	{
		GPIO_SetBits(P1, BIT4);	// led off
	}
}

// commentary:for structure changed
// void led_gpio_first_timing(void)
// {
// 	GPIO_ClearBits(P1, BIT4);
// 	// yu 首次设置定时器
// 	((ke_timer_set_handler)SVC_ke_timer_set)(MCU_GPIO_LED_TOGGLE_TIMER, TASK_APP, 20);	//400 * 10ms
// }

// void mcu_gpio_toggle_TimerCb(void)
// {
// 	 if(sys_first_ble_conn_flag == 0)
// 	 {
// 		++mcuGpioCnt;
// 		if(mcuGpioCnt >= GPIO_TIMER_PERIOD)
// 		{
// 			mcuGpioCnt = 0;
// 			GPIO_ClearBits(P1, BIT4);
// 			// GPIO_Store();
// 		}
// 		// GPIO_ClearBits(P1, BIT4);
// 	 }
// 	 else if(sys_first_ble_conn_flag == 1)
// 	 {
// 		GPIO_SetBits(P1, BIT4);
// 	 }
// }

///////////////////////////////////////////////adc_driver///////////////////////////////////////////////

// start adc convert
void mcu_adc_start_channel_convert(ADC_CHANNEL channel)
{
	ADC_Open(ADC, 0, 0, 0x01 << channel);
	ADC_START_CONV(ADC);
}

// mcu adc init
void mcu_adc_init(MCU_ADC_TAB *p_table, uint8 tableNum)
{
	int i;
	p_mcuAdcTable = p_table;
	mcuAdcTableNum = tableNum;
	
	for(i = 0; i < mcuAdcTableNum; i++)
	{
		GPIO_SetMode(adcCfgTable[i].p_gpio, adcCfgTable[i].pinMask, GPIO_MODE_INPUT);
		GPIO_PullUp(adcCfgTable[i].p_gpio, adcCfgTable[i].pinMask, GPIO_PULLUP_DISABLE);
		*adcCfgTable[i].mfpReg &= ~adcCfgTable[i].mfpMsk;
		*adcCfgTable[i].mfpReg |= adcCfgTable[i].mfpAdcCh;
		GPIO_DISABLE_DIGITAL_PATH(adcCfgTable[i].p_gpio, adcCfgTable[i].pinMask);
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

// 结束 adc 采样
void mcu_adc_deinit(void)
{
	CLK_DisableModuleClock(ADC_MODULE);
	ADC_POWER_DOWN(ADC);
	ADC_DisableInt(ADC, ADC_ADIF_INT);
	NVIC_DisableIRQ(ADC_IRQn);
}

// ADC中断回调
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

#define AVOID_INACCURATE_SAMPLE_NUM 20
/** @brief adc采集一轮后算电压
*	@return 返回-1表示异常,返回0采样结束,返回1未结束(case跳不出while(1)改用return)
*	@note 所有通道采完平均过设定次后会关ADC,低功耗需求
**/
int8 mcu_adc_main(void)
{
	uint8 i = 0;
	if(NULL == p_mcuAdcTable)
		return -1;	// 未初始化
	while(1){
	// uint8 rollCntFlag = 0;
	if(mcuAdcUserChIdx >= mcuAdcTableNum)
	{
		for(i = 0; i < mcuAdcTableNum; i++)
			{
				p_mcuAdcTable[i].filterCnt++;
				p_mcuAdcTable[i].adcCodeSum += p_mcuAdcTable[i].adcCode;
				if(p_mcuAdcTable[i].filterCnt >= p_mcuAdcTable[i].filterLen)	// 采够目标次数filterLen
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
			// if(!p_mcuAdcTable[i].rollCount) rollCntFlag = 1;	// 刚开始采的几次偏差较大,我设为20之后没问题
			// if(rollCntFlag) mcu_adc_start_channel_convert(p_mcuAdcTable[mcuAdcUserChIdx].adcChannel);
			if(p_mcuAdcTable[i].rollCount < AVOID_INACCURATE_SAMPLE_NUM)
				mcu_adc_start_channel_convert(p_mcuAdcTable[mcuAdcUserChIdx].adcChannel);
			else
			{
				mcu_adc_deinit();
				return 0;
			}
			
		}
	}
	return 1;
	}
}
float mcu_adc_get_voltage(uint8 index)
{
	if(NULL==p_mcuAdcTable || index>=mcuAdcTableNum)
		return 0;
	return p_mcuAdcTable[index].voltage;
}

MCU_ADC_TAB adcTable[ARRAY_NUM(adcCfgTable)];

void mcu_adc_user_init(void)
{
	// init global var
	memset(adcTable, 0, sizeof(adcTable));
	memcpy(adcTable, adcCfgTable, sizeof(adcTable));
	// adc init
	mcu_adc_init(adcTable, ARRAY_NUM(adcTable));
}



#include "mcu_hal.h"
#include "stack_svc_api.h"

// 用于追踪已完成的ADC转换次数
uint32_t adcAvgCnt;
uint32_t adcCodeSum;
float adcValue;

#define MCU_VREF 3.3
#define BIT_OF_ADC 0xfff

// 初始化GPIO,P10,ldo_en上拉
void gpio_init(void)
{
	SYS->P1_MFP &= ~(SYS_MFP_P10_Msk);
	SYS->P1_MFP |= SYS_MFP_P10_GPIO;
	GPIO_InitOutput(P1, BIT0, GPIO_HIGH_LEVEL);
	GPIO_PullUp(P1, BIT0, GPIO_PULLUP_ENABLE);
	GPIO_ENABLE_DIGITAL_PATH(P1, BIT0);
//	GPIO_SetBits(P1, BIT0);
	GPIO_ClearBits(P1, BIT0);
}

// 初始化ADC
void adc_init(void)
{
	CLK_EnableModuleClock(ADC_MODULE);

	//P13设置成ADC channel3 analog input
	SYS->P1_MFP &= ~(SYS_MFP_P13_Msk);
	SYS->P1_MFP |= SYS_MFP_P13_ADC_CH3;
	GPIO_DISABLE_DIGITAL_PATH(P1, BIT3);
	//P12设置成ADC channel2 analog input
	SYS->P1_MFP &= ~(SYS_MFP_P12_Msk);
	SYS->P1_MFP |= SYS_MFP_P12_ADC_CH2;
	GPIO_DISABLE_DIGITAL_PATH(P1, BIT2);

	((interrupt_register_handler)SVC_interrupt_register)(ADC_IRQ, adc_isr);

	// Enable channel 2
	// ADC_Open(ADC, 0, 0, 0x01 << 1);	// P10脚2.5v只采一次,P12采100次求平均值
	ADC_Open(ADC, 0, 0, 0x01 << 2);

	//Select ADC input range.1 means 0.4V~2.4V ;0 means 0.4V~1.4V.
	//0.4V~2.4V & 0.4V~1.4V both are theoretical value,the real range is determined by bandgap voltage.
	ADC_SelInputRange(ADC_INPUTRANGE_HIGH);

	// Power on ADC
	ADC_POWER_ON(ADC);

	// Enable ADC convert complete interrupt
	ADC_EnableInt(ADC, ADC_ADIF_INT);
	NVIC_EnableIRQ(ADC_IRQn);
	
	ADC_START_CONV(ADC);
}

// ADC中断服务程序
void adc_isr(void)
{
	uint16 adcFlag = ADC_GET_INT_FLAG(ADC, ADC_ADIF_INT);
	ADC_CLR_INT_FLAG(ADC, adcFlag);

	adcCodeSum += ADC_GET_CONVERSION_DATA(ADC, 0);

	// 增加已完成的转换次数
	adcAvgCnt++;

	if (adcAvgCnt >= 100) 
	{
		adcValue = adcCodeSum / adcAvgCnt * MCU_VREF / BIT_OF_ADC;
		adcCodeSum = 0;
		adcAvgCnt = 0;
	}
	ADC_START_CONV(ADC);
}

float adc_getVoltage(void)
{
	return adcValue;
}

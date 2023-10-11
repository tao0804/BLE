#ifndef _MCU_HAL_H_
#define _MCU_HAL_H_

#include "panble.h"

typedef enum{
	ADC_CH00 = 0,
	ADC_CH01,
	ADC_CH02,
	ADC_CH03,
	ADC_CH04,
	ADC_CH05,
	ADC_CH06,
	ADC_CH07,
}ADC_CHANNEL;
#pragma pack(1)	// 用于指定内存对齐方式的预处理指令
typedef struct{
	ADC_CHANNEL			adcChannel;
	uint16				filterLen;
	float				convRate;	// 转换率,adcCode * convRate = voltage
	GPIO_T				*p_gpio;
	uint32				pinMask;
	__IO uint32_t		*mfpReg;
	uint32				mfpMsk;
	uint32				mfpAdcCh;
	ADC_T				*p_adc;
	// 无需配置
	uint16			adcCode;
	uint32			adcCodeSum;
	uint16			filterCnt;
	uint16			rollCount;
	float			voltage;
	float			temperature;
}MCU_ADC_TAB;

enum {
	MCU_P12_ADC_CH2 = 0,
	MCU_P13_ADC_CH3,
	MCU_P15_ADC_CH5,
};

void mcu_gpio_user_init(void);
void mcu_adc_start_channel_convert(ADC_CHANNEL channel);
void mcu_adc_init(MCU_ADC_TAB *p_table, uint8 tableNum);
void mcu_adc_isr(void);
int8 mcu_adc_main(void);
float mcu_adc_get_voltage(uint8 index);
void mcu_adc_user_init(void);


#endif

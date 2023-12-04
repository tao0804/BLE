#ifndef _MCU_HAL_H_
#define _MCU_HAL_H_

#include "panble.h"
#include "stdbool.h"
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
#pragma pack(1)	// 用于指定内存对齐方式的预处理指令,对齐都是2的幂,通常出现在结构体前面
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
#pragma pack()
enum {
	MCU_P12_ADC_CH2 = 0,
	MCU_P13_ADC_CH3,
	MCU_P15_ADC_CH5,
};

// #define  GPIO_TIMER_PERIOD  1	// commentary:first_ble_conn, for structure changed

void mcu_gpio_user_init(void);
void mcu_gpio_led_init(void);
void mcu_led_light(bool light);

// void led_gpio_first_timing(void);
void mcu_gpio_toggle_TimerCb(void);
void mcu_adc_start_channel_convert(ADC_CHANNEL channel);
void mcu_adc_init(MCU_ADC_TAB *p_table, uint8 tableNum);
void mcu_adc_isr(void);
int8 mcu_adc_main(void);
float mcu_adc_get_voltage(uint8 index);
void mcu_adc_user_init(void);


#endif

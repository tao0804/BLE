#include "mcu_hal.h"
#include "math.h"
#include "PN102Series.h"
#include "temperature.h"
#include "string.h"

#define TEMP_VALUE_MAX_CFG	100
#define TEMP_VALUE_MIN_CFG	(-100)
#define TEMP_VALUE_OVER_MAX	127
#define TEMP_VALUE_LESS_MIN (-128)


int8 tempTable[TEMP_TABLE_MAX_LEN];
uint16 tempCnt;
uint8 tempTimeCnt;

void user_init(void)
{
	mcu_gpio_user_init();
	mcu_adc_user_init();
}

// 初始化全局变量
void temp_relate_init(void)
{
	int8 t;
	memset(tempTable, 0, sizeof(tempTable));
	tempCnt = 0;
	tempTimeCnt = 0;
	t = temp_temporary_sampling();
	temp_history_tempValue(t);
}

//ntc B值,NCP15WF104F03RC 4250-4299 B特征值
#define B_character_ntc	4250
//ntc在25度时标准电阻
#define R_ntc_25		100000.0

// adc电压值到温度值的转换
int8 adcVoltage_convert_temp(float voltage)
{
	int8 tempValue;
	float tempreture_ntc1;
	float NTC1_R;	//某温度时ntc实际阻值
	NTC1_R = (((6.25 - voltage) / (6.25 + voltage)) * 100000);	// 运放分析采集的电压与ntc阻值关系
	tempreture_ntc1 = 1.0 / (1.0 / (273.15 + 25) + 1.0 / B_character_ntc * log(NTC1_R / (R_ntc_25))) - 273.15;
	tempValue = actTemp_to_tempValue(tempreture_ntc1);
	return tempValue;
}

// 实际温度值转int8
int8 actTemp_to_tempValue(float temp)
{
	int32 t;
	//对结果四舍五入,比对初值舍进位再运算精度更高
	t = (int32)round((temp - ZERO_TEMP_VALUE_C) / PRECISION_TEMP_VALUE_C);
	if(t > TEMP_VALUE_OVER_MAX)
		return TEMP_VALUE_OVER_MAX;
	else if(t < TEMP_VALUE_LESS_MIN)
		return TEMP_VALUE_LESS_MIN;
	return (int8)t;
}

// 保存温度值到tempTable，同时更新tempCnt
void temp_history_tempValue(int8 tempvalue)
{
	tempTable[tempCnt % TEMP_TABLE_MAX_LEN] = tempvalue;
	tempCnt++;
}

#define TEMP_VALUE_ERROR	(-127)
//获取温度值,cnt表示第几次采样
int8 temp_get_tempValue(uint16 cnt)
{
	//当采到505次时,第3次数据已被覆盖
	if(cnt > tempCnt || cnt == 0 || 
	  (tempCnt > TEMP_TABLE_MAX_LEN && tempCnt - cnt >= TEMP_TABLE_MAX_LEN))
		return TEMP_VALUE_ERROR;
	return tempTable[(cnt - 1) % TEMP_TABLE_MAX_LEN];
}

// 采样完成次数
uint16 current_sampling_tempcnt(void)
{
	return tempCnt;
}

// 阻塞采一次温度,不存储
int8 temp_temporary_sampling(void)
{
    float v;
	int8 t;
	user_init();
	while(mcu_adc_main());	// 阻塞到采样完成
    GPIO_ClearBits(P1, BIT0);
	v = mcu_adc_get_voltage(MCU_P12_ADC_CH2);
	t = adcVoltage_convert_temp(v);
	return t;
}

// 每调用SAMPLE_TEMPER_PERIOD次,阻塞采一次温度,后存储
// 定时器回调每1min调一次此接口
void temp_sampleTimerCb(void)
{
	float v;
	int8 t;
	if(++tempTimeCnt >= TEMP_SAMPLING_PERIOD)
		tempTimeCnt = 0;	// 每过SAMPLE_TEMPER_PERIOD次真正采样一次
	else
		return;
	user_init();
    while(mcu_adc_main());	// 阻塞到采样完成
    GPIO_ClearBits(P1, BIT0);
	v = mcu_adc_get_voltage(MCU_P12_ADC_CH2);
	t = adcVoltage_convert_temp(v);
	temp_history_tempValue(t);
}

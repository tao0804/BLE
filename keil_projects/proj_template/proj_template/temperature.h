#ifndef _TEMPERATURE_H_
#define _TEMPERATURE_H_
#include "panble.h"

// TemperTable数组大小
#define TEMP_TABLE_MAX_LEN		500
// TemperValue 0值温度 （摄氏度）
#define ZERO_TEMP_VALUE_C		30.0f
// TemperValue 精度 （摄氏度）
#define PRECISION_TEMP_VALUE_C	0.05f
// TemperValue 转 真实温度 （摄氏度）
#define TEMP_VALUE_TO_C(value)  (ZERO_TEMP_VALUE_C + value * PRECISION_TEMP_VALUE_C)

// 用于指定内存对齐方式的预处理指令,通常出现在结构体前面
#pragma pack(1)

typedef struct TempCfg
{
	uint16 zeroTempValue;
	uint16 precisionTempValue;
	uint16 tempTableMaxLen;
	uint8 sampleTempPeriod;
}TempCfg_t;

typedef struct TempReadCfg
{
	uint16 startCnt;	// 从第几次采样开始读取
	uint16 readLen;		// 读取的长度
}TempReadCfg_t;
//恢复默认对齐方式,否则影响后面代码对齐方式.[调试出现app error、stack error]
#pragma pack()

extern const TempCfg_t g_TempCfg;
extern TempReadCfg_t g_TempReadCfg;

#define  TEMP_SAMPLING_PERIOD  1

void user_init(void);
void temp_relate_init(void);
int8 adcVoltage_convert_temp(float voltage);
int8 actTemp_to_tempValue(float temp);
void temp_history_tempValue(int8 tempvalue);
int8 temp_get_tempValue(uint16 cnt);
uint16 current_sampling_tempcnt(void);
int8 temp_temporary_sampling(void);
void temp_sampleTimerCb(void);

#endif

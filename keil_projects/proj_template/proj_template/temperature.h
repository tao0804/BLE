#ifndef _TEMPERATURE_H_
#define _TEMPERATURE_H_
#include "panble.h"

// TemperTable�����С
#define TEMP_TABLE_MAX_LEN		500
// TemperValue 0ֵ�¶� �����϶ȣ�
#define ZERO_TEMP_VALUE_C		30.0f
// TemperValue ���� �����϶ȣ�
#define PRECISION_TEMP_VALUE_C	0.05f
// TemperValue ת ��ʵ�¶� �����϶ȣ�
#define TEMP_VALUE_TO_C(value)  (ZERO_TEMP_VALUE_C + value * PRECISION_TEMP_VALUE_C)

// ����ָ���ڴ���뷽ʽ��Ԥ����ָ��,ͨ�������ڽṹ��ǰ��
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
	uint16 startCnt;	// �ӵڼ��β�����ʼ��ȡ
	uint16 readLen;		// ��ȡ�ĳ���
}TempReadCfg_t;
//�ָ�Ĭ�϶��뷽ʽ,����Ӱ����������뷽ʽ.[���Գ���app error��stack error]
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

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

const TempCfg_t g_TempCfg = {
	.zeroTempValue = ZERO_TEMP_VALUE_C * 1000,
	.precisionTempValue = PRECISION_TEMP_VALUE_C * 1000,
	.tempTableMaxLen = TEMP_TABLE_MAX_LEN,
	.sampleTempPeriod = TEMP_SAMPLING_PERIOD,
};

TempReadCfg_t g_TempReadCfg;

void user_init(void)
{
	mcu_gpio_user_init();
	mcu_adc_user_init();
}

// ��ʼ��ȫ�ֱ���
void temp_relate_init(void)
{
	int8 t;
	memset(&g_TempReadCfg, 0, sizeof(g_TempReadCfg));
	memset(tempTable, 0, sizeof(tempTable));
	tempCnt = 0;
	tempTimeCnt = 0;
	t = temp_temporary_sampling();
	temp_history_tempValue(t);
}

//ntc Bֵ,NCP15WF104F03RC 4250-4299 B����ֵ
#define B_character_ntc	4250
//ntc��25��ʱ��׼����
#define R_ntc_25		100000.0

// adc��ѹֵ���¶�ֵ��ת��
int8 adcVoltage_convert_temp(float voltage)
{
	int8 tempValue;
	float tempreture_ntc1;
	float NTC1_R;	//ĳ�¶�ʱntcʵ����ֵ
	NTC1_R = (((6.25 - voltage) / (6.25 + voltage)) * 100000);	// �˷ŷ����ɼ��ĵ�ѹ��ntc��ֵ��ϵ
	tempreture_ntc1 = 1.0 / (1.0 / (273.15 + 25) + 1.0 / B_character_ntc * log(NTC1_R / (R_ntc_25))) - 273.15;
	tempValue = actTemp_to_tempValue(tempreture_ntc1);
	return tempValue;
}

// ʵ���¶�ֵתint8
int8 actTemp_to_tempValue(float temp)
{
	int32 t;
	//�Խ����������,�ȶԳ�ֵ���λ�����㾫�ȸ���
	t = (int32)round((temp - ZERO_TEMP_VALUE_C) / PRECISION_TEMP_VALUE_C);
	if(t > TEMP_VALUE_OVER_MAX)
		return TEMP_VALUE_OVER_MAX;
	else if(t < TEMP_VALUE_LESS_MIN)
		return TEMP_VALUE_LESS_MIN;
	return (int8)t;
}

// �����¶�ֵ��tempTable��ͬʱ����tempCnt
void temp_history_tempValue(int8 tempvalue)
{
	tempTable[tempCnt % TEMP_TABLE_MAX_LEN] = tempvalue;
	tempCnt++;
}

#define TEMP_VALUE_ERROR	(-127)
//��ȡ�¶�ֵ,cnt��ʾ�ڼ��β���
int8 temp_get_tempValue(uint16 cnt)
{
	//���ɵ�505��ʱ,��3�������ѱ�����
	if(cnt > tempCnt || cnt == 0 || 
	  (tempCnt > TEMP_TABLE_MAX_LEN && tempCnt - cnt >= TEMP_TABLE_MAX_LEN))
		return TEMP_VALUE_ERROR;
	return tempTable[(cnt - 1) % TEMP_TABLE_MAX_LEN];
}

// ������ɴ���
uint16 current_sampling_tempcnt(void)
{
	return tempCnt;
}

// ������һ���¶�,���洢
int8 temp_temporary_sampling(void)
{
    float v;
	int8 t;
	user_init();
	while(mcu_adc_main());	// �������������
    GPIO_ClearBits(P1, BIT0);
	v = mcu_adc_get_voltage(MCU_P12_ADC_CH2);
	t = adcVoltage_convert_temp(v);
	return t;
}

// ÿ����SAMPLE_TEMPER_PERIOD��,������һ���¶�,��洢
// ��ʱ���ص�ÿ1min��һ�δ˽ӿ�
void temp_sampleTimerCb(void)
{
	float v;
	int8 t;
	if(++tempTimeCnt >= TEMP_SAMPLING_PERIOD)
		tempTimeCnt = 0;	// ÿ��SAMPLE_TEMPER_PERIOD����������һ��
	else
		return;
	user_init();
    while(mcu_adc_main());	// �������������
    GPIO_ClearBits(P1, BIT0);
	v = mcu_adc_get_voltage(MCU_P12_ADC_CH2);
	t = adcVoltage_convert_temp(v);
	temp_history_tempValue(t);
}

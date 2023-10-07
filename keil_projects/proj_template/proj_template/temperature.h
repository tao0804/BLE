#ifndef _TEMPERATURE_H_
#define _TEMPERATURE_H_
#include "panble.h"

#define TEMPVALUE_ARRAY_NUM 500
#define STANDARD_TEMPERATURE 37

void user_init(void);
float adc_convert_temperature(void);
int8 actTemperature_convert_tempValue(float tempreture_ntc1);
uint16 current_sampling_tempcnt(void);
void tempValue_historyRecord(void);
void temperature_relate_init(void);


#endif

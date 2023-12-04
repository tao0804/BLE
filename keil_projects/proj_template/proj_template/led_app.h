#ifndef _LED_APP_H_
#define _LED_APP_H_

#include "panble.h"



// enum + or not
typedef enum LedMode{
	LED_MODE_OFF = 0,
	LED_MODE_ON,
	LED_MODE_BLINK,
}LedMode_t;  

typedef struct LedInfo{
	uint8 ledStatus;	// spark 0:off 1:on
	LedMode_t ledMode;
	uint16 ledOn_time;	// spark uint16 instead of uint8
	uint16 ledIntv_time;	// unit: 10ms
}LedInfo_t;


// void set_blink_timer(void);
void led_reset_init(void);
void app_led_blink_timerCb(void);
void led_set_mode(LedMode_t mode);	// not sure the argument is appropriate(LedMode_t instead of uint8)

#endif


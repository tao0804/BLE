#include "led_app.h"
#include "mcu_hal.h"
#include "stack_svc_api.h"
// #include "stdbool.h"

#define BOOL_LED_ON  TRUE     // on
#define BOOL_LED_OFF FALSE    // off

LedInfo_t ledInfo;

static void led_blink_timer_off(void)
{
	// Clear active timer before re-set
	if (((ke_timer_active_handler)SVC_ke_timer_active)(APP_LED_BLINK_TIMER, TASK_APP)) {
		((ke_timer_clear_handler)SVC_ke_timer_clear)(APP_LED_BLINK_TIMER, TASK_APP);
	}
}

// argument can't be void.
static void led_blink_timer_on(uint16 time)
{
	led_blink_timer_off();
	((ke_timer_set_handler)SVC_ke_timer_set)(APP_LED_BLINK_TIMER, TASK_APP, time);
}

// how to package different interface into one ?
// refer:led_reset_init
static void led_control_status(bool light)
{
	mcu_led_light(light);
	ledInfo.ledStatus = light;
}

void led_reset_init(void)
{
	led_control_status(BOOL_LED_OFF);
	ledInfo.ledMode = LED_MODE_OFF;
	ledInfo.ledOn_time = 10;
	ledInfo.ledIntv_time = 200;    // unit:10ms
	led_set_mode(LED_MODE_BLINK);
}

void led_set_mode(LedMode_t mode)
{
	// ledInfo.ledMode:current mode; mode:led_set_mode(LED_MODE_BLINK)
	// come in useful
	if (ledInfo.ledMode == mode)
		return;

	switch (mode) {
	case LED_MODE_OFF: {
		led_blink_timer_off();
		led_control_status(BOOL_LED_OFF);
	} break;
	case LED_MODE_ON: {
		led_blink_timer_off();
		led_control_status(BOOL_LED_ON);
	} break;
	case LED_MODE_BLINK: {
		led_blink_timer_on(ledInfo.ledOn_time);
		led_control_status(BOOL_LED_ON);
	} break;
	default:
		return;
	}
	ledInfo.ledMode = mode;
}

void app_led_blink_timerCb(void)
{
	if (ledInfo.ledStatus == LED_MODE_ON) {
		led_blink_timer_on(ledInfo.ledIntv_time);
		mcu_led_light(LED_MODE_OFF);
	} else if (ledInfo.ledStatus == LED_MODE_OFF) {
		led_blink_timer_on(ledInfo.ledOn_time);
		mcu_led_light(LED_MODE_OFF);
	}
}

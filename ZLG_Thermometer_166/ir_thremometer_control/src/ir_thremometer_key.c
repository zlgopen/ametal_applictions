#include "am_gpio.h"
#include "zlg116_pin.h"
#include "am_input.h"
#include "ir_thremometer_beep.h"
#include "ir_thremometer_timer.h"
#include "am_softimer.h"
#include "ir_thremometer_key.h"

#define KEY_TIMEOUT 10000
#define KEY_SHORT_PRESS_TIMEOUT 5
#define KEY_LONG_PRESS_2S 200
#define KEY_LONG_PRESS_10S 1000
#define KEY_TIME_MS 10

static const int __g_key_pins[]  = {PIOA_12, PIOB_4, PIOB_3, PIOB_5};

typedef enum {
	IR_KEY_MEASURE = 0,
	IR_KEY_MODE,
	IR_KEY_RECORD,
	IR_KEY_UNIT,
	KEY_NO,
}key_code_t;

static key_press_t key_press = IR_KEY_RELEASE;
static am_softimer_t key_softimer;
static uint32_t key_pressed_cnt[4] = {0};
static key_code_t pressed_key_code = KEY_NO;
static am_bool_t stop_flag = AM_FALSE;

const static key_press_t long_press[4] = {
	IR_KEY_MEASURE_LONG, IR_KEY_MODE_LONG, IR_KEY_RECORD_LONG, IR_KEY_UNIT_LONG
};

const static key_press_t short_press[4] = {
	IR_KEY_MEASURE_SHORT, IR_KEY_MODE_SHORT, IR_KEY_RECORD_SHORT, IR_KEY_UNIT_SHORT
};

/**
 * \brief 关闭按键扫描
 */
void ir_thremometer_stop_key(void)
{
	stop_flag = AM_TRUE;
}

/**
 * \brief 打开按键扫描
 */
void ir_thremometer_start_key(void)
{
	stop_flag = AM_FALSE;
}

/**
 * \brief 定时回调按键扫描
 */
static void key_callback (void *p)
{
	key_code_t current_key_code = IR_KEY_MEASURE;

	if (stop_flag) {
		return;
	}
	
	while (current_key_code < KEY_NO) {
		if (am_gpio_get(__g_key_pins[current_key_code]) == 0) {
			break;
		} else {
			current_key_code++;
		}
	}
	
	if (current_key_code != KEY_NO) {
		pressed_key_code = current_key_code;
		if (key_pressed_cnt[pressed_key_code] == 0) {
			ir_thremometer_timer_clear_cnt();
		}
		if (key_pressed_cnt[pressed_key_code] >= KEY_LONG_PRESS_10S) {
			if (pressed_key_code == IR_KEY_UNIT) {
				key_press = IR_KEY_UNIT_LONG_PRESSED_10S;
			} else if (pressed_key_code == IR_KEY_MODE) {
				key_press = IR_KEY_MODE_LONG_PRESSED_10S;
			}
		} else if (key_pressed_cnt[pressed_key_code] == KEY_LONG_PRESS_2S) {
			key_press = long_press[pressed_key_code];
		}

		key_pressed_cnt[pressed_key_code]++;

	} else if (pressed_key_code != KEY_NO){

		if (key_pressed_cnt[pressed_key_code] > KEY_SHORT_PRESS_TIMEOUT && 
			key_pressed_cnt[pressed_key_code] < KEY_LONG_PRESS_2S) {
			key_press = short_press[pressed_key_code];;
		} 

		if (key_pressed_cnt[pressed_key_code] != 0) {
			key_pressed_cnt[pressed_key_code] = 0;
			ir_thremometer_timer_cnt();
		}
	}
}

/**
 * \brief 按键初始化，10ms定时扫描
 */
void ir_thremometer_key_init(void)
{
	for (int i = 0; i < 4; i++) {
		am_gpio_pin_cfg(__g_key_pins[i], AM_GPIO_INPUT | AM_GPIO_PULLUP);
	}

	am_softimer_init(&key_softimer, key_callback, NULL);
	am_softimer_start(&key_softimer, KEY_TIME_MS);

	ir_thremometer_timer_init();
	ir_thremometer_timer_start();
}

/**
 * \brief 获取按键值
 */
key_press_t ir_thremometer_get_key(void)
{
	return key_press;
}

/**
 * \brief 重置按键值
 */
void ir_thremometer_release_key(void)
{
	key_press = IR_KEY_RELEASE;
}

/**
 * \brief 测试按键短按
 */
void ir_thremometer_set_default_key(void)
{
	key_press = IR_KEY_MEASURE_SHORT;
}

/* end of file */

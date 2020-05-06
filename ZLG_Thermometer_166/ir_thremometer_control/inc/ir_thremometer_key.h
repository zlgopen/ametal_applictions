#ifndef _IR_THREMOMETER_KEY_H_
#define _IR_THREMOMETER_KEY_H_

typedef enum {
	IR_KEY_MEASURE_SHORT = 0,
	IR_KEY_MODE_SHORT,
	IR_KEY_RECORD_SHORT,
	IR_KEY_UNIT_SHORT,
	IR_KEY_MEASURE_LONG,
	IR_KEY_MODE_LONG,
	IR_KEY_RECORD_LONG,
	IR_KEY_UNIT_LONG,
	IR_KEY_MODE_LONG_PRESSED_10S,
	IR_KEY_UNIT_LONG_PRESSED_10S,
	IR_KEY_PRESSING,
	IR_KEY_RELEASE,
} key_press_t;

void ir_thremometer_key_init(void);

key_press_t ir_thremometer_get_key(void);

void ir_thremometer_release_key(void);

void ir_thremometer_set_default_key(void);

void ir_thremometer_stop_key(void);

void ir_thremometer_start_key(void);

#endif

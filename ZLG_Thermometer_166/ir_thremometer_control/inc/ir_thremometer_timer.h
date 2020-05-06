#ifndef _IR_THREMOMETER_TIMER_H_
#define _IR_THREMOMETER_TIMER_H_

#define TIMER_MS 10
#define POWER_OFF_TIMEOUT 1000
#define LCD_FLASH_TIMEOUT 100

void ir_thremometer_timer_init(void);

void ir_thremometer_timer_start(void);

void ir_thremometer_timer_stop(void);

void ir_thremometer_timer_clear_cnt(void);

void ir_thremometer_timer_cnt(void);

uint32_t ir_thremometer_timer_get_cnt(void);

am_bool_t ir_thremometer_time_to_powe_off(void);

#endif

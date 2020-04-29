#ifndef _IR_THREMOMETER_LCD_H_
#define _IR_THREMOMETER_LCD_H_

#include "am_user_lcd_display.h"

#define BAT_DISPLAY_CODE (AM_USER_BT3_DISP | AM_USER_BT2_DISP | AM_USER_BT1_DISP | AM_USER_BT_DISP)

typedef struct _display_t{
	uint32_t disp_flag;             /* 测量模式标志 */
	float num;                      /* 温度数据 */
	uint8_t id;
	am_bool_t flash;                /* 闪烁控制 */
	am_bool_t update;               /* 刷新标志 */
	am_bool_t just_power_on;
}display_t;

void ir_thremometer_lcd_init(void);

void ir_thremometer_lcd_display(display_t *p_display);

void ir_thremometer_lcd_display_float(float num);

void ir_thremometer_lcd_display_all(void);

void ir_thremometer_lcd_clear(void);

void ir_thremometer_lcd_display_alarm(am_user_lcd_disp_alarm_t alarm);

#endif

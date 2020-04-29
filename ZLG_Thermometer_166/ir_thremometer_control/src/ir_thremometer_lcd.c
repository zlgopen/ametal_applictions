#include "am_gpio.h"
#include "zlg116_pin.h"
#include "ametal.h"
#include "am_user_lcd_display.h"
#include "ir_thremometer_lcd.h"
#include "ir_thremometer_backlight.h"
#include "ir_thremometer_timer.h"
#include "ir_thremometer.h"
#include "am_delay.h"
#include "am_vdebug.h"

//#define BACKLIGHT_GREEN_LIMIT_C 38.000001
//#define BACKLIGHT_GREEN_LIMIT_F 100.400001

#define BAT_FLASH_TIMEOUT 50

#define BODY_DISPLAY_UPPER_C_LIMIT 43.000001
#define BODY_DISPLAY_LOWER_C_LIMIT 32.000001
#define BODY_DISPLAY_UPPER_F_LIMIT 109.400001
#define BODY_DISPLAY_LOWER_F_LIMIT 89.600001

static am_lcd_handle_t lcd_handle;

extern float _g_alarm;

/**
 * \brief  LCD初始化
 */
 void ir_thremometer_lcd_init(void)
{
	lcd_handle = am_user_lcd_disp_init();

	ir_thremometer_lcd_display_all();
	am_mdelay(2000);
    ir_thremometer_lcd_clear();

    ir_thremometer_backlight_on(BACKLIGNT_GREEN);
	am_user_lcd_disp_alarm(lcd_handle, AM_USER_DISP_STARTUP_DISP);
    am_user_lcd_disp_mem(lcd_handle, 100);
}

/**
 * \brief  LCD定时闪烁
 */
am_bool_t ir_thremometer_time_to_flash(void)
{
	static uint32_t old_cnt = 0;
	uint32_t cnt = ir_thremometer_timer_get_cnt() / LCD_FLASH_TIMEOUT;
	if (cnt != old_cnt) {
		old_cnt = ir_thremometer_timer_get_cnt() / LCD_FLASH_TIMEOUT;
		return AM_TRUE;
	} else {
		return AM_FALSE;
	}
}

/**
 * \brief  电池电量显示
 */
void ir_thremometer_lcd_set_bat(display_t *p_display)
{	
	static am_bool_t bat_display = AM_FALSE; 
	if (p_display->just_power_on != AM_TRUE && p_display->flash == AM_TRUE) {
		if (ir_thremometer_time_to_flash()) {
			bat_display = !bat_display;
		}
		if (bat_display == AM_FALSE) {
			am_user_lcd_disp_flag_disable(lcd_handle, BAT_DISPLAY_CODE);
		} else {
			p_display->update = AM_TRUE;
		}
	}

}

/**
 * \brief  LCD背光显示
 */
void ir_thremometer_lcd_set_backlight(display_t *p_display)
{	
	float greem_limit = 0;

	if (p_display->disp_flag & AM_USER_0F_DISP) {
		greem_limit = ir_thremometer_unit_ctof(_g_alarm);
	} else {
		greem_limit = _g_alarm;
	}
	
	if (p_display->disp_flag & AM_USER_BODY_DISP) {
		if (p_display->num <= greem_limit) {
			ir_thremometer_backlight_on(BACKLIGNT_GREEN);
			p_display->disp_flag |= AM_USER_XIAO_DISP;
			p_display->disp_flag &= (~AM_USER_KU_DISP);
		} else {
			ir_thremometer_backlight_on(BACKLIGNT_RED);
			p_display->disp_flag |= AM_USER_KU_DISP;
			p_display->disp_flag &= (~AM_USER_XIAO_DISP);
		}
	} else {
		ir_thremometer_backlight_on(BACKLIGNT_GREEN);
		p_display->disp_flag |= AM_USER_XIAO_DISP;
		p_display->disp_flag &= (~AM_USER_KU_DISP);
	}
}

/**
 * \brief  记忆值记忆数显示
 */
static void ir_thremometer_lcd_display_num(display_t *p_display)
{
	float upper = 0, lower = 0;
	
	if (p_display->just_power_on) {
		am_user_lcd_disp_alarm(lcd_handle, AM_USER_DISP_STARTUP_DISP);
		return;
	}
	
	if (p_display->disp_flag & AM_USER_BODY_DISP) {
		if (p_display->disp_flag & AM_USER_0F_DISP) {
			upper = BODY_DISPLAY_UPPER_F_LIMIT;
			lower = BODY_DISPLAY_LOWER_F_LIMIT;
		} else {
			upper = BODY_DISPLAY_UPPER_C_LIMIT;
			lower = BODY_DISPLAY_LOWER_C_LIMIT;
		}
		
		if (p_display->num > upper) {
            p_display->disp_flag &= (~AM_USER_P1_DISP);
			am_user_lcd_disp_alarm(lcd_handle, AM_USER_DISP_HO);
		} else if (p_display->num < lower) {
            p_display->disp_flag &= (~AM_USER_P1_DISP);
			am_user_lcd_disp_alarm(lcd_handle, AM_USER_DISP_LO);
		} else {
            p_display->disp_flag |= AM_USER_P1_DISP;
			am_user_lcd_disp_float(lcd_handle, p_display->num);
		}
	} else {
        p_display->disp_flag |= AM_USER_P1_DISP;
		am_user_lcd_disp_float(lcd_handle, p_display->num);
	}
}

/**
 * \brief  LCD显示
 */
void ir_thremometer_lcd_display(display_t *p_display)
{
	if (p_display->update) {
		
		ir_thremometer_lcd_set_backlight(p_display);
		
		ir_thremometer_lcd_set_bat(p_display);
		
		ir_thremometer_lcd_display_num(p_display);
			
		if (p_display->just_power_on) {
			am_user_lcd_disp_mem(lcd_handle, AM_USER_NO_MEM);
		} else {
			am_user_lcd_disp_mem(lcd_handle, p_display->id);
		}
		
		am_user_lcd_disp_flag_enable(lcd_handle, p_display->disp_flag | AM_USER_BZ_DISP | AM_USER_LOG_DISP);
		
		p_display->update = AM_FALSE;
	}
	ir_thremometer_lcd_set_bat(p_display);
}

/**
 * \brief 显示浮点数
 */
void ir_thremometer_lcd_display_float(float num)
{
	am_user_lcd_disp_float(lcd_handle, num);
}

/**
 * \brief 清屏
 */
void ir_thremometer_lcd_clear(void)
{
	am_user_lcd_disp_clr(lcd_handle);
}

/**
 * \brief 全显
 */
void ir_thremometer_lcd_display_all(void)
{
	uint32_t display  = 0x3FFF;

    am_user_lcd_disp_float(lcd_handle, 888.8); 
    am_user_lcd_disp_mem(lcd_handle, 88);   
    am_user_lcd_disp_flag_enable(lcd_handle, display);          
}

/**
 * \brief 显示警告
 */
void ir_thremometer_lcd_display_alarm(am_user_lcd_disp_alarm_t alarm)
{
	am_user_lcd_disp_alarm(lcd_handle, alarm);
}

/* end of file */

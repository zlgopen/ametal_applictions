#include "ametal.h"
#include "am_board.h"
#include "am_vdebug.h"
#include "am_delay.h"
#include "am_gpio.h"
#include "zlg116_pin.h"
#include "am_aml166_inst_init.h"
#include "am_hwconf_microport.h"
#include "am_softimer.h"
#include "am_user_lcd_display.h"
#include "zal_infrared_sensor_interpolation.h"
#include "ir_thremometer.h"
#include "ir_thremometer_adc.h"
#include "ir_thremometer_backlight.h"
#include "ir_thremometer_beep.h"
#include "ir_thremometer_key.h"
#include "ir_thremometer_lcd.h"
#include "ir_thremometer_storage.h"
#include "ir_thremometer_timer.h"
#include "ir_thremometer_power.h"
#include "ir_thremometer_calibration.h"

#define IR_THREMOMETER_GET_BAT(v) (v * 3)

static char _str[100] = {0};
static float _g_offset = 0;
float _g_alarm = 0;

/* 系统初始化 */
void ir_thremometer_init(void)
{
	ir_thremometer_power_init();
	ir_thremometer_beep_init();
	ir_thremometer_backlight_init();
	ir_thremometer_lcd_init();
	ir_thremometer_key_init();
	ir_thremometer_adc_init();
	ir_thremometer_storage_init();
}

/* 华氏度切摄氏度 */
float ir_thremometer_unit_ftoc(float temp)
{
	temp -= 32;
	temp /= 1.8;
	return temp;
}

/* 摄氏度切华氏度 */
float ir_thremometer_unit_ctof(float temp)
{
	temp *= 1.8;
	temp += 32;
	return temp;
}

/* 显示配置，单位，模式的显示 */
void ir_thremometer_set_display_config(display_t      *p_display,
                                       measure_mode_t *p_mode,
                                       unit_t         *p_unit)
{
	if (p_unit != NULL) {
		if (*p_unit == UNIT_FAHRENHRIT) {	
			p_display->disp_flag |= AM_USER_0F_DISP;
			p_display->disp_flag &= (~AM_USER_0C_DISP);			
		} else {
			p_display->disp_flag |= AM_USER_0C_DISP;
			p_display->disp_flag &= (~AM_USER_0F_DISP);
		}
	}
	if (p_mode != NULL) {
		if (*p_mode == MEASURE_BODY_MODE) {
			p_display->disp_flag |= AM_USER_BODY_DISP;
			p_display->disp_flag &= (~AM_USER_SUR_DISP);

		} else {
			p_display->disp_flag |= AM_USER_SUR_DISP;
			p_display->disp_flag &= (~AM_USER_BODY_DISP);
		}
	}
}

/* 获取当前单位 */
unit_t ir_thremometer_get_display_unit(display_t* p_display)
{
	if ((p_display->disp_flag & AM_USER_0C_DISP) != 0) {
		return UNIT_DAGREE_CELSIUS;
	} else {
		return UNIT_FAHRENHRIT;
	}
}

/* 获取当前模式 */
measure_mode_t ir_thremometer_get_display_mode(display_t* p_display)
{
	if ((p_display->disp_flag & AM_USER_SUR_DISP) != 0) {
		return MEASURE_THING_MODE;
	} else {
		return MEASURE_BODY_MODE;
	}
}

/* 单位切换，显示内容切换 */
int ir_thremometer_unit_switch(unit_t* p_unit, display_t* p_display)
{
	unit_t unit = ir_thremometer_get_display_unit(p_display);
	if (ir_thremometer_storage_check_in_record(p_display->id)) {
		p_unit = &unit;
	}
	
	if (*p_unit == UNIT_FAHRENHRIT) {		
		*p_unit = UNIT_DAGREE_CELSIUS;
		p_display->num = ir_thremometer_unit_ftoc(p_display->num);
	} else {
		*p_unit = UNIT_FAHRENHRIT;
		p_display->num = ir_thremometer_unit_ctof(p_display->num);
	}
	ir_thremometer_set_display_config(p_display, NULL, p_unit);
	return AM_OK;
}

/* 模式切换，显示内容切换 */
int ir_thremometer_mode_switch(measure_mode_t* p_mode, display_t* p_display)
{
	measure_mode_t mode = ir_thremometer_get_display_mode(p_display);
	if (ir_thremometer_storage_check_in_record(p_display->id)) {
		p_mode = &mode;
	}
	
	if (*p_mode == MEASURE_BODY_MODE) {
		*p_mode = MEASURE_THING_MODE;
		p_display->num = ir_thremometer_mode_btot(p_display->num);
	} else {
		*p_mode = MEASURE_BODY_MODE;
		p_display->num = ir_thremometer_mode_ttob(p_display->num);
	}
	ir_thremometer_set_display_config(p_display, p_mode, NULL);
	return AM_OK;
}

/* 显示测量 */
int ir_thremometer_measure(measure_mode_t mode, unit_t unit, display_t* p_display)
{
	p_display->num = ir_thremometer_get_obj_temp();
	
	p_display->num += _g_offset;
	
	if (mode == MEASURE_BODY_MODE) {
		p_display->num = ir_thremometer_mode_ttob(p_display->num);
	}
	
	if (unit == UNIT_FAHRENHRIT) {
		p_display->num = ir_thremometer_unit_ctof(p_display->num);
	}

	ir_thremometer_set_display_config(p_display, &mode, &unit);
	
	p_display->id = ir_thremometer_storage_temp(p_display->num, mode, unit);

	return AM_OK;
}

/* 读取显示存储记忆值 */
int ir_thremometer_get_temp_record(display_t* p_display)
{
	record_t record;
	measure_mode_t mode;
	unit_t unit;
	ir_thremometer_storage_load_record(&record, &mode, &unit);
	p_display->id = record.id;
	p_display->num = record.temp;
	
	ir_thremometer_set_display_config(p_display, &mode, &unit);
	
	return AM_OK;
}

/* 校准算法，采用两点校准 */
int ir_thremometer_calibration(void)
{
	float obj_temps[] = {35, 40};
	
	ir_thremometer_beep_on(2, BEEP_SHORT_NOISE);
	for(int i = 0; i < 2; i++) {
		ir_thremometer_lcd_display_float(obj_temps[i]);
		while (ir_thremometer_get_key() != IR_KEY_MEASURE_SHORT) {
			ir_thremometer_get_obj_temp();
		}
		AM_DBG_INFO("======%d=====", (int)obj_temps[i]);
		ir_thremometer_release_key();
		ir_thremometer_beep_on(3, BEEP_LONG_NOISE);
		ir_thremometer_set_calib_point(obj_temps[i]);
	}
	ir_thremometer_calib_run();
	
	ir_thremometer_release_key();
	
	return AM_OK;
}

/* 电池电压检测，大于3V满电，2.5以上2格，以下1格，同时闪烁提示 */
uint32_t ir_thremometer_bat_check(display_t* p_display)
{
	static uint8_t old_vbat_lv = 0;
	uint8_t vbat_lv = 0;
	float vbat = 0.0;
	uint32_t bat_display_code = 0;

	vbat = ir_thremometer_get_bat_vol();
	vbat = IR_THREMOMETER_GET_BAT(vbat);

	if (vbat >= 3000) {
		bat_display_code =  AM_USER_BT3_DISP | AM_USER_BT2_DISP | AM_USER_BT1_DISP;
		vbat_lv = 3;
	} else if (vbat >= 2500 && vbat < 3000) {
		bat_display_code = AM_USER_BT2_DISP | AM_USER_BT1_DISP;
		vbat_lv = 2;
	} else {
		bat_display_code =  AM_USER_BT1_DISP;
		vbat_lv = 1;
	}
	
	if (vbat < 2500) {
		bat_display_code = 0;
		p_display->flash = AM_TRUE;
	} else {
		p_display->flash = AM_FALSE;
	}
	p_display->disp_flag &= (~BAT_DISPLAY_CODE); 
	p_display->disp_flag |= bat_display_code | AM_USER_BT_DISP;
	if (old_vbat_lv != vbat_lv) {
		AM_DBG_INFO("vbat: %d\r\n", (uint32_t)vbat);
		if (old_vbat_lv != 0) {
			p_display->update = AM_TRUE;
		}
		old_vbat_lv = vbat_lv;
	}

	return bat_display_code;
}

am_bool_t ir_thremometer_calib_enter_check(void)//进入校准模式内容提示
{
	key_press_t key_press;
	
	ir_thremometer_release_key();
	ir_thremometer_backlight_on(BACKLIGNT_RED);
	ir_thremometer_lcd_clear();
    
	ir_thremometer_lcd_display_float(88.8);
	ir_thremometer_beep_on(3, BEEP_LONG_NOISE);
	ir_thremometer_release_key();
	
	while(ir_thremometer_get_key() == IR_KEY_UNIT_LONG_PRESSED_10S);
	ir_thremometer_release_key();

	while(1) {
		key_press = ir_thremometer_get_key();
		if (key_press != IR_KEY_RELEASE) {
			if (key_press == IR_KEY_MODE_LONG) {
				return AM_TRUE;
			} else if (key_press == IR_KEY_UNIT_LONG_PRESSED_10S){
				;
			} else {
				return AM_FALSE;
			}
		}
		ir_thremometer_lcd_clear();
		am_mdelay(300);
		ir_thremometer_lcd_display_float(88.8);
		am_mdelay(300);
	}

}

int ir_thremometer_setting(void)
{
	typedef int (*pfn_save)(float offset);
	int key_press;	
	int offset = 0;
	float *data[2] = {&_g_offset, &_g_alarm};
	am_user_lcd_disp_alarm_t alarms[2] = {AM_USER_DISP_F2, AM_USER_DISP_F4};
	pfn_save _save_func[2] = {
		ir_thremometer_storage_save_offset,
		ir_thremometer_storage_save_alarm};
	
	ir_thremometer_lcd_clear();

	for (int i = 0; i < 2; i++) {
		ir_thremometer_lcd_display_alarm(alarms[i]);
		am_mdelay(500);
		ir_thremometer_lcd_display_float(*data[i]);
		ir_thremometer_release_key();
		do {
			key_press = ir_thremometer_get_key();
			if (key_press != IR_KEY_RELEASE) {
				ir_thremometer_beep_on(1, BEEP_SHORT_NOISE);
				if (key_press == IR_KEY_MODE_SHORT) {
					offset = *(data[i]) * 10;
					offset++;
					*(data[i]) = (float)offset / 10;
				} else if (key_press == IR_KEY_UNIT_SHORT) {
					offset = *(data[i]) * 10;
					offset--;
					*(data[i]) = (float)offset / 10;
				} else if (key_press == IR_KEY_RECORD_SHORT) {
					_save_func[i](*data[i]);
				} else if (key_press == IR_KEY_MEASURE_SHORT) {
					return AM_OK;
				}
				ir_thremometer_lcd_display_float(*data[i]);
				ir_thremometer_release_key();
			}
		} while(key_press != IR_KEY_RECORD_SHORT);
	}

	return AM_OK;
}

void ir_thremometer_key_process(display_t* p_display)//按键检测，读取键值
{
	int key_press = ir_thremometer_get_key();	
	static unit_t unit = UNIT_DAGREE_CELSIUS;
	static measure_mode_t mode = MEASURE_BODY_MODE;
	
	if (key_press != IR_KEY_RELEASE) {
		AM_DBG_INFO("key_press: %d\r\n", key_press);
		ir_thremometer_stop_key();
		ir_thremometer_beep_on(1, BEEP_SHORT_NOISE);
		
		if (p_display->disp_flag & AM_USER_SUR_DISP) {
			mode = MEASURE_THING_MODE;
		} else {
			mode = MEASURE_BODY_MODE;
		}
	
		if (p_display->disp_flag & AM_USER_0F_DISP) {
			unit = UNIT_FAHRENHRIT;
		} else {
			unit = UNIT_DAGREE_CELSIUS;
		}
		
		switch (key_press) {
		case IR_KEY_UNIT_SHORT:
			ir_thremometer_unit_switch(&unit, p_display);
			break;
		case IR_KEY_MODE_SHORT:
			ir_thremometer_mode_switch(&mode, p_display);
			break;
		case IR_KEY_RECORD_SHORT:
			p_display->just_power_on = AM_FALSE;
			ir_thremometer_get_temp_record(p_display);
			break;
		case IR_KEY_RECORD_LONG:
			p_display->just_power_on = AM_TRUE;
            p_display->disp_flag &= ~AM_USER_P1_DISP;
			ir_thremometer_storage_clear();
			break;
		case IR_KEY_MEASURE_SHORT:
			p_display->just_power_on = AM_FALSE;
			ir_thremometer_measure(mode, unit, p_display);
			ir_thremometer_beep_on(2, BEEP_SHORT_NOISE);
			break;
		case IR_KEY_MEASURE_LONG:
			ir_thremometer_backlight_off();
			ir_thremometer_lcd_clear();
			ir_thremometer_power_off();
			while(1);
			break;
		case IR_KEY_UNIT_LONG_PRESSED_10S:
			ir_thremometer_start_key();
			AM_DBG_INFO("check enter calibration\r\n");
			if (ir_thremometer_calib_enter_check()) {
				AM_DBG_INFO("enter calibration\r\n");
				ir_thremometer_calibration();
			} else {
				AM_DBG_INFO("no calibration\r\n");
			}
			break;
		case IR_KEY_MODE_LONG:
			ir_thremometer_start_key();
			ir_thremometer_setting();
			break;
		default:
			break;
		}
		sprintf(_str, "_g_alarm = %f\r\n", _g_alarm);
		AM_DBG_INFO("%s", _str);		
		p_display->update = AM_TRUE;
		ir_thremometer_release_key();
		ir_thremometer_start_key();
	}
}

void ir_thremometer_check_calib(void)//校验校准系数，读取K B值
{
	if (ir_thremometer_if_already_calib() != AM_TRUE) {
		while(ir_thremometer_calib_enter_check() != AM_TRUE);
		if (ir_thremometer_calibration() == AM_OK) {
			ir_thremometer_calib_init();
		}
	} else {
		ir_thremometer_calib_init();
	}
}

void ir_thremometer_alarm(display_t *p_display)
{
	static float temp = 0;
	float alarm_temp = 0;
	
	if (p_display->disp_flag & AM_USER_SUR_DISP) {
		return;
	}
	
	if (p_display->disp_flag & AM_USER_0F_DISP) {
		alarm_temp = ir_thremometer_unit_ctof(_g_alarm);
	} else {
		alarm_temp = _g_alarm;
	}
	
	if (p_display->num < alarm_temp + 0.000001 && 
		p_display->num > alarm_temp - 0.000001 &&
		temp != p_display->num) {
		temp = p_display->num;
		ir_thremometer_beep_on(5, BEEP_LONG_NOISE);
	}
}

void ir_thremometer_run(void)
{
	display_t display = {
		.disp_flag = AM_USER_0C_DISP | AM_USER_BODY_DISP,
		.num = 0,
		.id = 0,
		.flash = AM_FALSE,
		.update = AM_TRUE,
		.just_power_on = AM_TRUE
	};

	ir_thremometer_check_calib();//读取KB值，校准系数
	_g_offset = ir_thremometer_storage_load_offset();
	_g_alarm = ir_thremometer_storage_load_alarm();
	while(1) {
		if (ir_thremometer_time_to_powe_off()) {//10S关机
			ir_thremometer_beep_on(1, BEEP_LONG_NOISE);
			ir_thremometer_power_off();
		}
		ir_thremometer_key_process(&display);//按键检测，读取键值处理
		ir_thremometer_bat_check(&display);//电池电压检测
		ir_thremometer_lcd_display(&display);//LCD显示
		ir_thremometer_alarm(&display);
        
        /* 用于调试 */
//		ir_thremometer_get_obj_temp();
//		am_mdelay(500);
	}
}

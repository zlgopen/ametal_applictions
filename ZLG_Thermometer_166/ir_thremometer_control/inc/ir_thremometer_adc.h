#ifndef _IR_THREMOMETER_ADC_H_
#define _IR_THREMOMETER_ADC_H_

#include "ir_thremometer.h"

void ir_thremometer_adc_init(void);

double ir_thremometer_get_ntc_res(void);

double ir_thremometer_get_ir_vol(void);

double ir_thremometer_get_obj_temp(void);

float ir_thremometer_get_bat_vol(void);

void ir_thremometer_calibration_vol(float obj_temp, float* vol_real, float* vol_in_table);

void ir_thremometer_set_calib_pram(float k, float b);

void ir_thremometer_adc_calib(void);

void ir_thremometer_adc_4_5(void);

#endif


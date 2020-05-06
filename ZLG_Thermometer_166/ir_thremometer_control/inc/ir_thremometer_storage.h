#ifndef _IR_THREMOMETER_STORAGE_H_
#define _IR_THREMOMETER_STORAGE_H_

#include "ir_thremometer.h"

typedef struct _record_t{
	uint8_t id;
	float temp;
}record_t;

void ir_thremometer_storage_init(void);

int ir_thremometer_storage_temp(float temp, measure_mode_t mode, unit_t unit);

int ir_thremometer_storage_k(float data);

int ir_thremometer_storage_b(float data);

void ir_thremometer_storage_load_record(record_t* p_record, measure_mode_t* p_mode, unit_t* p_unit);

void ir_thremometer_calib_init(void);

int ir_thremometer_storage_check_k_b(void);

int ir_thremometer_storage_set_k_b_flag(void);

am_bool_t ir_thremometer_storage_check_in_record(uint8_t id);

int ir_thremometer_storage_write(int start_addr, uint8_t *p_buf, int len);

int ir_thremometer_storage_reead(int start_addr, uint8_t *p_buf, int len);

int ir_thremometer_storage_save_offset(float offset);

float ir_thremometer_storage_load_offset(void);

int ir_thremometer_storage_save_alarm(float alarm);

float ir_thremometer_storage_load_alarm(void);

int ir_thremometer_storage_clear(void);

#endif

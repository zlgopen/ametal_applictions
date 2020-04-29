#ifndef _IR_THREMOMETER_H_
#define _IR_THREMOMETER_H_

typedef enum {
	MEASURE_BODY_MODE = 0,
	MEASURE_THING_MODE,
} measure_mode_t;

typedef enum {
	UNIT_DAGREE_CELSIUS = 0,
	UNIT_FAHRENHRIT,
} unit_t;

void ir_thremometer_init(void);

void ir_thremometer_run(void);

float ir_thremometer_unit_ftoc(float temp);//华氏度切摄氏度

float ir_thremometer_unit_ctof(float temp);//摄氏度切华氏度 

#endif

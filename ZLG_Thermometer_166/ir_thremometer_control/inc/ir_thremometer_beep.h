#ifndef _IR_THREMOMETER_BEEP_H_
#define _IR_THREMOMETER_BEEP_H_

#include "ametal.h"

typedef enum {
	BEEP_LONG_NOISE = 0,    /* 长叫 */
	BEEP_SHORT_NOISE,       /* 短叫 */
}beep_nosie_t;

/**
 * \brief  蜂鸣器初始化
 */
void ir_thremometer_beep_init(void);

/**
 * \brief  蜂鸣器鸣叫
 *
 * \param[in] times : 鸣叫次数
 * \param[in] nosie : 鸣叫类型
 *
 * \retval 无
 */
void ir_thremometer_beep_on(uint32_t times, beep_nosie_t nosie);

#endif

/* end of file */

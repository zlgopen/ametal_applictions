#ifndef _IR_THREMOMETER_BACKLIGHT_H_
#define _IR_THREMOMETER_BACKLIGHT_H_

/**
 * \brief  背光灯模式
 */
typedef enum {
	BACKLIGNT_RED = 0,
	BACKLIGNT_GREEN,
	BACKLIGNT_ORANGE,
} backlight_mode_t;

/**
 * \brief  背光灯初始化
 */
void ir_thremometer_backlight_init(void);

/**
 * \brief  背光灯开启
 *
 * \param[in] mode : 背光灯模式
 *
 * \retval 无
 */
void ir_thremometer_backlight_on(backlight_mode_t mode);

/**
 * \brief  背光灯关闭
 */
void ir_thremometer_backlight_off(void);

#endif

/* end of file */

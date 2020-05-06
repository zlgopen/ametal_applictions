#include "am_gpio.h"
#include "zlg116_pin.h"
#include "ir_thremometer_backlight.h"

/**
 * \brief  背光灯引脚
 */
#define BACKLIGHT_GREEN_PIN PIOA_15
#define BACKLIGHT_RED_PIN   PIOA_1

/**
 * \brief  背光灯初始化
 */
void ir_thremometer_backlight_init(void)
{
	am_gpio_pin_cfg(BACKLIGHT_GREEN_PIN, AM_GPIO_OUTPUT_INIT_HIGH);
	am_gpio_pin_cfg(BACKLIGHT_RED_PIN, AM_GPIO_OUTPUT_INIT_HIGH);

    ir_thremometer_backlight_off();
}

/**
 * \brief  背光灯开启
 *
 * \param[in] mode : 背光灯模式
 *
 * \retval 无
 */
void ir_thremometer_backlight_on(backlight_mode_t mode)
{
	if (mode == BACKLIGNT_RED) {
		am_gpio_set(BACKLIGHT_GREEN_PIN, 1);
		am_gpio_set(BACKLIGHT_RED_PIN, 0);
	} else if (mode == BACKLIGNT_GREEN) {
		am_gpio_set(BACKLIGHT_GREEN_PIN, 0);
		am_gpio_set(BACKLIGHT_RED_PIN, 1);
	} else {
		am_gpio_set(BACKLIGHT_GREEN_PIN, 0);
		am_gpio_set(BACKLIGHT_RED_PIN, 0);
	}
}

/**
 * \brief  背光灯关闭
 */
void ir_thremometer_backlight_off(void)
{
	am_gpio_set(BACKLIGHT_GREEN_PIN, 1);
	am_gpio_set(BACKLIGHT_RED_PIN, 1);
}

/* end of file */

#include "am_gpio.h"
#include "zlg116_pin.h"
#include "ir_thremometer_power.h"

/* 电源控制引脚 */
#define IR_THREM_POWER_PIN  PIOA_11

/* 电源初始化 */
void ir_thremometer_power_init(void)
{
	am_gpio_pin_cfg(IR_THREM_POWER_PIN, AM_GPIO_OUTPUT_INIT_HIGH);
}

/* 打开电源 */
void ir_thremometer_power_on(void)
{
	am_gpio_set(IR_THREM_POWER_PIN, 1);
}

/* 关闭电源 */
void ir_thremometer_power_off(void)
{
	am_gpio_set(IR_THREM_POWER_PIN, 0);
}

/* end of file */

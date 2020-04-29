#include "am_gpio.h"
#include "am_delay.h"
#include "zlg116_pin.h"
#include "ir_thremometer_beep.h"
#include "ametal.h"
#include "am_aml166_inst_init.h"

#define BEEP_CHAN  0                        /* 蜂鸣器使用的定时器通道 */
#define BEEP_FREQ  2500                     /* PWM频率，单位Hz */
#define BEEP_DUTY  50                       /* PWM占空比 */

/* 无需修改 */
#define TIM_PERIOD (1000000000/BEEP_FREQ)
#define TIM_DUTY   (TIM_PERIOD * BEEP_DUTY / 100)

static am_pwm_handle_t pwm_handle;

/**
 * \brief  蜂鸣器初始化
 */
void ir_thremometer_beep_init(void)
{
	pwm_handle = am_zlg116_tim1_pwm_inst_init();

    am_pwm_config(pwm_handle, BEEP_CHAN, TIM_DUTY, TIM_PERIOD);
	am_pwm_disable(pwm_handle, BEEP_CHAN);
	ir_thremometer_beep_on(1, BEEP_SHORT_NOISE);
}

/**
 * \brief  蜂鸣器鸣叫
 *
 * \param[in] times : 鸣叫次数
 * \param[in] nosie : 鸣叫类型
 *
 * \retval 无
 */
void ir_thremometer_beep_on(uint32_t times, beep_nosie_t nosie)
{
	uint32_t time_ms = 0;
	if (nosie == BEEP_LONG_NOISE) {
		time_ms = 500;
	} else {
		time_ms = 100;
	}
	for(int i = 0; i < times; i++) {
		am_pwm_enable(pwm_handle, BEEP_CHAN);
		am_mdelay(time_ms);
		am_pwm_disable(pwm_handle, BEEP_CHAN);
		am_mdelay(time_ms);
	}
}

/* end of file */

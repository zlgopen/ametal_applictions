#include "ametal.h"
#include "am_board.h"
#include "am_softimer.h"
#include "ir_thremometer_timer.h"

static am_softimer_t key_softimer;
static uint32_t cnt = 0;
static am_bool_t if_counting = AM_TRUE;

static void key_callback (void *p)
{
	if (if_counting == AM_TRUE) {
		cnt++;
	} else {
		cnt = 0;
	}
	
}

void ir_thremometer_timer_init(void)//定时器初始化
{
    am_softimer_init(&key_softimer, key_callback, NULL);
    
}

void ir_thremometer_timer_start(void)//开启定时器
{
	am_softimer_start(&key_softimer, TIMER_MS);
}

void ir_thremometer_timer_stop(void)//停止计时
{
	am_softimer_stop(&key_softimer);
}

void ir_thremometer_timer_clear_cnt(void)//清除计数
{
	if_counting = AM_FALSE;
	cnt = 0;
}

void ir_thremometer_timer_cnt(void)//继续计数
{
	if_counting = AM_TRUE;
}

uint32_t ir_thremometer_timer_get_cnt(void)//获取计数值
{
	return cnt;
}

am_bool_t ir_thremometer_time_to_powe_off(void)//超时关机
{
	if (ir_thremometer_timer_get_cnt() >= POWER_OFF_TIMEOUT) {
		return AM_TRUE;
	} else {
		return AM_FALSE;
	}
}


#ifndef __AM_USER_LCD_DISPLAY_H
#define __AM_USER_LCD_DISPLAY_H

#include "am_common.h"
#include "am_pcf8551.h"

#define AM_USER_BT_DISP         (0x0001)        /* 电池最外框    */
#define AM_USER_BZ_DISP         (0x0002)        /* 蜂鸣器        */
#define AM_USER_0F_DISP         (0x0004)        /* 华氏度单位    */
#define AM_USER_0C_DISP         (0x0008)        /* 摄氏度单位    */
#define AM_USER_P1_DISP         (0x0010)        /* 小数点        */
#define AM_USER_BT3_DISP        (0x0020)        /* 电量满        */
#define AM_USER_LOG_DISP        (0x0040)        /* 记忆          */
#define AM_USER_SUR_DISP        (0x0080)        /* 物体温度      */
#define AM_USER_KU_DISP         (0x0100)        /* 哭脸          */
#define AM_USER_XIAO_DISP       (0x0200)        /* 笑脸          */
#define AM_USER_BLUE_DISP       (0x0400)        /* 蓝牙          */
#define AM_USER_BODY_DISP       (0x0800)        /* 体温          */
#define AM_USER_BT2_DISP        (0x1000)        /* 电量中        */
#define AM_USER_BT1_DISP        (0x2000)        /* 电量低        */

#define AM_USER_NO_MEM          (100)

#define  am_lcd_handle_t  am_pcf8551_handle_t

/**
 * \brief  用户 LCD 初始化
 */
am_lcd_handle_t am_user_lcd_disp_init (void);

/**
 * \brief  用户 LCD 清屏
 */
int am_user_lcd_disp_clr (am_lcd_handle_t handle);

/****************(调用以下函数相当于一次全屏擦除并显示data)*******************/

/**
 * \brief  用户 LCD 写浮点数
 */
int am_user_lcd_disp_float (am_lcd_handle_t handle,
                            float           data);

/**
 * \brief  用户 LCD 显示相关报警
 *
 * \param[in] handle        :   LCD设备服务句柄
 * \param[in] alarm_type    :   报警类型：如 # AM_USER_DISP_ERR
 *
 * \retval 其他 : 对应错误码
 * \retval  0 : 执行成功
 */
typedef enum {
    AM_USER_DISP_ERR = 0,
    AM_USER_DISP_LO,
    AM_USER_DISP_HO,
    AM_USER_DISP_STARTUP_DISP,
	AM_USER_DISP_F2,
	AM_USER_DISP_F4
}am_user_lcd_disp_alarm_t;

int am_user_lcd_disp_alarm(am_lcd_handle_t              handle,
                           am_user_lcd_disp_alarm_t     alarm_type);

/*****************************************************************************/

/**
 * \brief  用户 LCD 写记忆数（取整）
 */
int am_user_lcd_disp_mem(am_lcd_handle_t    handle,
                         uint32_t           data);

/**
 * \brief  用户 LCD 记忆数显示相关报警
 */
typedef enum {
    AM_USER_MEM_DISP_F2 = 0,
    AM_USER_MEM_DISP_F4,
}am_user_lcd_disp_mem_alarm_t;

int am_user_lcd_disp_mem_alarm(am_lcd_handle_t                  handle,
                               am_user_lcd_disp_mem_alarm_t     alarm_type);

/**
 * \brief  用户 LCD 特殊显示使能
 *
 * \param[in] handle        :   LCD设备服务句柄
 * \param[in] flags         :   使能相应特殊显示，如：# AM_USER_BT_DISP
 *                              也可以同时或上其他宏：# AM_USER_BT_DISP | AM_USER_BODY_DISP
 *
 * \retval 其他 : 对应错误码
 * \retval  0 : 执行成功
 */
int am_user_lcd_disp_flag_enable(am_lcd_handle_t    handle,
                                 uint32_t           flags);

/**
 * \brief  用户 LCD 特殊显示禁能
 *
 * \param[in] handle        :   LCD设备服务句柄
 * \param[in] flags         :   禁能相应特殊显示，如：# AM_USER_BT_DISP
 *                              也可以同时或上其他宏：# AM_USER_BT_DISP | AM_USER_BODY_DISP
 *
 * \retval 其他 : 对应错误码
 * \retval  0 : 执行成功
 */
int am_user_lcd_disp_flag_disable(am_lcd_handle_t    handle,
                                  uint32_t           flags);

#endif


/*******************************************************************************
*                                 AMetal
*                       ----------------------------
*                       innovating embedded platform
*
* Copyright (c) 2001-2018 Guangzhou ZHIYUAN Electronics Co., Ltd.
* All rights reserved.
*
* Contact information:
* web site:    http://www.zlg.cn/
*******************************************************************************/

/**
 * \file
 * \brief 蓝牙收发
 *
 * \internal
 * \par Modification history
 * - 1.00 20-03-10  yjx, first implementation
 * \endinternal
 */
#include "am_gpio.h"
#include "hc32x3x_pin.h"
#include "ir_thremometer_ble.h"
#include "ametal.h"
#include "am_hc32l13x_inst_init.h"
#include "am_zlg9021.h"
#include "am_vdebug.h"
#include "string.h"

static am_zlg9021_handle_t ble_handle = 0;
/**
 * \brief 蓝牙初始化
 */
void ir_thremometer_ble_init(void)
{
	  uint8_t   temp[50] = {0};
    uint32_t  zlg9021_baudrate       = 0;
		uint8_t     ret = 0;
		
		ble_handle = am_zlg9021_inst_init();
		am_mdelay(500);    /* 等待模块初始化完成，完成发送"TTM:ZLG9021P0-1-TC") */
		
        am_uart_ioctl(ble_handle->uart_handle,
                      AM_UART_BAUD_SET,
                      (void *)9600);

        /* 尝试获取 ZLG9021 的波特率 */
       ret = am_zlg9021_ioctl(ble_handle,
                         AM_ZLG9021_BAUD_GET,
                         (void *)&zlg9021_baudrate);
    if (ret == AM_OK) {     /* 获取成功 */
        sprintf((char *)temp, "ZLG9021 BAUDRATE: %d", (int)zlg9021_baudrate);
        am_mdelay(100);
        ir_thremometer_ble_send(
                        (const uint8_t *)temp,
                         strlen((const char *)temp));
    }
		        am_zlg9021_ioctl(ble_handle,  /* 蓝牙重新命名 */
                         AM_ZLG9021_RENAME,
                         "ZLG_BLE");

}
/**
 * \brief 通过蓝牙发送数据
 */
void  ir_thremometer_ble_send(const uint8_t  *p_buf,  int  len)
{
			am_zlg9021_send ( ble_handle, p_buf,len);
}
/**
 * \brief 通过蓝牙接收数据
 */
int  ir_thremometer_ble_recv( uint8_t  *p_buf,  int  len)
{
 return am_zlg9021_recv ( ble_handle, p_buf,len);
	
}
/* end of file */


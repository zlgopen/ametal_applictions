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
#ifndef _IR_THREMOMETER_BLE_H_
#define _IR_THREMOMETER_BLE_H_

/**
 * \brief 蓝牙初始化
 *
 * \param[in] 无
 *
 * \return 无
 */
void ir_thremometer_ble_init(void);
/**
 * \brief 蓝牙数据发送
 *
 * \param[in] p_buf  发送数据
 * \param[in] len  发送数据长度
 *
 * \return 无
 */
void  ir_thremometer_ble_send(const uint8_t  *p_buf,  int  len);
/**
 * \brief 蓝牙数据接收
 *
 * \param[in] p_buf  接收数据
 * \param[in] len  接收数据长度
 *
 * \return 接收成功标志
 */
int  ir_thremometer_ble_recv( uint8_t  *p_buf,  int  len);

#endif
/* end of file */

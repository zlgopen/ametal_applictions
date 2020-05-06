/**
 * \file
 * \brief LCD驱动层实现
 *
 */

#include "ametal.h"
#include "am_gpio.h"
#include "am_zlg116.h"
#include "zlg116_pin.h"
#include "am_aml166_inst_init.h"
#include "am_user_lcd_display.h"
#include "am_hwconf_pcf8551.h"

/* 显存起止地址 */
#define DISP_ADDR_START                    (0X04)
#define DISP_ADDR_END                      (0X17)

/* 特殊显示宏定义 */
#define AMHW_USER_BT_LCDRAM                 (0)
#define AMHW_USER_BZ_LCDRAM                 (2)
#define AMHW_USER_0F_LCDRAM                 (4)
#define AMHW_USER_0C_LCDRAM                 (6)
#define AMHW_USER_P1_LCDRAM                 (0)
#define AMHW_USER_BT3_LCDRAM                (0)
#define AMHW_USER_LOG_LCDRAM                (0)
#define AMHW_USER_SUR_LCDRAM                (1)
#define AMHW_USER_KU_LCDRAM                 (1)
#define AMHW_USER_XIAO_LCDRAM               (1)
#define AMHW_USER_BLUE_LCDRAM               (1)
#define AMHW_USER_BODY_LCDRAM               (3)
#define AMHW_USER_BT2_LCDRAM                (5)
#define AMHW_USER_BT1_LCDRAM                (7)

#define AMHW_USER_BT_DISP                   (0x01)
#define AMHW_USER_BZ_DISP                   (0x01)
#define AMHW_USER_0F_DISP                   (0x01)
#define AMHW_USER_0C_DISP                   (0x01)
#define AMHW_USER_P1_DISP                   (0x04)
#define AMHW_USER_BT3_DISP                  (0x10)
#define AMHW_USER_LOG_DISP                  (0x40)
#define AMHW_USER_SUR_DISP                  (0x01)
#define AMHW_USER_KU_DISP                   (0x04)
#define AMHW_USER_XIAO_DISP                 (0x10)
#define AMHW_USER_BLUE_DISP                 (0x20)
#define AMHW_USER_BODY_DISP                 (0x20)
#define AMHW_USER_BT2_DISP                  (0x20)
#define AMHW_USER_BT1_DISP                  (0x20)

/* 定义LCD显示类型*/
typedef enum {
    AM_DISP_FLOAT = 0,
    AM_DISP_ALARM,
    AM_DISP_MEM,
    AM_DISP_MEM_ALARM,
    AM_DISP_FLAG
} am_lcd_disp_type_t;

/* 定义LCDRAM */
static uint8_t __g_lcdram[8] = {0};

/**
 * \brief 显存地址
 *
 * \note : 8个地址控制
 */
static uint8_t lcd_addr[8] = 
{ 
   0x04, 0x05, /* COM0，seg0~seg13 */
   0x09, 0x0A, /* COM1，seg0~seg13 */
   0x0E, 0x0F, /* COM2，seg0~seg13 */
   0X13, 0X14  /* COM3，seg0~seg13 */
};

/**
 * \brief 显示数据
 *
 * \note : LCD屏一共四个数字显示单元，LCD每个数字显示由4个字节的2位控制
 */
static uint8_t lcd_datacode[][4] = 
{ 
  /* COM0, COM1, COM2, COM3 */
    {0x01, 0x03, 0x01, 0x03},    /* 显示0 */
    {0x00, 0x01, 0x01, 0x00},    /* 显示1 */
    {0x01, 0x02, 0x03, 0x01},    /* 显示2 */
    {0x01, 0x01, 0x03, 0x01},    /* 显示3 */
    {0x00, 0x01, 0x03, 0x02},    /* 显示4 */
    {0x01, 0x01, 0x02, 0x03},    /* 显示5 */
    {0x01, 0x03, 0x02, 0x03},    /* 显示6 */
    {0x00, 0x01, 0x01, 0x01},    /* 显示7 */
    {0x01, 0x03, 0x03, 0x03},    /* 显示8 */
    {0x01, 0x01, 0x03, 0x03},    /* 显示9 */
    
    {0x00, 0x00, 0x00, 0x00},    /* 不显示 */
    {0x01, 0x02, 0x02, 0x03},    /* 显示E */
    {0x00, 0x03, 0x03, 0x03},    /* 显示R */
    {0x01, 0x02, 0x00, 0x02},    /* 显示L */
    {0x01, 0x03, 0x02, 0x00},    /* 显示o */
    {0x00, 0x03, 0x03, 0x02},    /* 显示H */
    {0x00, 0x00, 0x02, 0x00},    /* 显示- */
    {0x00, 0x02, 0x02, 0x03}     /* 显示F */
};

/* 显示特定字符位置，参照字符在段码表中的位置 
 * 数字 0~9 占数组的前10个，特定位置从 10 开始
 */
#define DISP_POS_BLANK          (10)  /* 不显示 */
#define DISP_POS_E              (11)  /* 显示E */
#define DISP_POS_R              (12)  /* 显示R */
#define DISP_POS_L              (13)  /* 显示L */
#define DISP_POS_o              (14)  /* 显示o */
#define DISP_POS_H              (15)  /* 显示H */
#define DISP_POS_SEG_g          (16)  /* 显示- */
#define DISP_POS_F              (17)  /* 显示F */

/**
 * \brief LCD显示数据
 */
static void __lcd_disp_data (am_lcd_handle_t     handle,
                             uint8_t            *data_bit,
                             am_lcd_disp_type_t  disp_type)
{
    uint8_t i = 0;

    if ((disp_type == AM_DISP_FLOAT) || (disp_type == AM_DISP_ALARM)) {

        /* 清除1、2位数据 */
        __g_lcdram[1] &= ~0x0A;
        __g_lcdram[3] &= ~0x1E;
        __g_lcdram[5] &= ~0x1E;
        __g_lcdram[7] &= ~0x1E;

        /* 清除3、4位数据 */
        __g_lcdram[0] &= ~0x0A;
        __g_lcdram[2] &= ~0x1E;
        __g_lcdram[4] &= ~0x1E;
        __g_lcdram[6] &= ~0x1E;
        
        /* 写入1、2位数据 */
        __g_lcdram[1] |= (lcd_datacode[data_bit[3]][0] << 3) |  /* 05h */
                         (lcd_datacode[data_bit[2]][0] << 1);
        __g_lcdram[3] |= (lcd_datacode[data_bit[3]][1] << 3) |  /* 0Ah */
                         (lcd_datacode[data_bit[2]][1] << 1);
        __g_lcdram[5] |= (lcd_datacode[data_bit[3]][2] << 3) |  /* 0Fh */
                         (lcd_datacode[data_bit[2]][2] << 1);
        __g_lcdram[7] |= (lcd_datacode[data_bit[3]][3] << 3) |  /* 14h */
                         (lcd_datacode[data_bit[2]][3] << 1);
        
        /* 小数点 */
        __g_lcdram[0] |= 0x04;
        
        /* 写入3、4位数据 */
        __g_lcdram[0] |= (lcd_datacode[data_bit[1]][0] << 3) |  /* 04h */
                         (lcd_datacode[data_bit[0]][0] << 1);   
        __g_lcdram[2] |= (lcd_datacode[data_bit[1]][1] << 3) |  /* 09h */
                         (lcd_datacode[data_bit[0]][1] << 1);   
        __g_lcdram[4] |= (lcd_datacode[data_bit[1]][2] << 3) |  /* 0Eh */
                         (lcd_datacode[data_bit[0]][2] << 1);   
        __g_lcdram[6] |= (lcd_datacode[data_bit[1]][3] << 3) |  /* 13h */
                         (lcd_datacode[data_bit[0]][3] << 1);

    } else if ((disp_type == AM_DISP_MEM) || (disp_type == AM_DISP_MEM_ALARM)) {
        
        /* 清除5、6位数据 */
        __g_lcdram[1] &= ~0x00;
        __g_lcdram[3] &= ~0x01;
        __g_lcdram[5] &= ~0x01;
        __g_lcdram[7] &= ~0x01;

        __g_lcdram[0] &= ~0xA0;
        __g_lcdram[2] &= ~0xE0;
        __g_lcdram[4] &= ~0xE0;
        __g_lcdram[6] &= ~0xE0;
        
        /* 写入5、6位数据 */
        __g_lcdram[1] |= (lcd_datacode[data_bit[1]][0] >> 1);   /* 05h */
        __g_lcdram[3] |= (lcd_datacode[data_bit[1]][1] >> 1);   /* 0Ah */
        __g_lcdram[5] |= (lcd_datacode[data_bit[1]][2] >> 1);   /* 0Fh */
        __g_lcdram[7] |= (lcd_datacode[data_bit[1]][3] >> 1);   /* 14h */

        __g_lcdram[0] |= (lcd_datacode[data_bit[1]][0] << 7) |  /* 04h */
                         (lcd_datacode[data_bit[0]][0] << 5);   
        __g_lcdram[2] |= (lcd_datacode[data_bit[1]][1] << 7) |  /* 09h */
                         (lcd_datacode[data_bit[0]][1] << 5);   
        __g_lcdram[4] |= (lcd_datacode[data_bit[1]][2] << 7) |  /* 0Eh */
                         (lcd_datacode[data_bit[0]][2] << 5);   
        __g_lcdram[6] |= (lcd_datacode[data_bit[1]][3] << 7) |  /* 13h */
                         (lcd_datacode[data_bit[0]][3] << 5);

    } else if (disp_type == AM_DISP_FLAG) {

    }
    
    for (i = 0; i < 8; i++) {

        am_pcf8551_display(handle, lcd_addr[i], &__g_lcdram[i], 1);
    }
}

/**
 * \brief 用户 LCD 初始化
 */
am_lcd_handle_t am_user_lcd_disp_init (void)
{
    am_lcd_handle_t handle = am_pcf8551_inst_init();

    am_pcf8551_display_on(handle);

    return handle;
}

/**
 * \brief 用户 LCD 清屏
 */
int am_user_lcd_disp_clr (am_lcd_handle_t handle)
{
    int i = 0;
    uint8_t temp = 0x00;

    if (handle == NULL){

        return AM_ERROR;
    }

    /* 清除显存数据 */
    for (i = DISP_ADDR_START; i <= DISP_ADDR_END; i++){

        am_pcf8551_display(handle, i, &temp, 1);
    }

    return AM_OK;
}

/****************(调用以下函数相当于一次全屏擦除并显示data)*******************/

/**
 * \brief 用户 LCD 写浮点数
 */
int am_user_lcd_disp_float (am_lcd_handle_t handle,
                            float           data)
{
    if (handle == NULL){
        return AM_ERROR;
    }

    uint8_t         i = 0;
    int             data_ten_times = 0;
    uint8_t         data_bit[4] = {0};
    uint32_t        data_div = 0;

    if (data < -0.000001) {
        /* data 为负数 */
        data_ten_times = (int)(data * 10);      /* 将 data 放大 10 倍，取整 */
        data_ten_times = 0 - data_ten_times;    /* 取绝对值     */

        data_div = data_ten_times;
        /* 分离各个位 */
        while (data_div != 0) {
            *(data_bit + i) = data_div % 10;
            i++;
            data_div /= 10;
        }

        /* 判断第二个数码管的值是否为0 */
        for (i = 2; i > 1; i--) {
            if (data_bit[i] != 0) {
                break;
            } else {
                data_bit[i] = DISP_POS_BLANK;
            }
        }

        /* 高位显示负号 “-” */
        data_bit[3] = DISP_POS_SEG_g;
    } else if ((data >= 0) && (data <= 999.9)) { 

        data_ten_times = (int)(data * 10);  /* 将 data 放大 10 倍，取整 */
        data_div = data_ten_times;
        /* 当data不为0时分离各个位，data为0时跳过while */
        while (data_div != 0) {
            *(data_bit + i) = data_div % 10;
            i++;
            data_div /= 10;
        }

        /*  
            此段作用：当高位为0时不显示（当 data = 0 时，只显示个位的0）
        例：
            四位数：全显示                                  1357    显示 1357
            三位数：最高位0不显示                           103     显示 x103
            二位数：最高位和次高位的0不显示                 78      显示 xx78
            一位数：最高三个位的0不显示，只显示最低位       5       显示 xxx5
                    （当数据为0时，显示xxx0）
        */
        for (i = 3; i > 1; i--) {        /* 从最高位往次低位判断 */
            if (data_bit[i] != 0) {
                break;
            } else {
                data_bit[i] = DISP_POS_BLANK;
            }
        }
    } else {

    }
    
    /* 显示数据 */
    __lcd_disp_data(handle, data_bit, AM_DISP_FLOAT);
    
    return AM_OK;
}

/**
 * \brief 用户 LCD 显示相关报警
 */
int am_user_lcd_disp_alarm(am_lcd_handle_t              handle,
                           am_user_lcd_disp_alarm_t     alarm_type)
{
    if (handle == NULL){
        return AM_ERROR;
    }

    uint8_t data_bit[4] = {0};

    if (alarm_type == AM_USER_DISP_ERR) {

        data_bit[0] = DISP_POS_R;       /*  字母R           */
        data_bit[1] = DISP_POS_R;       /*  字母R           */
        data_bit[2] = DISP_POS_E;       /*  字母E           */
        data_bit[3] = DISP_POS_BLANK;   /*  空白，无显示    */

    } else if (alarm_type == AM_USER_DISP_LO) {

        data_bit[0] = DISP_POS_BLANK;   /* 空白（无显示）   */
        data_bit[1] = DISP_POS_o;       /* 字母o            */
        data_bit[2] = DISP_POS_L;       /* 字母L            */
        data_bit[3] = DISP_POS_BLANK;   /* 空白（无显示）   */

    } else if (alarm_type == AM_USER_DISP_HO) {

        data_bit[0] = DISP_POS_BLANK;   /* 空白（无显示）   */
        data_bit[1] = DISP_POS_o;       /* 字母o            */
        data_bit[2] = DISP_POS_H;       /* 字母H            */
        data_bit[3] = DISP_POS_BLANK;   /* 空白（无显示）   */

    } else if (alarm_type == AM_USER_DISP_STARTUP_DISP){

        data_bit[0] = DISP_POS_SEG_g;   /* G段亮            */
        data_bit[1] = DISP_POS_SEG_g;   /* G段亮            */
        data_bit[2] = DISP_POS_SEG_g;   /* G段亮            */
        data_bit[3] = DISP_POS_BLANK;   /* 空白（无显示）   */

    } else if (alarm_type == AM_USER_DISP_F2){
		
        data_bit[0] = 2;  				/* 数字2            */
        data_bit[1] = DISP_POS_SEG_g;   /* G段亮            */
        data_bit[2] = DISP_POS_F;       /* 字母F            */
        data_bit[3] = DISP_POS_BLANK;   /* 空白（无显示）   */
    } else if (alarm_type == AM_USER_DISP_F4){
		
        data_bit[0] = 4;  				/* 数字4            */
        data_bit[1] = DISP_POS_SEG_g;   /* G段亮            */
        data_bit[2] = DISP_POS_F;       /* 字母F            */
        data_bit[3] = DISP_POS_BLANK;   /* 空白（无显示）   */
    } else {
		
	}

    /* 显示数据 */
    __lcd_disp_data(handle, data_bit, AM_DISP_ALARM);

    return AM_OK;
}

/*****************************************************************************/

/**
 * \brief 用户 LCD 写记忆数（取整）
 */
int am_user_lcd_disp_mem(am_lcd_handle_t    handle,
                         uint32_t           data)
{
    if (handle == NULL){
        return AM_ERROR;
    }

    uint8_t  i = 0;
    uint8_t  data_bit[2] = {0};
    uint32_t data_div = data;

    if (data > 99) {
		if (data == AM_USER_NO_MEM) {
			data_bit[0] = DISP_POS_SEG_g;
			data_bit[1] = DISP_POS_SEG_g;
		}
    } else {
        /* 当data不为0时分离各个位，data为0时跳过while */
        while (data_div != 0) {
            *(data_bit + i) = data_div % 10;
            i++;
            data_div /= 10;
        }
    }

    /* 显示数据 */
    __lcd_disp_data(handle, data_bit, AM_DISP_MEM);

    return AM_OK;
}

/**
 * \brief  用户 LCD 记忆数显示相关报警
 */
int am_user_lcd_disp_mem_alarm(am_lcd_handle_t                  handle,
                               am_user_lcd_disp_mem_alarm_t     alarm_type)
{
    if (handle == NULL){
        return AM_ERROR;
    }

    uint8_t data_bit[2] = {0};

    if (alarm_type == AM_USER_MEM_DISP_F2){

        data_bit[0] = 2;                /* 数字2 */
        data_bit[1] = DISP_POS_F;       /* 字母F */

    } else if (alarm_type == AM_USER_MEM_DISP_F4) {

        data_bit[0] = 4;                /* 数字4 */
        data_bit[1] = DISP_POS_F;       /* 字母F */

    } else {

    }

    /* 显示数据 */
    __lcd_disp_data(handle, data_bit, AM_DISP_MEM_ALARM);

    return AM_OK;
}

/**
 * \brief 用户 LCD 特殊显示使能
 */
int am_user_lcd_disp_flag_enable(am_lcd_handle_t    handle,
                                 uint32_t           flags)
{
    if (handle == NULL){
        return AM_ERROR;
    }

    if (flags & AM_USER_BT_DISP) {
        __g_lcdram[AMHW_USER_BT_LCDRAM] |= AMHW_USER_BT_DISP;
    } else {
        __g_lcdram[AMHW_USER_BT_LCDRAM] &= ~AMHW_USER_BT_DISP;
    }

    if (flags & AM_USER_BZ_DISP) {
        __g_lcdram[AMHW_USER_BZ_LCDRAM] |= AMHW_USER_BZ_DISP;
    } else {
        __g_lcdram[AMHW_USER_BZ_LCDRAM] &= ~AMHW_USER_BZ_DISP;
    }

    if (flags & AM_USER_0F_DISP) {
        __g_lcdram[AMHW_USER_0F_LCDRAM] |= AMHW_USER_0F_DISP;
    } else {
        __g_lcdram[AMHW_USER_0F_LCDRAM] &= ~AMHW_USER_0F_DISP;
    }

    if (flags & AM_USER_0C_DISP) {
        __g_lcdram[AMHW_USER_0C_LCDRAM] |= AMHW_USER_0C_DISP;
    } else  {
        __g_lcdram[AMHW_USER_0C_LCDRAM] &= ~AMHW_USER_0C_DISP;
    }

    if (flags & AM_USER_P1_DISP) {
        __g_lcdram[AMHW_USER_P1_LCDRAM] |= AMHW_USER_P1_DISP;
    } else {
        __g_lcdram[AMHW_USER_P1_LCDRAM] &= ~AMHW_USER_P1_DISP;
    }

    if (flags & AM_USER_BT3_DISP) {
        __g_lcdram[AMHW_USER_BT3_LCDRAM] |= AMHW_USER_BT3_DISP;
    } else {
        __g_lcdram[AMHW_USER_BT3_LCDRAM] &= ~AMHW_USER_BT3_DISP;
    }

    if (flags & AM_USER_LOG_DISP) {
        __g_lcdram[AMHW_USER_LOG_LCDRAM] |= AMHW_USER_LOG_DISP;
    } else {
        __g_lcdram[AMHW_USER_LOG_LCDRAM] &= ~AMHW_USER_LOG_DISP;
    }

    if (flags & AM_USER_SUR_DISP) {
        __g_lcdram[AMHW_USER_SUR_LCDRAM] |= AMHW_USER_SUR_DISP;
    } else {
        __g_lcdram[AMHW_USER_SUR_LCDRAM] &= ~AMHW_USER_SUR_DISP;
    }

    if (flags & AM_USER_KU_DISP) {
        __g_lcdram[AMHW_USER_KU_LCDRAM] |= AMHW_USER_KU_DISP;
    } else {
        __g_lcdram[AMHW_USER_KU_LCDRAM] &= ~AMHW_USER_KU_DISP;
    }

    if (flags & AM_USER_XIAO_DISP) {
        __g_lcdram[AMHW_USER_XIAO_LCDRAM] |= AMHW_USER_XIAO_DISP;
    } else {
        __g_lcdram[AMHW_USER_XIAO_LCDRAM] &= ~AMHW_USER_XIAO_DISP;
    }


    if (flags & AM_USER_BLUE_DISP) {
        __g_lcdram[AMHW_USER_BLUE_LCDRAM] |= AMHW_USER_BLUE_DISP;
    } else {
        __g_lcdram[AMHW_USER_BLUE_LCDRAM] &= ~AMHW_USER_BLUE_DISP;
    }

    if (flags & AM_USER_BODY_DISP) {
        __g_lcdram[AMHW_USER_BODY_LCDRAM] |= AMHW_USER_BODY_DISP;
    } else {
        __g_lcdram[AMHW_USER_BODY_LCDRAM] &= ~AMHW_USER_BODY_DISP;
    }
    if (flags & AM_USER_BT2_DISP) {
        __g_lcdram[AMHW_USER_BT2_LCDRAM] |= AMHW_USER_BT2_DISP;
    } else  {
        __g_lcdram[AMHW_USER_BT2_LCDRAM] &= ~AMHW_USER_BT2_DISP;
    } 
    if (flags & AM_USER_BT1_DISP) {
        __g_lcdram[AMHW_USER_BT1_LCDRAM] |= AMHW_USER_BT1_DISP;
    } else {
        __g_lcdram[AMHW_USER_BT1_LCDRAM] &= ~AMHW_USER_BT1_DISP;
    }

    /* 显示数据 */
    __lcd_disp_data(handle, NULL, AM_DISP_FLAG);

    return AM_OK;
}

/**
 * \brief 用户 LCD 特殊显示禁能
 */
int am_user_lcd_disp_flag_disable(am_lcd_handle_t    handle,
                                  uint32_t           flags)
{
    if (handle == NULL){
        return AM_ERROR;
    }

    if (flags & AM_USER_BT_DISP) {
        __g_lcdram[AMHW_USER_BT_LCDRAM] &= (~AMHW_USER_BT_DISP);
    }

    if (flags & AM_USER_BZ_DISP) {
        __g_lcdram[AMHW_USER_BZ_LCDRAM] &= (~AMHW_USER_BZ_DISP);
    }

    if (flags & AM_USER_0F_DISP) {
        __g_lcdram[AMHW_USER_0F_LCDRAM] &= (~AMHW_USER_0F_DISP);
    }

    if (flags & AM_USER_0C_DISP) {
        __g_lcdram[AMHW_USER_0C_LCDRAM] &= (~AMHW_USER_0C_DISP);
    }

    if (flags & AM_USER_P1_DISP) {
        __g_lcdram[AMHW_USER_P1_LCDRAM] &= (~AMHW_USER_P1_DISP);
    }

    if (flags & AM_USER_BT3_DISP) {
        __g_lcdram[AMHW_USER_BT3_LCDRAM] &= (~AMHW_USER_BT3_DISP);
    }

    if (flags & AM_USER_LOG_DISP) {
        __g_lcdram[AMHW_USER_LOG_LCDRAM] &= (~AMHW_USER_LOG_DISP);
    }

    if (flags & AM_USER_SUR_DISP) {
        __g_lcdram[AMHW_USER_SUR_LCDRAM] &= (~AMHW_USER_SUR_DISP);
    }

    if (flags & AM_USER_KU_DISP) {
        __g_lcdram[AMHW_USER_KU_LCDRAM] &= (~AMHW_USER_KU_DISP);
    }

    if (flags & AM_USER_XIAO_DISP) {
        __g_lcdram[AMHW_USER_XIAO_LCDRAM] &= (~AMHW_USER_XIAO_DISP);
    }

    if (flags & AM_USER_BLUE_DISP) {
        __g_lcdram[AMHW_USER_BLUE_LCDRAM] &= (~AMHW_USER_BLUE_DISP);
    }

    if (flags & AM_USER_BODY_DISP) {
        __g_lcdram[AMHW_USER_BODY_LCDRAM] &= (~AMHW_USER_BODY_DISP);
    }

    if (flags & AM_USER_BT2_DISP) {
        __g_lcdram[AMHW_USER_BT2_LCDRAM] &= (~AMHW_USER_BT2_DISP);
    }

    if (flags & AM_USER_BT1_DISP) {
        __g_lcdram[AMHW_USER_BT1_LCDRAM] &= (~AMHW_USER_BT1_DISP);
    }

    /* 显示数据 */
    __lcd_disp_data(handle, NULL, AM_DISP_FLAG);

    return AM_OK;
}

/* end of file */


#include "ametal.h"
#include "am_board.h"
#include "am_vdebug.h"
#include "am_delay.h"
#include "am_gpio.h"
#include "zlg116_pin.h"
#include "am_aml166_inst_init.h"
#include "zal_infrared_sensor_interpolation.h"
#include "ir_thremometer_adc.h"
#include "am_zml166_adc.h"
#include "am_hwconf_zml166_adc.h"
#include <stdlib.h>
#include "ir_thremometer_calibration.h"

/**
 * \brief AM_ZML166_ADC增益选择参数
 */
#define AM_ZML166_ADC_PGA_SET_1         0      /**< \brief 1倍增益选择参数 */
#define AM_ZML166_ADC_PGA_SET_2         1      /**< \brief 2倍增益选择参数 */
#define AM_ZML166_ADC_PGA_SET_4         2      /**< \brief 4倍增益选择参数 */
#define AM_ZML166_ADC_PGA_SET_8         3      /**< \brief 8倍增益选择参数 */
#define AM_ZML166_ADC_PGA_SET_16        4     /**< \brief 16倍增益选择参数 */
#define AM_ZML166_ADC_PGA_SET_32        5     /**< \brief 32倍增益选择参数 */
#define AM_ZML166_ADC_PGA_SET_64        6     /**< \brief 64倍增益选择参数 */
#define AM_ZML166_ADC_PGA_SET_128       7    /**< \brief 128倍增益选择参数 */
#define AM_ZML166_ADC_PGA_SET_256       8    /**< \brief 256倍增益选择参数 */


#define ADC_BAT_CHANNEL  0
#define ADC_SAMPLE_LEN   6

char str[100] = {0};

static am_zml166_adc_handle_t adc_handle = 0;
static int32_t   adc_samples[ADC_SAMPLE_LEN] = {0};

/**
 * \brief 获取ADC并求平均值
 */
static int32_t __adc_code_get_agv (am_adc_handle_t handle, int chan)
{
	int32_t agv = 0;
	int32_t min = 0x7FFFFFFF;
	int32_t max = 0;
	
	am_adc_read(handle, chan, adc_samples, ADC_SAMPLE_LEN);

	for (int i = 0; i < ADC_SAMPLE_LEN; i++){
		if (adc_samples[i] > max) {
			max = adc_samples[i];
		}
		
		if (adc_samples[i] < min) {
			min = adc_samples[i];
		}
		
		agv += adc_samples[i];
	}
	agv -= (max + min);
	agv /= (ADC_SAMPLE_LEN - 2);

	return agv;
}

/**
 * \brief 获取NTC电阻值
 */
double ir_thremometer_get_ntc_res(void)
{
	double  res = 0, vol1 = 0 , vol2 = 0;
	uint16_t gain = 1 << AM_ZML166_ADC_PGA_SET_1;

	am_zml166_adc_reg_set(adc_handle, AM_ZML166_ADC_ADC3_ADDR, AM_ZML166_ADC_LVSCP_DISABLE | AM_ZML166_ADC_LVSHIFT_DISABLE);
	am_zml166_adc_mux_set(adc_handle, AM_ZML166_ADC_INPS_AIN0 | AM_ZML166_ADC_INNS_AIN1);
	am_zml166_adc_gain_set(adc_handle, gain);

	vol1 = __adc_code_get_agv(&adc_handle->adc_serve, 0);

	am_zml166_adc_reg_set(adc_handle, AM_ZML166_ADC_ADC3_ADDR, AM_ZML166_ADC_LVSCP_ENABLE | AM_ZML166_ADC_LVSHIFT_ENABLE);
	am_zml166_adc_mux_set(adc_handle, AM_ZML166_ADC_INPS_AIN1 | AM_ZML166_ADC_INNS_GND);

	vol2 = __adc_code_get_agv(&adc_handle->adc_serve, 0);

	res = vol2 * 100 / vol1;

	sprintf(str, "ntc_in1: %.4f  ntc_in2: %.4f  ntc_res: %.4f  \r\n",
	        vol1, vol2, res);
	am_kprintf("%s", str);

	return res;
}

/**
 * \brief 获取红外电压
 */
double ir_thremometer_get_ir_vol(void)
{
	double  vol = 0, vol1 = 0 , vol2 = 0;
	uint16_t gain = 1 << AM_ZML166_ADC_PGA_SET_128;

	am_zml166_adc_reg_set(adc_handle, AM_ZML166_ADC_ADC3_ADDR, AM_ZML166_ADC_LVSCP_DISABLE | AM_ZML166_ADC_LVSHIFT_DISABLE);
	am_zml166_adc_gain_set(adc_handle, gain);
	am_zml166_adc_mux_set(adc_handle, AM_ZML166_ADC_INPS_AIN2 | AM_ZML166_ADC_INNS_AIN3);

    vol1 = __adc_code_get_agv(&adc_handle->adc_serve, 0);
    
	am_zml166_adc_reg_set(adc_handle, AM_ZML166_ADC_ADC3_ADDR, AM_ZML166_ADC_LVSCP_ENABLE | AM_ZML166_ADC_LVSHIFT_ENABLE);
	am_zml166_adc_gain_set(adc_handle, gain);
	am_zml166_adc_mux_set(adc_handle, AM_ZML166_ADC_INPS_AIN3 | AM_ZML166_ADC_INNS_AIN2);

    vol2 = __adc_code_get_agv(&adc_handle->adc_serve, 0);

	vol = (vol1 - vol2) * adc_handle->p_devinfo->vref / 2.0 / 8388607.0 / gain;
    
	sprintf(str, "vol1: %.4f  vol2: %.4f  vol: %.4f  \r\n",
	        vol1, vol2, vol);
	am_kprintf("%s", str);

	return vol;	
}

/**
 * \brief 获取目标温度
 */
double ir_thremometer_get_obj_temp(void)
{
	double res = 0, vol = -1;
	double ntc_temp = 0, obj_temp = 0;
	
	res = ir_thremometer_get_ntc_res();
	vol = ir_thremometer_get_ir_vol();

    vol = ir_thremometer_calib_vol(vol);
    
	ntc_temp = zal_environment_tem_value_get(res) + 0.000001; 
	obj_temp = zal_temperature_get(ntc_temp, vol) + 0.000001;

	sprintf(str, "res = %f k, vol = %f, ntc_temp = %f, obj_temp = %f\r\n", res, vol, ntc_temp, obj_temp);
	AM_DBG_INFO("%s", str);
	return obj_temp;
}

/**
 * \brief 获取电池电压
 */
float ir_thremometer_get_bat_vol(void)
{
    float bat_vol = 0;
    uint32_t vol_code  = 0;
	uint16_t gain = 1 << AM_ZML166_ADC_PGA_SET_1;

	am_zml166_adc_reg_set(adc_handle, AM_ZML166_ADC_ADC3_ADDR, AM_ZML166_ADC_LVSCP_DISABLE | AM_ZML166_ADC_LVSHIFT_DISABLE);
	am_zml166_adc_mux_set(adc_handle, AM_ZML166_ADC_INPS_1_2_VS | AM_ZML166_ADC_INNS_AIN4);
	am_zml166_adc_gain_set(adc_handle, gain);

	vol_code = __adc_code_get_agv(&adc_handle->adc_serve, 0);
    
	bat_vol = 1200 - (double)((double)(vol_code / 8388607.0) * adc_handle->p_devinfo->vref);

	return bat_vol;
}

/**
 * \brief 电池低电压检测
 */
void ir_thremometer_bat_low_check(void)
{
    int vbat[3];
	const uint16_t DIFF_CODE = 30;

	do {
		vbat[0] = ir_thremometer_get_bat_vol(); 
		vbat[1] = ir_thremometer_get_bat_vol(); 
		vbat[2] = ir_thremometer_get_bat_vol(); 
	} while(fabs(vbat[0] - vbat[1]) > DIFF_CODE ||
	        fabs(vbat[1] - vbat[2]) > DIFF_CODE ||
	        fabs(vbat[0] - vbat[2]) > DIFF_CODE);
}

/**
 * @brief ADC配置
 *
 * \param[in] handle ZML166 ADC 服务句柄
 * @return	  无；
 */
void ir_thremometer_adc_config(am_zml166_adc_handle_t  handle)
{
    am_zml166_adc_reg_set(handle, AM_ZML166_ADC_ADC1_ADDR, AM_ZML166_ADC_DR_12_5 |
                                                           AM_ZML166_ADC_BUFBP_ENABLE |
                                                           AM_ZML166_ADC_PGA_32 |
                                                           AM_ZML166_ADC_ADGN_4);

    am_zml166_adc_reg_set(handle, AM_ZML166_ADC_SYS_ADDR, AM_ZML166_ADC_TMODE_MANUAL |
                                                          AM_ZML166_ADC_PMODE_MANUAL |
                                                          AM_ZML166_ADC_ENREF_ENABLE |
                                                          AM_ZML166_ADC_ENADC_ENABLE |
                                                          AM_ZML166_ADC_ENLDO_ENABLE);

    am_zml166_adc_reg_set(handle, AM_ZML166_ADC_ADC4_ADDR, AM_ZML166_ADC_CHOPM_32_128 |
                                                           AM_ZML166_ADC_ADREFS_REFP_REFN);

    am_zml166_adc_reg_set(handle, AM_ZML166_ADC_ADC3_ADDR, AM_ZML166_ADC_LVSCP_ENABLE |
                                                           AM_ZML166_ADC_LVSHIFT_ENABLE);
}

/**
 * \brief ADC初始化
 */
void ir_thremometer_adc_init(void)
{
	adc_handle = am_zml166_adc_inst_init();
    ir_thremometer_adc_config(adc_handle);
    am_zml166_adc_gain_set(adc_handle, AM_ZML166_ADC_PGA_SET_128);    

	ir_thremometer_bat_low_check();
}

/* end of file */

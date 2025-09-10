/********************************************************************************************************
 * @file    battery_check.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
 *
 *          Licensed under the Apache License, Version 2.0 (the "License");
 *          you may not use this file except in compliance with the License.
 *          You may obtain a copy of the License at
 *
 *              http://www.apache.org/licenses/LICENSE-2.0
 *
 *          Unless required by applicable law or agreed to in writing, software
 *          distributed under the License is distributed on an "AS IS" BASIS,
 *          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *          See the License for the specific language governing permissions and
 *          limitations under the License.
 *
 *******************************************************************************************************/
#ifndef BATTERY_CHECK_H_
#define BATTERY_CHECK_H_


/**
 * @brief	Application battery check enable control. Default disable, use can enable it in app_conifg.h.
 	 	 	User must check battery voltage on mass production application to prevent abnormal writing or erasing Flash at a low voltage !!!
 */
#ifndef APP_BATT_CHECK_ENABLE
#define APP_BATT_CHECK_ENABLE								0
#endif


#if (APP_BATT_CHECK_ENABLE)

extern u32	lowBattDet_tick;


/**
 * @brief	Battery Voltage log enable
 */
#ifndef		APP_BATT_CHECK_LOG_EN
#define		APP_BATT_CHECK_LOG_EN								0
#endif



#define VBAT_DEEP_THRES_MV				2000   // 2000 mV low battery alarm

#if(MCU_CORE_TYPE==MCU_CORE_TC321X)

#define SD_ADC_SAMPLE_CLOCK_DIV     SD_ADC_SAPMPLE_CLK_2M_DIV(CLOCK_SYS_CLOCK_HZ)
#define SD_ADC_DOWN_SAMPLE_RATE     SD_ADC_DOWNSAMPLE_RATE_128

/**
 * -# SD_ADC_FIFO_DEPTH must be 2^n (n ranges from 3 to 10): 8/16/32/64/128/256/512/1024,
 * -# SD_ADC_SAMPLE_CNT must be multiple of 4 and < SD_ADC_FIFO_DEPTH and max is 1020 .
 * -# Due to the first 4 sample data anomaly after each call to dc_sample_start()
 *    and the call to dc_sort_and_get_average_code() for filtering,
 *    it is recommended that SD_ADC_SAMPLE_CNT be set to multiple of 4 and >= 16,
 *    SD_ADC_FIFO_DEPTH to 32/64/128/256/512/1024.
 */
#define SD_ADC_FIFO_DEPTH           32
#define SD_ADC_SAMPLE_CNT           16


volatile unsigned char dc_flag;
#endif

extern u8 adc_hw_initialized;

/**
 * @brief      This function is used to clear adc setting flag
 * @param      none
 * @return     none.
 */
static inline void battery_clear_adc_setting_flag (void)
{
	adc_hw_initialized = 0;
}


/**
 * @brief      This function is used to enable battery detect
 * @param[in]  en - 1: enable;  0: disable.
 * @return     none.
 */
void		   battery_set_detect_enable (int en);

/**
 * @brief      This function is used to get enable state of battery detect
 * @param[in]  none.
 * @return     0: Battery detect is disable 	 1:Battery detect is enable.
 */
int  		   battery_get_detect_enable (void);

/**
 * @brief		vbat detect initialization
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void adc_vbat_detect_init(void);

/**
 * @brief	   This is battery check function
 * @param[in]  alram_vol_mv - input battery value
 * @return     0: fail, 1: success
 */
int 		   app_battery_power_check(u16 alram_vol_mv);

#endif


#endif /* APP_BATTDET_H_ */

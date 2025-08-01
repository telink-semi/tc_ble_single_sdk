/********************************************************************************************************
 * @file    ext_calibration.h
 *
 * @brief   This is the header file for B85
 *
 * @author  Driver Group
 * @date    May 8,2018
 *
 * @par     Copyright (c) 2018, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifndef DRIVERS_TC321X_EXT_CALIBRATION_H_
#define DRIVERS_TC321X_EXT_CALIBRATION_H_

#include "../flash.h"
#include "../analog.h"

typedef struct
{
    unsigned short single_gpio_gain_10000x;
    signed short single_gpio_offset_10x;

    unsigned short vbat_gain_10000x;
    signed short vbat_offset_10x;

    unsigned short diff_gpio_gain_10000x;
    signed short diff_gpio_offset_10x;

}sd_adc_calib_t;


/**
 * @brief       This function is used to tighten the judgment of illegal values for gpio calibration and vbat calibration in the flash.
 * @param[in]   gain - the value of single_gpio_gain_10000x ,diff_gpio_gain_10000x and vbat_gain_10000x
 *              offset - the value of single_gpio_offset_10x ,diff_gpio_offset_10x and vbat_offset_10x
 *              calib_func - Function pointer to gpio_calibration or vbat_calibration.
 * @return      1:the calibration function is invalid; 0:the calibration function is valid.
 */
unsigned char flash_set_adc_calib_value(unsigned short gain, signed short offset, void (*calib_func)(unsigned short, signed short));


/**
 * @brief      This function is used to calib ADC 1.2V vref.
 */
int user_calib_adc_vref(sd_adc_calib_t  calib_value);

/**
 * @brief		This function serves to calibration the flash voltage(VDD_F),if the flash has the calib_value,we will use it,either will
 * 				trim vdd_f to 1.95V(2b'111 the max) if the flash is zb.
 */
int user_calib_vdd_f(unsigned char * calib_value);




/**
 * @brief		This function serves to set flash voltage vdd_f.TO ensure the vdd_f is enough to supply the flash,need to calibration the vdd_f.
 * @param[in]	voltage - the vdd_f need to set.The voltage is range from 0 to 0x07.
 * @return		none.
 */
static inline void flash_set_vdd_f(Flash_VoltageDef voltage)
{
	analog_write(0x0c, ((analog_read(0x0c) & 0xf8) | (voltage & 0x07)));
}

#endif



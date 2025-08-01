/********************************************************************************************************
 * @file    ext_calibration.c
 *
 * @brief   This is the source file for B85
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
#include "ext_calibration.h"
#include "drivers.h"
#include "vendor/common/ble_flash.h"

/**
 * @brief       This function is used to tighten the judgment of illegal values for gpio calibration and vbat calibration in the flash.
 * @param[in]   gain - the value of single_gpio_gain_10000x ,diff_gpio_gain_10000x and vbat_gain_10000x
 *              offset - the value of single_gpio_offset_10x ,diff_gpio_offset_10x and vbat_offset_10x
 *              calib_func - Function pointer to gpio_calibration or vbat_calibration.
 * @return      1:the calibration function is invalid; 0:the calibration function is valid.
 */
unsigned char flash_set_adc_calib_value(unsigned short gain, signed short offset, void (*calib_func)(unsigned short, signed short))
{
    /**
     * The legal range of gain for single_gpio/diff_gpio and vbat in flash is [9000,11000],
     * and the legal range of offset for single_gpio/diff_gpio and vbat is [-1000,1000].
     */
    if ((gain >= 9000) && (gain <= 11000) && (offset >= -1000) && (offset <= 1000)) {
        (*calib_func)(gain, offset);
        return 0;
    } else {
        return 1;
    }
}


/**
 * @brief      This function is used to calib ADC 1.2V vref.
 */

int user_calib_adc_vref(sd_adc_calib_t  calib_value) {
    if (flash_set_adc_calib_value(calib_value.single_gpio_gain_10000x, calib_value.single_gpio_offset_10x, adc_set_single_gpio_calib_vref) ||
        flash_set_adc_calib_value(calib_value.vbat_gain_10000x, calib_value.vbat_offset_10x, adc_set_vbat_calib_vref) ||
		flash_set_adc_calib_value(calib_value.diff_gpio_gain_10000x, calib_value.diff_gpio_offset_10x, adc_set_diff_gpio_calib_vref) )
    {
        return false;
    }else
    {
    	return true;
    }
}

/**
 * @brief		This function serves to calibration the flash voltage(VDD_F),if the flash has the calib_value,we will use it,either will
 * 				trim vdd_f to 1.95V(2b'111 the max) if the flash is zb.
 */
int user_calib_vdd_f(unsigned char * calib_value) {
	//Todo
	return 0;
}










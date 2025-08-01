/********************************************************************************************************
 * @file    qdec.h
 *
 * @brief   This is the header file for TC321X
 *
 * @author  Driver Group
 * @date    2024
 *
 * @par     Copyright (c) 2024, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#pragma once


#include "lib/include/pm/pm.h"
#include "analog.h"
#include "register.h"

/**
 * qedc input channel type of A
 */
typedef enum
{
	PA2A,
	PA3A,
	PB6A,
	PB7A,
	PC2A,
	PC3A,
	PD6A,
	PD7A,

}QDEC_InputAchTypeDef;

/**
 * qedc input channel type of B
 */
typedef enum
{
	PA2B,
	PA3B,
	PB6B,
	PB7B,
	PC2B,
	PC3B,
	PD6B,
	PD7B,
}QDEC_InputBchTypeDef;


/**
 * qedc mode
 */
typedef enum
{
	COMMON_MODE,
	DOUBLE_ACCURACY_MODE,
}qdec_mode_e;


/**
 *the hardware debouncing threshold
 */
typedef enum{
	QDEC_THRSH_P187p5us =0,
	QDEC_THRSH_P375p0us,
    QDEC_THRSH_P750p0us,
	QDEC_THRSH_P1500p0us,
	QDEC_THRSH_P3000p0us,
	QDEC_THRSH_P6000p0us,
	QDEC_THRSH_P12000p0us,
	QDEC_THRSH_P24000p0us,
}qdec_thrsh_e;

/**
 * @brief      This function servers to set input port.
 * @param[in]  chn_a - input types of a channel.
 * @param[in]  chn_b - input types of b channel.
 * @return     none.
 */
void qdec_set_pin(QDEC_InputAchTypeDef chn_a, QDEC_InputBchTypeDef chn_b);

/**
 * @brief      This function servers to set qdec mode,qdec mode:common mode and double accuracy mode.
 *             common mode:the qdec counter value is increased/decreased by 1 only when the same rising/falling edges are detected from the two phase signals.
 *             double accuracy mode:the qdec counter value is increased/decreased by 1 on each rising/falling edge of the two phase signals;
 *             the counter value will be increased/decreased by 2 for one wheel rolling.
 * @param[in]  mode - mode type to select.
 * @return     none.
 */
void qdec_set_mode(qdec_mode_e mode);

/**
 * @brief      This function servers to initials qedc source clock.
 * @param[in]  none.
 * @return     none.
 */
void qdec_clk_en(void);

/**
 * @brief      This function servers to read hardware counting value,
 *             After reading the value, if there is no input to channels A and B, then reading the value is 0 through this function,
 *             If the count is not read, the qdec read real time counting value increases or decreases with the direction of the wheel according to the qdec mode.
 * @param[in]  none.
 * @return     hardware counting value.
 */
signed char qdec_get_count_value(void);

/**
 * @brief      This function servers to reset qdec and the qdec counter value is cleared zero.
 * @param[in]  none.
 * @return     none.
 */
void qdec_reset(void);

/**
 * @brief      This function servers to set hardware debouncing.
 * @param[in]  thrsh - any signal with width lower than the threshold will be regarded as jitter,
 *             effective signals input from Channel A and B should contain high/low level with width more than the threshold.
 * @return     none.
 */
void qdec_set_debouncing(qdec_thrsh_e thrsh);

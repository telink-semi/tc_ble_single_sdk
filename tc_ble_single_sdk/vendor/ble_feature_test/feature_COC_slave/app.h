/********************************************************************************************************
 * @file    app.h
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
#ifndef APP_H_
#define APP_H_

#include "../feature_config.h"

#if (FEATURE_TEST_MODE == TEST_L2CAP_COC)


/**
 * @brief		user initialization when MCU power on or wake_up from deepSleep mode
 * @param[in]	none
 * @return      none
 */

void user_init_normal(void);

/**
 * @brief		user initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]	none
 * @return      none
 */
void user_init_deepRetn(void);


/**
 * @brief     BLE main loop
 * @param[in]  none.
 * @return     none.
 */
void main_loop(void);

/**
 * @brief	Initialize the L2CAP CoC channel, configure parameters such as MTU and SPSM.
 * @param[in]  none.
 * @return     none.
 */

void app_l2cap_coc_init(void);

/**
 * @brief      Handle  events related to CoC channel such as connection and disconnection.
 * @param[in]  h       event type
 * @param[in]  para    Pointer point to event parameter buffer.
 * @param[in]  n       the length of event parameter.
 * @return     none
 */

int app_host_coc_event_callback (u32 h, u8 *para, int n);

/**
 * @brief	Establish a connection for the LECoC channel.
 * @param[in]  none.
 * @return     none.
 */
void app_createLeCreditBasedConnect(void);

/**
 * @brief	Establish a connection for the CoC channel.
 * @param[in]  none.
 * @return     none.
 */

void app_createCreditBasedConnect(void);

/**
 * @brief	Send data to all connections on the CoC channel.
 * @param[in]  none.
 * @return     none.
 */

void app_sendCocData(void);

/**
 * @brief	Disconnect the CoC channel connection.
 * @param[in]  none.
 * @return     none.
 */

void app_disconnCocConnect(void);


#endif  //end of (FEATURE_TEST_MODE == ...)
#endif /* APP_H_ */

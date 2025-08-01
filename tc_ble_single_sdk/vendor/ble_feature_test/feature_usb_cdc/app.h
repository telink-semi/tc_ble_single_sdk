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

#if (FEATURE_TEST_MODE == TEST_USB_CDC)


/**
 * @brief		user initialization
 * @param[in]	none
 * @return      none
 */
void user_init(void);


/**
 * @brief     BLE main loop
 * @param[in]  none.
 * @return     none.
 */
void main_loop(void);

/**
 * @brief     USB CDC initialization
 * @param[in]  none.
 * @return     none.
 */
void usb_cdc_init(void);

/**
 * @brief     USB CDC main loop
 * @param[in]  none.
 * @return     none.
 */
void usb_cdc_loop(void);

/**
 * @brief     This external function handles USB interrupt events
 * @param[in]  none.
 * @return     none.
 */

extern void usb_handle_irq(void);

/**
 * @brief     This external function used to be initializing USB-related interrupt handling.
 * @param[in]  none.
 * @return     none.
 */

extern void usb_init_interrupt(void);


#endif  //end of (FEATURE_TEST_MODE == ...)
#endif /* APP_H_ */

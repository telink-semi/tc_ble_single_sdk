/********************************************************************************************************
 * @file    app_config.h
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
#pragma once

#include "../feature_config.h"

#if (FEATURE_TEST_MODE == TEST_USER_BLT_SOFT_UART)

///////////////////////// Feature Configuration////////////////////////////////////////////////
#define BLE_APP_PM_ENABLE								0
#define PM_DEEPSLEEP_RETENTION_ENABLE				0

#define APP_DEFAULT_HID_BATTERY_OTA_ATTRIBUTE_TABLE		1

///////////////////////// SOFT UART Feature Configuration////////////////////////////////////////////////
/**
 *  @brief  software uart enable and setting
 *   (The maximum baud rate is 9600)
 */
#define  	SOFT_UART_ENABLE                       1

#define 	SOFT_UART_BAUD_RATE                     9600
#define 	SOFT_UART_TX_IO                   		GPIO_PA4
#define 	SOFT_UART_RX_IO                   		GPIO_PB4



#define    TEST_RX_TX_RUN                           1
#define    TEST_ONLY_TX_RUN                         2

#define    TEST_SOFT_UART_RUN_MODEL                 TEST_RX_TX_RUN


///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE								0
#define UART_PRINT_DEBUG_ENABLE							1
#define APP_FLASH_INIT_LOG_EN							0
#define APP_LOG_EN										1

/////////////////////// Feature Test Board Select Configuration ///////////////////////////////
#if (__PROJECT_8258_FEATURE_TEST__)
	#define BOARD_SELECT								BOARD_825X_EVK_C1T139A30
#elif (__PROJECT_8278_FEATURE_TEST__)
	#define BOARD_SELECT								BOARD_827X_EVK_C1T197A30
#elif (__PROJECT_TC321X_FEATURE_TEST__)
	#define BOARD_SELECT								BOARD_TC321X_EVK_C1T357A20
#endif



///////////////////////// UI Configuration ////////////////////////////////////////////////////
#define	UI_KEYBOARD_ENABLE								0
#define	UI_LED_ENABLE									0



///////////////////////// System Clock  Configuration /////////////////////////////////////////
#define CLOCK_SYS_CLOCK_HZ  								48000000	//must select 48M clock



/////////////////////////////////////// PRINT DEBUG INFO ///////////////////////////////////////
#if (UART_PRINT_DEBUG_ENABLE)
	#define DEBUG_INFO_TX_PIN           	GPIO_PB1
	#define PULL_WAKEUP_SRC_PB1         	PM_PIN_PULLUP_10K
	#define PB1_OUTPUT_ENABLE         		1
	#define PB1_DATA_OUT                    1
#endif

#if (__PROJECT_TC321X_FEATURE_TEST__)
	#error "TC321X don't support soft UART!"
#endif

#include "vendor/common/default_config.h"


#endif  //end of (FEATURE_TEST_MODE == ...)

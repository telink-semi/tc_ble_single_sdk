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

#if (FEATURE_TEST_MODE == TEST_BLE_PHY)

///////////////////////// Feature Configuration////////////////////////////////////////////////
#define BLE_APP_PM_ENABLE								0
#define PM_DEEPSLEEP_RETENTION_ENABLE				0

#define APP_DEFAULT_HID_BATTERY_OTA_ATTRIBUTE_TABLE		1


///////////////////////// PHYTEST CONFIGURATION ////////////////////////////
#define 		PHYTEST_MODE_THROUGH_2_WIRE_UART		1   //Direct Test Mode through a 2-wire UART interface
#define 		PHYTEST_MODE_OVER_HCI_WITH_USB			2   //Direct Test Mode over HCI(USB hardware interface)
#define 		PHYTEST_MODE_OVER_HCI_WITH_UART			3   //Direct Test Mode over HCI(UART hardware interface)

#define         BLE_PHYTEST_MODE					    PHYTEST_MODE_THROUGH_2_WIRE_UART


///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE								0
#define APP_FLASH_INIT_LOG_EN							0

/////////////////////// Feature Test Board Select Configuration ///////////////////////////////
#if (__PROJECT_8258_FEATURE_TEST__)
	#define BOARD_SELECT								BOARD_825X_EVK_C1T139A30
#elif (__PROJECT_8278_FEATURE_TEST__)
	#define BOARD_SELECT								BOARD_827X_EVK_C1T197A30
#elif (__PROJECT_TC321X_FEATURE_TEST__)
	#define BOARD_SELECT								BOARD_TC321X_EVK_C1T357A20
#endif



///////////////////////// UI Configuration ////////////////////////////////////////////////////
#define	UI_KEYBOARD_ENABLE								1
#define	UI_LED_ENABLE									0



///////////////////////// System Clock  Configuration /////////////////////////////////////////
#define CLOCK_SYS_CLOCK_HZ  								16000000



///////////////////////// PHYTEST CONFIGURATION ////////////////////////////

/*----------------------------------------------*
 *	HCI TX FIFO  = 2 Bytes LEN + n Bytes Data.	*
 *												*
 *	T_txdata_buf = 4 Bytes LEN + n Bytes Data.	*
 *												*
 *	HCI_TXFIFO_SIZE = 2 + n.					*
 *												*
 *	UART_DATA_LEN = n.							*
 *												*
 *	UART_DATA_LEN = HCI_TXFIFO_SIZE - 2.		*
 * ---------------------------------------------*/
#define HCI_RXFIFO_SIZE		72
#define HCI_RXFIFO_NUM		2

#define HCI_TXFIFO_SIZE		72
#define HCI_TXFIFO_NUM		8

#define UART_DATA_LEN    	(HCI_TXFIFO_SIZE - 2)      // data max 252


#if(__PROJECT_TC321X_FEATURE_TEST__)
#define UART_NUM							UART0
#define UART_CONVERT(...)					UART_NUM,__VA_ARGS__
#define UART_TX_PIN							GPIO_PD0
#define UART_RX_PIN							GPIO_PD1
#else
#define UART_NUM
#define UART_CONVERT(...)					__VA_ARGS__
#define UART_TX_PIN							UART_TX_PB1
#define UART_RX_PIN							UART_RX_PB0
#endif

#define	UART_BAUDRATE       				(115200)

#include "vendor/common/default_config.h"


#endif  //end of (FEATURE_TEST_MODE == ...)

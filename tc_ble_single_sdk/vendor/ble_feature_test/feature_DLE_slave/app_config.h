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

#if (FEATURE_TEST_MODE == TEST_LL_DATA_LENGTH_EXTENSION_SLAVE)

///////////////////////// Feature Configuration////////////////////////////////////////////////
#define BLE_APP_PM_ENABLE								0
#define PM_DEEPSLEEP_RETENTION_ENABLE					0
#define BLE_APP_SECURITY_ENABLE      					1


///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE					0

#define UART_PRINT_DEBUG_ENABLE				1

#define APP_LOG_EN							1
#define APP_SMP_LOG_EN						0
#define APP_KEY_LOG_EN						1
#define APP_CONTR_EVENT_LOG_EN				1  //controller event log
#define APP_HOST_EVENT_LOG_EN				1  //host event log
#define APP_FLASH_INIT_LOG_EN				1


/////////////////////// Sample Board Select Configuration ///////////////////////////////
#if (__PROJECT_8258_FEATURE_TEST__)
	#define BOARD_SELECT								BOARD_825X_EVK_C1T139A30
#elif (__PROJECT_8278_FEATURE_TEST__)
	#define BOARD_SELECT								BOARD_827X_EVK_C1T197A30
#elif (__PROJECT_TC321X_FEATURE_TEST__)
	#define BOARD_SELECT								BOARD_TC321X_EVK_C1T357A20
#endif


///////////////////////// Slave Date length extension buffer length CONFIGURATION ////////////////////////////
/* DLE & MTU:
 * connMaxRxOctets & connMaxTxOctets & MTU size */

/**
 * @brief	connMaxRxOctets
 * refer to BLE Core Specification "4.5.10 Data PDU length management" & "2.4.2.21 LL_LENGTH_REQ and LL_LENGTH_RSP"
 * usage limitation:
 * 1. should be in range of 27 ~ 251
 */
#define ACL_CONN_MAX_RX_OCTETS			251	//user set value

/**
 * @brief	connMaxTxOctets
 * refer to BLE Core Specification: Vol 6, Part B, "4.5.10 Data PDU length management"
 * 					  Vol 6, Part B, "2.4.2.21 LL_LENGTH_REQ and LL_LENGTH_RSP"
 *
 * usage limitation:
 * 1. should be in range of 27 ~ 251
 */
#define ACL_CONN_MAX_TX_OCTETS			251	//user set value


/**
 * @brief	MTU MTU size
 * refer to BLE Core Specification: Vol 3, Part F, "3.2.8 Exchanging MTU size" & "3.4.2 MTU exchange"; Vol 3, Part G, "4.3.1 Exchange MTU"
 * usage limitation:
 * 1. must equal to or greater than 23(default MTU)
 * 2. if support LE Secure Connections, must equal to or bigger than 65
 * 3. should be in range of 23 ~ 247
 */
#define MTU_SIZE_SETTING					247  //user set value, user can change it in app_config.h


///////////////////////// UI Configuration ////////////////////////////////////////////////////
#define	UI_KEYBOARD_ENABLE								1
#define	UI_LED_ENABLE									1
#define	UI_BUTTON_ENABLE								0

#if (UI_KEYBOARD_ENABLE)
	#define			CR_VOL_UP				0xf0
	#define			CR_VOL_DN				0xf1

	/**
	 *  @brief  Normal keyboard map
	 */
	#define		KB_MAP_NORMAL	{	{CR_VOL_DN,		VK_1},	 \
									{CR_VOL_UP,		VK_2}, }

	#define		KB_MAP_NUM		KB_MAP_NORMAL
	#define		KB_MAP_FN		KB_MAP_NORMAL
#endif


///////////////////////// System Clock  Configuration /////////////////////////////////////////
#define CLOCK_SYS_CLOCK_HZ  								16000000



/////////////////////////////////////// PRINT DEBUG INFO ///////////////////////////////////////
#if (UART_PRINT_DEBUG_ENABLE)
	#define DEBUG_INFO_TX_PIN           	GPIO_PB1
	#define PULL_WAKEUP_SRC_PB1         	PM_PIN_PULLUP_10K
	#define PB1_OUTPUT_ENABLE         		1
	#define PB1_DATA_OUT                    1
#endif

#include "../common/default_config.h"


#endif  //end of (FEATURE_TEST_MODE == ...)

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

#if (FEATURE_TEST_MODE == TEST_LL_DATA_LENGTH_EXTENSION_MASTER)

///////////////////////// Feature Configuration////////////////////////////////////////////////
#define BLE_HOST_SMP_ENABLE				                1 //Master SMP strongly recommended enabled

///////////////////////// MASTER DLE CONFIGURATION ////////////////////////////
/* DLE & MTU:
 * connMaxRxOctets & connMaxTxOctets & MTU size */
#define ACL_CONN_MAX_RX_OCTETS							251
#define ACL_CONN_MAX_TX_OCTETS							251
#define MTU_SIZE_SETTING								247	//MTU size >= DLE - 4






///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE								0
#define UART_PRINT_DEBUG_ENABLE							1

#define APP_LOG_EN										1
#define APP_FLASH_INIT_LOG_EN							1

/////////////////////// Feature Test Board Select Configuration ///////////////////////////////
#if (__PROJECT_8258_FEATURE_TEST__)
	#define BOARD_SELECT							BOARD_825X_EVK_C1T139A30
#elif (__PROJECT_8278_FEATURE_TEST__)
	#define BOARD_SELECT							BOARD_827X_EVK_C1T197A30
#elif (__PROJECT_TC321X_FEATURE_TEST__)
	#define BOARD_SELECT							BOARD_TC321X_EVK_C1T357A20
#endif


///////////////////////// UI Configuration ////////////////////////////////////////////////////
#define	UI_KEYBOARD_ENABLE								1
#define	UI_LED_ENABLE									1



#if (UI_KEYBOARD_ENABLE)
	#define KEY_SW2						0x01
	#define KEY_SW3						0x02

	#define	KEY_SW4							0x03  ////
	#define	KEY_SW5							0x04


	/**
	 *  @brief  Normal keyboard map
	 */
	#define		KB_MAP_NORMAL	{	{KEY_SW2,	KEY_SW3} ,	 \
									{KEY_SW4,	KEY_SW5},     }
#endif



///////////////////////// System Clock  Configuration /////////////////////////////////////////
#define CLOCK_SYS_CLOCK_HZ  								32000000



/////////////////////////////////////// PRINT DEBUG INFO ///////////////////////////////////////
#if (UART_PRINT_DEBUG_ENABLE)
	#define DEBUG_INFO_TX_PIN           	GPIO_PB1
	#define PULL_WAKEUP_SRC_PB1         	PM_PIN_PULLUP_10K
	#define PB1_OUTPUT_ENABLE         		1
	#define PB1_DATA_OUT                    1
#endif

#include "../common/default_config.h"


#endif  //end of (FEATURE_TEST_MODE == ...)

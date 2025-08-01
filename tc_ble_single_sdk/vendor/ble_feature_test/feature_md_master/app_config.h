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

#if (FEATURE_TEST_MODE == TEST_MD_MASTER)


#include  "application/audio/audio_common.h"

//need define att handle same with slave(Here: we use feature_md_slave as slave device)
#define			SPP_HANDLE_DATA_S2C			0x11
#define			SPP_HANDLE_DATA_C2S			0x15


///////////////////////// Feature Configuration////////////////////////////////////////////////
#define BLE_HOST_SMP_ENABLE				            1 //Master SMP strongly recommended enabled


///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE							0

#define UART_PRINT_DEBUG_ENABLE               		1   //GPIO simulate UART print


#define APP_LOG_EN									1
#define APP_FLASH_INIT_LOG_EN						1

/////////////////////// Feature Test Board Select Configuration ///////////////////////////////
#if (__PROJECT_8258_FEATURE_TEST__)
	#define BOARD_SELECT							BOARD_825X_EVK_C1T139A30
#elif (__PROJECT_8278_FEATURE_TEST__)
	#define BOARD_SELECT							BOARD_827X_EVK_C1T197A30
#elif (__PROJECT_TC321X_FEATURE_TEST__)
	#define BOARD_SELECT							BOARD_TC321X_EVK_C1T357A20
#endif


///////////////////////// UI Configuration ////////////////////////////////////////////////////
#define UI_KEYBOARD_ENABLE							1
#define UI_LED_ENABLE								1



#if (UI_KEYBOARD_ENABLE)
	#define BTN_PAIR						0x01
	#define BTN_UNPAIR						0x02

	#define	BTN_1							0x03  ////
	#define	BTN_2							0x04


	/**
	 *  @brief  Normal keyboard map
	 */
	#define		KB_MAP_NORMAL	{	{BTN_1,	BTN_2} ,	 \
									{BTN_PAIR,	BTN_UNPAIR},     }
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

/////////////////// set default   ////////////////

#include "vendor/common/default_config.h"


#endif  //end of (FEATURE_TEST_MODE == ...)

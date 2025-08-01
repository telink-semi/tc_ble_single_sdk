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

#if (FEATURE_TEST_MODE == TEST_SMP_SECURITY)

///////////////////////// Feature Configuration////////////////////////////////////////////////
#define BLE_APP_PM_ENABLE								0
#define PM_DEEPSLEEP_RETENTION_ENABLE				0

#define APP_DEFAULT_HID_BATTERY_OTA_ATTRIBUTE_TABLE		1

///////////////////////// SMP SECURITY CONFIGURATION ////////////////////////////
// LE_Security_Mode_1_Level_1, no authentication and no encryption
#define 	SMP_TEST_NO_SECURITY				1


// LE_Security_Mode_1_Level_2, unauthenticated pairing with encryption
#define 	SMP_TEST_LEGACY_PAIRING_JUST_WORKS	2 //JustWorks
#define 	SMP_TEST_SC_PAIRING_JUST_WORKS		3 //JustWorks

// LE_Security_Mode_1_Level_3, authenticated pairing with encryption
#define 	SMP_TEST_LEGACY_PASSKEY_ENTRY_SDMI	4 //PK_Resp_Dsply_Init_Input
#define 	SMP_TEST_LEGACY_PASSKEY_ENTRY_MDSI	5 //PK_Init_Dsply_Resp_Input
#define 	SMP_TEST_LEGACY_PASSKEY_ENTRY_MISI	6 //PK_BOTH_INPUT, not test
#define 	SMP_TEST_LEGACY_PASSKEY_ENTRY_OOB	7 //OOB_Authentication

// LE_Security_Mode_1_Level_4, authenticated pairing with encryption
#define 	SMP_TEST_SC_NUMERIC_COMPARISON		8 //Numric_Comparison
#define 	SMP_TEST_SC_PASSKEY_ENTRY_SDMI		9 //PK_Resp_Dsply_Init_Input
#define 	SMP_TEST_SC_PASSKEY_ENTRY_MDSI		10//PK_Init_Dsply_Resp_Input
#define 	SMP_TEST_SC_PASSKEY_ENTRY_MISI		11//PK_BOTH_INPUT, not test
#define 	SMP_TEST_SC_PASSKEY_ENTRY_OOB		12//OOB_Authentication, not test




#define     SMP_TEST_MODE						SMP_TEST_LEGACY_PAIRING_JUST_WORKS


///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE								0
#define UART_PRINT_DEBUG_ENABLE							1
#define APP_FLASH_INIT_LOG_EN							0
#define APP_LOG_EN										1
#define APP_KEYBOARD_LOG_EN								1

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
#define UI_LED_ENABLE                                   0



///////////////////////// System Clock  Configuration /////////////////////////////////////////
#define CLOCK_SYS_CLOCK_HZ  								16000000



/////////////////////////////////////// PRINT DEBUG INFO ///////////////////////////////////////
#if (UART_PRINT_DEBUG_ENABLE)
		#define DEBUG_INFO_TX_PIN           					GPIO_PB3
		#define PULL_WAKEUP_SRC_PB3         					PM_PIN_PULLUP_10K
		#define PB3_OUTPUT_ENABLE         						1
        #define PB3_DATA_OUT                                    1 //must
#endif

#include "vendor/common/default_config.h"


#endif  //end of (FEATURE_TEST_MODE == ...)

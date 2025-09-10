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


///////////////////////// Feature Configuration////////////////////////////////////////////////
#define BLE_APP_PM_ENABLE								1
#define PM_DEEPSLEEP_RETENTION_ENABLE					1
#define TEST_CONN_CURRENT_ENABLE            			0 	//test connection current, disable UI to have a pure power
#define BLE_APP_SECURITY_ENABLE      					1	//ACL Slave device SMP, strongly recommended enabled
#define BLE_OTA_SERVER_ENABLE							0

/**
 *  @brief  2.4g Feature select
 */
#define NO_TEST_2P4G                                0x00

#define GEN_FSK_RX                                  0x10
#define GEN_FSK_SRX                                 0x11
#define GEN_FSK_SRX2TX                              0x12
#define GEN_FSK_STX                                 0x13
#define GEN_FSK_STX2RX                              0x14
#define GEN_FSK_TX                                  0x15

#define TPLL_PRX                                    0x20
#define TPLL_PTX                                    0x21

#define TPSLL_SRX                                   0x30
#define TPSLL_SRX2TX                                0x31
#define TPSLL_STX                                   0x32
#define TPSLL_STX2RX                                0x33

#define TL_TPLL_PRX                                 0x40
#define TL_TPLL_PTX                                 0x41

#define TEST_2P4G_MODE                              TPSLL_STX2RX

/* Flash Protection:
 * 1. Flash protection is enabled by default in SDK. User must enable this function on their final mass production application.
 * 2. User should use "Unlock" command in Telink BDT tool for Flash access during development and debugging phase.
 * 3. Flash protection demonstration in SDK is a reference design based on sample code. Considering that user's final application may
 *    different from sample code, for example, user's final firmware size is bigger, or user have a different OTA design, or user need
 *    store more data in some other area of Flash, all these differences imply that Flash protection reference design in SDK can not
 *    be directly used on user's mass production application without any change. User should refer to sample code, understand the
 *    principles and methods, then change and implement a more appropriate mechanism according to their application if needed.
 */
#define APP_FLASH_PROTECTION_ENABLE						1

/* User must check battery voltage on mass production application to prevent abnormal writing or erasing Flash at a low voltage !!! */
#define APP_BATT_CHECK_ENABLE							0

///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE					0
#define UART_PRINT_DEBUG_ENABLE				1
#define APP_LOG_EN							1
#define APP_SMP_LOG_EN						0
#define APP_KEY_LOG_EN						1
#define APP_CONTR_EVENT_LOG_EN				1  //controller event log
#define APP_HOST_EVENT_LOG_EN				1  //host event log
#define APP_OTA_LOG_EN						1
#define APP_FLASH_INIT_LOG_EN				1
#define APP_FLASH_PROT_LOG_EN				1
#define APP_BATT_CHECK_LOG_EN				1

/////////////////////// Sample Board Select Configuration ///////////////////////////////
#if (__PROJECT_8258_BLE_SLAVE_2P4G_SWITCH__)
	//Only support BOARD_825X_EVK_C1T139A30 & BOARD_825X_DONGLE_C1T139A3
	#define BOARD_SELECT							BOARD_825X_EVK_C1T139A30
#elif (__PROJECT_8278_BLE_SLAVE_2P4G_SWITCH__)
	//Only support BOARD_827X_EVK_C1T197A30 & BOARD_827X_DONGLE_C1T201A3
	#define BOARD_SELECT							BOARD_827X_EVK_C1T197A30
#elif (__PROJECT_TC321X_BLE_SLAVE_2P4G_SWITCH__)
	//Only support BOARD_TC321X_EVK_C1T357A20
	#define BOARD_SELECT							BOARD_TC321X_EVK_C1T357A20
#endif



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

#if (__PROJECT_TC321X_BLE_SLAVE_2P4G_SWITCH__)
    //////////////////// KEY CONFIG (EVK board) ///////////////////////////
    #define  KB_DRIVE_PINS  {GPIO_PC6, GPIO_PC7}
    #define  KB_SCAN_PINS   {GPIO_PC4, GPIO_PC5}

    //drive pin as gpio
    #define PC6_FUNC                AS_GPIO
    #define PC7_FUNC                AS_GPIO

    //drive pin need 100K pulldown
    #define PULL_WAKEUP_SRC_PC6     MATRIX_ROW_PULL
    #define PULL_WAKEUP_SRC_PC7     MATRIX_ROW_PULL

    //drive pin open input to read gpio wakeup level
    #define PC6_INPUT_ENABLE        1
    #define PC7_INPUT_ENABLE        1

    //scan pin as gpio
    #define PC4_FUNC                AS_GPIO
    #define PC5_FUNC                AS_GPIO

    //scan  pin need 10K pullup
    #define PULL_WAKEUP_SRC_PC4     MATRIX_COL_PULL
    #define PULL_WAKEUP_SRC_PC5     MATRIX_COL_PULL

    //scan pin open input to read gpio level
    #define PC4_INPUT_ENABLE        1
    #define PC5_INPUT_ENABLE        1
#endif
	#define		KB_MAP_NUM		KB_MAP_NORMAL
	#define		KB_MAP_FN		KB_MAP_NORMAL
#endif

/////////////////// DEEP SAVE FLG //////////////////////////////////
#if (__PROJECT_TC321X_BLE_SLAVE_2P4G_SWITCH__)
	#define USED_DEEP_ANA_REG				PM_ANA_REG_WD_CLR_BUF1
#else
	#define USED_DEEP_ANA_REG               DEEP_ANA_REG0 //u8,can save 8 bit info when deep
#endif
#define	LOW_BATT_FLG					    BIT(0) //if 1: low battery
#define CONN_DEEP_FLG	                    BIT(4) //if 1: conn deep, 0: ADV deep
#define RF_WORKING_MODE                     BIT(5)


///////////////////////// System Clock  Configuration /////////////////////////////////////////
#define CLOCK_SYS_CLOCK_HZ  								16000000


/////////////////// watchdog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE		0
#define WATCHDOG_INIT_TIMEOUT		500  //ms


/////////////////////////////////////// PRINT DEBUG INFO ///////////////////////////////////////
#if (UART_PRINT_DEBUG_ENABLE)
	#define DEBUG_INFO_TX_PIN           	GPIO_PB1
	#define PULL_WAKEUP_SRC_PB1         	PM_PIN_PULLUP_10K
	#define PB1_OUTPUT_ENABLE         		1
	#define PB1_DATA_OUT                    1
#endif

#if TEST_CONN_CURRENT_ENABLE
	#if DEBUG_GPIO_ENABLE || UART_PRINT_DEBUG_ENABLE || UI_KEYBOARD_ENABLE || UI_LED_ENABLE || UI_BUTTON_ENABLE
		#error "If testing current, the above definitions must be disable!!!"
	#endif
#endif


#include "vendor/common/default_config.h"

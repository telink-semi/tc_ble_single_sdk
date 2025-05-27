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


#include  "application/audio/audio_common.h"

/////////////////// FEATURE SELECT /////////////////////////////////
#define BLE_APP_PM_ENABLE				1
#define PM_DEEPSLEEP_RETENTION_ENABLE		1
#define BLE_REMOTE_SECURITY_ENABLE      	1
#define BLE_REMOTE_OTA_ENABLE				1
#define REMOTE_IR_ENABLE					0
#define REMOTE_IR_LEARN_ENABLE				0
#define BLE_AUDIO_ENABLE					1
#define UI_LED_ENABLE						1
#define UI_KEYBOARD_ENABLE						1	//must enable
#define BLT_TEST_SOFT_TIMER_ENABLE			0

#define UNUSED_GPIO_PULLDOWN_ENABLE			0
#define FIRMWARE_CHECK_ENABLE				0   //flash firmware_check
#define FIRMWARES_SIGNATURE_ENABLE          0   //firmware check
#define AUDIO_TRANS_USE_2M_PHY_ENABLE		0

/* User must check battery voltage on mass production application to prevent abnormal writing or erasing Flash at a low voltage !!! */
#define APP_BATT_CHECK_ENABLE       		1

/* Flash Protection:
 * 1. Flash protection is enabled by default in SDK. User must enable this function on their final mass production application.
 * 2. User should use "Unlock" command in Telink BDT tool for Flash access during development and debugging phase.
 * 3. Flash protection demonstration in SDK is a reference design based on sample code. Considering that user's final application may
 *    different from sample code, for example, user's final firmware size is bigger, or user have a different OTA design, or user need
 *    store more data in some other area of Flash, all these differences imply that Flash protection reference design in SDK can not
 *    be directly used on user's mass production application without any change. User should refer to sample code, understand the
 *    principles and methods, then change and implement a more appropriate mechanism according to their application if needed.
 */
#define APP_FLASH_PROTECTION_ENABLE			1

#if (BLT_TEST_SOFT_TIMER_ENABLE)
	#define BLT_SOFTWARE_TIMER_ENABLE		1
#endif


/////////////////// DEBUG Configuration /////////////////////////////////
#define UART_PRINT_DEBUG_ENABLE             1   //GPIO simulate uart print func

#define APP_LOG_EN							1
#define APP_SMP_LOG_EN						0
#define APP_KEY_LOG_EN						1
#define APP_CONTR_EVENT_LOG_EN				1  //controller event log
#define APP_HOST_EVENT_LOG_EN				1  //host event log
#define APP_OTA_LOG_EN						1
#define APP_FLASH_INIT_LOG_EN				1
#define APP_FLASH_PROT_LOG_EN				1
#define APP_BATT_CHECK_LOG_EN				1

/////////////////////// RCU Board Select Configuration ///////////////////////////////
#if (__PROJECT_8258_BLE_REMOTE__)
	/* can only choose BOARD_825X_RCU_C1T139A5*/
	#define BOARD_SELECT							BOARD_825X_RCU_C1T139A5
#elif (__PROJECT_8278_BLE_REMOTE__)
	/* can only choose BOARD_827X_RCU_C1T197A5*/
	#define BOARD_SELECT							BOARD_827X_RCU_C1T197A5
#endif

/////////////////// DEEP SAVE FLG //////////////////////////////////
#define USED_DEEP_ANA_REG                   DEEP_ANA_REG0 //u8,can save 8 bit info when deep
#define	LOW_BATT_FLG					    BIT(0) //if 1: low battery
#define CONN_DEEP_FLG	                    BIT(4) //if 1: conn deep, 0: ADV deep
#define IR_MODE_DEEP_FLG	 				BIT(2) //if 1: IR mode, 0: BLE mode
#define LOW_BATT_SUSPEND_FLG				BIT(3) //if 1 : low battery, < 1.8v

/**
 *  @brief  LED Configuration
 */
#if (UI_LED_ENABLE)
	#define BLT_APP_LED_ENABLE 1
#endif


/**
 *  @brief  AUDIO Configuration
 */
#if (BLE_AUDIO_ENABLE)

	#define BLE_DMIC_ENABLE					0  //0: Amic   1: Dmic
	#define IIR_FILTER_ENABLE				0

	/* RCU Audio MODE:
	 * TL_AUDIO_RCU_ADPCM_GATT_TLEINK
	 * TL_AUDIO_RCU_ADPCM_GATT_GOOGLE
	 * TL_AUDIO_RCU_ADPCM_HID
	 * TL_AUDIO_RCU_SBC_HID
	 * TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB
	 * TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB
	 * TL_AUDIO_RCU_MSBC_HID
	 */
	#define TL_AUDIO_MODE  						TL_AUDIO_RCU_ADPCM_GATT_TLEINK

#endif

//////////////////////////// KEYSCAN/MIC  GPIO //////////////////////////////////
#define DEEPBACK_FAST_KEYSCAN_ENABLE	0   //proc fast scan when deepsleep back triggered by key press, in case key loss
#define LONG_PRESS_KEY_POWER_OPTIMIZE	1   //lower power when pressing key without release

//stuck key
#define STUCK_KEY_PROCESS_ENABLE		0
#define STUCK_KEY_ENTERDEEP_TIME		60  //in s

//repeat key
#define KB_REPEAT_KEY_ENABLE			0
#define	KB_REPEAT_KEY_INTERVAL_MS		200
#define KB_REPEAT_KEY_NUM				1
#define KB_MAP_REPEAT					{VK_1, }

//////////////////// key type ///////////////////////
#define IDLE_KEY	   			0
#define CONSUMER_KEY   	   		1
#define KEYBOARD_KEY   	   		2
#define IR_KEY   	   			3

#define			CR_VOL_UP				0xf0  ////
#define			CR_VOL_DN				0xf1
#define			CR_VOL_MUTE				0xf2
#define			CR_CHN_UP				0xf3
#define			CR_CHN_DN				0xf4  ////
#define			CR_POWER				0xf5
#define			CR_SEARCH				0xf6
#define			CR_RECORD				0xf7
#define			CR_PLAY					0xf8  ////
#define			CR_PAUSE				0xf9
#define			CR_STOP					0xfa
#define			CR_FAST_BACKWARD		0xfb
#define			CR_FAST_FORWARD			0xfc  ////
#define			CR_HOME					0xfd
#define			CR_BACK					0xfe
#define			CR_MENU					0xff

//special key
#define		 	VOICE					0xc0
#define 		KEY_MODE_SWITCH			0xc1
#define		 	PHY_TEST				0xc2


#define 		IR_VK_0			0x00
#define 		IR_VK_1			0x01
#define 		IR_VK_2			0x02
#define			IR_VK_3			0x03
#define			IR_VK_4			0x04
#define 		IR_VK_5			0x05
#define 		IR_VK_6			0x06
#define 		IR_VK_7			0x07
#define 		IR_VK_8			0x08
#define 		IR_VK_9			0x09

#define 		IR_POWER		0x12
#define			IR_AUDIO_MUTE	0x0d
#define 		IR_NETFLIX		0x0f
#define			IR_BACK			0x0e
#define			IR_VOL_UP		0x0b
#define			IR_VOL_DN		0x0c
#define 		IR_NEXT			0x20
#define 		IR_PREV			0x21
#define			IR_MENU			0x23
#define 		IR_HOME			0x24
#define 		IR_OPER_KEY		0x2e
#define 		IR_INFO			0x2f
#define			IR_REWIND		0x32
#define 		IR_FAST_FORWARD	0x34
#define 		IR_PLAY_PAUSE	0x35
#define			IR_GUIDE		0x41
#define 		IR_UP			0x45
#define			IR_DN			0x44
#define 		IR_LEFT			0x42
#define 		IR_RIGHT		0x43
#define			IR_SEL			0x46
#define 		IR_RED_KEY		0x6b
#define 		IR_GREEN_KEY	0x6c
#define 		IR_YELLOW_KEY	0x6d
#define 		IR_BLUE_KEY		0x6e
#define 		IR_RECORD		0x72
#define 		IR_OPTION		0x73
#define 		IR_STOP			0x74
#define 		IR_SEARCH		0x75
#define 		IR_TEXT			0x76
#define 		IR_VOICE		0x77
#define 		IR_PAUSE		0x78

#define			T_VK_CH_UP		0xd0
#define			T_VK_CH_DN		0xd1


#if (REMOTE_IR_ENABLE)  //with IR keymap
//	#define 		GPIO_IR_CONTROL			GPIO_PD0

	#define		KB_MAP_NORMAL	{\
					{0, 	    1,				2,			3,		4,		}, \
					{VOICE,	 	KEY_MODE_SWITCH,7,			8,		9		}, \
					{10,		11,				12,			13,		14,		}, \
					{15,		16,				17,			18,		19,		}, \
					{20,		21,				22,			23,		24,		}, \
					{25,		26,				27,			28,		29,		}, }


	#define		KB_MAP_BLE	{\
					VK_NONE,	VK_NONE,		VK_NONE,		VK_NONE,			VK_NONE,	 \
					VOICE,		VK_NONE,		VK_NONE,		CR_VOL_UP,			CR_VOL_DN,	 \
					VK_2,		VK_NONE,		VK_NONE,		VK_3,				VK_1,	 \
					VK_5,		VK_NONE,		VK_NONE,		VK_6,				VK_4,	 \
					VK_8,		VK_NONE,		VK_NONE,		VK_9,				VK_7,	 \
					VK_0,		VK_NONE,		VK_NONE,		VK_NONE,			VK_NONE,	 }


	#define		KB_MAP_IR	{\
					VK_NONE,	VK_NONE,		VK_NONE,	VK_NONE,			VK_NONE,	 \
					VK_NONE,	VK_NONE,		VK_NONE,	VK_NONE,			VK_NONE,	 \
					IR_VK_2,	VK_NONE,		VK_NONE,	IR_VK_3,			IR_VK_1, 	 \
					IR_VK_5,	VK_NONE,		VK_NONE,	IR_VK_6,			IR_VK_4,	 \
					IR_VK_8,	VK_NONE,		VK_NONE,	IR_VK_9,			IR_VK_7,	 \
					IR_VK_0,	VK_NONE,		VK_NONE,	VK_NONE,			VK_NONE,	 }

#else   //key map

	#define		KB_MAP_NORMAL	{\
					{VK_B,		CR_POWER,		VK_NONE,		VK_C,				CR_HOME},	 \
					{VOICE,		VK_NONE,		VK_NONE,		CR_VOL_UP,			CR_VOL_DN},	 \
					{VK_2,		VK_RIGHT,		CR_VOL_DN,		VK_3,				VK_1},	 \
					{VK_5,		VK_ENTER,		CR_VOL_UP,		VK_6,				VK_4},	 \
					{VK_8,		VK_DOWN,		VK_UP ,			VK_9,				VK_7},	 \
					{VK_0,		CR_BACK,		VK_LEFT,		VOICE,				CR_MENU},	 }

#endif  //end of REMOTE_IR_ENABLE


#define		KB_MAP_NUM		KB_MAP_NORMAL
#define		KB_MAP_FN		KB_MAP_NORMAL


///////////////////////// System Clock  Configuration /////////////////////////////////////////
#if (TL_AUDIO_MODE & (TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_MSBC_MODE))
	#define CLOCK_SYS_CLOCK_HZ  								32000000
	#else
	#define CLOCK_SYS_CLOCK_HZ  								16000000
#endif


/////////////////// watchdog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE		0
#define WATCHDOG_INIT_TIMEOUT		500  //ms

#define BLE_PHYTEST_MODE						PHYTEST_MODE_DISABLE


/////////////////////////////////////// PRINT DEBUG INFO ///////////////////////////////////////
#if (UART_PRINT_DEBUG_ENABLE)
	#define DEBUG_INFO_TX_PIN           					GPIO_PB5
	#define PULL_WAKEUP_SRC_PB5         					PM_PIN_PULLUP_10K
	#define PB5_OUTPUT_ENABLE         						1
	#define PB5_DATA_OUT                                    1 //must
#endif


#include "vendor/common/default_config.h"

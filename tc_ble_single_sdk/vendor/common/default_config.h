/********************************************************************************************************
 * @file    default_config.h
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

#include "config.h"
#include "boards/boards_config.h"

//////////// product  Information  //////////////////////////////
#ifndef ID_VENDOR
#define ID_VENDOR			0x248a			// for report 
#endif
#ifndef ID_PRODUCT_BASE
#define ID_PRODUCT_BASE		0x8800
#endif
#ifndef STRING_VENDOR
#define STRING_VENDOR		L"Telink"
#endif
#ifndef STRING_PRODUCT
#define STRING_PRODUCT		L"2.4G Wireless Audio"
#endif
#if (MCU_CORE_TYPE == MCU_CORE_827x)
	#ifndef STRING_SERIAL
	#define STRING_SERIAL		L"TLSR8278"
	#endif
#else
	#ifndef STRING_SERIAL
	#define STRING_SERIAL		L"TLSR8258"
	#endif
#endif



#ifndef MODULE_WATCHDOG_ENABLE
#define MODULE_WATCHDOG_ENABLE	0
#endif



#ifndef WATCHDOG_INIT_TIMEOUT
#define WATCHDOG_INIT_TIMEOUT			500		//  in ms
#endif


#ifndef BLE_APP_SECURITY_ENABLE
#define BLE_APP_SECURITY_ENABLE			1
#endif

#ifndef BLE_APP_PM_ENABLE
#define BLE_APP_PM_ENABLE				0
#endif


#ifndef PM_DEEPSLEEP_RETENTION_ENABLE
#define PM_DEEPSLEEP_RETENTION_ENABLE	0
#endif


#if(!BLE_APP_PM_ENABLE && PM_DEEPSLEEP_RETENTION_ENABLE)
	#error "can not use deep retention when PM disable !!!"
#endif



#if(APPLICATION_DONGLE)
	#ifndef MODULE_MOUSE_ENABLE
	#define MODULE_MOUSE_ENABLE		0
	#endif
	#ifndef MODULE_KEYBOARD_ENABLE
	#define MODULE_KEYBOARD_ENABLE	0
	#endif
	#ifndef MODULE_MIC_ENABLE
	#define MODULE_MIC_ENABLE		0
	#endif
	#ifndef MODULE_SPEAKER_ENABLE
	#define MODULE_SPEAKER_ENABLE	0			// device , not dongle
	#endif
	#ifndef MODULE_USB_ENABLE
	#define MODULE_USB_ENABLE		1
	#endif
#else
	#ifndef MODULE_MOUSE_ENABLE
	#define MODULE_MOUSE_ENABLE		1
	#endif
	#ifndef MODULE_KEYBOARD_ENABLE
	#define MODULE_KEYBOARD_ENABLE	1
	#endif

	#ifndef MODULE_MIC_ENABLE
	#define MODULE_MIC_ENABLE		0
	#endif
	#ifndef MODULE_SPEAKER_ENABLE
	#define MODULE_SPEAKER_ENABLE	0			// device , not dongle
	#endif

	#ifndef MODULE_USB_ENABLE
	#define MODULE_USB_ENABLE		0
	#endif
#endif


///////////////////  USB   /////////////////////////////////
#ifndef IRQ_USB_PWDN_ENABLE
#define	IRQ_USB_PWDN_ENABLE  	0
#endif


#ifndef USB_PRINTER_ENABLE
#define	USB_PRINTER_ENABLE 		0
#endif
#ifndef USB_SPEAKER_ENABLE
#define	USB_SPEAKER_ENABLE 		0
#endif
#ifndef USB_MIC_ENABLE
#define	USB_MIC_ENABLE 			0
#endif
#ifndef USB_MOUSE_ENABLE
#define	USB_MOUSE_ENABLE 			0
#endif
#ifndef USB_KEYBOARD_ENABLE
#define	USB_KEYBOARD_ENABLE 		0
#endif
#ifndef USB_SOMATIC_ENABLE
#define	USB_SOMATIC_ENABLE 		0
#endif
#ifndef USB_CUSTOM_HID_REPORT
#define	USB_CUSTOM_HID_REPORT 		0
#endif
#ifndef USB_AUDIO_441K_ENABLE
#define USB_AUDIO_441K_ENABLE  	0
#endif

#ifndef MIC_CHANNEL_COUNT
#define MIC_CHANNEL_COUNT  			2
#endif



#ifndef USB_KEYBOARD_POLL_INTERVAL
#define USB_KEYBOARD_POLL_INTERVAL		10		// in ms
#endif

#ifndef USB_MOUSE_POLL_INTERVAL
#define USB_MOUSE_POLL_INTERVAL			4		// in ms
#endif

#ifndef USB_SOMATIC_POLL_INTERVAL
#define USB_SOMATIC_POLL_INTERVAL     	8		// in ms
#endif

#define USB_KEYBOARD_RELEASE_TIMEOUT    (450000) // in us
#define USB_MOUSE_RELEASE_TIMEOUT       (200000) // in us

///////////////////  Board configuration   /////////////////////////////////
#if ( BOARD_SELECT == BOARD_825X_EVK_C1T139A30)
	#include "boards/C1T139A30.h"
#elif ( BOARD_SELECT == BOARD_825X_DONGLE_C1T139A3)
	#ifndef UI_BUTTON_ENABLE
		#define UI_BUTTON_ENABLE	1
	#endif
	#include "boards/C1T139A3.h"
#elif ( BOARD_SELECT == BOARD_825X_RCU_C1T139A5)
	#include "boards/C1T139A5.h"
#elif ( BOARD_SELECT == BOARD_827X_EVK_C1T197A30)
	#include "boards/C1T197A30.h"
#elif ( BOARD_SELECT == BOARD_827X_DONGLE_C1T201A3)
	#ifndef UI_BUTTON_ENABLE
		#define UI_BUTTON_ENABLE	1
	#endif
	#include "boards/C1T201A3.h"
#elif ( BOARD_SELECT == BOARD_827X_RCU_C1T197A5)
	#include "boards/C1T197A5.h"
#elif ( BOARD_SELECT == BOARD_TC321X_EVK_C1T357A20)
	#include "boards/C1T357A20.h"
#elif ( BOARD_SELECT == BOARD_TC321X_EVK_C1T357A20_V2_1)
    #include "boards/C1T357A20_V2_1.h"
#elif ( BOARD_SELECT == BOARD_TC321X_RCU_C1T362A5)
	#include "boards/C1T362A5.h"
#endif

#ifndef BOARD_SELECT

	#if (MCU_CORE_TYPE == MCU_CORE_825x)
	/* can only choose BOARD_825X_EVK_C1T139A30 or BOARD_825X_DONGLE_C1T139A3 or BOARD_825X_RCU_C1T139A5,
	 * default use EVK, user can change */
		#define BOARD_SELECT								BOARD_825X_EVK_C1T139A30
	#elif (MCU_CORE_TYPE == MCU_CORE_827x)
	/* can only choose BOARD_827X_EVK_C1T197A30 or BOARD_827X_DONGLE_C1T201A3 or BOARD_827X_RCU_C1T197A5,
	 * default use EVK, user can change */
		#define BOARD_SELECT								BOARD_827X_EVK_C1T197A30
	#elif (MCU_CORE_TYPE == MCU_CORE_TC321X)
	/* can only choose BOARD_TC321X_EVK_C1T357A20 or BOARD_TC321X_RCU_C1T362A5 or BOARD_TC321X_EVK_C1T357A20_V2_1,
	 * default use EVK, user can change */
		#define BOARD_SELECT								BOARD_TC321X_EVK_C1T357A20_V2_1
	#else
		#error "SDK do not support this MCU!"
	#endif
#endif

///////////////////  UI configuration   /////////////////////////////////
#ifndef UI_KEYBOARD_ENABLE
#define UI_KEYBOARD_ENABLE		0
#endif

#ifndef UI_BUTTON_ENABLE
#define UI_BUTTON_ENABLE		0
#endif

#ifndef UI_LED_ENABLE
#define UI_LED_ENABLE			0
#endif

///////////////////  DEBUG configuration   /////////////////////////////////
#ifndef DEBUG_GPIO_ENABLE
#define DEBUG_GPIO_ENABLE  								0
#endif

#if (DEBUG_GPIO_ENABLE)
	#ifdef GPIO_CHN0
		#define DBG_CHN0_LOW		gpio_write(GPIO_CHN0, 0)
		#define DBG_CHN0_HIGH		gpio_write(GPIO_CHN0, 1)
		#define DBG_CHN0_TOGGLE		gpio_toggle(GPIO_CHN0)
	#else
		#define DBG_CHN0_LOW
		#define DBG_CHN0_HIGH
		#define DBG_CHN0_TOGGLE
	#endif

	#ifdef  GPIO_CHN1
		#define DBG_CHN1_LOW		gpio_write(GPIO_CHN1, 0)
		#define DBG_CHN1_HIGH		gpio_write(GPIO_CHN1, 1)
		#define DBG_CHN1_TOGGLE		gpio_toggle(GPIO_CHN1)
	#else
		#define DBG_CHN1_LOW
		#define DBG_CHN1_HIGH
		#define DBG_CHN1_TOGGLE
	#endif

	#ifdef  GPIO_CHN2
		#define DBG_CHN2_LOW		gpio_write(GPIO_CHN2, 0)
		#define DBG_CHN2_HIGH		gpio_write(GPIO_CHN2, 1)
		#define DBG_CHN2_TOGGLE		gpio_toggle(GPIO_CHN2)
	#else
		#define DBG_CHN2_LOW
		#define DBG_CHN2_HIGH
		#define DBG_CHN2_TOGGLE
	#endif

	#ifdef  GPIO_CHN3
		#define DBG_CHN3_LOW		gpio_write(GPIO_CHN3, 0)
		#define DBG_CHN3_HIGH		gpio_write(GPIO_CHN3, 1)
		#define DBG_CHN3_TOGGLE		gpio_toggle(GPIO_CHN3)
	#else
		#define DBG_CHN3_LOW
		#define DBG_CHN3_HIGH
		#define DBG_CHN3_TOGGLE
	#endif

	#ifdef GPIO_CHN4
		#define DBG_CHN4_LOW		gpio_write(GPIO_CHN4, 0)
		#define DBG_CHN4_HIGH		gpio_write(GPIO_CHN4, 1)
		#define DBG_CHN4_TOGGLE		gpio_toggle(GPIO_CHN4)
	#else
		#define DBG_CHN4_LOW
		#define DBG_CHN4_HIGH
		#define DBG_CHN4_TOGGLE
	#endif

	#ifdef  GPIO_CHN5
		#define DBG_CHN5_LOW		gpio_write(GPIO_CHN5, 0)
		#define DBG_CHN5_HIGH		gpio_write(GPIO_CHN5, 1)
		#define DBG_CHN5_TOGGLE		gpio_toggle(GPIO_CHN5)
	#else
		#define DBG_CHN5_LOW
		#define DBG_CHN5_HIGH
		#define DBG_CHN5_TOGGLE
	#endif

	#ifdef  GPIO_CHN6
		#define DBG_CHN6_LOW		gpio_write(GPIO_CHN6, 0)
		#define DBG_CHN6_HIGH		gpio_write(GPIO_CHN6, 1)
		#define DBG_CHN6_TOGGLE		gpio_toggle(GPIO_CHN6)
	#else
		#define DBG_CHN6_LOW
		#define DBG_CHN6_HIGH
		#define DBG_CHN6_TOGGLE
	#endif

	#ifdef  GPIO_CHN7
		#define DBG_CHN7_LOW		gpio_write(GPIO_CHN7, 0)
		#define DBG_CHN7_HIGH		gpio_write(GPIO_CHN7, 1)
		#define DBG_CHN7_TOGGLE		gpio_toggle(GPIO_CHN7)
	#else
		#define DBG_CHN7_LOW
		#define DBG_CHN7_HIGH
		#define DBG_CHN7_TOGGLE
	#endif

	#ifdef GPIO_CHN8
		#define DBG_CHN8_LOW		gpio_write(GPIO_CHN8, 0)
		#define DBG_CHN8_HIGH		gpio_write(GPIO_CHN8, 1)
		#define DBG_CHN8_TOGGLE		gpio_toggle(GPIO_CHN8)
	#else
		#define DBG_CHN8_LOW
		#define DBG_CHN8_HIGH
		#define DBG_CHN8_TOGGLE
	#endif

	#ifdef  GPIO_CHN9
		#define DBG_CHN9_LOW		gpio_write(GPIO_CHN9, 0)
		#define DBG_CHN9_HIGH		gpio_write(GPIO_CHN9, 1)
		#define DBG_CHN9_TOGGLE		gpio_toggle(GPIO_CHN9)
	#else
		#define DBG_CHN9_LOW
		#define DBG_CHN9_HIGH
		#define DBG_CHN9_TOGGLE
	#endif

	#ifdef  GPIO_CHN10
		#define DBG_CHN10_LOW		gpio_write(GPIO_CHN10, 0)
		#define DBG_CHN10_HIGH		gpio_write(GPIO_CHN10, 1)
		#define DBG_CHN10_TOGGLE	gpio_toggle(GPIO_CHN10)
	#else
		#define DBG_CHN10_LOW
		#define DBG_CHN10_HIGH
		#define DBG_CHN10_TOGGLE
	#endif

	#ifdef  GPIO_CHN11
		#define DBG_CHN11_LOW		gpio_write(GPIO_CHN11, 0)
		#define DBG_CHN11_HIGH		gpio_write(GPIO_CHN11, 1)
		#define DBG_CHN11_TOGGLE	gpio_toggle(GPIO_CHN11)
	#else
		#define DBG_CHN11_LOW
		#define DBG_CHN11_HIGH
		#define DBG_CHN11_TOGGLE
	#endif

	#ifdef GPIO_CHN12
		#define DBG_CHN12_LOW		gpio_write(GPIO_CHN12, 0)
		#define DBG_CHN12_HIGH		gpio_write(GPIO_CHN12, 1)
		#define DBG_CHN12_TOGGLE	gpio_toggle(GPIO_CHN12)
	#else
		#define DBG_CHN12_LOW
		#define DBG_CHN12_HIGH
		#define DBG_CHN12_TOGGLE
	#endif

	#ifdef  GPIO_CHN13
		#define DBG_CHN13_LOW		gpio_write(GPIO_CHN13, 0)
		#define DBG_CHN13_HIGH		gpio_write(GPIO_CHN13, 1)
		#define DBG_CHN13_TOGGLE	gpio_toggle(GPIO_CHN13)
	#else
		#define DBG_CHN13_LOW
		#define DBG_CHN13_HIGH
		#define DBG_CHN13_TOGGLE
	#endif

	#ifdef  GPIO_CHN14
		#define DBG_CHN14_LOW		gpio_write(GPIO_CHN14, 0)
		#define DBG_CHN14_HIGH		gpio_write(GPIO_CHN14, 1)
		#define DBG_CHN14_TOGGLE	gpio_toggle(GPIO_CHN14)
	#else
		#define DBG_CHN14_LOW
		#define DBG_CHN14_HIGH
		#define DBG_CHN14_TOGGLE
	#endif

	#ifdef  GPIO_CHN15
		#define DBG_CHN15_LOW		gpio_write(GPIO_CHN15, 0)
		#define DBG_CHN15_HIGH		gpio_write(GPIO_CHN15, 1)
		#define DBG_CHN15_TOGGLE	gpio_toggle(GPIO_CHN15)
	#else
		#define DBG_CHN15_LOW
		#define DBG_CHN15_HIGH
		#define DBG_CHN15_TOGGLE
	#endif
#else
	#define DBG_CHN0_LOW
	#define DBG_CHN0_HIGH
	#define DBG_CHN0_TOGGLE
	#define DBG_CHN1_LOW
	#define DBG_CHN1_HIGH
	#define DBG_CHN1_TOGGLE
	#define DBG_CHN2_LOW
	#define DBG_CHN2_HIGH
	#define DBG_CHN2_TOGGLE
	#define DBG_CHN3_LOW
	#define DBG_CHN3_HIGH
	#define DBG_CHN3_TOGGLE
	#define DBG_CHN4_LOW
	#define DBG_CHN4_HIGH
	#define DBG_CHN4_TOGGLE
	#define DBG_CHN5_LOW
	#define DBG_CHN5_HIGH
	#define DBG_CHN5_TOGGLE
	#define DBG_CHN6_LOW
	#define DBG_CHN6_HIGH
	#define DBG_CHN6_TOGGLE
	#define DBG_CHN7_LOW
	#define DBG_CHN7_HIGH
	#define DBG_CHN7_TOGGLE
	#define DBG_CHN8_LOW
	#define DBG_CHN8_HIGH
	#define DBG_CHN8_TOGGLE
	#define DBG_CHN9_LOW
	#define DBG_CHN9_HIGH
	#define DBG_CHN9_TOGGLE
	#define DBG_CHN10_LOW
	#define DBG_CHN10_HIGH
	#define DBG_CHN10_TOGGLE
	#define DBG_CHN11_LOW
	#define DBG_CHN11_HIGH
	#define DBG_CHN11_TOGGLE
	#define DBG_CHN12_LOW
	#define DBG_CHN12_HIGH
	#define DBG_CHN12_TOGGLE
	#define DBG_CHN13_LOW
	#define DBG_CHN13_HIGH
	#define DBG_CHN13_TOGGLE
	#define DBG_CHN14_LOW
	#define DBG_CHN14_HIGH
	#define DBG_CHN14_TOGGLE
	#define DBG_CHN15_LOW
	#define DBG_CHN15_HIGH
	#define DBG_CHN15_TOGGLE
#endif

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



/////////////////// MODULE Configuration/////////////////////////////////
#define BLE_APP_PM_ENABLE				1
#define PM_DEEPSLEEP_RETENTION_ENABLE		1
#define BLE_OTA_ENABLE						1
#define TELIK_SPP_SERVICE_ENABLE			1
#define BLE_MODULE_INDICATE_DATA_TO_MCU		1


/* User must check battery voltage on mass production application to prevent abnormal writing or erasing Flash at a low voltage !!! */
#define APP_BATT_CHECK_ENABLE		       	1

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

//firmware check
#define FIRMWARES_SIGNATURE_ENABLE          0

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


//////////////// SMP SETTING  //////////////////////////////
#define BLE_SECURITY_ENABLE 			   	1



/////////////////// DEEP SAVE FLG //////////////////////////////////
#if (__PROJECT_TC321X_MODULE__)
	#define USED_DEEP_ANA_REG				PM_ANA_REG_WD_CLR_BUF1
#else
	#define USED_DEEP_ANA_REG               DEEP_ANA_REG0 //u8,can save 8 bit info when deep
#endif
#define	LOW_BATT_FLG					    BIT(0)

/////////////////////// Module Board Select Configuration ///////////////////////////////
#if (__PROJECT_8258_MODULE__)
	/* can only choose BOARD_825X_EVK_C1T139A30*/
	#define BOARD_SELECT							BOARD_825X_EVK_C1T139A30
#elif (__PROJECT_8278_MODULE__)
	/* can only choose BOARD_827X_EVK_C1T197A30*/
	#define BOARD_SELECT							BOARD_827X_EVK_C1T197A30
#elif (__PROJECT_TC321X_MODULE__)
	/* can only choose BOARD_TC321X_EVK_C1T357A20*/
	#define BOARD_SELECT							BOARD_TC321X_EVK_C1T357A20
#endif



///////////////////////// UI Configuration ////////////////////////////////////////////////////

#define	UI_LED_ENABLE									1

/**
 *  @brief  Battery_check Configuration
 */

//////////////////////////// MODULE PM GPIO	(EVK board) /////////////////////////////////
#define GPIO_WAKEUP_MODULE					GPIO_PC6   //mcu wakeup module
#define	PC6_FUNC							AS_GPIO
#define PC6_INPUT_ENABLE					1
#define	PC6_OUTPUT_ENABLE					0
#define	PC6_DATA_OUT						0
#define GPIO_WAKEUP_MODULE_HIGH				gpio_setup_up_down_resistor(GPIO_WAKEUP_MODULE, PM_PIN_PULLUP_10K);
#define GPIO_WAKEUP_MODULE_LOW				gpio_setup_up_down_resistor(GPIO_WAKEUP_MODULE, PM_PIN_PULLDOWN_100K);

#define GPIO_WAKEUP_MCU						GPIO_PC7   //module wakeup mcu
#define	PC7_FUNC							AS_GPIO
#define PC7_INPUT_ENABLE					1
#define	PC7_OUTPUT_ENABLE					1
#define	PC7_DATA_OUT						0
#define GPIO_WAKEUP_MCU_HIGH				do{gpio_set_output_en(GPIO_WAKEUP_MCU, 1); gpio_write(GPIO_WAKEUP_MCU, 1);}while(0)
#define GPIO_WAKEUP_MCU_LOW					do{gpio_set_output_en(GPIO_WAKEUP_MCU, 1); gpio_write(GPIO_WAKEUP_MCU, 0);}while(0)
#define GPIO_WAKEUP_MCU_FLOAT				do{gpio_set_output_en(GPIO_WAKEUP_MCU, 0); gpio_write(GPIO_WAKEUP_MCU, 0);}while(0)




///////////////////////// System Clock  Configuration /////////////////////////////////////////
#define CLOCK_SYS_CLOCK_HZ  								16000000



/////////////////// watchdog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE				0
#define WATCHDOG_INIT_TIMEOUT				500  //ms





/////////////////////HCI UART variables///////////////////////////////////////
/*----------------------------------------------*
 *	SPP TX FIFO  = 2 Bytes LEN + n Bytes Data.	*
 *												*
 *	T_txdata_buf = 4 Bytes LEN + n Bytes Data.	*
 *												*
 *	SPP_TXFIFO_SIZE = 2 + n.					*
 *												*
 *	UART_DATA_LEN = n.							*
 *												*
 *	UART_DATA_LEN = SPP_TXFIFO_SIZE - 2.		*
 * ---------------------------------------------*/
#define SPP_RXFIFO_SIZE		72
#define SPP_RXFIFO_NUM		2

#define SPP_TXFIFO_SIZE		72
#define SPP_TXFIFO_NUM		8

#define UART_DATA_LEN    	(SPP_TXFIFO_SIZE - 2)   // data max 252
typedef struct{
    unsigned int len; // data max 252
    unsigned char data[UART_DATA_LEN];
}uart_data_t;


#if(__PROJECT_TC321X_MODULE__)
#define UART_MODULE_SEL						UART0
#define UART_CONVERT(...)					UART_MODULE_SEL,__VA_ARGS__
#define UART_TX_PIN							GPIO_PD0
#define UART_RX_PIN							GPIO_PD1
#else
#define UART_MODULE_SEL
#define UART_CONVERT(...)					__VA_ARGS__
#define UART_TX_PIN							UART_TX_PB1
#define UART_RX_PIN							UART_RX_PB0
#endif

#define UART_BAUD_RATE  					115200

/////////////////////////////////////// PRINT DEBUG INFO ///////////////////////////////////////
#if (UART_PRINT_DEBUG_ENABLE)
	#define DEBUG_INFO_TX_PIN           	GPIO_PB6
	#define PULL_WAKEUP_SRC_PB6         	PM_PIN_PULLUP_10K
	#define PB6_OUTPUT_ENABLE         		1
	#define PB6_DATA_OUT                    1
#endif

/////////////////// set default   ////////////////

#include "vendor/common/default_config.h"


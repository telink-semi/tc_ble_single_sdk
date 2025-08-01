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

#define 	BQB_MASTER_TEST										0


/////////////////////HCI ACCESS OPTIONS/////////////////////
#define HCI_USE_UART	1
#define HCI_USE_USB		0
#define HCI_ACCESS		HCI_USE_UART



/////////////////// MODULE /////////////////////////////////
#define BLE_APP_PM_ENABLE				0




///////////////////////// DEBUG  Configuration ////////////////////////////////////////////////
#define DEBUG_GPIO_ENABLE					0
#define PRINT_DEBUG_INFO               		0	//open/close myprintf
#define APP_FLASH_INIT_LOG_EN				0
#define APP_LOG_EN							1
#define UART_PRINT_DEBUG_ENABLE				1

/////////////////////// HCI Board Select Configuration ///////////////////////////////
#if (__PROJECT_8258_HCI__)
	/* can only choose BOARD_825X_EVK_C1T139A30,*/
	#define BOARD_SELECT							BOARD_825X_EVK_C1T139A30
#elif (__PROJECT_8278_HCI__)
	/* can only choose BOARD_827X_EVK_C1T197A30,*/
	#define BOARD_SELECT							BOARD_827X_EVK_C1T197A30
#elif (__PROJECT_TC321X_HCI__)
	/* can only choose BOARD_TC321X_EVK_C1T357A20,*/
	#define BOARD_SELECT							BOARD_TC321X_EVK_C1T357A20
#endif



///////////////////////// UI Configuration ////////////////////////////////////////////////////

#define	UI_LED_ENABLE									0

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
#if BQB_MASTER_TEST
	#define CLOCK_SYS_CLOCK_HZ      					32000000
#else
	#define CLOCK_SYS_CLOCK_HZ      					16000000
#endif



/////////////////// watchdog  //////////////////////////////
#define MODULE_WATCHDOG_ENABLE				0
#define WATCHDOG_INIT_TIMEOUT				500  //ms





/////////////////////HCI UART variables///////////////////////////////////////
//module spp Tx / Rx fifo
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
#define HCI_RXFIFO_SIZE		80
#define HCI_RXFIFO_NUM		4

#define HCI_TXFIFO_SIZE		80
#define HCI_TXFIFO_NUM		8

#define UART_DATA_LEN    	(HCI_TXFIFO_SIZE - 2)      // data max 252
typedef struct{
    unsigned int len; // data max 252
    unsigned char data[UART_DATA_LEN];
}uart_data_t;


#if(__PROJECT_TC321X_HCI__)
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

#define   UART__BAUDRATE        			(1000000)

/////////////////////////////////////// PRINT DEBUG INFO ///////////////////////////////////////
#if (UART_PRINT_DEBUG_ENABLE)
	#define DEBUG_INFO_TX_PIN           	GPIO_PB5
	#define PULL_WAKEUP_SRC_PB5         	PM_PIN_PULLUP_10K
	#define PB5_OUTPUT_ENABLE         		1
	#define PB5_DATA_OUT                    1
#endif

/////////////////// set default   ////////////////

#include "vendor/common/default_config.h"

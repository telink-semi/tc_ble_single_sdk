/********************************************************************************************************
 * @file    C1T139A3.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifndef C1T139A3_H_
#define C1T139A3_H_



/**
 *  @brief  Keyboard Configuration
 */
#if (UI_KEYBOARD_ENABLE)   // if test pure power, kyeScan GPIO setting all disabled
	/* 825X EVK and 827X EVK use Keyboard */
	#error "add keyboard definition by yourself !"
#endif


/**
 *  @brief  Button Configuration
 */
#if (UI_BUTTON_ENABLE)
	/* 825X Dongle and 827X Dongle use same GPIO for Button: PD5/PD6 */
	#define	SW1_GPIO				GPIO_PD5
	#define	SW2_GPIO				GPIO_PD6
	#define PD5_FUNC				AS_GPIO
	#define PD6_FUNC				AS_GPIO
	#define PD5_INPUT_ENABLE		1
	#define PD6_INPUT_ENABLE		1
	#define PULL_WAKEUP_SRC_PD5     PM_PIN_PULLUP_10K
	#define PULL_WAKEUP_SRC_PD6     PM_PIN_PULLUP_10K
#endif

/**
 *  @brief  LED Configuration
 */
#if (UI_LED_ENABLE)
	/* 825X Dongle and 827X Dongle use same GPIO for LED: PA3/PB1/PA2/PB0/PA4 */
	#define	GPIO_LED_RED			GPIO_PA3
	#define	GPIO_LED_WHITE			GPIO_PB1
	#define	GPIO_LED_GREEN			GPIO_PA2
	#define	GPIO_LED_BLUE			GPIO_PB0
	#define	GPIO_LED_YELLOW			GPIO_PA4

	#define PA3_FUNC				AS_GPIO
	#define PB1_FUNC				AS_GPIO
	#define PA2_FUNC				AS_GPIO
	#define PB0_FUNC				AS_GPIO
	#define PA4_FUNC				AS_GPIO

	#define	PA3_OUTPUT_ENABLE		1
	#define	PB1_OUTPUT_ENABLE		1
	#define PA2_OUTPUT_ENABLE		1
	#define	PB0_OUTPUT_ENABLE		1
	#define	PA4_OUTPUT_ENABLE		1

	#define LED_ON_LEVEL 			1 		//gpio output high voltage to turn on led
#endif

#if(DEBUG_GPIO_ENABLE)
	#define GPIO_CHN0							GPIO_PB2
	#define GPIO_CHN1							GPIO_PB3
	#define GPIO_CHN2							GPIO_PB4
	#define GPIO_CHN3							GPIO_PB5
	#define GPIO_CHN4							GPIO_PC2
	#define GPIO_CHN5							GPIO_PC3

	#define PB2_OUTPUT_ENABLE					1
	#define PB3_OUTPUT_ENABLE					1
	#define PB4_OUTPUT_ENABLE					1
	#define PB5_OUTPUT_ENABLE					1
	#define PC2_OUTPUT_ENABLE					1
	#define PC3_OUTPUT_ENABLE					1
#endif  //end of DEBUG_GPIO_ENABLE

#if (UART_PRINT_DEBUG_ENABLE)
    #define DEBUG_INFO_TX_PIN               GPIO_PB2
    #define PULL_WAKEUP_SRC_PB2             PM_PIN_PULLUP_10K
    #define PB2_OUTPUT_ENABLE               1
    #define PB2_DATA_OUT                    1
#endif

/**
 *  @brief  Battery_check Configuration
 */
#if (APP_BATT_CHECK_ENABLE)
	//Telink device: you must choose one gpio with adc function to output high level(voltage will equal to vbat), then use adc to measure high level voltage
	//use PB7 output high level, then adc measure this high level voltage
	#define GPIO_VBAT_DETECT				GPIO_PB7
	#define PB7_FUNC						AS_GPIO
	#define PB7_INPUT_ENABLE				0
	#define ADC_INPUT_PCHN					B7P    //corresponding  ADC_InputPchTypeDef in adc.h
#endif



#endif /* C1T139A3_H_ */

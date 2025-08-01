/********************************************************************************************************
 * @file    C1T139A5.h
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
#ifndef C1T139A5_H_
#define C1T139A5_H_


/**
 *  @brief  Keyboard Configuration
 */
#if (UI_KEYBOARD_ENABLE)   // if test pure power, kyeScan GPIO setting all disabled
	//////////////////// KEY CONFIG (RCU board) ///////////////////////////
	#define	MATRIX_ROW_PULL					PM_PIN_PULLDOWN_100K
	#define	MATRIX_COL_PULL					PM_PIN_PULLUP_10K
	#define	KB_LINE_HIGH_VALID				0   //dirve pin output 0 when keyscan, scanpin read 0 is valid

	#define  KB_DRIVE_PINS  {GPIO_PD5, GPIO_PD2, GPIO_PD4, GPIO_PD6, GPIO_PD7}			// last pin 'GPIO_PD7' abnormal
	#define  KB_SCAN_PINS   {GPIO_PC5, GPIO_PA0, GPIO_PB2, GPIO_PA4, GPIO_PA3, GPIO_PD3}// second pin 'GPIO_PA0' abnormal

	//drive pin as gpio
	#define	PD5_FUNC				AS_GPIO
	#define	PD2_FUNC				AS_GPIO
	#define	PD4_FUNC				AS_GPIO
	#define	PD6_FUNC				AS_GPIO
	#define	PD7_FUNC				AS_GPIO

	//drive pin need 100K pulldown
	#define	PULL_WAKEUP_SRC_PD5		MATRIX_ROW_PULL
	#define	PULL_WAKEUP_SRC_PD2		MATRIX_ROW_PULL
	#define	PULL_WAKEUP_SRC_PD4		MATRIX_ROW_PULL
	#define	PULL_WAKEUP_SRC_PD6		MATRIX_ROW_PULL
	#define	PULL_WAKEUP_SRC_PD7		MATRIX_ROW_PULL

	//drive pin open input to read gpio wakeup level
	#define PD5_INPUT_ENABLE		1
	#define PD2_INPUT_ENABLE		1
	#define PD4_INPUT_ENABLE		1
	#define PD6_INPUT_ENABLE		1
	#define PD7_INPUT_ENABLE		1

	//scan pin as gpio
	#define	PC5_FUNC				AS_GPIO
	#define	PA0_FUNC				AS_GPIO
	#define	PB2_FUNC				AS_GPIO
	#define	PA4_FUNC				AS_GPIO
	#define	PA3_FUNC				AS_GPIO
	#define	PD3_FUNC				AS_GPIO

	//scan  pin need 10K pullup
	#define	PULL_WAKEUP_SRC_PC5		MATRIX_COL_PULL
	#define	PULL_WAKEUP_SRC_PA0		MATRIX_COL_PULL
	#define	PULL_WAKEUP_SRC_PB2		MATRIX_COL_PULL
	#define	PULL_WAKEUP_SRC_PA4		MATRIX_COL_PULL
	#define	PULL_WAKEUP_SRC_PA3		MATRIX_COL_PULL
	#define	PULL_WAKEUP_SRC_PD3		MATRIX_COL_PULL

	//scan pin open input to read gpio level
	#define PC5_INPUT_ENABLE		1
	#define PA0_INPUT_ENABLE		1
	#define PB2_INPUT_ENABLE		1
	#define PA4_INPUT_ENABLE		1
	#define PA3_INPUT_ENABLE		1
	#define PD3_INPUT_ENABLE		1
#endif


/**
 *  @brief  Button Configuration
 */
#if (UI_BUTTON_ENABLE)
	/* 825X Dongle and 827X Dongle use Button */
	#error "add button definition by yourself !"
#endif

/**
 *  @brief  LED Configuration
 */
#if (UI_LED_ENABLE)
	#define LED_ON_LEVEL 						1 			//gpio output high voltage to turn on led
	#define	GPIO_LED							GPIO_PC6
	#define PC6_FUNC							AS_GPIO
#endif

/**
 *  @brief  AUDIO Configuration
 */
#if (BLE_AUDIO_ENABLE)
	#define GPIO_AMIC_BIAS					GPIO_PC4
	#define GPIO_AMIC_SP					GPIO_PC0
	#define GPIO_AMIC_SN					GPIO_PC1
#endif

/**
 *  @brief  IR Configuration
 */
#if (REMOTE_IR_ENABLE)
	//PB3 IRout 100K pulldown when  IR not working,  when IR begin, disable this 100K pulldown
	#define	PULL_WAKEUP_SRC_PB3		PM_PIN_PULLDOWN_100K
#endif

/**
 *  @brief  DEBUG_GPIO Configuration
 */
#if(DEBUG_GPIO_ENABLE)
	//define debug GPIO here according to your hardware
	#define GPIO_CHN0							GPIO_PB4
	#define GPIO_CHN1							GPIO_PB5
	#define GPIO_CHN2							GPIO_PB6
	#define GPIO_CHN3							GPIO_PB0
	#define GPIO_CHN4							GPIO_PB1


	#define PB4_OUTPUT_ENABLE					1
	#define PB5_OUTPUT_ENABLE					1
	#define PB6_OUTPUT_ENABLE					1
	#define PB0_OUTPUT_ENABLE					1
	#define PB1_OUTPUT_ENABLE					1
#endif  //end of DEBUG_GPIO_ENABLE

#if UNUSED_GPIO_PULLDOWN_ENABLE
	#define PULL_WAKEUP_SRC_PA5			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PA6			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB0			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB1			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB4			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB5			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB6			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB7			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PD0			PM_PIN_PULLDOWN_100K    //note: A0 version, if enable pull down 100k, will make current leakage
	#define PULL_WAKEUP_SRC_PD1			PM_PIN_PULLDOWN_100K
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


#endif /* C1T139A5_H_ */

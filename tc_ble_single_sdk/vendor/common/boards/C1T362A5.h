/********************************************************************************************************
 * @file    C1T362A5.h
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
#ifndef C1T362A5_H_
#define C1T362A5_H_



/**
 *  @brief  Keyboard Configuration
 */
#if (UI_KEYBOARD_ENABLE)   // if test pure power, kyeScan GPIO setting all disabled
	//////////////////// KEY CONFIG (RCU board) ///////////////////////////
	#define	MATRIX_ROW_PULL					PM_PIN_PULLDOWN_100K
	#define	MATRIX_COL_PULL					PM_PIN_PULLUP_10K
	#define	KB_LINE_HIGH_VALID				0   //dirve pin output 0 when keyscan, scanpin read 0 is valid

	#define  KB_DRIVE_PINS  {GPIO_PB3, GPIO_PA5, GPIO_PA7, GPIO_PB2, GPIO_PB1}			// last pin 'GPIO_PD7' abnormal
	#define  KB_SCAN_PINS   {GPIO_PA0, GPIO_PB0, GPIO_PD0, GPIO_PD5, GPIO_PD6, GPIO_PA6}// second pin 'GPIO_PA0' abnormal

	//drive pin as gpio
	#define	PB3_FUNC				AS_GPIO
	#define	PA5_FUNC				AS_GPIO
	#define	PA7_FUNC				AS_GPIO
	#define	PB2_FUNC				AS_GPIO
	#define	PB1_FUNC				AS_GPIO

	//drive pin need 100K pulldown
	#define	PULL_WAKEUP_SRC_PB3		MATRIX_ROW_PULL
	#define	PULL_WAKEUP_SRC_PA5		MATRIX_ROW_PULL
	#define	PULL_WAKEUP_SRC_PA7		MATRIX_ROW_PULL
	#define	PULL_WAKEUP_SRC_PB2		MATRIX_ROW_PULL
	#define	PULL_WAKEUP_SRC_PB1		MATRIX_ROW_PULL

	//drive pin open input to read gpio wakeup level
	#define PB3_INPUT_ENABLE		1
	#define PA5_INPUT_ENABLE		1
	#define PA7_INPUT_ENABLE		1
	#define PB2_INPUT_ENABLE		1
	#define PB1_INPUT_ENABLE		1

	//scan pin as gpio
	#define	PA0_FUNC				AS_GPIO
	#define	PB0_FUNC				AS_GPIO
	#define	PD0_FUNC				AS_GPIO
	#define	PD5_FUNC				AS_GPIO
	#define	PD6_FUNC				AS_GPIO
	#define	PA6_FUNC				AS_GPIO

	//scan  pin need 10K pullup
	#define	PULL_WAKEUP_SRC_PA0		MATRIX_COL_PULL
	#define	PULL_WAKEUP_SRC_PB0		MATRIX_COL_PULL
	#define	PULL_WAKEUP_SRC_PD0		MATRIX_COL_PULL
	#define	PULL_WAKEUP_SRC_PD5		MATRIX_COL_PULL
	#define	PULL_WAKEUP_SRC_PD6		MATRIX_COL_PULL
	#define	PULL_WAKEUP_SRC_PA6		MATRIX_COL_PULL

	//scan pin open input to read gpio level
	#define PA0_INPUT_ENABLE		1
	#define PB0_INPUT_ENABLE		1
	#define PD0_INPUT_ENABLE		1
	#define PD5_INPUT_ENABLE		1
	#define PD6_INPUT_ENABLE		1
	#define PA6_INPUT_ENABLE		1
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
	#define	GPIO_LED							GPIO_PA1
	#define PA1_FUNC							AS_GPIO
#endif

/**
 *  @brief  AUDIO Configuration
 */
#if (BLE_AUDIO_ENABLE)
	#define GPIO_AMIC_BIAS					GPIO_PC2
	#define GPIO_AMIC_SP					GPIO_PC1
	#define GPIO_AMIC_SN					GPIO_PC0
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
	#define GPIO_CHN0							GPIO_PA3
	#define PA3_OUTPUT_ENABLE					1
#endif  //end of DEBUG_GPIO_ENABLE

#if UNUSED_GPIO_PULLDOWN_ENABLE
	#define PULL_WAKEUP_SRC_PA4			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB4			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB5			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB6			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PB7			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PC3			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PC4			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PC5			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PC6			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PC7			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PD3			PM_PIN_PULLDOWN_100K    //note: A0 version, if enable pull down 100k, will make current leakage
	#define PULL_WAKEUP_SRC_PD4			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PD7			PM_PIN_PULLDOWN_100K
	#define PULL_WAKEUP_SRC_PE0			PM_PIN_PULLDOWN_100K
#endif

/**
 *  @brief  Battery_check Configuration
 */
#if (APP_BATT_CHECK_ENABLE)
	#if 0
		//use VBAT(only 827x) , then adc measure this VBAT voltage
		#define ADC_INPUT_PCHN					VBAT    //corresponding  ADC_InputPchTypeDef in adc.h
	#else
		//Telink device: you must choose one gpio with adc function to output high level(voltage will equal to vbat), then use adc to measure high level voltage
		//use PB7 output high level, then adc measure this high level voltage
		#define GPIO_VBAT_DETECT				SD_ADC_GPIO_PB5P
	#endif
#endif



#endif /* C1T362A5_H_ */

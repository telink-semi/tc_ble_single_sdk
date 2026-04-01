/********************************************************************************************************
 * @file    C1T357A20_V2_1.h
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
#ifndef C1T357A20_V2_1_H_
#define C1T357A20_V2_1_H_



/**
 *  @brief  Keyboard Configuration
 */
#if (UI_KEYBOARD_ENABLE)   // if test pure power, kyeScan GPIO setting all disabled
	/* TC321X EVK use GPIO for Keyboard: PB0/PA1/PA2/PA4 */
	#define	MATRIX_ROW_PULL					PM_PIN_PULLDOWN_100K
	#define	MATRIX_COL_PULL					PM_PIN_PULLUP_10K
	#define	KB_LINE_HIGH_VALID				0   //Drive pin output 0 when keyscan, scanpin read 0 is valid

	//////////////////// KEY CONFIG (EVK board) ///////////////////////////
	//---------------  KeyMatrix PD7/PD5/PB3/PB1 -----------------------------
	#define  KB_DRIVE_PINS  {GPIO_PD7, GPIO_PD5}
	#define  KB_SCAN_PINS   {GPIO_PB3, GPIO_PB1}

	//drive pin as gpio
	#define	PD7_FUNC				AS_GPIO
	#define	PD5_FUNC				AS_GPIO

	//drive pin need 100K pulldown
	#define	PULL_WAKEUP_SRC_PD7		MATRIX_ROW_PULL
	#define	PULL_WAKEUP_SRC_PD5		MATRIX_ROW_PULL

	//drive pin open input to read gpio wakeup level
	#define PD7_INPUT_ENABLE		1
	#define PD5_INPUT_ENABLE		1

	//scan pin as gpio
	#define	PB3_FUNC				AS_GPIO
	#define	PB1_FUNC				AS_GPIO

	//scan  pin need 10K pullup
	#ifndef PULL_WAKEUP_SRC_PB3
		#define	PULL_WAKEUP_SRC_PB3		MATRIX_COL_PULL
	#endif
	#ifndef PULL_WAKEUP_SRC_PB1
		#define	PULL_WAKEUP_SRC_PB1		MATRIX_COL_PULL
	#endif

	//scan pin open input to read gpio level
	#define PB3_INPUT_ENABLE		1
	#define PB1_INPUT_ENABLE		1

	#ifndef CR_VOL_UP
		#define			CR_VOL_UP				0xf0
	#endif
	#ifndef CR_VOL_DN
		#define			CR_VOL_DN				0xf1
	#endif

	/**
	 *  @brief  Normal keyboard map
	 */
	#ifndef KB_MAP_NORMAL
		#define		KB_MAP_NORMAL	{	{CR_VOL_DN,		VK_1},	 \
									{CR_VOL_UP,		VK_2}, }
	#endif

	#ifndef KB_MAP_NUM
		#define		KB_MAP_NUM		KB_MAP_NORMAL
	#endif

	#ifndef KB_MAP_FN
		#define		KB_MAP_FN		KB_MAP_NORMAL
	#endif
#endif


/**
 *  @brief  Button Configuration
 */
#if (UI_BUTTON_ENABLE)
	/* 825X Dongle and 827X Dongle use Button */
//	#error "add button definition by yourself !"
#endif

/**
 *  @brief  LED Configuration
 */
#if (UI_LED_ENABLE)
	/* TC321X EVK use GPIO for LED: PB4/PB6/PB7/PC4 */
	#define	GPIO_LED_WHITE			GPIO_PB4
	#define	GPIO_LED_RED			GPIO_PB7
	#define	GPIO_LED_GREEN			GPIO_PB6
	#define	GPIO_LED_BLUE			GPIO_PD2

	#define PB4_FUNC				AS_GPIO
	#define PB7_FUNC				AS_GPIO
	#define PB6_FUNC				AS_GPIO
	#define PD2_FUNC				AS_GPIO

	#define	PB4_OUTPUT_ENABLE		1
	#define	PB7_OUTPUT_ENABLE		1
	#define PB6_OUTPUT_ENABLE		1
	#define	PD2_OUTPUT_ENABLE		1

	#define LED_ON_LEVEL 			1 		//gpio output high voltage to turn on led
#endif

#if(DEBUG_GPIO_ENABLE)
	#define GPIO_CHN0							GPIO_PC2
	#define GPIO_CHN1							GPIO_PC7
	#define GPIO_CHN2							GPIO_PD0
	#define GPIO_CHN3							GPIO_PD1
	#define GPIO_CHN4							GPIO_PC6
	#define GPIO_CHN5							GPIO_PC3
	#define GPIO_CHN6							GPIO_PA2
	#define GPIO_CHN7							GPIO_PA4
    #define GPIO_CHN8                           GPIO_PA7
    #define GPIO_CHN9                           GPIO_PB0
    #define GPIO_CHN10                          GPIO_PA0

	#define PC2_OUTPUT_ENABLE					1
	#define PC7_OUTPUT_ENABLE					1
	#define PD0_OUTPUT_ENABLE					1
    #define PD1_OUTPUT_ENABLE                   1
	#define PC6_OUTPUT_ENABLE					1
	#define PC3_OUTPUT_ENABLE					1
	#define PA2_OUTPUT_ENABLE					1
	#define PA4_OUTPUT_ENABLE					1
	#define PA7_OUTPUT_ENABLE					1
    #define PB0_OUTPUT_ENABLE                   1
    #define PA0_OUTPUT_ENABLE                   1
#endif  //end of DEBUG_GPIO_ENABLE

/**
 *  @brief  AUDIO Configuration
 */
#if (BLE_AUDIO_ENABLE)
	#define GPIO_AMIC_BIAS					GPIO_PC5
	#define GPIO_AMIC_SP					GPIO_PC1
	#define GPIO_AMIC_SN					GPIO_PC0
#endif

/**
 *  @brief  PRINT DEBUG INFO
 */
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
	#if 1
		//use VBAT , then adc measure this VBAT voltage
		#define ADC_INPUT_PCHN					VBAT    //corresponding  ADC_InputPchTypeDef in adc.h
	#else
	//Telink device: you must choose one gpio with adc function to output high level(voltage will equal to vbat), then use adc to measure high level voltage
	//use PB2 output high level, then adc measure this high level voltage
	#define GPIO_VBAT_DETECT				SD_ADC_GPIO_PB2P
#endif
#endif



#endif /* C1T357A20_V2_1_H_ */

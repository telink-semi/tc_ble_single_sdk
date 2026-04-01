/********************************************************************************************************
 * @file    rf_pa.h
 *
 * @brief   This is the header file for B85
 *
 * @author  Driver Group
 * @date    May 8,2018
 *
 * @par     Copyright (c) 2018, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifndef BLT_PA_H_
#define BLT_PA_H_

#include "gpio.h"


#ifndef PA_ENABLE
#define PA_ENABLE                           0
#endif

#define BLE_PA_ENABLE						1
#define PRIVATE_PA_ENABLE					2
#define PA_MODE								BLE_PA_ENABLE


#if (PA_MODE == BLE_PA_ENABLE)
	#ifndef PA_TXEN_PIN
	#define PA_TXEN_PIN                         GPIO_PB2
	#endif

	#ifndef PA_RXEN_PIN
	#define PA_RXEN_PIN                         GPIO_PB3
	#endif
#elif (PA_MODE == PRIVATE_PA_ENABLE)
	#ifndef PRIVATE_PA_TXEN_PIN
	#define PRIVATE_PA_TXEN_PIN                         GPIO_PD2
	#endif

	#ifndef PRIVATE_PA_RXEN_PIN
	#define PRIVATE_PA_RXEN_PIN                         GPIO_PD1
	#endif
#endif


#define PA_TYPE_OFF							0
#define PA_TYPE_TX_ON						1
#define PA_TYPE_RX_ON						2

#define PRIVATE_PA_TYPE_OFF					PA_TYPE_OFF
#define PRIVATE_PA_TYPE_TX_ON				PA_TYPE_TX_ON
#define PRIVATE_PA_TYPE_RX_ON				PA_TYPE_RX_ON

/**
 * @brief	software PA control Callback
 */
typedef void (*rf_pa_callback_t)(int type);
extern rf_pa_callback_t  blc_rf_pa_cb;


/**
 * @brief	RF software PA initialization
 * @param	none
 * @return	none
 */
void rf_pa_init(void);
_attribute_ram_code_sec_ void rf_pa_handler(short int flag);

#endif /* BLT_PA_H_ */

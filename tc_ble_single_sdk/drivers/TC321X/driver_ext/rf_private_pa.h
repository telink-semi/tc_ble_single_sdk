/********************************************************************************************************
 * @file    rf_private_pa.h
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
#ifndef RF_PRIVATE_PA_H_
#define RF_PRIVATE_PA_H_

#include "gpio.h"
#include "config.h"

#ifndef PRIVATE_PA_ENABLE
#define PRIVATE_PA_ENABLE                           1
#endif

/**
 * @brief the state of power amplifier (PA)
 */

typedef enum {
  PRIVATE_PA_TYPE_OFF = 0,
  PRIVATE_PA_TYPE_TX_ON,
  PRIVATE_PA_TYPE_RX_ON,
}pa_type;

#ifndef PRIVATE_PA_TXEN_PIN
#define PRIVATE_PA_TXEN_PIN                         GPIO_PD2
#endif

#ifndef PRIVATE_PA_RXEN_PIN
#define PRIVATE_PA_RXEN_PIN                         GPIO_PD1
#endif



typedef void (*rf_private_pa_callback_t)(pa_type type);
extern rf_private_pa_callback_t  rf_pa_cb;



void rf_pa_init(void);
_attribute_ram_code_sec_ void rf_pa_handler(short int flag);


#endif /* BLT_PA_H_ */


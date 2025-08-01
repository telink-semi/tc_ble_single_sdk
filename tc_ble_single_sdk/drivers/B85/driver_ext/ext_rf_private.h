/********************************************************************************************************
 * @file    ext_rf_private.h
 *
 * @brief   This is the header file for 2.4G SDK
 *
 * @author  2.4G Group
 * @date    2025
 *
 * @par     Copyright (c) 2025, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifndef RF_PRIVATE_H_
#define RF_PRIVATE_H_
#include "bsp.h"

/**
 *  @brief  Define RF Tx/Rx/Auto mode
 */

typedef enum {
    PRI_MODE_TX = 0,
    PRI_MODE_RX = 1,
    PRI_MODE_AUTO=2
} PRI_StatusTypeDef;

/** An enum describing the radio's data rate.
 *
 */
typedef enum {
    GEN_FSK_DATARATE_250KBPS = BIT(6),
    GEN_FSK_DATARATE_500KBPS = BIT(7),
    GEN_FSK_DATARATE_1MBPS = BIT(8),
    GEN_FSK_DATARATE_2MBPS = BIT(9),
} gen_fsk_datarate_t;

/** An enum describing the radio's data rate.
 *
 */
typedef enum {
    TPSLL_DATARATE_1MBPS = 0,
    TPSLL_DATARATE_2MBPS = 1,
} tpsll_datarate_t;

/**@brief Telink primary link layer bitrate mode. */
typedef enum {
    TPLL_BITRATE_1MBPS = 0,      /**< 1Mbit radio mode. */
    TPLL_BITRATE_2MBPS,          /**< 2Mbit radio mode. */
    TPLL_BITRATE_500kBPS,        /**< 500kbit radio mode. */
    TPLL_BITRATE_250KBPS,        /**< 250Kbit radio mode. */
} TPLL_BitrateTypeDef;

/**
*	@brief	  	This function serves to judge RF Tx/Rx state.
*	@param[in]	rf_status - Tx/Rx status.
*	@param[in]	rf_channel - RF channel.
*	@return	 	failed -1,else success.
*/
int rf_pri_trx_state_set(PRI_StatusTypeDef rf_status, signed short rf_channel);

/**
 * @brief       This function serves to set general fsk link layer bitrate.
 * @note        This function must be called at the beginning of the TPLL configuration.
 * @param       bitrate  Radio bitrate.
 * @return      none.
 */
void rf_pri_gen_fsk_datarate_set(gen_fsk_datarate_t datarate);

/**
 * @brief      This function servers to set Telink Proprietary Synchronous Link Layer radio's on-air datarate.
 * @param[in]  datarate  specify the radio's datarate
 * @param[out] none
 * @return     none.
 */
void rf_pri_tpsll_init(tpsll_datarate_t datarate);

/**
 * @brief      This function servers to set Telink primary link layer radio's on-air datarate.
 * @param[in]  datarate  specify the radio's datarate
 * @param[out] none
 * @return     none.
 */
void RF_Pri_TPLL_Init(TPLL_BitrateTypeDef bitrate);

/**
*	@brief	  	This function serves to get RF status.
*	@param[in]	none.
*	@return	 	RF Rx/Tx status.
*/
PRI_StatusTypeDef rf_pri_trx_state_get(void);
#endif /* RF_PRIVATE_H_ */

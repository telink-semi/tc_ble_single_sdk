/********************************************************************************************************
 * @file    ext_misc.h
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
#ifndef DRIVERS_8278_EXT_MISC_H_
#define DRIVERS_8278_EXT_MISC_H_
#include "../register.h"


/******************************* rf start **********************************************************************/
/**
 * @brief     This function serves to set BLE mode of RF.
 * @return	  none.
 */
void rf_drv_ble_init(void);

#define		RF_POWER_P3dBm					RF_POWER_P3p13dBm
#define		RF_POWER_P0dBm					RF_POWER_N0p28dBm

/******************************* rf end  **********************************************************************/


/******************************* watchdog_start ***********************************************************************/

#define WATCHDOG_TIMEOUT_COEFF	18		//  check register definition, 0x622

#define WATCHDOG_DISABLE	( reg_tmr_ctrl &= ~FLD_TMR_WD_EN )

/******************************** watchdog_end  **********************************************************************/


/******************************* stimer_start ******************************************************************/
//#define reg_system_tick_irq 		REG_ADDR32(0x744)//reg_system_tick_irq_level

/**
 * @brief   system Timer : 16Mhz, Constant
 */
enum{
	SYSTEM_TIMER_TICK_1US 		= 16,
	SYSTEM_TIMER_TICK_1MS 		= 16000,
	SYSTEM_TIMER_TICK_1S 		= 16000000,

	SYSTEM_TIMER_TICK_625US  	= 10000,  //625*16
	SYSTEM_TIMER_TICK_1250US 	= 20000,  //1250*16
};


#define reg_system_tick_irq reg_system_tick_irq_level

/******************************* stimer_end ********************************************************************/



/******************************* ext_clock begin ******************************************************************/

/**
 * @brief     This function is used to check if system clock is 16m
 * @param[in] none
 * @return	  1: system clock is 16m
 *            0: system clock is not 16m
 */
static inline unsigned char clock_is_system_clock_16m(void)
{
	extern unsigned char system_clk_type;
	return (system_clk_type == SYS_CLK_16M_Crystal);
}

/******************************* ext_clock end ********************************************************************/



/******************************* dma_start ******************************************************************/

/**
 * @brief	ACL RX Data buffer length = maxRxOct + 21, then 16 Byte align
 *			maxRxOct + 21 = 4(DMA_len) + 2(BLE header) + maxRxOct + 4(MIC) + 3(CRC) + 8(ExtraInfo)
			RX buffer size must be be 16*n, due to MCU design
 */
//actually +21.The purpose of +22 is to deal with extreme situations. Due to DMA design,at least one byte buffer can not be unusable.

#define		TLK_RF_RX_EXT_LEN		(22)	//4(DMA_len) + 2(BLE header) + ISORxOct + 4(MIC) + 3(CRC) + 8(ExtraInfo)
#define 	CAL_LL_ACL_RX_BUF_SIZE(maxRxOct)	(((maxRxOct + TLK_RF_RX_EXT_LEN) + 15) / 16 *16)


/**
 * @brief	ACL TX Data buffer length = maxTxOct + 10, then 4 Byte align
 *			maxTxOct + 10 = 4(DMA_len) + 2(BLE header) + maxTxOct + 4(MIC)
			TX buffer size must be be 4*n, due to MCU design
 */

#define     TLK_RF_TX_EXT_LEN		(10)	//10 = 4(DMA_len) + 2(BLE header) + 4(MIC)
#define 	CAL_LL_ACL_TX_BUF_SIZE(maxTxOct)	(((maxTxOct + TLK_RF_TX_EXT_LEN) + 3) / 4 *4)

/******************************* dma_end ******************************************************************/

/******************************* random start ******************************************************************/
/**
 * @brief      This function performs to get a serial of random number.
 * @param[in]  len- the length of random number
 * @param[in]  data - the first address of buffer store random number in
 * @return     the result of a serial of random number..
 */
void generateRandomNum(int len, unsigned char *data);
/******************************* random end ******************************************************************/
#endif

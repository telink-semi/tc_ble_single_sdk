/********************************************************************************************************
 * @file    app_tpll_prx.c
 *
 * @brief   This is the source file for B80
 *
 * @author  2.4G Group
 * @date    12,2021
 *
 * @par     Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "../app_config.h"
#include "driver.h"
#include "2p4g_common.h"

#include "tpll/tpll.h"

#if (TEST_2P4G_MODE==TPLL_PRX)

#define ACK_PAYLOAD_LEN         32
static unsigned char   ack_payload[ACK_PAYLOAD_LEN] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                                      0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                                                      0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                                                      0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f};
static volatile unsigned int rx_timestamp                 = 0;
volatile unsigned char       rx_flag                      = 0;
volatile unsigned char       rx_dr_flag                   = 0;
volatile unsigned char       tx_flag                      = 0;
volatile unsigned char       ds_flag                      = 0;
volatile unsigned char       invalid_pid_flag             = 0;
volatile unsigned char       rx_payload[128]              = {0};
unsigned short               length_pip_ret               = 0;
volatile unsigned char       preamble_len                 = 0;
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
volatile static unsigned int irq_cnt_rx_sync              = 0;
volatile static unsigned int irq_cnt_rx_head_done         = 0;
#endif
volatile unsigned int        irq_cnt_rx_dr                = 0;
volatile unsigned int        irq_cnt_invalid_pid          = 0;
volatile unsigned int        irq_cnt_rx                   = 0;
volatile unsigned int        irq_cnt_tx                   = 0;
volatile unsigned int        irq_cnt_tx_ds                = 0;
volatile unsigned int        irq_cnt_rx_crc_ok            = 0;

__attribute__((section(".ram_code"))) __attribute__((optimize("-Os")))
void irq_2p4_handler(void)
{
    unsigned short src_rf = rf_irq_src_get();

    if (src_rf & FLD_RF_IRQ_RX_DR)
    {
      rf_irq_clr_src(FLD_RF_IRQ_RX_DR);
      irq_cnt_rx_dr++;
      rx_dr_flag = 1;
    }

    if (src_rf & FLD_RF_IRQ_RX)
    {
      rf_irq_clr_src(FLD_RF_IRQ_RX);
      irq_cnt_rx++;
      if( TPLL_RF_PACKET_CRC_OK(TPLL_GetRxPacket()) ){
          irq_cnt_rx_crc_ok++;
      }
      u32 pending_tick = clock_time();//200ms
      while (!TPLL_TxFifoEmpty(0) && !clock_time_exceed(pending_tick,200*1000));   // tx finish
      TPLL_WriteAckPayload(TPLL_PIPE0, ack_payload, ACK_PAYLOAD_LEN);
      rx_flag = 1;
    }

    if (src_rf & FLD_RF_IRQ_INVALID_PID)
    {
      rf_irq_clr_src(FLD_RF_IRQ_INVALID_PID);
      irq_cnt_invalid_pid++;
      invalid_pid_flag = 1;
    }

    if (src_rf & FLD_RF_IRQ_TX)
    {
      rf_irq_clr_src(FLD_RF_IRQ_TX);
      irq_cnt_tx++;
      tx_flag = 1;
    }

    if (src_rf & FLD_RF_IRQ_TX_DS)
    {
      rf_irq_clr_src(FLD_RF_IRQ_TX_DS);
      irq_cnt_tx_ds++;
      ds_flag = 1;
    }
    #if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    if (src_rf & FLD_RF_IRQ_HIT_SYNC) //if rf rx irq occurs
    {
    rf_irq_clr_src(FLD_RF_IRQ_HIT_SYNC);
      irq_cnt_rx_sync++;
    }
    if (src_rf & FLD_RF_IRQ_HEADER_DONE) //if rf rx irq occurs
    {
    rf_irq_clr_src(FLD_RF_IRQ_HEADER_DONE);
      irq_cnt_rx_head_done++;
    }
    #endif
    irq_clr_src();
    rf_irq_clr_src(FLD_RF_IRQ_ALL);
}

void tpll_config_init(signed short chnn)
{
    /*
     * rf configuration
     * notes:b80 rx does not support multiple pipes
     */
    unsigned char rx_address[5] = {0xe7,0xe7,0xe7,0xe7,0xe7};
    TPLL_Init(TPLL_BITRATE_2MBPS);
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    TPLL_SetOutputPower(RF_POWER_P0p00dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_825x)
    TPLL_SetOutputPower(RF_POWER_P0p04dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_827x)
    TPLL_SetOutputPower(RF_POWER_P0p52dBm);
#endif
    TPLL_SetAddressWidth(ADDRESS_WIDTH_5BYTES);
    TPLL_ClosePipe(TPLL_PIPE_ALL);
    TPLL_SetAddress(TPLL_PIPE0, rx_address);
    TPLL_OpenPipe(TPLL_PIPE0);
    TPLL_SetTXPipe(TPLL_PIPE0);
    TPLL_SetRFChannel(7);
    TPLL_TxSettleSet(149);
    TPLL_RxSettleSet(80);
    TPLL_ModeSet(TPLL_MODE_PRX);

    irq_clr_src();
    rf_irq_clr_src(FLD_RF_IRQ_ALL);
    irq_enable_type(FLD_IRQ_ZB_RT_EN);
    rf_irq_disable(FLD_RF_IRQ_ALL);
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    rf_irq_enable(FLD_RF_IRQ_TX | FLD_RF_IRQ_TX_DS | FLD_RF_IRQ_RX_DR | FLD_RF_IRQ_HIT_SYNC | FLD_RF_IRQ_HEADER_DONE | FLD_RF_IRQ_INVALID_PID | FLD_RF_IRQ_RX);
#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
    rf_irq_enable(FLD_RF_IRQ_TX | FLD_RF_IRQ_TX_DS | FLD_RF_IRQ_RX_DR | FLD_RF_IRQ_INVALID_PID | FLD_RF_IRQ_RX);
#endif
    irq_enable();
}
volatile unsigned char AA_test1;
int app_mainloop_2p4g(void)
{
    AA_test1++;
    tpll_config_init(7);

    TPLL_PRXTrig();
    rx_timestamp = clock_time();

    while (1)
    {
        if (1 == rx_flag)
        {
            rx_flag = 0;
            #if (UI_LED_ENABLE)
            gpio_toggle(GPIO_LED_GREEN);
            #endif
            length_pip_ret = TPLL_ReadRxPayload((unsigned char *)&rx_payload);
        }
    	app_idle_loop_2p4g();
    }


    return 0;
}

#endif

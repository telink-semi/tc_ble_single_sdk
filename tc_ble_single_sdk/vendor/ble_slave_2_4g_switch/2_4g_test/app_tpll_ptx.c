/********************************************************************************************************
 * @file    app_tpll_ptx.c
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

#if (TEST_2P4G_MODE==TPLL_PTX)

#define PTX_CHANNEL              0//warning:B80 only support pipe0,B80B support pipe0~5
volatile unsigned char maxretry_flag  = 0;
volatile unsigned char rx_dr_flag     = 0;
volatile unsigned char rx_flag        = 0;
unsigned char          tx_payload_len = 32;
unsigned char          preamble_len   = 0;
volatile unsigned char ds_flag        = 0;
int                    ret            = 0;
static unsigned char tx_payload[32] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                                 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                                                 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                                 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20};
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
volatile static unsigned int irq_cnt_rx_sync              = 0;
volatile static unsigned int irq_cnt_rx_head_done         = 0;
#endif
volatile unsigned int        irq_cnt_invalid_pid          = 0;
volatile unsigned int        irq_cnt_max_retry_tx         = 0;
volatile unsigned int        irq_cnt_tx_ds                = 0;
volatile unsigned int        irq_cnt_tx                   = 0;
volatile unsigned int        irq_cnt_rx                   = 0;
volatile unsigned int        irq_cnt_rx_dr                = 0;
volatile unsigned int        irq_cnt_rx_crc_ok            = 0;

extern volatile unsigned char rx_flag, rx_dr_flag, ds_flag, maxretry_flag;

__attribute__((section(".ram_code")))__attribute__((optimize("-Os")))
void irq_2p4_handler(void)
{
    unsigned short src_rf = rf_irq_src_get();
    unsigned char pipe = TPLL_GetTXPipe();
    if (src_rf & FLD_RF_IRQ_TX)
    {
      rf_irq_clr_src(FLD_RF_IRQ_TX);
      irq_cnt_tx++;
    }
    if (src_rf & FLD_RF_IRQ_INVALID_PID)
    {
      rf_irq_clr_src(FLD_RF_IRQ_INVALID_PID);
      irq_cnt_invalid_pid++;
    }
    #if (MCU_CORE_TYPE == MCU_CORE_TC321X)
    if (src_rf & FLD_RF_IRQ_TX_RETRYCNT)
    {
      rf_irq_clr_src(FLD_RF_IRQ_TX_RETRYCNT);
      irq_cnt_max_retry_tx++;
      maxretry_flag = 1;
      //adjust rptr
      TPLL_UpdateTXFifoRptr(pipe);
    }
    #elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
    if (src_rf & FLD_RF_IRQ_RETRY_HIT)
    {
      rf_irq_clr_src(FLD_RF_IRQ_RETRY_HIT);
      irq_cnt_max_retry_tx++;
      maxretry_flag = 1;
      //adjust rptr
      TPLL_UpdateTXFifoRptr(pipe);
    }
    #endif
    if (src_rf & FLD_RF_IRQ_TX_DS)
    {
      rf_irq_clr_src(FLD_RF_IRQ_TX_DS);
      irq_cnt_tx_ds++;
      ds_flag = 1;
    }
    if (src_rf & FLD_RF_IRQ_RX_DR)
    {
      rf_irq_clr_src(FLD_RF_IRQ_RX_DR);
      irq_cnt_rx_dr++;
      rx_dr_flag = 1;
    }
    if (src_rf & FLD_RF_IRQ_RX)
    {
      rf_irq_clr_src(FLD_RF_IRQ_RX);
      if( TPLL_RF_PACKET_CRC_OK(TPLL_GetRxPacket()) ){
          irq_cnt_rx_crc_ok++;
          rx_flag = 1;
      }
      irq_cnt_rx++;
      rx_flag = 1;
    }
    #if (MCU_CORE_TYPE == MCU_CORE_TC321X)
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
    //rf configuration
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

#if PTX_CHANNEL == 0
    //unsigned char tx_address[3] = {0xe7,0xe7,0xe7};
    unsigned char tx_address[5] = {0xe7,0xe7,0xe7,0xe7,0xe7}; //{0xaa,0xbb,0xcc,0xdd,0xee};
    TPLL_SetAddress(TPLL_PIPE0, tx_address);
    TPLL_OpenPipe(TPLL_PIPE0);
    TPLL_SetTXPipe(TPLL_PIPE0);
#endif

#if PTX_CHANNEL == 1
    unsigned char tx_address1[5] = {0x55, 0x44, 0x33, 0x22, 0x11};
//    unsigned char tx_address1[5] = {0xe7, 0xe7, 0xe7, 0xe7, 0xe7};
    TPLL_SetAddress(TPLL_PIPE1, tx_address1);
    TPLL_OpenPipe(TPLL_PIPE1);
    TPLL_SetTXPipe(TPLL_PIPE1);
#endif

#if PTX_CHANNEL == 2
    unsigned char tx_address2[5] = {0x55, 0x44, 0x33, 0x22, 0x11};
    TPLL_SetAddress(TPLL_PIPE1, tx_address2);
    tx_address2[0] = 0x22;
    TPLL_SetAddress(TPLL_PIPE2, &tx_address2[0]);
    TPLL_OpenPipe(TPLL_PIPE2);
    TPLL_SetTXPipe(TPLL_PIPE2);
#endif

#if PTX_CHANNEL == 3
    unsigned char tx_address3[5] = {0x55, 0x44, 0x33, 0x22, 0x11};
    TPLL_SetAddress(TPLL_PIPE1, tx_address3);
    tx_address3[0] = 0x33;
    TPLL_SetAddress(TPLL_PIPE3, &tx_address3[0]);
    TPLL_OpenPipe(TPLL_PIPE3);
    TPLL_SetTXPipe(TPLL_PIPE3);
#endif

    TPLL_ModeSet(TPLL_MODE_PTX);

    TPLL_SetRFChannel(chnn);

    TPLL_SetAutoRetry(0, 150);  //5,150

    TPLL_RxTimeoutSet(500);//if the mode is 250k ,the rx_time_out need more time, as so 1000us is ok!

    TPLL_RxSettleSet(80);

    TPLL_TxSettleSet(149);

    TPLL_Preamble_Set(8);

    WaitUs(150);
    //configure irq
    irq_clr_src();
    rf_irq_clr_src(FLD_RF_IRQ_ALL);

    irq_enable_type(FLD_IRQ_ZB_RT_EN); //enable RF irq
    rf_irq_disable(FLD_RF_IRQ_ALL);
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    rf_irq_enable(FLD_RF_IRQ_TX | FLD_RF_IRQ_TX_DS | FLD_RF_IRQ_TX_RETRYCNT | FLD_RF_IRQ_RX_DR | FLD_RF_IRQ_INVALID_PID | FLD_RF_IRQ_RX | FLD_RF_IRQ_HIT_SYNC | FLD_RF_IRQ_HEADER_DONE);
#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
    rf_irq_enable(FLD_RF_IRQ_TX | FLD_RF_IRQ_TX_DS | FLD_RF_IRQ_RETRY_HIT | FLD_RF_IRQ_RX_DR | FLD_RF_IRQ_INVALID_PID | FLD_RF_IRQ_RX);
#endif
    irq_enable(); //enable general irq
}

int app_mainloop_2p4g(void)
{

    tpll_config_init(7);

    preamble_len = TPLL_Preamble_Read();

    int ret = 0;
    ds_flag = 1; // for first start

    while (1)
    {
        if (1 == ds_flag || 1 == maxretry_flag)
        {
            if (1 == ds_flag)
            {
                #if (UI_LED_ENABLE)
                gpio_toggle(GPIO_LED_GREEN);
                #endif
            }
            ds_flag = 0;
            maxretry_flag = 0;
            WaitMs(2);
            tx_payload[1]++;
            ret = TPLL_WriteTxPayload(PTX_CHANNEL, tx_payload, tx_payload_len);
            if (ret)
            {
                TPLL_PTXTrig();
            }
        }
    	app_idle_loop_2p4g();

    }

    return 0;
}

#endif

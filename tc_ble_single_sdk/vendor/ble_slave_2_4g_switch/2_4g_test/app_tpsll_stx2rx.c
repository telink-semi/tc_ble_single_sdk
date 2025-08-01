/********************************************************************************************************
 * @file    app_tpsll_stx2rx.c
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

#include "tpsll/tpsll.h"

#if (TEST_2P4G_MODE==TPSLL_STX2RX)
#define RX_BUF_SIZE                     255
volatile static unsigned char tpsll_rxbuf[RX_BUF_SIZE]  __attribute__((aligned(4)));

unsigned char payload_len = 32;                                  //payload_len best to be 4n-1;
volatile static unsigned char payload[32] = {
        TPSLL_SYNC_DATA,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
        0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,
        0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33
};

volatile static unsigned char rx_flag                 = 0;
volatile static unsigned char rx_timeout_flag         = 0;
volatile static unsigned char tx_done_flag            = 0;
volatile static unsigned int irq_cnt_rx               = 0;
volatile static unsigned int irq_cnt_tx               = 0;
volatile static unsigned int irq_cnt_rx_crc_ok        = 0;
volatile static unsigned int irq_cnt_rx_timeout       = 0;
volatile static unsigned int irq_cnt_rx_sync          = 0;
volatile static unsigned int irq_cnt_rx_head_done     = 0;

__attribute__((section(".ram_code"))) __attribute__((optimize("-Os")))
void irq_2p4_handler(void)
{
    unsigned int irq_src = irq_get_src();
    unsigned short rf_irq_src = rf_irq_src_get();

    if (irq_src & FLD_IRQ_ZB_RT_EN) {//if rf irq occurs
      if (rf_irq_src & FLD_RF_IRQ_RX) {//if rf rx irq occurs
          rf_irq_clr_src(FLD_RF_IRQ_RX);
          irq_cnt_rx++;
          if (tpsll_is_rx_crc_ok((unsigned char *)tpsll_rxbuf)) {
            irq_cnt_rx_crc_ok++;
              rx_flag = 1;
          }
      }
      if (rf_irq_src & FLD_RF_IRQ_RX_TIMEOUT) {//if rf tx irq occurs
          rf_irq_clr_src(FLD_RF_IRQ_RX_TIMEOUT);
          rx_timeout_flag = 1;
          irq_cnt_rx_timeout++;
      }
      if (rf_irq_src & FLD_RF_IRQ_TX) {//if rf tx irq occurs
              rf_irq_clr_src(FLD_RF_IRQ_TX);
              tx_done_flag = 1;
              irq_cnt_tx++;
          }
    #if(MCU_CORE_TYPE == MCU_CORE_TC321X)
      if (rf_irq_src & FLD_RF_IRQ_HIT_SYNC) //if rf rx irq occurs
      {
        rf_irq_clr_src(FLD_RF_IRQ_HIT_SYNC);
          irq_cnt_rx_sync++;
      }
      if (rf_irq_src & FLD_RF_IRQ_HEADER_DONE) //if rf rx irq occurs
      {
        rf_irq_clr_src(FLD_RF_IRQ_HEADER_DONE);
          irq_cnt_rx_head_done++;
      }
    #endif
    }
    irq_clr_sel_src(FLD_IRQ_ALL);
}

void tpsll_config_init(void)
{
    unsigned char sync_word[4] = {0xDF, 0x56, 0xD9, 0x35};
    //init Link Layer configuratioin
    tpsll_init(TPSLL_DATARATE_2MBPS);
    tpsll_channel_set(7);
    tpsll_preamble_len_set(2);
    tpsll_sync_word_len_set(TPSLL_SYNC_WORD_LEN_4BYTE);

    if (sync_word_is_invalid(sync_word, 4)) {
        tlk_printf("sync_word invalid!\n");
        while(1);
    }

    tpsll_sync_word_set(TPSLL_PIPE0,sync_word);
    tpsll_pipe_open(TPSLL_PIPE0);
    tpsll_tx_pipe_set(TPSLL_PIPE0);
    tpsll_rx_buffer_set((unsigned char *)tpsll_rxbuf,RX_BUF_SIZE);
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    tpsll_radio_power_set(RF_POWER_P0p00dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_825x)
    tpsll_radio_power_set(RF_POWER_P0p04dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_827x)
    tpsll_radio_power_set(RF_POWER_P0p52dBm);
#endif
    tpsll_tx_settle_set(113);
    tpsll_rx_settle_set(90);

    //irq configuration
    rf_irq_disable(FLD_RF_IRQ_ALL);
    rf_irq_enable(FLD_RF_IRQ_TX|FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT); //enable rf rx and rx first timeout irq
    irq_enable_type(FLD_IRQ_ZB_RT_EN); //enable RF irq
    irq_enable(); //enable general irq
}

int app_mainloop_2p4g(void)
{
    tpsll_config_init();

    //start the SRX
    tpsll_tx_write_payload((unsigned char *)payload,payload_len);
    tpsll_stx2rx_start(clock_time()+50*16,250);

    while (1)
    {
        if (rx_timeout_flag) {
            rx_timeout_flag = 0;
//            WaitMs(100);
            tpsll_tx_write_payload((unsigned char *)payload,payload_len);
            tpsll_stx2rx_start(clock_time()+50*16,250);
        }

        if (rx_flag) {
            rx_flag = 0;
            gpio_toggle(GPIO_LED_GREEN);
//            WaitMs(100);
            payload[4]++;
            tpsll_tx_write_payload((unsigned char *)payload,payload_len);
            tpsll_stx2rx_start(clock_time()+50*16,250);
        }

    	app_idle_loop_2p4g();
    }

    return 0;
}



#endif

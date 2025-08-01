/********************************************************************************************************
 * @file    app_gen_fsk_tx.c
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

#include "genfsk_ll/genfsk_ll.h"

#if (TEST_2P4G_MODE==GEN_FSK_TX)

unsigned char tx_payload[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
static unsigned char __attribute__ ((aligned (4)))         tx_buffer[64] = {0};
volatile static unsigned char                                 tx_done_flag = 0;
volatile unsigned int                                               tx_cnt = 0;
static volatile unsigned int                                        irq_cnt_tx;

__attribute__((section(".ram_code"))) __attribute__((optimize("-Os")))
void irq_2p4_handler(void)
{
    unsigned int irq_src = irq_get_src();
    unsigned short rf_irq_src = rf_irq_src_get();

    if (irq_src & FLD_IRQ_ZB_RT_EN) // if rf irq occurs
    {
        if (rf_irq_src & FLD_RF_IRQ_TX) // if rf tx irq occurs
        {
            tx_done_flag = 1;
            irq_cnt_tx++;
        }
        rf_irq_clr_src(FLD_RF_IRQ_ALL);
    }
    irq_clr_sel_src(FLD_IRQ_ALL);
}

void gen_fsk_config_init(void)
{
    unsigned char sync_word[4] = {0x53, 0x78, 0x56, 0x52};
    // it needs to notice that this api is different from B87 / B85
    gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS); //Note that this API must be invoked first before all other APIs

    gen_fsk_preamble_len_set(4);

    gen_fsk_sync_word_len_set(SYNC_WORD_LEN_4BYTE);


    if (sync_word_is_invalid(sync_word, 4)) {
        tlk_printf("sync_word invalid!\n");
        while(1);
    }

    gen_fsk_sync_word_set(GEN_FSK_PIPE0, sync_word); //set pipe0's sync word

    gen_fsk_pipe_open(GEN_FSK_PIPE0); // enable pipe0's reception

    gen_fsk_tx_pipe_set(GEN_FSK_PIPE0); //set pipe0 as the TX pipe

    gen_fsk_packet_format_set(GEN_FSK_PACKET_FORMAT_FIXED_PAYLOAD, sizeof(tx_payload));

#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    gen_fsk_radio_power_set(RF_POWER_P0p00dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_825x)
    gen_fsk_radio_power_set(RF_POWER_P0p04dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_827x)
    gen_fsk_radio_power_set(RF_POWER_P0p52dBm);
#endif

    gen_fsk_channel_set(7);

    gen_fsk_radio_state_set(GEN_FSK_STATE_TX); // set transceiver to basic TX state

    gen_fsk_tx_settle_set(149);

    WaitUs(130); // wait for tx settle

    rf_irq_enable(FLD_RF_IRQ_TX); // enable rf tx irq
    irq_enable_type(FLD_IRQ_ZB_RT_EN); // enable RF irq
    irq_enable(); // enable general irq
}

int app_mainloop_2p4g(void)
{
    gen_fsk_config_init();

    //fill the dma info in tx buffer
    tx_buffer[0] = sizeof(tx_payload);
    tx_buffer[1] = 0x00;
    tx_buffer[2] = 0x00;
    tx_buffer[3] = 0x00;
    memcpy(&tx_buffer[4], tx_payload, sizeof(tx_payload));

    while (1)
    {
        tx_done_flag = 0;
        gen_fsk_tx_start(tx_buffer); // start the Radio transmission
        while (tx_done_flag == 0);   // tx finish
        tx_cnt++;
        tx_buffer[4]++;
        gpio_toggle(GPIO_LED_GREEN);
        WaitMs(2);
    	app_idle_loop_2p4g();
    }

    return 0;
}

#endif

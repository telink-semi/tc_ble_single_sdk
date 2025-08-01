/********************************************************************************************************
 * @file    app_gen_fsk_srx2tx.c
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

#if (TEST_2P4G_MODE==GEN_FSK_SRX2TX)

#define RX_BUF_LEN              64
#define RX_BUF_NUM              4

volatile static unsigned char rx_buf[RX_BUF_LEN * RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
volatile static unsigned char rx_ptr                                                        = 0;
volatile static unsigned char rx_flag                                                       = 0;
volatile static unsigned char rx_first_timeout_flag                                         = 0;
volatile static unsigned char rx_payload_len                                                = 0;
volatile static unsigned int  rx_timestamp                                                  = 0;
volatile static unsigned char rx_rssi                                                       = 0;
volatile static unsigned char *rx_payload                                                   = 0;
volatile static unsigned char *rx_packet                                                    = 0;
volatile static unsigned int irq_cnt_rx_first_timeout                                       = 0;
volatile static unsigned int irq_cnt_rx                                                     = 0;
volatile static unsigned int irq_cnt_crc_ok                                                 = 0;
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
volatile static unsigned int irq_cnt_rx_sync                                                = 0;
volatile static unsigned int irq_cnt_rx_head_done                                           = 0;
#endif

static unsigned char __attribute__ ((aligned (4))) tx_buffer[64] = {0};
unsigned char                                      tx_payload[8] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xaa, 0xbb};
volatile static unsigned char                      tx_done_flag  = 0;
volatile static unsigned int                       irq_cnt_tx    = 0;

__attribute__((section(".ram_code"))) __attribute__((optimize("-Os")))
void concurrent_irq_handler(void)
{
    unsigned int irq_src = irq_get_src();
    unsigned short rf_irq_src = rf_irq_src_get();

    if (irq_src & FLD_IRQ_ZB_RT_EN) // rf irq occurs
    {
        if (rf_irq_src & FLD_RF_IRQ_RX) // rf rx irq occurs
        {
            rf_irq_clr_src(FLD_RF_IRQ_RX);
            irq_cnt_rx++;
            rx_packet = rx_buf + rx_ptr * RX_BUF_LEN;
            rx_ptr = (rx_ptr + 1) % RX_BUF_NUM;
            gen_fsk_rx_buffer_set((unsigned char *)(rx_buf + rx_ptr * RX_BUF_LEN), RX_BUF_LEN);

            if (gen_fsk_is_rx_crc_ok((unsigned char *)rx_packet))
            {
              irq_cnt_crc_ok++;
            }
            rx_flag = 1;
        }
        if (rf_irq_src & FLD_RF_IRQ_FIRST_TIMEOUT) // if rf first timeout irq occurs
        {
            rf_irq_clr_src(FLD_RF_IRQ_FIRST_TIMEOUT);
            rx_first_timeout_flag = 1;
            irq_cnt_rx_first_timeout++;
        }

        if (rf_irq_src & FLD_RF_IRQ_TX) // rf tx irq occurs
        {
            rf_irq_clr_src(FLD_RF_IRQ_TX);
            irq_cnt_tx++;
            tx_done_flag = 1;
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

void gen_fsk_config_init(void)
{
	rf_2_4g_state_reset();
    irq_bleModeFlag = 0;
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

    gen_fsk_pipe_open(GEN_FSK_PIPE0); //enable pipe0's reception

    gen_fsk_tx_pipe_set(GEN_FSK_PIPE0); //set pipe0 as the TX pipe

    gen_fsk_packet_format_set(GEN_FSK_PACKET_FORMAT_FIXED_PAYLOAD, 8);

#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    gen_fsk_radio_power_set(RF_POWER_P0p00dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_825x)
    gen_fsk_radio_power_set(RF_POWER_P0p04dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_827x)
    gen_fsk_radio_power_set(RF_POWER_P0p52dBm);
#endif

    gen_fsk_rx_buffer_set((unsigned char *)(rx_buf + rx_ptr * RX_BUF_LEN), RX_BUF_LEN);

    gen_fsk_channel_set(7); //set rf freq as 2403.5MHz

    gen_fsk_radio_state_set(GEN_FSK_STATE_AUTO);

    gen_fsk_rx_settle_set(89);

    //irq configuration
    rf_irq_disable(FLD_RF_IRQ_ALL);
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    rf_irq_enable(FLD_RF_IRQ_RX | FLD_RF_IRQ_TX | FLD_RF_IRQ_HIT_SYNC |FLD_RF_IRQ_HEADER_DONE | FLD_RF_IRQ_FIRST_TIMEOUT);
#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
    rf_irq_enable(FLD_RF_IRQ_RX | FLD_RF_IRQ_TX | FLD_RF_IRQ_FIRST_TIMEOUT);
#endif
    irq_enable_type(FLD_IRQ_ZB_RT_EN); //enable RF irq
    irq_enable(); //enable general irq
}

int app_mainloop_2p4g(unsigned int  wakeup_tick)
{
	u32 base_tick = clock_time();
	unsigned int span = (unsigned int)(wakeup_tick - CONCURRENT_THRESHOLD_TIME * sys_tick_per_us - base_tick);
	if (span > 0xE0000000)  //BIT(31)+BIT(30)+BIT(29)   7/8 cycle of 32bit, 268.44*7/8 = 234.88 S
	{
		return  0;
	}

    u32 r = irq_disable();  //must disable irq.
    gen_fsk_config_init();

    //fill the dma info in tx buffer
    tx_buffer[0] = sizeof(tx_payload);
    tx_buffer[1] = 0x00;
    tx_buffer[2] = 0x00;
    tx_buffer[3] = 0x00;
    memcpy(&tx_buffer[4], tx_payload, sizeof(tx_payload));

    //start the SRX2TX
    gen_fsk_srx2tx_start(tx_buffer, clock_time() + 50 * 16, 0);

    while (1)
    {
        if (rx_flag)
        {
            rx_flag = 0;
            rx_payload = gen_fsk_rx_payload_get((unsigned char *)rx_packet, (unsigned char *)&rx_payload_len);
            rx_rssi = gen_fsk_rx_packet_rssi_get((unsigned char *)rx_packet) + 110;
            rx_timestamp = gen_fsk_rx_timestamp_get((unsigned char *)rx_packet);
            gpio_toggle(GPIO_LED_GREEN);
        }

        if (tx_done_flag)
        {
            tx_done_flag = 0;
            //start the SRX2TX
            gen_fsk_srx2tx_start(tx_buffer, clock_time() + 50 * 16, 0);
            gpio_toggle(GPIO_LED_WHITE);
        }
    	app_idle_loop_2p4g();
        if(clock_time_exceed(base_tick, span/sys_tick_per_us))
        {

            app_exit_2p4gMode();
            break;
        }
    }

    irq_restore(r);

    return 0;
}

#endif

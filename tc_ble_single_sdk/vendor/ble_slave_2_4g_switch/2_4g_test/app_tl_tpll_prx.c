/********************************************************************************************************
 * @file    app_tl_tpll_prx.c
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

#include "tl_tpll/tl_tpll.h"

#if (TEST_2P4G_MODE==TL_TPLL_PRX)
trf_tpll_payload_t ack_payload = TRF_TPLL_CREATE_PAYLOAD(TRF_TPLL_PIPE0,
                                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                                0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20);
trf_tpll_payload_t rx_payload;

volatile unsigned int irq_cnt_invalid_pid = 0;
volatile unsigned int irq_cnt_tx_ds       = 0;
volatile unsigned int irq_cnt_tx          = 0;
volatile unsigned int irq_cnt_rx          = 0;
volatile unsigned int irq_cnt_rx_crc_ok   = 0;

//extern void trf_tpll_event_handler(trf_tpll_event_id_t evt_id);
static trf_tpll_event_handler_t m_event_handler;
unsigned char err_code          = 0;
unsigned int  evt_tx_finish_cnt = 0;
unsigned int  evt_tx_failed_cnt = 0;
unsigned int  evt_rx_cnt        = 0;

static trf_tpll_event_handler_t m_event_handler;
__attribute__((section(".ram_code"))) __attribute__((optimize("-Os")))
void irq_2p4_handler(void)
{
    unsigned short src_rf = rf_irq_src_get();
    m_event_handler = trf_tpll_get_event_handler();
    if (src_rf & FLD_RF_IRQ_RX)
    {
        rf_irq_clr_src(FLD_RF_IRQ_RX);
        if( trf_tpll_is_crc_vaild(trf_tpll_get_rx_packet()) ){
            irq_cnt_rx_crc_ok++;
        }
        trf_tpll_rxirq_handler(m_event_handler);
        irq_cnt_rx++;
    }
    if (src_rf & FLD_RF_IRQ_TX)
    {
        rf_irq_clr_src(FLD_RF_IRQ_TX);
        m_event_handler(TRF_TPLL_EVENT_TX_FINISH);
        irq_cnt_tx++;
    }
    if (src_rf & FLD_RF_IRQ_TX_DS)
    {
        rf_irq_clr_src(FLD_RF_IRQ_TX_DS);
        irq_cnt_tx_ds++;
    }
    if (src_rf & FLD_RF_IRQ_INVALID_PID)
    {
        rf_irq_clr_src(FLD_RF_IRQ_INVALID_PID);
        irq_cnt_invalid_pid++;
    }
    irq_clr_src();
    rf_irq_clr_src(FLD_RF_IRQ_ALL);
}


__attribute__((section(".ram_code"))) __attribute__((optimize("-Os")))
void trf_tpll_event_handler(trf_tpll_event_id_t evt_id)
{
    switch (evt_id)
    {
    case TRF_TPLL_EVENT_TX_FINISH:
        #if (UI_LED_ENABLE)
        gpio_toggle(GPIO_LED_GREEN);
        #endif
        evt_tx_finish_cnt++;
        break;
    case TRF_TPLL_EVENT_TX_FALIED:
        evt_tx_failed_cnt++;
        break;
    case TRF_TPLL_EVENT_RX_RECEIVED:
        trf_tpll_read_rx_payload(&rx_payload);
        #if (UI_LED_ENABLE)
        gpio_toggle(GPIO_LED_WHITE);
        #endif
        trf_tpll_flush_tx(ack_payload.pipe_id); // it is necessary in this demo
        ack_payload.data[2] = rx_payload.data[2];

        if (TRF_SUCCESS == trf_tpll_write_payload(&ack_payload))
        {

        }
        else
        {
            #if (UI_LED_ENABLE)
            gpio_toggle(GPIO_LED_RED);
            #endif
        }
        evt_rx_cnt++;
        break;
    }
}

unsigned char tpll_config_init(void)
{
    unsigned char err_code = 0;
    unsigned char base_address_0[4] = {0xe7, 0xe7, 0xe7, 0xe7};
    unsigned char base_address_1[4] = {0xc2, 0xc2, 0xc2, 0xc2};
    unsigned char addr_prefix[6] = {0xe7, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6};

    trf_tpll_config_t trf_tpll_config = TRF_TPLL_DEFALT_CONFIG;
    trf_tpll_config.mode             = TRF_TPLL_MODE_PRX;
    trf_tpll_config.bitrate          = TRF_TPLL_BITRATE_2MBPS;
    trf_tpll_config.crc              = TRF_TPLL_CRC_16BIT;
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    trf_tpll_config.tx_power         = RF_POWER_P0p00dBm;
#elif(MCU_CORE_TYPE == MCU_CORE_825x)
    trf_tpll_config.tx_power         = RF_POWER_P0p04dBm;
#elif(MCU_CORE_TYPE == MCU_CORE_827x)
    trf_tpll_config.tx_power         = RF_POWER_P0p52dBm;
#endif
    trf_tpll_config.event_handler    = trf_tpll_event_handler;
    trf_tpll_config.preamble_len     = 2;
    trf_tpll_config.payload_len      = 32;

    err_code = trf_tpll_init(&trf_tpll_config);
    // init irq
    irq_clr_src();
    rf_irq_clr_src(FLD_RF_IRQ_ALL);
    irq_enable_type(FLD_IRQ_ZB_RT_EN);
    rf_irq_disable(FLD_RF_IRQ_ALL);
    rf_irq_enable(FLD_RF_IRQ_TX | FLD_RF_IRQ_TX_DS |
#if (MCU_CORE_TYPE == MCU_CORE_TC321X)
    FLD_RF_IRQ_INVALID_PID | FLD_RF_IRQ_TX_RETRYCNT | FLD_RF_IRQ_RX);
#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
    FLD_RF_IRQ_INVALID_PID | FLD_RF_IRQ_RETRY_HIT | FLD_RF_IRQ_RX);
#endif
    irq_enable();

    TRF_RETVAL_CHECK((err_code == TRF_SUCCESS));

    trf_tpll_set_address_width(TRF_TPLL_ADDRESS_WIDTH_5BYTES);

    err_code = trf_tpll_set_base_address_0(base_address_0);
    TRF_RETVAL_CHECK((err_code == TRF_SUCCESS));

    err_code = trf_tpll_set_base_address_1(base_address_1);
    TRF_RETVAL_CHECK((err_code == TRF_SUCCESS));

    err_code = trf_tpll_set_prefixes(addr_prefix, 6);
    TRF_RETVAL_CHECK((err_code == TRF_SUCCESS));

    trf_tpll_set_txpipe(TRF_TPLL_PIPE0);

    trf_tpll_set_rf_channel(5);
    return err_code;
}

int app_mainloop_2p4g(void)
{
	unsigned char err_code;
    err_code = tpll_config_init();
    TRF_RETVAL_CHECK((err_code == TRF_SUCCESS));

    trf_tpll_enable_crcfilter(1);

    trf_tpll_start_rx();

    ack_payload.data[1] = 0xbc; // for debug
    while (1)
    {
    	app_idle_loop_2p4g();
    }

    return 0;
}

#endif

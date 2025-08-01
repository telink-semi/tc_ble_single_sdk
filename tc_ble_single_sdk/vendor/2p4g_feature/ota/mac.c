/********************************************************************************************************
 * @file    mac.c
 *
 * @brief   This is the source file for 2.4G SDK
 *
 * @author  2.4G GROUP
 * @date    02,2025
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
#include "mac.h"
#include "driver.h"
#include "tl_common.h"
#include "../../../stack/2p4g/genfsk_ll/genfsk_ll.h"
#include "../../stack/ble/debug/debug.h"

#if (FEATURE_TEST_MODE == OTA)
#define MAC_TX_BUF_LEN                64
#define MAC_RX_BUF_LEN                64
#define MAC_RX_BUF_NUM                4
#define MAC_STX_WAIT                  30 //in us
#define MAC_SRX_WAIT                  5  //in us
#define MAC_RX_WAIT                   200000 //in us

#define MAC_RX_PACKET_LENGTH_OK(p)    (p[0] == p[12]+13)
#define MAC_RX_PACKET_CRC_OK(p)       ((p[p[0]+3] & 0x01) == 0x00)

typedef struct {
    unsigned short Channel;
    MAC_Cb RxCb;
    MAC_Cb RxTimeoutCb;
    MAC_Cb RxFirstTimeoutCb;
} MAC_InfoTypeDef;

MAC_InfoTypeDef mac_Info = {0, 0, 0};

static unsigned char mac_TxBuf[MAC_TX_BUF_LEN] __attribute__ ((aligned (4))) = {};
static unsigned char mac_RxBuf[MAC_RX_BUF_LEN*MAC_RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
static unsigned char mac_RxPtr = 0;


void MAC_Init(const unsigned short Channel,
              const MAC_Cb RxCb,
              const MAC_Cb RxTimeoutCb,
              const MAC_Cb RxFirstTimeoutCb)
{
    mac_Info.Channel = Channel;
    mac_Info.RxCb = RxCb;
    mac_Info.RxTimeoutCb = RxTimeoutCb;
    mac_Info.RxFirstTimeoutCb = RxFirstTimeoutCb;

    unsigned char sync_word[4] = {0xF5, 0x28, 0xD7, 0x69};
    //generic FSK Link Layer configuratioin
    gen_fsk_datarate_set(GEN_FSK_DATARATE_2MBPS); //Note that this API must be invoked first before all other APIs
    gen_fsk_preamble_len_set(4);
    gen_fsk_sync_word_len_set(SYNC_WORD_LEN_4BYTE);

    if (sync_word_is_invalid(sync_word, 4)) {
        tlk_printf("sync_word invalid!\n");
        while(1);
    }

    gen_fsk_sync_word_set(GEN_FSK_PIPE0, sync_word); //set pipe0's sync word
    gen_fsk_pipe_open(GEN_FSK_PIPE0); //enable pipe0's reception
    gen_fsk_packet_format_set(GEN_FSK_PACKET_FORMAT_VARIABLE_PAYLOAD, 0);
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    gen_fsk_radio_power_set(RF_POWER_P0p00dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_825x)
    gen_fsk_radio_power_set(RF_POWER_N0p14dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_827x)
    gen_fsk_radio_power_set(RF_POWER_P0p52dBm);
#endif

    gen_fsk_rx_buffer_set(mac_RxBuf + mac_RxPtr*MAC_RX_BUF_LEN , MAC_RX_BUF_LEN);
    gen_fsk_channel_set(Channel);

    gen_fsk_radio_state_set(GEN_FSK_STATE_AUTO); //set transceiver to basic TX state
    gen_fsk_tx_settle_set(149);

    //irq configuration
    rf_irq_disable(FLD_RF_IRQ_ALL);
    rf_irq_enable(FLD_RF_IRQ_RX | FLD_RF_IRQ_TX | FLD_RF_IRQ_RX_TIMEOUT|FLD_RF_IRQ_FIRST_TIMEOUT);
    irq_enable_type(FLD_IRQ_ZB_RT_EN); //enable RF irq
    irq_enable(); //enable general irq
}

void MAC_SendData(const unsigned char *Payload,
                 const int PayloadLen)
{
    gen_fsk_stx2rx_start(mac_TxBuf, clock_time()+MAC_STX_WAIT*16, MAC_RX_WAIT);
    memcpy(&mac_TxBuf[5], Payload, PayloadLen); //payload
    mac_TxBuf[0] = PayloadLen + 1;
    mac_TxBuf[1] = 0x00;
    mac_TxBuf[2] = 0x00;
    mac_TxBuf[3] = 0x00;
    mac_TxBuf[4] = PayloadLen;
}

void MAC_RecvData(unsigned int TimeUs)
{
	gen_fsk_srx_start(clock_time()+MAC_SRX_WAIT*16, TimeUs);
}
extern u32 stkLog_mask;
unsigned char type_debug,rec_flag_debug;
void MAC_RxIrqHandler(void)
{
    //set next rx_buf
    unsigned char *RxPacket = mac_RxBuf + mac_RxPtr*MAC_RX_BUF_LEN;
    mac_RxPtr = (mac_RxPtr + 1) % MAC_RX_BUF_NUM;
    gen_fsk_rx_buffer_set(mac_RxBuf + mac_RxPtr*MAC_RX_BUF_LEN , MAC_RX_BUF_LEN);

    /* clear the interrupt flag */
    reg_rf_irq_status = FLD_RF_IRQ_RX;

    if (!MAC_RX_PACKET_CRC_OK(RxPacket)) {
        if (mac_Info.RxCb) {
            mac_Info.RxCb(NULL);
        }
        return;
    }
    if (mac_Info.RxCb) {
//        tlkapi_send_string_data((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] RxPacket", &RxPacket[4], 8);
        mac_Info.RxCb(&RxPacket[4]);
    }
}

void MAC_RxTimeOutHandler(void)
{
    /* clear the interrupt flag */
	reg_rf_irq_status = FLD_RF_IRQ_RX_TIMEOUT;
//	tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW]timeout %d\n",__LINE__);
    if (mac_Info.RxTimeoutCb) {
        mac_Info.RxTimeoutCb(NULL);
    }
}

void MAC_RxFirstTimeOutHandler(void)
{
    /* clear the interrupt flag */
	reg_rf_irq_status = FLD_RF_IRQ_FIRST_TIMEOUT;

    if (mac_Info.RxFirstTimeoutCb) {
        mac_Info.RxFirstTimeoutCb(NULL);
    }
}
#endif

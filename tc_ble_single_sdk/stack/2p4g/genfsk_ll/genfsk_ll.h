/********************************************************************************************************
 * @file    genfsk_ll.h
 *
 * @brief   This is the header file for 2.4G
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
#ifndef _GEN_FSK_LL_H_
#define _GEN_FSK_LL_H_

#include "drivers.h"

/** An enum describing the radio's sync word length.
 *
 */
typedef enum {
    SYNC_WORD_LEN_3BYTE = 3,      /**< Set  length to 3 bytes */
    SYNC_WORD_LEN_4BYTE,          /**< Set  length to 4 bytes */
    SYNC_WORD_LEN_5BYTE           /**< Set  length to 5 bytes */
} gen_fsk_sync_word_len_t;

/** An enum describing the pipe IDs.
 *
 */
typedef enum {
    GEN_FSK_PIPE0 = 0,          /**< Select pipe0 */
    GEN_FSK_PIPE1,              /**< Select pipe1 */
    GEN_FSK_PIPE2,              /**< Select pipe2 */
    GEN_FSK_PIPE3,              /**< Select pipe3 */
    GEN_FSK_PIPE4,              /**< Select pipe4 */
    GEN_FSK_PIPE5,              /**< Select pipe5 */
    GEN_FSK_PIPE_ALL = 0x3F     /**< Close or open all pipes*/
} gen_fsk_pipe_id_t;

/** An enum describing the radio's crc length.
 *
 */
typedef enum {
    CRC_DISABLE = 0,          /**< disable crc */
    CRC_1BYTE,                /**< 1byte crc */
    CRC_2BYTE                 /**< 2byte crc */
} gen_fsk_crc_len_t;

/**@brief RF modulation index. */
typedef enum {
    GEN_FSK_RF_MI_0000 = 0,           /**< MI = 0 */
    GEN_FSK_RF_MI_0076 = 76,          /**< MI = 0.076 This gear is only available in private mode*/
    GEN_FSK_RF_MI_0320 = 320,         /**< MI = 0.32 */
    GEN_FSK_RF_MI_0500 = 500,         /**< MI = 0.5 */
    GEN_FSK_RF_MI_0600 = 600,         /**< MI = 0.6 */
    GEN_FSK_RF_MI_0700 = 700,         /**< MI = 0.7 */
    GEN_FSK_RF_MI_0800 = 800,         /**< MI = 0.8 */
    GEN_FSK_RF_MI_0900 = 900,         /**< MI = 0.9 */
    GEN_FSK_RF_MI_1200 = 1200,        /**< MI = 1.2 */
    GEN_FSK_RF_MI_1300 = 1300,        /**< MI = 1.3 */
    GEN_FSK_RF_MI_1400 = 1400,        /**< MI = 1.4 */
}GEN_MIVauleTypeDef;

/** An enum describing the radio's state.
 *
 */
typedef enum {
    GEN_FSK_STATE_TX = 0,      //config RF transceiver in TX state
    GEN_FSK_STATE_RX,      //config RF transceiver in RX state
    GEN_FSK_STATE_AUTO,    //enable the automatic Single TX/RX/TX2RX/RX2TX FSM engine
    GEN_FSK_STATE_OFF = 0xff, //close the RF transceiver
} gen_fsk_state_t;

/** An enum describing the radio's routine mode.
 *
 */
typedef enum {
	GEN_FSK_MD_TX ,         //gen_fsk_tx
	GEN_FSK_MD_RX,          //gen_fsk_rx
	GEN_FSK_MD_STX,         //gen_fsk_stx
	GEN_FSK_MD_SRX,         //gen_fsk_srx
	GEN_FSK_MD_SRX2TX,      //gen_fsk_srx2rx
	GEN_FSK_MD_STX2RX,      //gen_fsk_stx2tx
} gen_fsk_mode_t;
extern volatile  gen_fsk_mode_t gen_fsk_current_mode;

/** An enum describing the packet format of on-air data.
 *
 */
typedef enum {
    GEN_FSK_PACKET_FORMAT_FIXED_PAYLOAD = 0x03, //preamble + sync word + payload + crc, payload is of fixed-length
    GEN_FSK_PACKET_FORMAT_VARIABLE_PAYLOAD = 0x02 //preamble + sync word + header + payload + crc, payload is of variable-length
} gen_fsk_packet_format_t;

//extern void gen_fsk_RadioConfigRxFreDev(rx_freq_dev_t mi);

//extern void gen_fsk_RadioConfigTxFreDev(tx_freq_dev_t mi);

/**
 * @brief      This function servers to set the radio's TX output power.
 * @param[in]  level  specify the tx power level
 * @param[out] none
 * @return     none.
 */
extern void gen_fsk_radio_power_set(RF_PowerTypeDef level);

/**
 * @brief      This function servers to set radio's on-air datarate.
 * @param[in]  datarate  specify the radio's datarate
 * @param[out] none
 * @return     none.
 */
extern void gen_fsk_datarate_set(gen_fsk_datarate_t datarate);

/**
 * @brief      This function servers to set radio's RF channel.
 *             rf central frequency = (2400 + channel_num*0.5) MHz
 * @param[in]  channel_num  specify the radio's channel
 * @param[out] none
 * @return     none.
 */
extern void gen_fsk_channel_set(signed short channel_num);

/**
 * @brief      This function servers to set rf transceiver's operation state.
 * @param[in]  channel_num  specify the radio's channel
 * @param[out] none
 * @return     none.
 */
extern void gen_fsk_radio_state_set(gen_fsk_state_t state);

/**
 * @brief      This function servers to set the length in octet of the preamble
 *             field of the on-air data packet.Note that the valid range is 1-16.
 * @param[in]  preamble_len  specify the length of preamble field in octet.
 * @param[out] none
 * @return     none.
 */
extern void gen_fsk_preamble_len_set(unsigned char preamble_len);


/**
 * @brief      This function servers to set the length in octet of the sync word
 *             field of the on-air data packet.
 * @param[in]  length  specify the length of sync word field in octet, note that
 *                     the valid range of it is 3-5.
 * @param[out] none
 * @return     none.
 */
extern void gen_fsk_sync_word_len_set(gen_fsk_sync_word_len_t length);

/**
 * @brief      This function servers to set the length in octet of the sync word
 *             field of the on-air data packet.
 * @param[in]  pipe  specify which pipe the sync word is to set for
 * @param[in]  sync_word  pointer to the sync word needs to be set
 * @param[out] none
 * @return     none.
 */
extern void gen_fsk_sync_word_set(gen_fsk_pipe_id_t pipe, unsigned char * sync_word);

/**
 * @brief      This function servers to open one or all pipes.
 *             field of the on-air data packet.
 * @param[in]  pipe  specify which pipe needs to be open
 * @param[out] none
 * @return     none.
 */
extern void gen_fsk_pipe_open(gen_fsk_pipe_id_t pipe);

/**
 * @brief      This function servers to close one or all pipes.
 *             field of the on-air data packet.
 * @param[in]  pipe  specify which pipe needs to be closed
 * @param[out] none
 * @return     none.
 */
extern void gen_fsk_pipe_close(gen_fsk_pipe_id_t pipe);

/**
 * @brief      This function servers to set TX pipe manually
 * @param[in]  pipe  The pipe is selected as TX pipe
 * @param[out] none
 * @return     none.
 */
extern void gen_fsk_tx_pipe_set(gen_fsk_pipe_id_t pipe);

/**
 * @brief      This function servers to set the paket format of on-air data.
 * @param[in]  format
 *                    -----------------------------------------------------------------------
 *                    |GEN_FSK_PACKET_FORMAT_FIXED_PAYLOAD:                                 |
 *                    |preamble + sync word + payload + crc, payload is of fixed-length     |
 *                    -----------------------------------------------------------------------
 *                    -----------------------------------------------------------------------
 *                    |GEN_FSK_PACKET_FORMAT_VARIABLE_PAYLOAD:                              |
 *                    |preamble + sync word + header + payload + crc, the header contains   |
 *                    |a length field indicating the length of payload                      |
 *                    -----------------------------------------------------------------------
 * @param[in]  payload_len if the GEN_FSK_PACKET_FORMAT_FIXED_PAYLOAD format is selected,
 *                         this parameter specifies the fixed length of payload, otherwise,
 *                         this parameter should be ignored
 * @param[out] none
 * @return     none.
 */
extern void gen_fsk_packet_format_set(gen_fsk_packet_format_t format, unsigned char payload_len);

/**
 * @brief      This function sets the length of CRC field
 * @param[in]  crc_len  length of CRC field
 * @param[out] none
 * @return     none.
 */
extern void gen_fsk_crc_len_set(gen_fsk_crc_len_t crc_len);

/**
 * @brief      This function sets DMA RX buffer. Note that the start address of RX
 *             buffer must be 4-byte aligned and the length of RX buffer must be
 *             integral multiple of 16-byte. Also the RX buffer length must be larger
 *             than the whole packet length plus 16 to accommodate data and necessary
 *             RX related information.
 * @param[in]  rx_buffer  pointer to the RX buffer
 * @param[in]  rx_buffer_len  length of RX buffer in octet
 * @param[out] none
 * @return     none.
 */
extern void gen_fsk_rx_buffer_set(unsigned char *rx_buffer, unsigned char rx_buffer_len);

/**
 * @brief      This function determines whether the result of RX CRC check is ok.
 * @param[in]  rx_buffer  pointer to the RX buffer containing currently received packet.
 * @param[out] none
 * @return     1: the RX CRC check passes. 0: the RX CRC check fails.
 */
extern unsigned char gen_fsk_is_rx_crc_ok(unsigned char *rx_buffer);

/**
 * @brief      This function returns the pointer to the payload field of received packet.
 * @param[in]  rx_buffer  pointer to the RX buffer containing currently received packet.
 * @param[out] payload_len return the length of received payload.
 * @return     pointer to the the payload field of received packet.
 */
extern unsigned char *gen_fsk_rx_payload_get(unsigned char *rx_buffer, unsigned char *payload_len);

/**
 * @brief      This function returns the freezing rssi of received packet.
 * @param[in]  rx_buffer  pointer to the RX buffer containing currently received packet.
 * @param[out] none
 * @return     rssi in dBm of the received packet.
 */
extern signed char gen_fsk_rx_packet_rssi_get(unsigned char *rx_buffer);

/**
 * @brief      This function returns the instantaneous rssi of current channel.
 * @param[in]  none
 * @param[out] none
 * @return     instantaneous rssi in dBm of current channel.
 */
extern signed char gen_fsk_rx_instantaneous_rssi_get(void);

/**
 * @brief      This function returns the timestamp of received packet.
 * @param[in]  rx_buffer  pointer to the RX buffer containing currently received packet.
 * @param[out] none
 * @return     timestamp of the received packet, i.e. the freezing system-timer tick count
 *             value once the sync word is recongnized.
 */
extern unsigned int gen_fsk_rx_timestamp_get(unsigned char *rx_buffer);

/**
 * @brief      This function manually starts the transmission of a prepared
 *             packet(i.e. the packet has been written into the TX Buffer) immediately.
 * @param[in]  tx_buffer  pointer to the TX buffer which has been filled with the packet needs transmitting.
 * @param[out] none
 * @return     none
 */
extern void gen_fsk_tx_start(unsigned char *tx_buffer);

/**
*   @brief      This function determines whether the transmission activity started by gen_fsk_tx_start() is finished.
*   @param[in]  none.
*   @return     0: tx activity is not done. otherwhise: tx is done
*/
extern unsigned char gen_fsk_is_tx_done(void);

/**
*   @brief      This function serves to clear the Tx done status bit once it is set.
*
*   @param      none
*   @return     none
*/
extern void gen_fsk_tx_done_status_clear(void);

/**
 * @brief      This function sets the tx settle period of transceiver for automatic Single-TX , Single-TX-to-RX and Single-RX-to-TX.
 *             In those three automatic modes, the actual transmission starts a short while later after the transceiver enters TX
 *             state. That short while is so-called tx settle period which is used to wait for the RF PLL settling down before
 *             transmission. The TX settle period must be larger than 130uS. The default value is 150uS.
 * @param[in]  period_us  specifying the TX settle period in microsecond.
 * @param[out] none
 * @return     none
 */
 void gen_fsk_tx_settle_set(unsigned short period_us);

/**
 * @brief      This function sets the rx settle period of transceiver for automatic Single-RX, Single-RX-to-TX and Single-TX-to-RX.
 *             In those three automatic modes, the actual reception starts a short while later after the transceiver enters RX
 *             state. That short while is so-called rx settle period which is used to wait for the RF PLL settling down before
 *             reception. The RX settle period must be larger than 85uS. The default value is 90uS.
 * @param[in]  period_us  specifying the RX settle period in microsecond.
 * @param[out] none
 * @return     none
 */
 void gen_fsk_rx_settle_set(unsigned short period_us);

/**
 * @brief      This function sets the tx wait period of transceiver for automatic Single-RX-to-TX mode. In that mode,
 *             after the reception the transceiver will wait for a short while before transitting into TX state.
 *             That short while is so-called tx wait period which can be used by APP to parse the received packet and
 *             prepare the Acknowledge Packet.
 * @param[in]  period_us  specifying the tx wait period in microsecond.
 * @param[out] none
 * @return     none
 */
extern void gen_fsk_tx_wait_set(unsigned short period_us);

/**
 * @brief      This function sets the rx wait period of transceiver for automatic Single-TX-to-RX mode. In that mode,
 *             after the trasmission the transceiver will wait for a short while before transitting into RX state.
 *             That short while is so-called tx wait period which can be used by APP to wait for the tx peer's transmission.
 * @param[in]  period_us  specifying the rx wait period in microsecond.
 * @param[out] none
 * @return     none
 */
extern void gen_fsk_rx_wait_set(unsigned short period_us);

/**
 * @brief      This function starts the automatic Single-TX FSM to schedule one transmission activity.
 * @param[in]  tx_buffer  pointer to the TX buffer which has been filled with the packet needs transmitting.
 * @param[in]  start_point  a timer tick count value, when the system-timer's tick count matches this value,
 *                          the transmission activity starts immediately.
 * @param[out] none
 * @return     none
 */
extern void gen_fsk_stx_start(unsigned char *tx_buffer, unsigned int start_point);

/**
 * @brief      This function starts the automatic Single-RX FSM to schedule one reception activity.
 * @param[in]  start_point  a timer tick count value, when the system-timer's tick count matches this value,
 *                          the transceiver enters RX state and waits for a packet coming.
 * @param[in]  timeout_us  specifies the period the transceiver will remain in RX state to waits for
 *                         a packet coming, and a RX first timeout irq occurs if no packet is received
 *                         during that period. If timeout_us equals to 0, it denotes the RX first timeout is
 *                         disabled and the transceiver won't exit the RX state until a packet arrives. Note
 *                         that RX settle period is included in timeout_us, namely:
 *                         timeout_us = Rx settle period + Actual Rx period.
 * @param[out] none
 * @return     none
 */
extern void gen_fsk_srx_start(unsigned int start_point, unsigned int timeout_us);

/**
 * @brief      This function starts the automatic Single-TX-to-RX FSM to schedule one transmission and
 *             corresponding reception activity. The transceiver enters TX state and start the packet's
 *             transmission when the system timer matches the start_point. Once the trannsmission is done,
 *             the transceiver transits into RX State and waits for a packet coming. The RX state lasts for
 *             timeout_us microseconds at most. If no packet arrives during that period, a RX Timeout irq
 *             will occur immediately. It usually applies to the case where a packet needs transmitting
 *             with an ACK packet required.
 * @param[in]  tx_buffer  pointer to the TX buffer which has been filled with the packet needs transmitting.
 * @param[in]  start_point  a timer tick count value, when the system-timer's tick count matches this value,
 *                          the transceiver transits into TX state and starts the trannsmission of the packet
 *                          in TX Buffer.
 * @param[in]  timeout_us  specifies the period the transceiver will remain in RX state to waits for
 *                         a packet coming, and a RX timeout irq occurs if no packet is received
 *                         during that period. Note that RX settle period is included in timeout_us,
 *                         namely:
 *                         timeout_us = Rx settle period + Actual Rx period.
 * @param[out] none
 * @return     none
 */
extern void gen_fsk_stx2rx_start(unsigned char *tx_buffer, unsigned int start_point, unsigned int timeout_us);

/**
 * @brief      This function starts the automatic Single-RX-to-TX FSM to schedule one reception and
 *             corresponding transmission activity. The transceiver enters RX state and waits for a
 *             packet coming. The RX state lasts for timeout_us microseconds at most. If no packet arrives,
 *             a RX First Timeout irq will occur immediately. If a packet is received before the timeout,
 *             the transceiver transits into TX state and start the transmission of the packet in TX Buffer.
 *             It usually applies to the case where a packet needs receiving first and then a response packet
 *             needs sending back.
 * @param[in]  tx_buffer  pointer to the TX buffer which has been filled with the packet needs transmitting.
 * @param[in]  start_point  a timer tick count value, when the system-timer's tick count matches this value,
 *                          the transceiver transits into RX state and waits for a packet coming.
 * @param[in]  timeout_us  specifies the period the transceiver will remain in RX state to waits for
 *                         a packet coming, and a RX first timeout irq occurs if no packet is received
 *                         during that period. If timeout_us equals to 0, it denotes the RX first timeout is
 *                         disabled and the transceiver won't exit the RX state until a packet arrives. Note
 *                         that RX settle period is included in timeout_us, namely:
 *                         timeout_us = Rx settle period + Actual Rx period.
 * @param[out] none
 * @return     none
 */
extern void gen_fsk_srx2tx_start(unsigned char *tx_buffer, unsigned int start_point, unsigned int timeout_us);

/**
 * @brief      This function disables the auto pid feature, which is necessary to set the pid manuallly.
 *
 * @param[in]  none
 * @param[out] none
 * @return     none
 */
void gen_fsk_auto_pid_disable(void);

/**
 * @brief      This function serves to set the pid field(2bit) manually in the packet header(9bit) of DPL-
 *             format packet.
 * @param[in]  tx_buffer  pointer to the TX buffer which has been filled with the packet needs transmitting.
 * @param[in]  pid  the valule of the pid needs to be set. 0 =< pid <=3.
 * @param[out] none
 * @return     none
 */
void gen_fsk_set_pid(unsigned char *tx_buffer, unsigned char pid);

/**
 * @brief       Set the frequency deviation of the transmitter, which follows the equation below.
 *              modulation index = frequency deviation*2/bitrate
 * @param       mi_value    Modulation index.
 * @return      none.
 */
void gen_fsk_tx_set_mi(GEN_MIVauleTypeDef mi_value);

/**
 * @brief       Set the frequency deviation of the receiver, which follows the equation below.
 *              modulation index = frequency deviation*2/bitrate
 * @param       mi_value    Modulation index.
 * @return      none.
 */
void gen_fsk_rx_set_mi(GEN_MIVauleTypeDef mi_value);
#endif /* _GEN_FSK_LL_H_ */

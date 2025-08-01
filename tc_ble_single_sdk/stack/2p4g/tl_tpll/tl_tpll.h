/********************************************************************************************************
 * @file    tl_tpll.h
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
#ifndef _TL_TPLL_H_
#define _TL_TPLL_H_

#include "tl_tpll_util.h"

#define trf_delay_us                    sleep_us
#define trf_delay_ms(t)                 sleep_us((t)*1000)

#define TRF_SUCCESS                     0
#define TRF_ERROR_NULL_PARAM            1
#define TRF_ERROR_INVALID_PARAM         2
#define TRF_ERROR_BUSY                  3
#define TRF_ERROR_INVALID_STATE         4

#define TRF_RX_WAIT_US                  5
#define TRF_TX_WAIT_US                  5
#define TRF_RX_SETTLE_US                114
#define TRF_TX_SETTLE_US                114
#define TRF_RX_WAIT_ACK_TIMEOUT_US      500 // default 500us
// if the mode is 250k ,the rx_time_out need more time, as so 1000us is ok!

#define TRF_TPLL_RX_WAIT_TIME_MAX        (4096)
#define TRF_TPLL_TX_WAIT_TIME_MAX        (4096)

#define TRF_TPLL_RX_SETTLE_TIME_MIN      (85)
#define TRF_TPLL_RX_SETTLE_TIME_MAX      (4095)

#define TRF_TPLL_TX_SETTLE_TIME_MIN      (108)
#define TRF_TPLL_TX_SETTLE_TIME_MAX      (4095)

#define TRF_TPLL_RX_TIMEOUT_TIME_MIN     (85)
#define TRF_TPLL_RX_TIMEOUT_TIME_MAX     (4095)

#define TRF_TPLL_MAX_PAYLOAD_LENGTH      64

#ifndef TRF_TPLL_PIPE_COUNT
#define TRF_TPLL_PIPE_COUNT              6
#endif
STATIC_ASSERT(TRF_TPLL_PIPE_COUNT <= 6);

#define TRF_TPLL_CREATE_PAYLOAD(_pipeid, ...)                    \
{.pipe_id = _pipeid, .length = TRF_NUM_VA_ARGS(__VA_ARGS__),    \
.data = {__VA_ARGS__}};                                         \
STATIC_ASSERT((TRF_NUM_VA_ARGS(__VA_ARGS__) > 0 && TRF_NUM_VA_ARGS(__VA_ARGS__) <= 64))

#define TRF_TPLL_ADDR_DEFALT                                     \
{                                                               \
    .base_address_0     = {0xe7, 0xe7, 0xe7, 0xe7},             \
    .base_address_1     = {0xc2, 0xc2, 0xc2, 0xc2},             \
    .pipe_prefixes      = {0xe7, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6}, \
    .addr_length        = 5,                                    \
    .pipe_nums          = 6                                     \
}
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
#define TRF_TPLL_DEFALT_CONFIG                                   \
{                                                               \
    .mode               = TRF_TPLL_MODE_PTX,                     \
    .bitrate            = TRF_TPLL_BITRATE_2MBPS,                \
    .crc                = TRF_TPLL_CRC_16BIT,                    \
    .tx_power           = RF_POWER_P0p00dBm,                   \
    .event_handler      = 0,                                    \
    .retry_delay        = 150,                                  \
    .retry_times        = 5,                                    \
    .preamble_len       = 8,                                    \
    .payload_len        = 32                                    \
}
#elif(MCU_CORE_TYPE == MCU_CORE_825x)
#define TRF_TPLL_DEFALT_CONFIG                                   \
{                                                               \
    .mode               = TRF_TPLL_MODE_PTX,                     \
    .bitrate            = TRF_TPLL_BITRATE_2MBPS,                \
    .crc                = TRF_TPLL_CRC_16BIT,                    \
    .tx_power           = RF_POWER_P0p04dBm,                   \
    .event_handler      = 0,                                    \
    .retry_delay        = 150,                                  \
    .retry_times        = 5,                                    \
    .preamble_len       = 8,                                    \
    .payload_len        = 32                                    \
}
#elif(MCU_CORE_TYPE == MCU_CORE_827x)
#define TRF_TPLL_DEFALT_CONFIG                                   \
{                                                               \
    .mode               = TRF_TPLL_MODE_PTX,                     \
    .bitrate            = TRF_TPLL_BITRATE_2MBPS,                \
    .crc                = TRF_TPLL_CRC_16BIT,                    \
    .tx_power           = RF_POWER_P0p52dBm,                   \
    .event_handler      = 0,                                    \
    .retry_delay        = 150,                                  \
    .retry_times        = 5,                                    \
    .preamble_len       = 8,                                    \
    .payload_len        = 32                                    \
}
#endif
/**@brief Telink primary link layer address width. */
typedef enum {
    TRF_TPLL_ADDRESS_WIDTH_3BYTES = 3,      /**< Set address width to 3 bytes */
    TRF_TPLL_ADDRESS_WIDTH_4BYTES,          /**< Set address width to 4 bytes */
    TRF_TPLL_ADDRESS_WIDTH_5BYTES           /**< Set address width to 5 bytes */
} trf_tpll_address_width_t;

/**@brief Telink primary link layer pipe IDs. */
typedef enum {
    TRF_TPLL_PIPE0 = 0,          /**< Select pipe0 */
    TRF_TPLL_PIPE1,              /**< Select pipe1 */
    TRF_TPLL_PIPE2,              /**< Select pipe2 */
    TRF_TPLL_PIPE3,              /**< Select pipe3 */
    TRF_TPLL_PIPE4,              /**< Select pipe4 */
    TRF_TPLL_PIPE5,              /**< Select pipe5 */
    TRF_TPLL_TX,                 /**< Refer to TX address*/
    TRF_TPLL_PIPE_ALL = 0xFF     /**< Close or open all pipes*/
} trf_tpll_pipeid_t;

/**@brief Telink primary link layer state machine status. */
typedef enum {
    TRF_TPLL_STATE_MACHINE_STATUS_IDLE = 0,          /**< Idle */
    TRF_TPLL_STATE_MACHINE_STATUS_TX_SETTLE,         /**< TX Settle*/
    TRF_TPLL_STATE_MACHINE_STATUS_TX,                /**< TX */
    TRF_TPLL_STATE_MACHINE_STATUS_RX_WAIT,           /**< RX Wait */
    TRF_TPLL_STATE_MACHINE_STATUS_RX,                /**< RX */
    TRF_TPLL_STATE_MACHINE_STATUS_TX_WAIT,           /**< RX Wait */
} trf_state_machine_status_t;

typedef enum {
    TRF_TPLL_MODE_PTX = 0,
    TRF_TPLL_MODE_PRX
} trf_tpll_mode_t;

typedef enum {
    TRF_TPLL_BITRATE_1MBPS = 0,      /**< 1Mbit radio mode. */
    TRF_TPLL_BITRATE_2MBPS,          /**< 2Mbit radio mode. */
    TRF_TPLL_BITRATE_500KBPS,        /**< 500kbit radio mode. */
    TRF_TPLL_BITRATE_250KBPS,        /**< 250Kbit radio mode. */
} trf_tpll_bitrate_t;

/**@brief Telink primary link layer modulation index. */
typedef enum {
	TRF_TPLL_MI_P0p00 = 0,        /**< MI = 0 */
	TRF_TPLL_MI_P0p076 = 76,	    /**< MI = 0.076 This gear is only available in private mode*/
	TRF_TPLL_MI_P0p32 = 320,		/**< MI = 0.32 */
	TRF_TPLL_MI_P0p50 = 500,		/**< MI = 0.5 */
	TRF_TPLL_MI_P0p60 = 600,		/**< MI = 0.6 */
	TRF_TPLL_MI_P0p70 = 700,		/**< MI = 0.7 */
	TRF_TPLL_MI_P0p80 = 800,		/**< MI = 0.8 */
	TRF_TPLL_MI_P0p90 = 900,		/**< MI = 0.9 */
	TRF_TPLL_MI_P1p20 = 1200,		/**< MI = 1.2 */
	TRF_TPLL_MI_P1p30 = 1300,		/**< MI = 1.3 */
	TRF_TPLL_MI_P1p40 = 1400,		/**< MI = 1.4 */
} trf_tpll_mi_t;

typedef enum {
    TRF_TPLL_CRC_16BIT = 0,
    TRF_TPLL_CRC_8BIT,
} trf_tpll_crc_t;

typedef enum {
    TRF_TPLL_EVENT_TX_FINISH,    // transmit finished
    TRF_TPLL_EVENT_TX_FALIED,    // retransmit reached threshold
    TRF_TPLL_EVENT_RX_RECEIVED,  // receive a new-valid-packet
} trf_tpll_event_id_t;

typedef void (*trf_tpll_event_handler_t)(trf_tpll_event_id_t evt_id);

typedef struct {
    trf_tpll_mode_t mode;
    trf_tpll_bitrate_t bitrate;
    trf_tpll_crc_t crc;
    RF_PowerTypeDef tx_power;
    trf_tpll_event_handler_t event_handler;
    unsigned short retry_delay;
    unsigned char retry_times;
    unsigned char preamble_len;
    unsigned char payload_len;
} trf_tpll_config_t;

typedef struct {
    unsigned char length;
    unsigned char pipe_id;
    unsigned char noack;
    unsigned char pid;
    signed int rssi;
    unsigned char data[TRF_TPLL_MAX_PAYLOAD_LENGTH];
} trf_tpll_payload_t;

/**
 * @brief       Initiate the the Telink primary link layer module.
 * @note        This function must be called at the beginning of the TPLL configuration.
 * @param       bitrate  Radio bitrate.
 * @return      error code for init result.
 */
unsigned char trf_tpll_init(trf_tpll_config_t *tpll_config);

/**
 * @brief       Set the radio bitrate.
 * @param       bitrate  Radio bitrate.
 * @return      error code for init result.
 */
unsigned char trf_tpll_set_bitrate(trf_tpll_bitrate_t bitrate);

/**
 * @brief       Set the channel to use for the radio.
 * @param       channel Channel to use for the radio.
 * @return      none.
 */
void trf_tpll_set_rf_channel(unsigned char channel);

/**
 * @brief       Set the new channel to use for the radio.
 * @param       channel New channel to use for the radio.
 * @return      none.
 */
void trf_tpll_set_new_rf_channel(unsigned char channel);

/**
 * @brief       Set the radio output power.
 * @param       power   Output power.
 * @return      none.
  */
void trf_tpll_set_txpower(RF_PowerTypeDef power);

/**
 * @brief       Set one pipe as a TX pipe.
 * @param       pipe_id Pipe to be set as a TX pipe.
 * @return      none.
 */
void trf_tpll_set_txpipe(trf_tpll_pipeid_t pipe_id);

/**
 * @brief       Get the current TX pipe.
 * @param       none.
 * @return      The pipe set as a TX pipe.
*/
unsigned char trf_tpll_get_txpipe(void);

/**
 * @brief       Get the current TX pipe.
 * @param       none.
 * @return      The pipe set as a TX pipe.
*/
unsigned char trf_tpll_get_txpipe(void);

/**
 * @brief       Update the read pointer of the TX FIFO.
 * @param       pipe_id Pipe id.
 * @return      none.
 */
void trf_tpll_update_txfifo_rptr(trf_tpll_pipeid_t pipe_id);

/**
 * @brief       Open one or all pipes.
 * @param       pipe_id Radio pipes to open.
 * @return      none.
 */
void trf_tpll_open_pipe(trf_tpll_pipeid_t pipe_id);

/**
 * @brief       Close one or all pipes.
 * @param       pipe_id Radio pipes to close.
 * @return      none.
 */
void trf_tpll_close_pipe(trf_tpll_pipeid_t pipe_id);

/**
 * @brief       Set the address for pipes.
 *              Beware of the difference for single and multibyte address registers
 * @param       pipe_id Radio pipe to set.
 * @param       addr    Buffer from which the address is stored in
 * @return      /
 */
void trf_tpll_set_address(trf_tpll_pipeid_t pipe_id, const unsigned char *addr);
/**
 * @brief       Get the address for selected pipe.
 * @param       pipe_id Pipe for which to get the address.
 * @param       addr    Pointer t a buffer that address bytes are written to.
 *               <BR><BR>For pipes containing only LSB byte of address, this byte is returned.
 * @return      Numbers of bytes copied to addr.
 */
unsigned char trf_tpll_get_address(trf_tpll_pipeid_t pipe_id, unsigned char *addr);

/**
 * @brief       Set the width of the address.
 * @param       address_width   Width of the TPLL address (in bytes).
 * @return      none.
 */
unsigned char trf_tpll_set_address_width(trf_tpll_address_width_t address_width);
/**
 * @brief       Get the width of the address.
 * @param       none.
 * @return      Width of the TPLL address (in bytes).
 */
unsigned char trf_tpll_get_address_width(void);

/**
 * @brief       Set the the number of retransmission attempts and the packet retransmit delay.
 * @param       retr_times  Number of retransmissions. Setting the parmater to 0 disables retransmission.
 * @param       retry_delay Delay between retransmissions.
 * @return      none.
 */
void trf_tpll_set_auto_retry(unsigned char retry_times, unsigned short retry_delay);

/**
 * @brief       Check status for a selected pipe.
 * @param       pipe_id Pipe number to check status for.
 * @return      Pipe status.
 */
unsigned char trf_tpll_get_pipe_status(trf_tpll_pipeid_t pipe_id);

/**
 * @brief       Get the packet(s) counter.
 * @param       none.
 * @return      packet lost counter.
 */
unsigned char trf_tpll_get_packet_lost_ctr(void);

/**
 * @brief       Check if the TX FIFO is empty.
 * @param       pipe_id pipe id for which to check.
 * @return      1: the TX FIFO is empty; 0: the packet is not empty.
 */
unsigned char trf_tpll_txfifo_empty(trf_tpll_pipeid_t pipe_id);

/**
 * @brief       Check if TX FIFO is full.
 * @param       pipe_id pipe id for which to check.
 * @return      TRUE TX FIFO empty or not.
 */
unsigned char trf_tpll_txfifo_full(trf_tpll_pipeid_t pipe_id);

/**
 * @brief       Get the number of retransmission attempts.
 * @return      Number of retransmissions.
 */
unsigned char trf_tpll_get_transmit_attempts(void);

/**
 * @brief       Read an RX payload.
 * @param       rx_pload   Pointer to buffer where the RX payload is stored.
 * @return      pipe number (MSB byte) and packet length (LSB byte).
 */
unsigned short trf_tpll_read_rx_payload(trf_tpll_payload_t *p_payload);

/**
 * @brief       Get the RX timestamp.
 * @note        It is required to call trf_tpll_read_rx_payload() before this function is called.
 * @return      RX timestamp.
 */
unsigned int trf_tpll_get_timestamp(void);

/**
 * @brief       Write a packet of TX payload into the radio.
 * @param       payload     TX payload.
 * @return      the length of payload written in to tx-fifo.
 * @retval      return 0  :error
 * @retval      return >0 :success, the length of written bytes
 */
unsigned char trf_tpll_write_payload(trf_tpll_payload_t *p_payload);

/**
 * @brief       Reuse the last transmitted payload for the next packet.
 * @param       pipe_id pipe id.
 * @return      none.
 */
void trf_tpll_reuse_tx(trf_tpll_pipeid_t pipe_id);

/**
 * @brief       Remove remaining items from the TX buffer.
 * @param       pipe_id Pipe id.
 * @return      none.
 */
void trf_tpll_flush_tx(trf_tpll_pipeid_t pipe_id);

/**
 * @brief       Remove remaining items from the RX buffer.
 * @param       none.
 * @return      none.
 */
void trf_tpll_flush_rx(void);

/**
 * @brief       Trigger the transmission in the specified pipe
 * @param       none.
 * @return      none.
 */
int trf_tpll_start_tx(void);

/**
 * @brief       Trigger transmission in the specified pipe.
 * @retval      TRF_SUCCESS                If the operation completed successfully.
 * @retval      TRF_ERROR_BUSY             If the function failed because the radio was busy.
 */
int trf_tpll_start_rx(void);

/**
 * @brief       Set the TX wait time.
 * @param       wait_us   Period in microseconds.
 * @return      none.
 */
int trf_tpll_set_tx_wait(unsigned short wait_us);

/**
 * @brief       Set the wait time between the end of an Ack-required packet's transmission
 *              and the start of Ack receiving to accommodate with another chip.
 * @param       wait_us Wait time between the end of an Ack-required packet's transmission
 *              and the start of Ack receiving.
 * @return      none.
 */
int trf_tpll_set_rx_wait(unsigned short wait_us);

/**
 * @brief       Set the rx duration when an Ack-required packet has been
 *              transmitted and an Ack is expected.
 * @param       period_us   specifies the time of rx duration.
 * @return      none.
 */
int trf_tpll_set_rx_timeout(unsigned short period_us);

/**
 * @brief       Set the TX Settle phase.
 * @param       period_us   specifies the time.
 * @return      none.
 */
int trf_tpll_set_tx_settle(unsigned short period_us);

/**
 * @brief       Set the RX Settle phase.
 * @param       period_us   specifies the time.
 * @return      none.
 */
int trf_tpll_set_rx_settle(unsigned short period_us);

/**
 * @brief       Set the mode of TPLL radio.
 * @param       mode    TPLL_MODE_PTX or TPLL_MODE_PRX.
 * @return      none.
 */
void trf_tpll_set_mode(trf_tpll_mode_t mode);

/**
 * @brief       Stop the TPLL state machine.
 * @param       none.
 * @return      none.
 */
void trf_tpll_disable(void);

/**
 * @brief       Set the frequency deviation of the transmitter, which follows the equation below.
 *              frequency deviation = bitrate/(modulation index)^2
 * @param       mi_value    Modulation index.
 * @return      none.
 */
void trf_tpll_set_txmi(trf_tpll_mi_t mi_value);

/**
 * @brief       Set the frequency deviation of the transmitter, which follows the equation below.
 *              frequency deviation = bitrate/(modulation index)^2
 * @param       mi_value    Modulation index.
 * @return      none.
 */
void trf_tpll_set_rxmi(trf_tpll_mi_t mi_value);

/**
 * @brief      Set the length of the preamble field of the on-air data packet.
 * @note       The valid range of this parameter is 1-16.
 * @param      preamble_len Preamble length.
 * @return     error code for result.
 */
unsigned char trf_tpll_set_preamble_len(unsigned char preamble_len);

/**
 * @brief      Read the length of the preamble field of the on-air data packet.
 * @return     Preamble length.
 */
unsigned char trf_tpll_get_preamble_len(void);

/**
 * @brief      disable the receiver preamble detection banking duiring the first byte of pdu.
 * @param      none.
 * @return     none.
 */
void trf_tpll_disable_preamble_detect(void);

/**
 * @brief      Function for setting the base address for pipe 0.
 * @param      base_addr_0.
 * @return     error code for result. 1:bad addr, 0:success.
 */
unsigned char trf_tpll_set_base_address_0(unsigned char *base_addr);

/**
 * @brief      Function for setting the base address for pipe 1 ~ pipe 5.
 * @param      base_addr_1.
 * @return     error code for result. 1:bad addr, 0:success.
 */
unsigned char trf_tpll_set_base_address_1(unsigned char *base_addr);

/**
 * @brief      Function for configures the number of available pipes's prefiex and enables the pipes.
 * @param      prefix for appointed pipe.
 * @return     error code for result. 2: bad pipe nums, 1:bad prefix, 0:success.
 */
unsigned char trf_tpll_set_prefixes(unsigned char *addr_prefix, unsigned char pipe_num);

/**
 * @brief      enable sending an ACK packet when a crc error occurs
 * @param      enable / disable
 * @return     none
 */
void trf_tpll_enable_crcfilter(unsigned char enable);

// Note that this api is not intended for user
void trf_tpll_rxirq_handler(trf_tpll_event_handler_t p_event_handler);
// Note that this api is not intended for user
trf_tpll_event_handler_t trf_tpll_get_event_handler(void);

/**
 * @brief      get rx packet
 * @param      none
 * @return     rx packet address
 */
unsigned char *trf_tpll_get_rx_packet(void);

/**
 * @brief      judge whether the crc is valid
 * @param      rx packet address
 * @return     valid return 1,invalid return 0
 */
unsigned char trf_tpll_is_crc_vaild(unsigned char *rx_packet);


#endif /*_TL_TPLL_H_*/

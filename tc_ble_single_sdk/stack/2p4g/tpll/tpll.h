/********************************************************************************************************
 * @file    tpll.h
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
#ifndef _TPLL_H_
#define _TPLL_H_
#define RX_SETTLE_TIME_US                120
#define TX_SETTLE_TIME_US                110
#define PTX_RETRY_DELAY_TIME_US                 10


#define TPLL_RF_PACKET_LENGTH_OK(p)           (p[0] == (p[4] & 0x3f) + 11)
#define TPLL_RF_PACKET_CRC_OK(p)              ((p[p[0]+3] & 0x01) == 0x00)

#define TLSR_SUCCESS                            (0)
#define TLSR_ERROR_INVALID_PARAM                (-1)
#define TLSR_ERROR_BUSY                         (-2)


/**@brief Telink primary link layer address width. */
typedef enum {
    ADDRESS_WIDTH_3BYTES = 3,      /**< Set address width to 3 bytes */
    ADDRESS_WIDTH_4BYTES,          /**< Set address width to 4 bytes */
    ADDRESS_WIDTH_5BYTES           /**< Set address width to 5 bytes */
} TPLL_AddressWidthTypeDef;

/**@brief Telink primary link layer pipe IDs. */
typedef enum {
    TPLL_PIPE0 = 0,          /**< Select pipe0 */
    TPLL_PIPE1,              /**< Select pipe1 */
    TPLL_PIPE2,              /**< Select pipe2 */
    TPLL_PIPE3,              /**< Select pipe3 */
    TPLL_PIPE4,              /**< Select pipe4 */
    TPLL_PIPE5,              /**< Select pipe5 */
    TPLL_TX,                 /**< Refer to TX address*/
    TPLL_PIPE_ALL = 0xFF     /**< Close or open all pipes*/
} TPLL_PipeIDTypeDef;

/**@brief Telink primary link layer state machine status. */
typedef enum {
    TPLL_STATE_MACHINE_STATUS_IDLE = 0,          /**< Idle */
    TPLL_STATE_MACHINE_STATUS_TX_SETTLE,         /**< TX Settle*/
    TPLL_STATE_MACHINE_STATUS_TX,                /**< TX */
    TPLL_STATE_MACHINE_STATUS_RX_WAIT,           /**< RX Wait */
    TPLL_STATE_MACHINE_STATUS_RX,                /**< RX */
    TPLL_STATE_MACHINE_STATUS_TX_WAIT,           /**< RX Wait */
} TPLL_StatemachineStatusTypeDef;

/**@brief Telink primary link layer mode. */
typedef enum {
    TPLL_MODE_PTX = 0,      /**< PTX Mode */
    TPLL_MODE_PRX,          /**< PRX Mode */
} TPLL_ModeTypeDef;

/**@brief Telink primary link layer modulation index. */
typedef enum {
	TPLL_RF_MI_0000 = 0,             /**< MI = 0 */
	TPLL_RF_MI_0076 = 76,            /**< MI = 0.076 This gear is only available in private mode*/
	TPLL_RF_MI_0320 = 320,		 	/**< MI = 0.32 */
	TPLL_RF_MI_0500 = 500,		  	/**< MI = 0.5 */
	TPLL_RF_MI_0600 = 600,		  	/**< MI = 0.6 */
	TPLL_RF_MI_0700 = 700,		  	/**< MI = 0.7 */
	TPLL_RF_MI_0800 = 800,		  	/**< MI = 0.8 */
	TPLL_RF_MI_0900 = 900,		  	/**< MI = 0.9 */
	TPLL_RF_MI_1200 = 1200,		    /**< MI = 1.2 */
	TPLL_RF_MI_1300 = 1300,		    /**< MI = 1.3 */
	TPLL_RF_MI_1400 = 1400,		    /**< MI = 1.4 */
}TPLL_MIVauleTypeDef;

typedef enum{
    TPLL_RX_SETTLE_TIME_77US         = 0, /**<  disable rx_ldo_trim calibration,reduce 8.5us of rx settle time. Do a normal calibration at the beginning.*/
    TPLL_RX_SETTLE_TIME_44US         = 1, /**<  disable rx_ldo_trim and rx_dcoc calibration,reduce 48.5us of rx settle time.Receive for a period of time and then do a normal calibration. */
}TPLL_FS_RxSettleTimeTypeDef;

typedef enum{

    TPLL_TX_SETTLE_TIME_101US       = 0, /**<  disable tx_ldo_trim function,reduce 8.5us of tx settle time. Do a normal calibration at the beginning.*/
    TPLL_TX_SETTLE_TIME_53US        = 1, /**<  disable tx_ldo_trim function and tx_hpmc,reduce 61.5us of tx settle time.After frequency hopping, a normal calibration must be done.*/
}TPLL_FS_TxSettleTimeTypeDef;

/**
 * @brief       Initiate the the Telink primary link layer module.
 * @note        This function must be called at the beginning of the TPLL configuration.
 * @param       bitrate  Radio bitrate.
 * @return      none.
 */
extern void TPLL_Init(TPLL_BitrateTypeDef bitrate);

/**
 * @brief       Set the channel to use for the radio. 
 * @param       channel Channel to use for the radio.
 * @return      none.
 */
extern void TPLL_SetRFChannel(signed short channel);

/**
 * @brief       Set the new channel to use for the radio. 
 * @param       channel New channel to use for the radio.
 * @return      none.
 */
extern void TPLL_SetNewRFChannel(signed short channel);

/**
 * @brief       Set the radio output power.
 * @param       power   Output power.
 * @return      none.
  */
extern void TPLL_SetOutputPower(RF_PowerTypeDef power);

/**
 * @brief       Set one pipe as a TX pipe.
 * @param       pipe_id Pipe to be set as a TX pipe.
 * @return      none.
 */
extern void TPLL_SetTXPipe(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Get the current TX pipe.
 * @return      The pipe set as a TX pipe.
*/
extern unsigned char TPLL_GetTXPipe(void);

/**
 * @brief       Update the read pointer of the TX FIFO.
 * @param       pipe_id Pipe id.
 * @return      none.
 */
extern void TPLL_UpdateTXFifoRptr(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Enable the W_TX_PAYLOAD_NOACK command.
 * @param       enable  Whether to enable or disable NoAck option in 9-bit Packet control field.
 * @return      none.
 */
extern void TPLL_EnableNoAck(unsigned char enable);

/**
 * @brief   Enables the ACK payload feature
 *
 * @param   enable                  Whether to enable or disable ACK payload
 */
extern void TPLL_Set_Local_Ack_Bit(unsigned char enable);

/**
 * @brief       Write the payload that will be transmitted with ACK in the specified pipe.
 * @param       pipe_id     Pipe that transmits the payload.
 * @param       payload     Pointer to the payload data.
 * @param       length      Size of the data to transmit.
 * @return
 */
extern void TPLL_WriteAckPayload(TPLL_PipeIDTypeDef pipe_id, const unsigned char *payload, unsigned char length);

/**
 * @brief       Open one or all pipes.
 * @param       pipe_id Radio pipes to open.
 * @return      none.
 */
extern void TPLL_OpenPipe(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Close one or all pipes.
 * @param       pipe_id Radio pipes to close.
 * @return      none.
 */
extern void TPLL_ClosePipe(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Set the address for pipes.
 * @param       pipe_id Radio pipe to set the address for.
 * @param       addr    Pointer to the address data.
 * @return      none.
 */
extern void TPLL_SetAddress(TPLL_PipeIDTypeDef pipe_id, const unsigned char *addr);

/**
 * @brief       Get the address for pipes.
 * @param       pipe_id Radio pipe to get the address for
 * @param       addr    Pointer to a buffer that address data are written to.
 * @return      Numbers of bytes copied to addr.
 */
extern unsigned char TPLL_GetAddress(TPLL_PipeIDTypeDef pipe_id, unsigned char *addr);

/**
 * @brief       Set the the number of retransmission attempts and the packet retransmit delay.
 * @param       retry_times Number of retransmissions. Setting the parmater to 0 disables retransmission.
 * @param       retry_delay Delay between retransmissions.
 * @return      none.
 */
extern void TPLL_SetAutoRetry(unsigned char retry_times, unsigned short retry_delay);

/**
 * @brief       Set the width of the address.
 * @param       address_width   Width of the TPLL address (in bytes).
 * @return      none.
 */
extern void TPLL_SetAddressWidth(TPLL_AddressWidthTypeDef address_width);

/**
 * @brief       Get the width of the address.
 * @return      Width of the TPLL address (in bytes).
 */
extern unsigned char TPLL_GetAddressWidth(void);

/**
 * @brief       Check status for a selected pipe.
 * @param       pipe_id Pipe id to check status for.
 * @return      Pipe status.
 */
extern unsigned char TPLL_GetPipeStatus(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Get the dropped packet count.
 * @return      Dropped packet count.
 */
extern unsigned char TPLL_GetPacketLostCtr(void);

/* Status functions prototypes */

/**
 * @brief       Check if the TX FIFO is empty.
 * @param       pipe_id pipe id for which to check.
 * @return      1: the TX FIFO is empty; 0: the packet is not empty.
 */
extern unsigned char TPLL_TxFifoEmpty(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Check if TX FIFO is full.
 * @param       pipe_id pipe id for which to check.
 * @return      TRUE TX FIFO empty or not.
 */
extern unsigned char TPLL_TxFifoFull(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Get the number of retransmission attempts.
 * @return      Number of retransmissions.
 */
extern unsigned char TPLL_GetTransmitAttempts(void);

/**
 * @brief       Get the carrier detect status.
 * @return      Carrier detect status.
 */
extern unsigned char TPLL_GetCarrierDetect(void);

/**
 * @brief       Get the pipe that has received a packet.
 * @return      Pipe id.
 */
extern unsigned char TPLL_GetRxDataSource(void);

/**
 * @brief       Read an RX payload.
 * @param       rx_pload   Pointer to the buffer where the payload will be stored.
 * @return      pipe number (MSB) and packet length (LSB).
 */
extern unsigned short TPLL_ReadRxPayload(unsigned char *rx_pload);

/**
 * @brief       Get the RX timestamp. 
 * @note        It is required to call TPLL_ReadRxPayload() before this function is called.
 * @return      RX timestamp.
 */
extern unsigned int TPLL_GetTimestamp(void);

/**
 * @brief       Get the RX RSSI value. 
 * @note        It is required to call TPLL_ReadRxPayload() before this function is called.
 * @return      RSSI value.
 */
extern signed int TPLL_GetRxRssiValue(void);

/**
 * @brief       Write a payload for transmission.
 * @param       pipe_id     Pipe id used for this payload.
 * @param       tx_pload    Pointer to the buffer that contains the payload data.
 * @param       length      Length of the payload.
 * @retval      0           Error
 * @retval      >0          The length of written bytes
 */
extern unsigned char TPLL_WriteTxPayload(TPLL_PipeIDTypeDef pipe_id, const unsigned char *tx_pload, unsigned char length);

/**
 * @brief       Reuse the last transmitted payload for the next packet.
 * @param       pipe_id pipe id.
 * @return      none.
 */
extern void TPLL_ReuseTx(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Remove remaining items from the RX buffer.
 * @return      none.
 */
extern void TPLL_FlushRx(void);

/**
 * @brief       Remove remaining items from the TX buffer.
 * @param       pipe_id Pipe id.
 * @return      none.
 */
extern void TPLL_FlushTx(TPLL_PipeIDTypeDef pipe_id);

/**
 * @brief       Trigger transmission in the specified pipe.
 * @retval      TLSR_SUCCESS                If the operation completed successfully.
 * @retval      TLSR_ERROR_BUSY             If the function failed because the radio was busy.
 */
extern int TPLL_PTXTrig(void);

/**
 * @brief       Trigger reception in the specified pipe.
 * @retval      TLSR_SUCCESS                If the operation completed successfully.
 * @retval      TLSR_ERROR_BUSY             If the function failed because the radio was busy.
 */
extern int TPLL_PRXTrig(void);

/**
 * @brief       Set the RX wait time.
 * @param       wait_us     Period in microseconds.
 * @retval      TLSR_SUCCESS                If the operation completed successfully.
 * @retval      TLSR_ERROR_INVALID_PARAM    If the function failed because the input parameter was out of valid range.
 */
extern int TPLL_RxWaitSet(unsigned short wait_us);

/**
 * @brief       Set the TX wait time.
 * @param       wait_us     Period in microseconds.
 * @retval      TLSR_SUCCESS                If the operation completed successfully.
 * @retval      TLSR_ERROR_INVALID_PARAM    If the function failed because the input parameter was out of valid range.
 */
extern int TPLL_TxWaitSet(unsigned short wait_us);

/**
 * @brief       Set the fast TX wait time.
 * @param       wait_us   Period in microseconds.
 * @return      none.
 */
int TPLL_Fast_TxWaitSet(unsigned short wait_us);

/**
 * @brief       Set the fast RX wait time.
 * @param       wait_us   Period in microseconds.
 * @return      none.
 */
int TPLL_Fast_RxWaitSet(unsigned short wait_us);

/**
 * @brief       Set the RX timerout time.
 * @param       period_us   Period in microseconds.
 * @retval      TLSR_SUCCESS                If the operation completed successfully.
 * @retval      TLSR_ERROR_INVALID_PARAM    If the function failed because the input parameter was out of valid range.
 */
extern int TPLL_RxTimeoutSet(unsigned short period_us);

/**
 * @brief       Set the TX settle time.
 * @param       period_us   Period in microseconds.
 * @retval      TLSR_SUCCESS                If the operation completed successfully.
 * @retval      TLSR_ERROR_INVALID_PARAM    If the function failed because the input parameter was out of valid range.
 */
extern int TPLL_TxSettleSet(unsigned short period_us);

/**
 * @brief       Set the fast TX Settle phase.
 * @param       period_us   specifies the time.
 * @return      none.
 */
int TPLL_Fast_TxSettleSet(unsigned short period_us);

/**
 * @brief       Set the RX settle time.
 * @param       period_us   Period in microseconds.
 * @retval      TLSR_SUCCESS                If the operation completed successfully.
 * @retval      TLSR_ERROR_INVALID_PARAM    If the function failed because the input parameter was out of valid range.
 */
extern int TPLL_RxSettleSet(unsigned short period_us);

/**
 * @brief       Set the fast RX Settle phase.
 * @param       period_us   specifies the time.
 * @return      none.
 */
int TPLL_Fast_RxSettleSet(unsigned short period_us);

/**
 * @brief       Set the mode of TPLL radio.
 * @param       mode    TPLL_MODE_PTX or TPLL_MODE_PRX.
 * @return      none.
 */
extern void TPLL_ModeSet(TPLL_ModeTypeDef mode);

/**
 * @brief       Stop the TPLL state machine.
 * @return      none.
 */
extern void TPLL_ModeStop(void);

/**
 * @brief       Check whether the received packet is valid.
 * @return      1: the packet is valid; 0: the packet is invalid.
 */
extern unsigned char TPLL_IsRxPacketValid(void);

/**
 * @brief       Get the packet received.
 * @param       none.
 * @return      rx_packet.
 */
unsigned char *TPLL_GetRxPacket(void);

/**
 * @brief       Get the pid of the received packet.
 * @param       rx_packet.
 * @return      packet id.
 */
unsigned char TPLL_GetRxPacketId(unsigned char *rx_packet);

/**
 * @brief       Disable / enable sending an ACK packet when a crc error occurs.
 * @param       1:enable,0:disable.
 * @return      none.
 */
__attribute__((section(".ram_code"))) __attribute__((optimize("-Os")))
void TPLL_EnableCrcfilter(unsigned char enable);

/**
 * @brief       Get the crc of the received packet.
 * @param       rx_packet.
 * @return      packet crc.
 */
__attribute__((section(".ram_code"))) __attribute__((optimize("-Os")))
unsigned short TPLL_GetRxPacketCrc(unsigned char *rx_packet);
/**
 * @brief       Check if the crc of the received packet is valid.
 * @param       rx_packet.
 * @return      1:crc valid,0:crc invalid.
 */
__attribute__((section(".ram_code"))) __attribute__((optimize("-Os")))
unsigned char TPLL_IsCrcVaild(unsigned char *rx_packet);
/**
 * @brief       Check if the crc len of the received packet is valid.
 * @param       rx_packet.
 * @return      1:crc len valid,0:crc len invalid.
 */
__attribute__((section(".ram_code"))) __attribute__((optimize("-Os")))
unsigned char TPLL_IsPacketLenVaild(unsigned char *rx_packet);
/* Check if the payload length is zero */
/**
 * @brief       Check if the payload length is zero.
 * @param       rx_packet.
 * @return      1:payload length is zero,0:payload length is not zero.
 */
__attribute__((section(".ram_code"))) __attribute__((optimize("-Os")))
unsigned char TPLL_IsPacketEmpty(unsigned char *rx_packet);

/* Get the local pid */
__attribute__((section(".ram_code"))) __attribute__((optimize("-Os")))
unsigned char TPLL_GetLocalPid(TPLL_PipeIDTypeDef pipe_id);

/* Set the pid reset bit to 0 */
void TPLL_Pid_Reset_Disable(void);

/**
 * @brief       Set the frequency deviation of the transmitter, which follows the equation below.
 *              modulation index = frequency deviation*2/bitrate
 * @param       mi_value    Modulation index.
 * @return      none.
 */
extern void TPLL_SetTxMI(TPLL_MIVauleTypeDef mi_value);

/**
 * @brief       Set the frequency deviation of the receiver, which follows the equation below.
 *              modulation index = frequency deviation*2/bitrate
 * @param       mi_value    Modulation index.
 * @return      none.
 */
extern void  TPLL_SetRxMI(TPLL_MIVauleTypeDef mi_value);

/**
 * @brief      Set the length of the preamble field of the on-air data packet.
 * @note       The valid range of this parameter is 1-16.
 * @param      preamble_len Preamble length.
 * @return     none.
 */
extern void TPLL_Preamble_Set(unsigned char preamble_len);

/**
 * @brief      Read the length of the preamble field of the on-air data packet.
 * @return     Preamble length.
 */
extern unsigned char TPLL_Preamble_Read(void);

/**
 * @brief      disable the receiver preamble detection banking duiring the first byte of pdu.
 * @param      none.
 * @return     none.
 */
extern void TPLL_Preamble_Detect_Disable(void);
#if(1)
/**
 *  @brief      this function serve to reset buffer.
 *  @param[in]  none.
 *  @return     none.
*/
void TPLL_ResetBuf();

/**
 *  @brief      this function serve to enable tx_en_man, so hpmc will be gernerated at 0X12F8~0x12ff.
 *  @param[in]  none.
 *  @return     none.
*/
void TPLL_EnableHpmc();

/**
 *  @brief      this function serve to adjust fast rx settle timing sequence.
 *  @param[in]  rx_settle_us    After adjusting the timing sequence, the time required for rx to settle.
 *  @return     none
 *  @note       TPLL_RX_SETTLE_TIME_77US :Do a normal calibration at the beginning.
 *              TPLL_RX_SETTLE_TIME_44US :Receive for a period of time and then do a normal calibration.
*/
void TPLL_Fast_RxSettleInit(TPLL_FS_RxSettleTimeTypeDef rx_settle_us);

/**
 *  @brief      this function serve to adjust fast tx settle timing sequence.
 *  @param[in]  tx_settle_us    After adjusting the timing sequence, the time required for tx to settle.
 *  @return     none
 *  @note       TPLL_TX_SETTLE_TIME_101US : After frequency hopping, a normal calibration must be done.
 *              TPLL_TX_SETTLE_TIME_53US  : Do a normal calibration at the beginning.
*/
void TPLL_Fast_TxSettleInit(TPLL_FS_TxSettleTimeTypeDef tx_settle_us);

/**
 *  @brief      this function serve to enable the fast tx timing sequence adjusted.
 *  @param[in]  none.
 *  @return     none.
*/
void TPLL_Fast_TxSettleEnable(void);

/**
 *  @brief      this function serve to disable the fast tx timing sequence adjusted.
 *  @param[in]  none.
 *  @return     none.
*/
void TPLL_Fast_TxSettleDisable(void);

/**
 *  @brief      this function serve to enable the fast rx timing sequence adjusted.
 *  @param[in]  none.
 *  @return     none.
*/
void TPLL_Fast_RxSettleEnable(void);

/**
 *  @brief      this function serve to disable the fast rx timing sequence adjusted.
 *  @param[in]  none.
 *  @return     none.
*/
void TPLL_Fast_RxSettleDisable(void);
/**
 * @brief       get Hpmc calibration value.
 * @param       none.
 * @return      Hpmc calibration value.
 * @note        to get the calibration value of different channel need to trigger tx in different channel.
 */
__attribute__((section(".ram_code")))unsigned short TPLL_GetHpmcCalVal(void);

/**
 * @brief       set Hpmc calibration value.
 * @param       value Hpmc calibration value of the corresponding channel.
 * @return      none.
 */
__attribute__((section(".ram_code")))void TPLL_SetHpmcCalVal(unsigned short value);
#endif
#endif /*_TPLL_H_*/

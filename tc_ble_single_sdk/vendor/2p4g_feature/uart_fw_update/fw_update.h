/********************************************************************************************************
 * @file    fw_update.h
 *
 * @brief   This is the header file for 2.4G SDK
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
#ifndef _FW_UPDATE_H_
#define _FW_UPDATE_H_

#define FW_UPDATE_FRAME_TYPE_CMD        0x01
#define FW_UPDATE_FRAME_TYPE_DATA       0x02
#define FW_UPDATE_FRAME_TYPE_ACK        0x03

#define FW_UPDATE_CMD_ID_START_REQ      0x01
#define FW_UPDATE_CMD_ID_START_RSP      0x02
#define FW_UPDATE_CMD_ID_END_REQ        0x03
#define FW_UPDATE_CMD_ID_END_RSP        0x04
#define FW_UPDATE_CMD_ID_VERSION_REQ    0x05
#define FW_UPDATE_CMD_ID_VERSION_RSP    0x06

#define FW_UPDATE_FRAME_PAYLOAD_MAX     (2+64)
#define FW_UPDATE_RETRY_MAX             3
#define FW_APPEND_INFO_LEN              2 // FW_CRC 2 BYTE
#define FW_BOOT_ADDR                    0x7f000

#define FW_UPDATE_BIN_ADDR_20000  0x20000
#define FW_UPDATE_BIN_ADDR_40000  0x40000
#define FW_UPDATE_SLAVE_BIN_ADDR        FW_UPDATE_BIN_ADDR_20000
#define FW_UPDATE_MASTER_BIN_ADDR       FW_UPDATE_BIN_ADDR_20000

#define FW_UPDATE_DURATION                (5 * 1000 * 1000)
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
#define UART_TX_PIN             GPIO_PD0
#define UART_RX_PIN             GPIO_PD1
#endif
/* uart select */
#define UART_MODULE_SEL         0/* UART0 */

typedef struct {
    unsigned int FlashAddr;
    unsigned int TotalBinSize;
    unsigned short MaxBlockNum;
    unsigned short BlockNum;
    unsigned short FwVersion;
    unsigned short FwCRC;
    unsigned short PktCRC;
    unsigned short TargetFwCRC;
    unsigned char State;
    unsigned char RetryTimes;
    unsigned char FinishFlag;
} FW_UPDATE_CtrlTypeDef;

typedef struct {
    unsigned char CheckSum;
    unsigned char Type;
    unsigned short Len;
    unsigned char Payload[FW_UPDATE_FRAME_PAYLOAD_MAX];
} FW_UPDATE_FrameTypeDef;

enum {
    FW_UPDATE_MASTER_STATE_IDLE = 0,
    FW_UPDATE_MASTER_STATE_FW_VER_WAIT,
    FW_UPDATE_MASTER_STATE_START_RSP_WAIT,
    FW_UPDATE_MASTER_STATE_DATA_ACK_WAIT,
    FW_UPDATE_MASTER_STATE_END_RSP_WAIT,
    FW_UPDATE_MASTER_STATE_END,
    FW_UPDATE_MASTER_STATE_ERROR,
};

enum {
    FW_UPDATE_SLAVE_STATE_IDLE = 0,
    FW_UPDATE_SLAVE_STATE_FW_VERSION_READY,
    FW_UPDATE_SLAVE_STATE_START_READY,
    FW_UPDATE_SLAVE_STATE_DATA_READY,
    FW_UPDATE_SLAVE_STATE_END_READY,
    FW_UPDATE_SLAVE_STATE_END,
    FW_UPDATE_SLAVE_STATE_ERROR
};

enum {
    FW_UPDATE_MSG_TYPE_INVALID_DATA = 0,
    FW_UPDATE_MSG_TYPE_DATA,
    FW_UPDATE_MSG_TYPE_TIMEOUT,
};

extern void FW_UPDATE_MasterInit(unsigned int FWBinAddr, unsigned short FwVer);
extern void FW_UPDATE_MasterStart(void);

extern void FW_UPDATE_SlaveInit(unsigned int FWBinAddr, unsigned short FwVer);
extern void FW_UPDATE_SlaveStart(void);

extern void FW_UPDATE_RxIrq(unsigned char *Data);

#endif /*_FW_UPDATE_H_*/

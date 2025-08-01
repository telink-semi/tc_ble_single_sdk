/********************************************************************************************************
 * @file    ota.h
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
#ifndef _OTA_H_
#define _OTA_H_
#if (FEATURE_TEST_MODE == OTA)

//if ota bin address should be same as ota_program_bootAddr !!!
#define OTA_BIN_ADDR_0x40000   0x40000
#define OTA_BIN_ADDR_0x20000   0x20000
#define OTA_MASTER_BIN_ADDR     OTA_BIN_ADDR_0x20000
#define OTA_SLAVE_BIN_ADDR      OTA_BIN_ADDR_0x20000

#define OTA_FRAME_TYPE_CMD        0x01
#define OTA_FRAME_TYPE_DATA       0x02
#define OTA_FRAME_TYPE_ACK        0x03

#define OTA_CMD_ID_START_REQ      0x01
#define OTA_CMD_ID_START_RSP      0x02
#define OTA_CMD_ID_END_REQ        0x03
#define OTA_CMD_ID_END_RSP        0x04
#define OTA_CMD_ID_VERSION_REQ    0x05
#define OTA_CMD_ID_VERSION_RSP    0x06


#define OTA_DURATION                (5 * 1000 * 1000)

#define OTA_FRAME_PAYLOAD_MAX     (48+2)
#define OTA_RETRY_MAX             10
#define OTA_APPEND_INFO_LEN              2 // FW_CRC 2 BYTE

typedef struct {
    unsigned int FlashAddr;
    unsigned int TotalBinSize;
    unsigned short MaxBlockNum;
    unsigned short BlockNum;
    unsigned short PeerAddr;
    unsigned short FwVersion;
    unsigned short FwCRC;
    unsigned short PktCRC;
    unsigned short TargetFwCRC;
    unsigned char State;
    unsigned char RetryTimes;
    unsigned char FinishFlag;
} OTA_CtrlTypeDef;

typedef struct {
    unsigned char Type;
    unsigned char Payload[OTA_FRAME_PAYLOAD_MAX];
} OTA_FrameTypeDef;

enum {
    OTA_MASTER_STATE_IDLE = 0,
    OTA_MASTER_STATE_FW_VER_WAIT,
    OTA_MASTER_STATE_START_RSP_WAIT,
    OTA_MASTER_STATE_DATA_ACK_WAIT,
    OTA_MASTER_STATE_END_RSP_WAIT,
    OTA_MASTER_STATE_END,
    OTA_MASTER_STATE_ERROR,
};

enum {
    OTA_SLAVE_STATE_IDLE = 0,
    OTA_SLAVE_STATE_FW_VERSION_READY,
    OTA_SLAVE_STATE_START_READY,
    OTA_SLAVE_STATE_DATA_READY,
    OTA_SLAVE_STATE_END_READY,
    OTA_SLAVE_STATE_END,
    OTA_SLAVE_STATE_ERROR
};

enum {
    OTA_MSG_TYPE_INVALID_DATA = 0,
    OTA_MSG_TYPE_DATA,
    OTA_MSG_TYPE_TIMEOUT,
};



typedef enum{

	GEN_FSK_STX_MODE = 0,
	GEN_FSK_SRX_MODE = 1,

}Gen_Fsk_Mode_Slect;

extern void OTA_MasterInit(unsigned int OTABinAddr, unsigned short FwVer);
extern void OTA_MasterStart(void);

extern void OTA_SlaveInit(unsigned int OTABinAddr, unsigned short FwVer);
extern void OTA_SlaveStart(void);

extern void OTA_RxIrq(unsigned char *Data);
extern void OTA_RxTimeoutIrq(unsigned char *Data);


#endif /*_OTA_H_*/
#endif

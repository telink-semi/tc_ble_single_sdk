/********************************************************************************************************
 * @file    ota.c
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
#include "ota.h"
#include "driver.h"
#include "tl_common.h"
#include "mac.h"
#include "../../../stack/2p4g/genfsk_ll/genfsk_ll.h"
#include "../../stack/ble/debug/debug.h"
#include "app_config.h"
#if (FEATURE_TEST_MODE == OTA)

#define MSG_QUEUE_LEN          4
#define OTA_BIN_SIZE_OFFSET    0x18

#define OTA_REBOOT_WAIT        (50 * 1000) //in us
#define OTA_BOOT_FLAG_OFFSET   8

typedef struct {
    unsigned int Type;
    unsigned char *Data; //content of msg
} OTA_MsgTypeDef;

typedef struct {
    OTA_MsgTypeDef Msg[MSG_QUEUE_LEN]; //buffer storing msg
    unsigned char Cnt; //current num of msg
    unsigned char ReadPtr; //ptr of first msg should be read next
} OTA_MsgQueueTypeDef;
OTA_MsgQueueTypeDef MsgQueue;

OTA_FrameTypeDef TxFrame;
OTA_FrameTypeDef RxFrame;

u32 state_tick = 0;

static int OTA_MsgQueuePush(const unsigned char *Data, const unsigned int Type, OTA_MsgQueueTypeDef *Queue)
{
    if (Queue->Cnt < MSG_QUEUE_LEN) {
        Queue->Msg[(Queue->ReadPtr + Queue->Cnt) % MSG_QUEUE_LEN].Data = (unsigned char *)Data;
        Queue->Msg[(Queue->ReadPtr + Queue->Cnt) % MSG_QUEUE_LEN].Type = Type;
        Queue->Cnt++;
        return 1;
    }

    return 0;
}

static OTA_MsgTypeDef *OTA_MsgQueuePop(OTA_MsgQueueTypeDef *Queue)
{
    OTA_MsgTypeDef *ret = NULL;

    if (Queue->Cnt > 0) {
        ret = &(Queue->Msg[Queue->ReadPtr]);
        Queue->ReadPtr = (Queue->ReadPtr + 1) % MSG_QUEUE_LEN;
        Queue->Cnt--;
    }

    return ret;
}

static int OTA_BuildCmdFrame(OTA_FrameTypeDef *Frame, const unsigned char CmdId, const unsigned char *Value, unsigned short Len)
{
    Frame->Type = OTA_FRAME_TYPE_CMD;
    Frame->Payload[0] = CmdId;
    if (Value) {
        memcpy(&Frame->Payload[1], Value, Len);
    }
    unsigned int fram_length = Len+2;
    return fram_length;
}

void OTA_RxIrq(unsigned char *Data)
{
    if (NULL == Data) {
        OTA_MsgQueuePush(NULL, OTA_MSG_TYPE_INVALID_DATA, &MsgQueue);
    }
    else {
        if (Data[0]) {
            OTA_MsgQueuePush(Data, OTA_MSG_TYPE_DATA, &MsgQueue);
        }
        else {
            OTA_MsgQueuePush(NULL, OTA_MSG_TYPE_INVALID_DATA, &MsgQueue);
        }
    }
}

void OTA_RxTimeoutIrq(unsigned char *Data)
{
    OTA_MsgQueuePush(NULL, OTA_MSG_TYPE_TIMEOUT, &MsgQueue);
}


#if (OTA_ROLE == MASTER)

static OTA_CtrlTypeDef MasterCtrl = {0};
#if(INTERNAL_TEST)
extern unsigned int master_test_success_cnt;
extern unsigned int master_test_fail_cnt;
#endif
static int OTA_IsBlockNumMatch(unsigned char *Payload)
{
    unsigned short BlockNum = Payload[1];
    BlockNum <<= 8;
    BlockNum += Payload[0];

    if (BlockNum == MasterCtrl.BlockNum) {
        return 1;
    }
    else {
        return 0;
    }
}

static int OTA_BuildDataFrame(OTA_FrameTypeDef *Frame)
{
	unsigned int fram_length;
    Frame->Type = OTA_FRAME_TYPE_DATA;
    if ((MasterCtrl.TotalBinSize - MasterCtrl.BlockNum*(OTA_FRAME_PAYLOAD_MAX-2)) > (OTA_FRAME_PAYLOAD_MAX-2)) {
    	fram_length = OTA_FRAME_PAYLOAD_MAX;
    }
    else {
    	fram_length = MasterCtrl.TotalBinSize - MasterCtrl.BlockNum*(OTA_FRAME_PAYLOAD_MAX-2) + 2;
        MasterCtrl.FinishFlag = 1;
    }
    MasterCtrl.BlockNum++;
    memcpy(Frame->Payload, &MasterCtrl.BlockNum, 2);
    flash_read_page(MasterCtrl.FlashAddr,fram_length-2, &Frame->Payload[2]);
    MasterCtrl.FlashAddr += fram_length-2;
    return (1 + fram_length);
}

void OTA_MasterInit(unsigned int OTABinAddr, unsigned short FwVer)
{
    MasterCtrl.FlashAddr = OTABinAddr;
    //read the size of OTA_bin file
    flash_read_page((unsigned long)MasterCtrl.FlashAddr + OTA_BIN_SIZE_OFFSET, 4, ( unsigned char *)&MasterCtrl.TotalBinSize);
    MasterCtrl.TotalBinSize += OTA_APPEND_INFO_LEN; // APPEND CRC INFO IN BIN TAIL
    MasterCtrl.MaxBlockNum = (MasterCtrl.TotalBinSize + (OTA_FRAME_PAYLOAD_MAX - 2) - 1) / (OTA_FRAME_PAYLOAD_MAX - 2);
    MasterCtrl.BlockNum = 0;
    MasterCtrl.FwVersion = FwVer;
    MasterCtrl.State = OTA_MASTER_STATE_IDLE;
    MasterCtrl.RetryTimes = 0;
    MasterCtrl.FinishFlag = 0;
    state_tick            = 0;
}
void OTA_MasterStart(void)
{
    OTA_MsgTypeDef *Msg = OTA_MsgQueuePop(&MsgQueue);
    static int Len = 0;

    if (OTA_MASTER_STATE_IDLE == MasterCtrl.State) {
      if (!state_tick) {
          state_tick = clock_time() | 1;
      }
//      tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] OTA_MASTER_STATE_IDLE %d\n",__LINE__);
        Len = OTA_BuildCmdFrame(&TxFrame, OTA_CMD_ID_VERSION_REQ, 0, 0);
        MAC_SendData((unsigned char*)&TxFrame, Len);
        MasterCtrl.State = OTA_MASTER_STATE_FW_VER_WAIT;
    }
    else if (OTA_MASTER_STATE_FW_VER_WAIT == MasterCtrl.State) {

      if(state_tick && clock_time_exceed(state_tick, OTA_DURATION)){
        MasterCtrl.State = OTA_MASTER_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota time out%d\n",__LINE__);
        return;
      }
//      tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] OTA_MASTER_STATE_FW_VER_WAIT %d\n",__LINE__);
        if (Msg) {
//          tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
//          tlkapi_send_string_data((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota packet", Msg, 8);
            //if receive a valid rf packet
            if (Msg->Type == OTA_MSG_TYPE_DATA) {
            	Len = (int)Msg->Data[0];
            	RxFrame.Type = Msg->Data[1];
            	memcpy(RxFrame.Payload, Msg->Data+2, Len-1);
                //if receive the valid FW version response
                if ((OTA_FRAME_TYPE_CMD == RxFrame.Type) &&
                (OTA_CMD_ID_VERSION_RSP == RxFrame.Payload[0])) {
                    MasterCtrl.RetryTimes = 0;
                    //compare the received version with that of OTA_bin
                    unsigned short Version = RxFrame.Payload[2];
                    Version <<= 8;
                    Version += RxFrame.Payload[1];

                    if (Version < MasterCtrl.FwVersion) {
                        MasterCtrl.State = OTA_MASTER_STATE_START_RSP_WAIT;
                        state_tick = clock_time() | 1;
                        Len = OTA_BuildCmdFrame(&TxFrame, OTA_CMD_ID_START_REQ, (unsigned char *)&MasterCtrl.MaxBlockNum, sizeof(MasterCtrl.MaxBlockNum));
                        MAC_SendData((unsigned char*)&TxFrame, Len);
                    }
                    else {
                        MasterCtrl.State = OTA_MASTER_STATE_ERROR;
                        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                    }
                    return;
                }
            }

            if (MasterCtrl.RetryTimes == OTA_RETRY_MAX) {
                MasterCtrl.State = OTA_MASTER_STATE_ERROR;
                tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                return;
            }
            MasterCtrl.RetryTimes++;
            MAC_SendData((unsigned char*)&TxFrame, Len);
        }
    }
    else if (OTA_MASTER_STATE_START_RSP_WAIT == MasterCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, OTA_DURATION)){
        MasterCtrl.State = OTA_MASTER_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota time out%d\n",__LINE__);
        return;
      }
//      tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] OTA_MASTER_STATE_START_RSP_WAIT %d\n",__LINE__);
        if (Msg) {
//          tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
//          tlkapi_send_string_data((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota packet", Msg, 8);
            //if receive a valid rf packet
            if (Msg->Type == OTA_MSG_TYPE_DATA) {
            	Len = (int)Msg->Data[0];
            	RxFrame.Type = Msg->Data[1];
            	memcpy(RxFrame.Payload, Msg->Data+2, Len-1);
                //if receive the valid FW version response
                if ((OTA_FRAME_TYPE_CMD == RxFrame.Type) &&
                (OTA_CMD_ID_START_RSP == RxFrame.Payload[0])) {
                    MasterCtrl.RetryTimes = 0;
                    //read OTA_bin from flash and packet it in OTA data frame
                    MasterCtrl.State = OTA_MASTER_STATE_DATA_ACK_WAIT;
                    state_tick = clock_time() | 1;
                    Len = OTA_BuildDataFrame(&TxFrame);
                    MAC_SendData((unsigned char*)&TxFrame, Len);
                    return;
                }
            }

            if (MasterCtrl.RetryTimes == OTA_RETRY_MAX) {
                MasterCtrl.State = OTA_MASTER_STATE_ERROR;
                tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                return;
            }
            MasterCtrl.RetryTimes++;
            MAC_SendData((unsigned char*)&TxFrame, Len);
        }
    }

    else if (OTA_MASTER_STATE_DATA_ACK_WAIT == MasterCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, OTA_DURATION)){
        MasterCtrl.State = OTA_MASTER_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota time out%d\n",__LINE__);
        return;
      }
//      tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] OTA_MASTER_STATE_DATA_ACK_WAIT %d\n",__LINE__);
        if (Msg) {
//          tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
//          tlkapi_send_string_data((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota packet", Msg, 8);
//            //if receive a valid rf packet
            if (Msg->Type == OTA_MSG_TYPE_DATA) {
            	Len = (int)Msg->Data[0];
            	RxFrame.Type = Msg->Data[1];
            	memcpy(RxFrame.Payload, Msg->Data+2, Len-1);
                //if receive the valid OTA data ack
                if ((OTA_FRAME_TYPE_ACK == RxFrame.Type) && OTA_IsBlockNumMatch(RxFrame.Payload)) {
                    MasterCtrl.RetryTimes = 0;
                    if (MasterCtrl.FinishFlag) {
                        MasterCtrl.State = OTA_MASTER_STATE_END_RSP_WAIT;
                        state_tick = clock_time() | 1;
                        Len = OTA_BuildCmdFrame(&TxFrame, OTA_CMD_ID_END_REQ, (unsigned char *)&MasterCtrl.TotalBinSize, sizeof(MasterCtrl.TotalBinSize));
                        MAC_SendData((unsigned char*)&TxFrame, Len);
//                        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                    }
                    else {
                        //read OTA_bin from flash and packet it in OTA data frame
                        Len = OTA_BuildDataFrame(&TxFrame);
                        MAC_SendData((unsigned char*)&TxFrame, Len);
//                        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                    }
                    return;
                }
            }

            if (MasterCtrl.RetryTimes == OTA_RETRY_MAX) {
                MasterCtrl.State = OTA_MASTER_STATE_ERROR;
                tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                return;
            }
            MasterCtrl.RetryTimes++;
            MAC_SendData((unsigned char*)&TxFrame, Len);
        }
    }

    else if (OTA_MASTER_STATE_END_RSP_WAIT == MasterCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, OTA_DURATION)){
        MasterCtrl.State = OTA_MASTER_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota time out%d\n",__LINE__);
        return;
      }
//      tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] OTA_MASTER_STATE_END_RSP_WAIT %d\n",__LINE__);
        if (Msg) {
//          tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
//          tlkapi_send_string_data((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota packet", Msg, 8);
            //if receive a valid rf packet
            if (Msg->Type == OTA_MSG_TYPE_DATA) {
            	Len = (int)Msg->Data[0];
            	RxFrame.Type = Msg->Data[1];
            	memcpy(RxFrame.Payload, Msg->Data+2, Len-1);
                //if receive the valid FW version response
                if ((OTA_FRAME_TYPE_CMD == RxFrame.Type) &&
                    (OTA_CMD_ID_END_RSP == RxFrame.Payload[0])) {
                    MasterCtrl.RetryTimes = 0;
                    MasterCtrl.State = OTA_MASTER_STATE_END;
                    state_tick = clock_time() | 1;
                    return;
                }
            }
            if (MasterCtrl.RetryTimes == OTA_RETRY_MAX) {
                MasterCtrl.State = OTA_MASTER_STATE_ERROR;
                tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                return;
            }
            MasterCtrl.RetryTimes++;
            MAC_SendData((unsigned char*)&TxFrame, Len);
        }
    }
    else if (OTA_MASTER_STATE_END == MasterCtrl.State) {
      tlkapi_printf(APP_OTA_LOG_EN, "[OTA][FLW] master ota success\n");
#if (APP_FLASH_PROTECTION_ENABLE)
      flash_prot_op_cb(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END, 0, 0);
#endif
            gpio_set_func(GPIO_LED_WHITE, AS_GPIO);
            gpio_set_output_en(GPIO_LED_WHITE, 1);
            gpio_write(GPIO_LED_WHITE, 1);
            WaitMs(60);
            gpio_write(GPIO_LED_WHITE, 0);
            WaitMs(120);
            gpio_write(GPIO_LED_WHITE, 1);
            WaitMs(60);
            gpio_write(GPIO_LED_WHITE, 0);
            WaitMs(120);
//            start_reboot();
#if(INTERNAL_TEST)
        master_test_success_cnt++;
#endif
#if(APP_PM_ENABLE && APP_PM_DEEPSLEEP_RETENTION_ENABLE)
            cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW32K, PM_WAKEUP_TIMER, ClockTime() + OTA_REBOOT_WAIT * 16);
#else
            start_reboot();
#endif
    }
    else if (OTA_MASTER_STATE_ERROR == MasterCtrl.State) {
      tlkapi_printf(APP_OTA_LOG_EN, "[OTA][FLW] master ota failed\n");
#if (APP_FLASH_PROTECTION_ENABLE)
      flash_prot_op_cb(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END, 0, 0);
#endif
    	gpio_set_func(GPIO_LED_RED, AS_GPIO);
		gpio_set_output_en(GPIO_LED_RED, 1);
		gpio_write(GPIO_LED_RED, 1);
		WaitMs(60);
		gpio_write(GPIO_LED_RED, 0);
		WaitMs(120);
		gpio_write(GPIO_LED_RED, 1);
		WaitMs(60);
		gpio_write(GPIO_LED_RED, 0);
		WaitMs(120);
//		start_reboot();
#if(INTERNAL_TEST)
        master_test_fail_cnt++;
#endif
#if(APP_PM_ENABLE && APP_PM_DEEPSLEEP_RETENTION_ENABLE)
            cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW32K, PM_WAKEUP_TIMER, ClockTime() + OTA_REBOOT_WAIT * 16);
#else
            start_reboot();
#endif
    }
}

#else /*OTA_MASTER_EN*/
#define OTA_MASTER_FIRST_RX_DURATION    (5*1000*1000) //in us
#define OTA_MASTER_RESPONSE_RX_DURATION   (1*1000*1000) //in us

static OTA_CtrlTypeDef SlaveCtrl = {0};

static unsigned short OTA_CRC16_Cal(unsigned short crc, unsigned char* pd, int len)
{
    // unsigned short       crc16_poly[2] = { 0, 0xa001 }; //0x8005 <==> 0xa001
    unsigned short      crc16_poly[2] = { 0, 0x8408 }; //0x1021 <==> 0x8408
    //unsigned short        crc16_poly[2] = { 0, 0x0811 }; //0x0811 <==> 0x8810
    //unsigned short        crc = 0xffff;
    int i, j;

    for (j = len; j > 0; j--)
    {
        unsigned char ds = *pd++;
        for (i = 0; i < 8; i++)
        {
            crc = (crc >> 1) ^ crc16_poly[(crc ^ ds) & 1];
            ds = ds >> 1;
        }
    }

    return crc;
}

static int OTA_BuildAckFrame(OTA_FrameTypeDef *Frame, unsigned short BlockNum)
{
	unsigned int fram_length;
    Frame->Type = OTA_FRAME_TYPE_ACK;
    fram_length = 2;
    Frame->Payload[0] = BlockNum & 0xff;
    Frame->Payload[1] = BlockNum >> 8;


    return (1 + fram_length);
}

static void OTA_FlashErase(void)
{
    int SectorAddr = SlaveCtrl.FlashAddr;
    int i = 0;
    for (i = 0; i < 15; i++) {
        flash_erase_sector(SectorAddr);
        SectorAddr += 0x1000;
    }
}

void OTA_SlaveInit(unsigned int OTABinAddr, unsigned short FwVer)
{
    SlaveCtrl.FlashAddr = OTABinAddr;
    SlaveCtrl.BlockNum = 0;
    SlaveCtrl.FwVersion = FwVer;
    SlaveCtrl.State = OTA_SLAVE_STATE_IDLE;
    SlaveCtrl.RetryTimes = 0;
    SlaveCtrl.FinishFlag = 0;
    SlaveCtrl.FwCRC = SlaveCtrl.PktCRC = 0;

    //erase the OTA write area
    OTA_FlashErase();
}

void OTA_SlaveStart(void)
{
    OTA_MsgTypeDef *Msg = OTA_MsgQueuePop(&MsgQueue);
    static int Len = 0;
    if (OTA_SLAVE_STATE_IDLE == SlaveCtrl.State) {
      if (!state_tick) {
          state_tick = clock_time() | 1;
      }
        SlaveCtrl.State = OTA_SLAVE_STATE_FW_VERSION_READY;
        MAC_RecvData(OTA_MASTER_FIRST_RX_DURATION);
    }
    else if (OTA_SLAVE_STATE_FW_VERSION_READY == SlaveCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, OTA_DURATION)){
        SlaveCtrl.State = OTA_SLAVE_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota time out%d\n",__LINE__);
        return;
      }
//      tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] OTA_SLAVE_STATE_FW_VERSION_READY \n");
        if (Msg) {
//          tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
//          tlkapi_send_string_data((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota packet", Msg, 8);
            //if receive a valid rf packet
            if (Msg->Type == OTA_MSG_TYPE_DATA) {
            	Len = (int)Msg->Data[0];
            	RxFrame.Type = Msg->Data[1];
            	memcpy(RxFrame.Payload, Msg->Data+2, Len-1);
                //if receive the valid FW version request
                if ((OTA_FRAME_TYPE_CMD == RxFrame.Type) &&
                (OTA_CMD_ID_VERSION_REQ == RxFrame.Payload[0])) {
                    SlaveCtrl.RetryTimes = 0;
                    //send the FW version response to master
                    SlaveCtrl.State = OTA_SLAVE_STATE_START_READY;
                    state_tick = clock_time() | 1;
                    Len = OTA_BuildCmdFrame(&TxFrame, OTA_CMD_ID_VERSION_RSP, (unsigned char *)&SlaveCtrl.FwVersion, sizeof(SlaveCtrl.FwVersion));
                    MAC_SendData((unsigned char *)&TxFrame, Len);
                    return;
                }
            }

            if (SlaveCtrl.RetryTimes == OTA_RETRY_MAX) {
                SlaveCtrl.State = OTA_SLAVE_STATE_ERROR;
                tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                return;
            }
            SlaveCtrl.RetryTimes++;
            MAC_RecvData(OTA_MASTER_RESPONSE_RX_DURATION);
        }
    }
    else if (OTA_SLAVE_STATE_START_READY == SlaveCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, OTA_DURATION)){
        SlaveCtrl.State = OTA_SLAVE_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota time out%d\n",__LINE__);
        return;
      }
//      tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] OTA_SLAVE_STATE_START_READY\n");
        if (Msg) {
//          tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
//          tlkapi_send_string_data((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota packet", Msg, 8);
            //if receive a valid rf packet
            if (Msg->Type == OTA_MSG_TYPE_DATA) {
				Len = (int)Msg->Data[0];
				RxFrame.Type = Msg->Data[1];
				memcpy(RxFrame.Payload, Msg->Data+2, Len-1);
                if (OTA_FRAME_TYPE_CMD == RxFrame.Type) {
                    //if receive the FW version request again
                    if (OTA_CMD_ID_VERSION_REQ == RxFrame.Payload[0]) {
                        SlaveCtrl.RetryTimes = 0;
                        //send the FW version response again to master
                        MAC_SendData((unsigned char *)&TxFrame, Len);
//                        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                        return;
                    }
                    //if receive the OTA start request
                    if (OTA_CMD_ID_START_REQ == RxFrame.Payload[0]) {
//                      tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                        SlaveCtrl.RetryTimes = 0;
                        memcpy(&SlaveCtrl.MaxBlockNum, &RxFrame.Payload[1], sizeof(SlaveCtrl.MaxBlockNum));
                        //send the OTA start response to master
                        SlaveCtrl.State = OTA_SLAVE_STATE_DATA_READY;
                        state_tick = clock_time() | 1;
                        Len = OTA_BuildCmdFrame(&TxFrame, OTA_CMD_ID_START_RSP, 0, 0);
                        MAC_SendData((unsigned char *)&TxFrame, Len);
                        return;
                    }
                }
            }

            if (SlaveCtrl.RetryTimes == OTA_RETRY_MAX) {
                SlaveCtrl.State = OTA_SLAVE_STATE_ERROR;
                tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                return;
            }
            SlaveCtrl.RetryTimes++;
            MAC_RecvData(OTA_MASTER_RESPONSE_RX_DURATION);
        }
    }
    else if (OTA_SLAVE_STATE_DATA_READY == SlaveCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, OTA_DURATION)){
        SlaveCtrl.State = OTA_SLAVE_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota time out%d\n",__LINE__);
        return;
      }
//      tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] OTA_SLAVE_STATE_DATA_READY\n");
        if (Msg) {
//          tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
//          tlkapi_send_string_data((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota packet", Msg, 8);
            //if receive a valid rf packet
            if (Msg->Type == OTA_MSG_TYPE_DATA) {
				Len = (int)Msg->Data[0];
				RxFrame.Type = Msg->Data[1];
				memcpy(RxFrame.Payload, Msg->Data+2, Len-1);
                //if receive the OTA start request again
                if (OTA_FRAME_TYPE_CMD == RxFrame.Type) {
                    if (OTA_CMD_ID_START_REQ == RxFrame.Payload[0]) {
                        SlaveCtrl.RetryTimes = 0;
                        //send the OTA start response again to master
                        MAC_SendData((unsigned char *)&TxFrame, Len);//need change
                        return;
                    }
                }
                //if receive the OTA data frame
                if (OTA_FRAME_TYPE_DATA == RxFrame.Type) {
                    //check the block number included in the incoming frame
                    unsigned short BlockNum = RxFrame.Payload[1];
                    BlockNum <<= 8;
                    BlockNum += RxFrame.Payload[0];
                    //if receive the same OTA data frame again, just respond with the same ACK
                    if (BlockNum == SlaveCtrl.BlockNum) {
                        SlaveCtrl.RetryTimes = 0;
                        //send the OTA data ack again to master
                        MAC_SendData((unsigned char *)&TxFrame, Len);
                        return;
                    }
                    //if receive the next OTA data frame, just respond with an ACK
                    if (BlockNum == SlaveCtrl.BlockNum + 1) {
                        SlaveCtrl.RetryTimes = 0;
//                        printf("block_num:%d, len:%d, PktCRC:%2x\r\n", BlockNum, Len - 3, SlaveCtrl.PktCRC);
                        /*
                         * write received data to flash,
                         * and avoid first block data writing boot flag as head of time.
                         */
                        if(1 == BlockNum)
                        {
                            // unfill boot flag in ota procedure
                            flash_write_page(SlaveCtrl.FlashAddr, 8, &RxFrame.Payload[2]);
                            flash_write_page(SlaveCtrl.FlashAddr + 12, Len - 3 - 12, &RxFrame.Payload[2 + 12]);
                        }
                        else
                        {

                            //write received data to flash
                            flash_write_page(SlaveCtrl.FlashAddr + SlaveCtrl.TotalBinSize, Len - 3, &RxFrame.Payload[2]);
                        }

                        SlaveCtrl.BlockNum = BlockNum;
                        SlaveCtrl.TotalBinSize += (Len - 3);

                        if (SlaveCtrl.MaxBlockNum == BlockNum) {
                        	SlaveCtrl.PktCRC = OTA_CRC16_Cal(SlaveCtrl.PktCRC, &RxFrame.Payload[2], Len - 3 - OTA_APPEND_INFO_LEN);
                            SlaveCtrl.State = OTA_SLAVE_STATE_END_READY;
                            state_tick = clock_time() | 1;
                        }
                        else
                        {
                        	SlaveCtrl.PktCRC = OTA_CRC16_Cal(SlaveCtrl.PktCRC, &RxFrame.Payload[2], Len - 3);
                        }
                        //send the OTA data ack to master
                        unsigned int Length = OTA_BuildAckFrame(&TxFrame, BlockNum);
                        MAC_SendData((unsigned char *)&TxFrame, Length);
                        return;
                    }
                }
            }

            if (SlaveCtrl.RetryTimes == OTA_RETRY_MAX) {
                SlaveCtrl.State = OTA_SLAVE_STATE_ERROR;
                tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                return;
            }
            SlaveCtrl.RetryTimes++;
            MAC_RecvData(OTA_MASTER_RESPONSE_RX_DURATION);
        }
    }
    else if (OTA_SLAVE_STATE_END_READY == SlaveCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, OTA_DURATION)){
        SlaveCtrl.State = OTA_SLAVE_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota time out%d\n",__LINE__);
        return;
      }

//      tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] OTA_SLAVE_STATE_END_READY\n");
        if (Msg) {
//          tlkapi_send_string_data((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] ota packet", Msg, 8);
//          tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
            //if receive a valid rf packet
            if (Msg->Type == OTA_MSG_TYPE_DATA) {
				Len = (int)Msg->Data[0];
				RxFrame.Type = Msg->Data[1];
				memcpy(RxFrame.Payload, Msg->Data+2, Len-1);
//				tlkapi_send_string_data((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] MaxBlockNum", &SlaveCtrl.MaxBlockNum, 2);
                //if receive the last OTA data frame again
                if (OTA_FRAME_TYPE_DATA == RxFrame.Type) {
                    //check the block number included in the incoming frame
                    unsigned short BlockNum = RxFrame.Payload[1];
                    BlockNum <<= 8;
                    BlockNum += RxFrame.Payload[0];
                    //if receive the same OTA data frame again, just respond with the same ACK
                    if (BlockNum == SlaveCtrl.BlockNum) {
                        SlaveCtrl.RetryTimes = 0;
                        //send the OTA data ack again to master
                        MAC_SendData((unsigned char *)&TxFrame, Len);
                        return;
                    }
                }
                //if receive the OTA end request
                if (OTA_FRAME_TYPE_CMD == RxFrame.Type) {
                    if (OTA_CMD_ID_END_REQ == RxFrame.Payload[0]) {
                        SlaveCtrl.RetryTimes = 0;
                        unsigned int BinSize = 0;
                        memcpy(&BinSize, &RxFrame.Payload[1], sizeof(BinSize));
                        if (SlaveCtrl.TotalBinSize != BinSize) {
                            SlaveCtrl.State = OTA_SLAVE_STATE_ERROR;
                            tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                            return;
                        }
                        SlaveCtrl.State = OTA_SLAVE_STATE_END;
                        state_tick = clock_time() | 1;
                        //send the OTA end response to master
                        Len = OTA_BuildCmdFrame(&TxFrame, OTA_CMD_ID_END_RSP, 0, 0);
                        MAC_SendData((unsigned char *)&TxFrame, Len);
                        WaitMs(5); //wait for transmission finished
                        return;
                    }
                }
            }

            if (SlaveCtrl.RetryTimes == OTA_RETRY_MAX) {
                SlaveCtrl.State = OTA_SLAVE_STATE_ERROR;
                tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] %d\n",__LINE__);
                return;
            }
            SlaveCtrl.RetryTimes++;
            MAC_RecvData(OTA_MASTER_RESPONSE_RX_DURATION);
        }
    }
    else if (OTA_SLAVE_STATE_END == SlaveCtrl.State) {
//      tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] OTA_SLAVE_STATE_END\n");
#if 1
        // 1. todo FW crc check
//        int max_block_num = (SlaveCtrl.TotalBinSize + OTA_FRAME_PAYLOAD_MAX -2 - 1) / (OTA_FRAME_PAYLOAD_MAX - 2);
        unsigned char bin_buf[48] = {0};
        int block_idx = 0;
        int len = 0;
        flash_read_page((unsigned long)SlaveCtrl.FlashAddr + SlaveCtrl.TotalBinSize - OTA_APPEND_INFO_LEN,
                2, (unsigned char*)&SlaveCtrl.TargetFwCRC);
        while (1)
        {
            if (SlaveCtrl.TotalBinSize - block_idx * (OTA_FRAME_PAYLOAD_MAX - 2) > (OTA_FRAME_PAYLOAD_MAX - 2))
            {
                len = OTA_FRAME_PAYLOAD_MAX - 2;
                flash_read_page((unsigned long)SlaveCtrl.FlashAddr + block_idx * (OTA_FRAME_PAYLOAD_MAX - 2),
                        len, &bin_buf[0]);
                if (0 == block_idx)
                {
                    // fill the boot flag mannually
                    bin_buf[8] = 0x4b;
                    bin_buf[9] = 0x4e;
                    bin_buf[10] = 0x4c;
                    bin_buf[11] = 0x54;
                }
                SlaveCtrl.FwCRC = OTA_CRC16_Cal(SlaveCtrl.FwCRC, &bin_buf[0], len);
            }
            else
            {
                len = SlaveCtrl.TotalBinSize - (block_idx * (OTA_FRAME_PAYLOAD_MAX - 2))- OTA_APPEND_INFO_LEN;
                flash_read_page((unsigned long)SlaveCtrl.FlashAddr + block_idx * (OTA_FRAME_PAYLOAD_MAX - 2),
                        len, &bin_buf[0]);
                SlaveCtrl.FwCRC = OTA_CRC16_Cal(SlaveCtrl.FwCRC, &bin_buf[0], len);
                break;
            }
            block_idx++;
//            printf("fw block idx:%d, len:%d, FwCRC:%2x\r\n", block_idx, len, SlaveCtrl.FwCRC);
        }
//        printf("fw block idx:%d, len:%d, FwCRC:%2x  \r\n", block_idx + 1, len, SlaveCtrl.FwCRC);
//        printf("pkt_crc:%2x, fw_crc:%2x, target_fw_crc:%2x\r\n", SlaveCtrl.PktCRC, SlaveCtrl.FwCRC, SlaveCtrl.TargetFwCRC);
        if (SlaveCtrl.FwCRC != SlaveCtrl.PktCRC || SlaveCtrl.TargetFwCRC != SlaveCtrl.FwCRC)
        {
            SlaveCtrl.State = OTA_SLAVE_STATE_ERROR;
            tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] Crc Check Error\n");
            return;
        }
#endif

        // set next boot flag
        unsigned int utmp = 0x544C4E4B;
        flash_write_page(SlaveCtrl.FlashAddr + 8, 4, (unsigned char *)&utmp);

        //clear current boot flag
        unsigned char tmp = 0x00;
#if (OTA_SLAVE_BIN_ADDR == OTA_BIN_ADDR_0x20000)
        flash_write_page(SlaveCtrl.FlashAddr ? 0x08 : 0x20008, 1, &tmp);
#else
		flash_write_page(0x20008, 1, &tmp);
		flash_write_page(SlaveCtrl.FlashAddr ? 0x08 : 0x40008, 1, &tmp);
#endif
//        if (SlaveCtrl.FlashAddr == 0x00)
//        {
//           printf("cur_boot_addr:%4x, next_boot_addr:%4x\r\n", OTA_SLAVE_BIN_ADDR, 0x0000);

//        }
//        else if (SlaveCtrl.FlashAddr == OTA_BIN_ADDR_0x20000)
//        {
//        	printf("cur_boot_addr:%4x, next_boot_addr:%4x\r\n", 0x0000, OTA_SLAVE_BIN_ADDR_0x20000);
//        }
//        else
//        {
//        	printf("cur_boot_addr:%4x, next_boot_addr:%4x\r\n", 0x0000, OTA_SLAVE_BIN_ADDR_0x40000);
//        }
        tlkapi_printf(APP_OTA_LOG_EN, "[OTA][FLW] slave ota success\n");
#if (APP_FLASH_PROTECTION_ENABLE)
        flash_prot_op_cb(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END, 0, 0);
#endif
        gpio_set_func(GPIO_LED_WHITE, AS_GPIO);
        gpio_set_output_en(GPIO_LED_WHITE, 1);
        gpio_write(GPIO_LED_WHITE, 1);
        WaitMs(60);
        gpio_write(GPIO_LED_WHITE, 0);
        WaitMs(120);
        gpio_write(GPIO_LED_WHITE, 1);
        WaitMs(60);
        gpio_write(GPIO_LED_WHITE, 0);
        WaitMs(120);
        //reboot
        irq_disable();
//        WaitMs(1000);
//        start_reboot();
#if(APP_PM_ENABLE)
            cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_TIMER, ClockTime() + OTA_REBOOT_WAIT * 16);
#else
            start_reboot();
#endif
    }
    else if (OTA_SLAVE_STATE_ERROR == SlaveCtrl.State) {
      tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] OTA_SLAVE_STATE_ERROR\n");
        //erase the OTA write area
        OTA_FlashErase();
        irq_disable();
        tlkapi_printf(APP_OTA_LOG_EN, "[OTA][FLW] slave ota failed\n");
#if (APP_FLASH_PROTECTION_ENABLE)
        flash_prot_op_cb(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END, 0, 0);
#endif

        	gpio_set_func(GPIO_LED_RED, AS_GPIO);
			gpio_set_output_en(GPIO_LED_RED, 1);
			gpio_write(GPIO_LED_RED, 1);
			WaitMs(60);
			gpio_write(GPIO_LED_RED, 0);
			WaitMs(120);
			gpio_write(GPIO_LED_RED, 1);
			WaitMs(60);
			gpio_write(GPIO_LED_RED, 0);
			WaitMs(120);
#if(APP_PM_ENABLE)
			cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_TIMER, ClockTime() + OTA_REBOOT_WAIT * 16);
#else
            start_reboot();
#endif
    }
}

#endif /*OTA_MASTER_EN*/
#endif
















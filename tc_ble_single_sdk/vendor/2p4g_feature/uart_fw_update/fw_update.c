/********************************************************************************************************
 * @file    fw_update.c
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
#include "driver.h"
#include "tl_common.h"
#include "fw_update.h"
#include "fw_update_phy.h"
#include "timer_event.h"
#include "../../stack/ble/debug/debug.h"
#include "app_config.h"
#if (FEATURE_TEST_MODE == UART_FW_UPDATE)
#define MSG_QUEUE_LEN                       4
#define FW_UPDATE_BIN_SIZE_OFFSET           0x18

#define FW_UPDATE_RESPONSE_WAIT_TIME        (1000*1000) //in us
#define FW_UPDATE_REBOOT_WAIT               (100*1000) //in us
#define FW_UPDATE_BOOT_FLAG_OFFSET          8

typedef struct {
    unsigned int Type;
    unsigned char *Data; //content of msg
} FW_UPDATE_MsgTypeDef;

typedef struct {
    FW_UPDATE_MsgTypeDef Msg[MSG_QUEUE_LEN]; //buffer storing msg
    unsigned char Cnt; //current num of msg
    unsigned char ReadPtr; //ptr of first msg should be read next
} FW_UPDATE_MsgQueueTypeDef;
FW_UPDATE_MsgQueueTypeDef MsgQueue;

FW_UPDATE_FrameTypeDef TxFrame;
FW_UPDATE_FrameTypeDef RxFrame;

u32 state_tick = 0;

static int FW_UPDATE_MsgQueuePush(const unsigned char *Data, const unsigned int Type, FW_UPDATE_MsgQueueTypeDef *Queue)
{
    if (Queue->Cnt < MSG_QUEUE_LEN) {
        Queue->Msg[(Queue->ReadPtr + Queue->Cnt) % MSG_QUEUE_LEN].Data = (unsigned char *)Data;
        Queue->Msg[(Queue->ReadPtr + Queue->Cnt) % MSG_QUEUE_LEN].Type = Type;
        Queue->Cnt++;
        return 1;
    }

    return 0;
}

static FW_UPDATE_MsgTypeDef *FW_UPDATE_MsgQueuePop(FW_UPDATE_MsgQueueTypeDef *Queue)
{
    FW_UPDATE_MsgTypeDef *ret = NULL;

    if (Queue->Cnt > 0) {
        ret = &(Queue->Msg[Queue->ReadPtr]);
        Queue->ReadPtr = (Queue->ReadPtr + 1) % MSG_QUEUE_LEN;
        Queue->Cnt--;
    }

    return ret;
}

static ev_time_event_t *FW_UPDATE_rspWaitTimer = NULL;

static int FW_UPDATE_rspWaitTimerCb(void* arg)
{
    FW_UPDATE_MsgQueuePush(NULL, FW_UPDATE_MSG_TYPE_TIMEOUT, &MsgQueue);
    return -1;
}

static unsigned char FW_UPDATE_CalculateCheckSum(const unsigned char *Buf, unsigned short Len)
{
    unsigned char CheckSum = *Buf;
    unsigned short i;
    for (i = 1; i < Len; i++) {
        CheckSum ^= Buf[i];
    }
    return CheckSum;
}

static int FW_UPDATE_IsCheckSumRight(unsigned char *Data)
{
    unsigned short PayloadLen = Data[3];
    PayloadLen <<= 8;
    PayloadLen += Data[2];

    if (Data[0] == FW_UPDATE_CalculateCheckSum(&Data[1], 3 + PayloadLen)) {
        return 1;
    }
    else {
        return 0;
    }
}

static int FW_UPDATE_BuildCmdFrame(FW_UPDATE_FrameTypeDef *Frame, const unsigned char CmdId, const unsigned char *Value, unsigned short Len)
{
    Frame->Type = FW_UPDATE_FRAME_TYPE_CMD;
    Frame->Len = 1 + Len;
    Frame->Payload[0] = CmdId;
    if (Value) {
        memcpy(&Frame->Payload[1], Value, Len);
    }
    Frame->CheckSum = FW_UPDATE_CalculateCheckSum(&Frame->Type, 3 + Frame->Len);

    return (4 + Frame->Len);
}

unsigned char aaa = 0;
void FW_UPDATE_RxIrq(unsigned char *Data)
{

    if (NULL == Data) {
    	aaa  = 1;
        FW_UPDATE_MsgQueuePush(NULL, FW_UPDATE_MSG_TYPE_INVALID_DATA, &MsgQueue);
    }
    else {

        if (FW_UPDATE_IsCheckSumRight(Data)) {
        	aaa  = 2;
            FW_UPDATE_MsgQueuePush(Data, FW_UPDATE_MSG_TYPE_DATA, &MsgQueue);
        }
        else {
        	aaa  = 3;
            FW_UPDATE_MsgQueuePush(NULL, FW_UPDATE_MSG_TYPE_INVALID_DATA, &MsgQueue);
        }
    }
}

#if (UART_FW_UPDATE_ROLE == SLAVE)
static unsigned short FW_CRC16_Cal(unsigned short crc, unsigned char* pd, int len)
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
#endif

#if (UART_FW_UPDATE_ROLE == MASTER)
#if(INTERNAL_TEST)
extern unsigned int master_test_success_cnt;
extern unsigned int master_test_fail_cnt;
#endif
static FW_UPDATE_CtrlTypeDef MasterCtrl = {0};

static int FW_UPDATE_IsBlockNumMatch(unsigned char *Payload)
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

static int FW_UPDATE_BuildDataFrame(FW_UPDATE_FrameTypeDef *Frame)
{
    Frame->Type = FW_UPDATE_FRAME_TYPE_DATA;
    if ((MasterCtrl.TotalBinSize - MasterCtrl.BlockNum*(FW_UPDATE_FRAME_PAYLOAD_MAX-2)) > (FW_UPDATE_FRAME_PAYLOAD_MAX-2)) {
        Frame->Len = FW_UPDATE_FRAME_PAYLOAD_MAX;
    }
    else {
        //the last data block comes
        Frame->Len = MasterCtrl.TotalBinSize - MasterCtrl.BlockNum*(FW_UPDATE_FRAME_PAYLOAD_MAX-2) + 2;
        MasterCtrl.FinishFlag = 1;
    }
    MasterCtrl.BlockNum++;
    memcpy(Frame->Payload, &MasterCtrl.BlockNum, 2);
    flash_read_page(MasterCtrl.FlashAddr, Frame->Len-2, &Frame->Payload[2]);
    MasterCtrl.FlashAddr += Frame->Len-2;
    Frame->CheckSum = FW_UPDATE_CalculateCheckSum(&Frame->Type, 3 + Frame->Len);

    return (4 + Frame->Len);
}

void FW_UPDATE_MasterInit(unsigned int FWBinAddr, unsigned short FwVer)
{
    MasterCtrl.FlashAddr = FWBinAddr;
    //read the size of FW_UPDATE_bin file
    flash_read_page(MasterCtrl.FlashAddr + FW_UPDATE_BIN_SIZE_OFFSET, 4, (unsigned char *)&MasterCtrl.TotalBinSize);
    MasterCtrl.TotalBinSize += FW_APPEND_INFO_LEN; // APPEND CRC INFO IN BIN TAIL
    MasterCtrl.MaxBlockNum = (MasterCtrl.TotalBinSize + (FW_UPDATE_FRAME_PAYLOAD_MAX - 2) - 1) / (FW_UPDATE_FRAME_PAYLOAD_MAX - 2);
    MasterCtrl.BlockNum = 0;
    MasterCtrl.FwVersion = FwVer;
    MasterCtrl.State = FW_UPDATE_MASTER_STATE_IDLE;
    MasterCtrl.RetryTimes = 0;
    MasterCtrl.FinishFlag = 0;
    state_tick            = 0;
}

void FW_UPDATE_MasterStart(void)
{
    FW_UPDATE_MsgTypeDef *Msg = FW_UPDATE_MsgQueuePop(&MsgQueue);
    static int Len = 0;

    if (FW_UPDATE_MASTER_STATE_IDLE == MasterCtrl.State) {
      if (!state_tick) {
          state_tick = clock_time() | 1;
      }
        Len = FW_UPDATE_BuildCmdFrame(&TxFrame, FW_UPDATE_CMD_ID_VERSION_REQ, 0, 0);
        FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
        MasterCtrl.State = FW_UPDATE_MASTER_STATE_FW_VER_WAIT;
        /* Start the response wait timer*/
        if (FW_UPDATE_rspWaitTimer) {
            ev_unon_timer(&FW_UPDATE_rspWaitTimer);
        }
        FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
    }
    else if (FW_UPDATE_MASTER_STATE_FW_VER_WAIT == MasterCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, FW_UPDATE_DURATION)){
        MasterCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] fw update time out%d\n",__LINE__);
        return;
      }
        if (Msg) {
            //if receive a valid uart packet
            if (Msg->Type == FW_UPDATE_MSG_TYPE_DATA) {
                memcpy(&RxFrame, Msg->Data, 4);
                memcpy(RxFrame.Payload, Msg->Data + 4, RxFrame.Len);
                //if receive the valid FW version response
                if ((FW_UPDATE_FRAME_TYPE_CMD == RxFrame.Type) &&
                (FW_UPDATE_CMD_ID_VERSION_RSP == RxFrame.Payload[0])) {
                    MasterCtrl.RetryTimes = 0;
                    /* Cancel the response wait timer*/
                    if (FW_UPDATE_rspWaitTimer) {
                        ev_unon_timer(&FW_UPDATE_rspWaitTimer);
                    }
                    //compare the received version with that of FW_UPDATE_bin
                    unsigned short Version = RxFrame.Payload[2];
                    Version <<= 8;
                    Version += RxFrame.Payload[1];

                    if (Version < MasterCtrl.FwVersion) {
                        MasterCtrl.State = FW_UPDATE_MASTER_STATE_START_RSP_WAIT;
                        state_tick = clock_time() | 1;
                        Len = FW_UPDATE_BuildCmdFrame(&TxFrame, FW_UPDATE_CMD_ID_START_REQ, (unsigned char *)&MasterCtrl.MaxBlockNum, sizeof(MasterCtrl.MaxBlockNum));
                        FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
                        /* Start the response wait timer*/
                        FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
                    }
                    else {
                        MasterCtrl.State = FW_UPDATE_MASTER_STATE_ERROR;
                    }
                    return;
                }
            }

            if (MasterCtrl.RetryTimes == FW_UPDATE_RETRY_MAX) {
                MasterCtrl.State = FW_UPDATE_MASTER_STATE_ERROR;
                return;
            }
            MasterCtrl.RetryTimes++;
            FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
            /* Start the response wait timer again*/
            if (FW_UPDATE_rspWaitTimer) {
                ev_unon_timer(&FW_UPDATE_rspWaitTimer);
            }
            FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
        }
    }
    else if (FW_UPDATE_MASTER_STATE_START_RSP_WAIT == MasterCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, FW_UPDATE_DURATION)){
        MasterCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] fw update time out%d\n",__LINE__);
        return;
      }
        if (Msg) {
            //if receive a valid rf packet
            if (Msg->Type == FW_UPDATE_MSG_TYPE_DATA) {
                memcpy(&RxFrame, Msg->Data, 4);
                memcpy(RxFrame.Payload, Msg->Data + 4, RxFrame.Len);
                //if receive the valid FW version response
                if ((FW_UPDATE_FRAME_TYPE_CMD == RxFrame.Type) &&
                (FW_UPDATE_CMD_ID_START_RSP == RxFrame.Payload[0])) {
                    MasterCtrl.RetryTimes = 0;
                    /* Cancel the response wait timer*/
                    if (FW_UPDATE_rspWaitTimer) {
                        ev_unon_timer(&FW_UPDATE_rspWaitTimer);
                    }
                    //read FW_UPDATE_bin from flash and packet it in FW_UPDATE data frame
                    MasterCtrl.State = FW_UPDATE_MASTER_STATE_DATA_ACK_WAIT;
                    state_tick = clock_time() | 1;
                    Len = FW_UPDATE_BuildDataFrame(&TxFrame);
                    FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
                    /* Start the response wait timer*/
                    FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
                    return;
                }
            }

            if (MasterCtrl.RetryTimes == FW_UPDATE_RETRY_MAX) {
                MasterCtrl.State = FW_UPDATE_MASTER_STATE_ERROR;
                return;
            }
            MasterCtrl.RetryTimes++;
            FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
            /* Start the response wait timer again*/
            if (FW_UPDATE_rspWaitTimer) {
                ev_unon_timer(&FW_UPDATE_rspWaitTimer);
            }
            FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
        }
    }
    else if (FW_UPDATE_MASTER_STATE_DATA_ACK_WAIT == MasterCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, FW_UPDATE_DURATION)){
        MasterCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] fw update time out%d\n",__LINE__);
        return;
      }
        if (Msg) {
            //if receive a valid rf packet
            if (Msg->Type == FW_UPDATE_MSG_TYPE_DATA) {
                memcpy(&RxFrame, Msg->Data, 4);
                memcpy(RxFrame.Payload, Msg->Data + 4, RxFrame.Len);
                //if receive the valid FW_UPDATE data ack
                if ((FW_UPDATE_FRAME_TYPE_ACK == RxFrame.Type) && FW_UPDATE_IsBlockNumMatch(RxFrame.Payload)) {
                    MasterCtrl.RetryTimes = 0;
                    /* Cancel the response wait timer*/
                    if (FW_UPDATE_rspWaitTimer) {
                        ev_unon_timer(&FW_UPDATE_rspWaitTimer);
                    }

                    if (MasterCtrl.FinishFlag) {
                        MasterCtrl.State = FW_UPDATE_MASTER_STATE_END_RSP_WAIT;
                        state_tick = clock_time() | 1;
                        Len = FW_UPDATE_BuildCmdFrame(&TxFrame, FW_UPDATE_CMD_ID_END_REQ, (unsigned char *)&MasterCtrl.TotalBinSize, sizeof(MasterCtrl.TotalBinSize));
                        FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
                    }
                    else {
                        //read FW_UPDATE_bin from flash and packet it in FW_UPDATE data frame
                        Len = FW_UPDATE_BuildDataFrame(&TxFrame);
                        FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
                    }
                    /* Start the response wait timer*/
                    FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
                    return;
                }
            }

            if (MasterCtrl.RetryTimes == FW_UPDATE_RETRY_MAX) {
                MasterCtrl.State = FW_UPDATE_MASTER_STATE_ERROR;
                return;
            }
            MasterCtrl.RetryTimes++;
            FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
            /* Start the response wait timer again*/
            if (FW_UPDATE_rspWaitTimer) {
                ev_unon_timer(&FW_UPDATE_rspWaitTimer);
            }
            FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
        }
    }
    else if (FW_UPDATE_MASTER_STATE_END_RSP_WAIT == MasterCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, FW_UPDATE_DURATION)){
        MasterCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] fw update time out%d\n",__LINE__);
        return;
      }
        if (Msg) {
            //if receive a valid rf packet
            if (Msg->Type == FW_UPDATE_MSG_TYPE_DATA) {
                memcpy(&RxFrame, Msg->Data, 4);
                memcpy(RxFrame.Payload, Msg->Data + 4, RxFrame.Len);
                //if receive the valid END response
                if ((FW_UPDATE_FRAME_TYPE_CMD == RxFrame.Type) &&
                    (FW_UPDATE_CMD_ID_END_RSP == RxFrame.Payload[0])) {
                    MasterCtrl.RetryTimes = 0;
                    MasterCtrl.State = FW_UPDATE_MASTER_STATE_END;
                    state_tick = clock_time() | 1;
                    /* Cancel the response wait timer*/
                    if (FW_UPDATE_rspWaitTimer) {
                        ev_unon_timer(&FW_UPDATE_rspWaitTimer);
                    }
                    return;
                }
            }

            if (MasterCtrl.RetryTimes == FW_UPDATE_RETRY_MAX) {
                MasterCtrl.State = FW_UPDATE_MASTER_STATE_ERROR;
                return;
            }
            MasterCtrl.RetryTimes++;
            FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
            /* Start the response wait timer*/
            if (FW_UPDATE_rspWaitTimer) {
                ev_unon_timer(&FW_UPDATE_rspWaitTimer);
            }
            FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
        }
    }
    else if (FW_UPDATE_MASTER_STATE_END == MasterCtrl.State) {
      tlkapi_printf(APP_FW_UPDATE_LOG_EN, "[OTA][FLW] master fw update success\n");
#if (APP_FLASH_PROTECTION_ENABLE)
      flash_prot_op_cb(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END, 0, 0);
#endif
        gpio_set_output_en(GPIO_LED_WHITE, 1); //enable output
        gpio_set_input_en(GPIO_LED_WHITE, 0); //disable input
        gpio_write(GPIO_LED_WHITE, 0); //LED Off
        unsigned char cnt = 20;
		while(cnt --) {
			gpio_toggle(GPIO_LED_WHITE);//white
			WaitMs(60);
		}
#if(INTERNAL_TEST)
		master_test_success_cnt++;
#endif
		//start_reboot();
#if(APP_PM_ENABLE && APP_PM_DEEPSLEEP_RETENTION_ENABLE)
		cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW32K, PM_WAKEUP_TIMER, ClockTime() + FW_UPDATE_REBOOT_WAIT * 16);
#else
		start_reboot();
#endif
    }
    else if (FW_UPDATE_MASTER_STATE_ERROR == MasterCtrl.State) {
      tlkapi_printf(APP_FW_UPDATE_LOG_EN, "[OTA][FLW] master fw update failed\n");
#if (APP_FLASH_PROTECTION_ENABLE)
      flash_prot_op_cb(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END, 0, 0);
#endif
        gpio_set_output_en(GPIO_LED_RED, 1); //enable output
        gpio_set_input_en(GPIO_LED_RED, 0); //disable input
        gpio_write(GPIO_LED_RED, 0); //LED Off
		unsigned char cnt = 20;
		while(cnt --) {
			gpio_toggle(GPIO_LED_RED);//red
			WaitMs(60);
		}
#if(INTERNAL_TEST)
        master_test_fail_cnt++;
#endif
		//start_reboot();
#if(APP_PM_ENABLE && APP_PM_DEEPSLEEP_RETENTION_ENABLE)
		cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW32K, PM_WAKEUP_TIMER, ClockTime() + FW_UPDATE_REBOOT_WAIT * 16);
#else
        start_reboot();
#endif
    }
}

#else /*FW_UPDATE_MASTER_EN*/
#define FW_UPDATE_MASTER_LISTENING_DURATION    (5*1000*1000) //in us

static FW_UPDATE_CtrlTypeDef SlaveCtrl = {0};
volatile unsigned char debug_step = 0;
static int FW_UPDATE_BuildAckFrame(FW_UPDATE_FrameTypeDef *Frame, unsigned short BlockNum)
{
    Frame->Type = FW_UPDATE_FRAME_TYPE_ACK;
    Frame->Len = 2;
    Frame->Payload[0] = BlockNum & 0xff;
    Frame->Payload[1] = BlockNum >> 8;

    Frame->CheckSum = FW_UPDATE_CalculateCheckSum(&Frame->Type, 3 + Frame->Len);

    return (4 + Frame->Len);
}

static void FW_UPDATE_FlashErase(void)
{
    int SectorAddr = SlaveCtrl.FlashAddr;
    int i = 0;
    for (i = 0; i < 15; i++) {
        flash_erase_sector(SectorAddr);
        SectorAddr += 0x1000;
    }
}

void FW_UPDATE_SlaveInit(unsigned int FWBinAddr, unsigned short FwVer)
{
    SlaveCtrl.FlashAddr = FWBinAddr;
    SlaveCtrl.BlockNum = 0;
    SlaveCtrl.FwVersion = FwVer;
    SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_IDLE;
    SlaveCtrl.RetryTimes = 0;
    SlaveCtrl.FinishFlag = 0;
    SlaveCtrl.FwCRC = 0;
    SlaveCtrl.PktCRC = 0;
    SlaveCtrl.TargetFwCRC = 0;
    state_tick            = 0;
    //erase the FW_UPDATE write area
    FW_UPDATE_FlashErase();
}

void FW_UPDATE_SlaveStart(void)
{
    FW_UPDATE_MsgTypeDef *Msg = FW_UPDATE_MsgQueuePop(&MsgQueue);
    static int Len = 0;

    if (FW_UPDATE_SLAVE_STATE_IDLE == SlaveCtrl.State) {
        SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_FW_VERSION_READY;
        if (!state_tick) {
            state_tick = clock_time() | 1;
        }
        /* Start the response wait timer*/
        if (FW_UPDATE_rspWaitTimer) {
            ev_unon_timer(&FW_UPDATE_rspWaitTimer);
        }
        FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_MASTER_LISTENING_DURATION);
    }
    else if (FW_UPDATE_SLAVE_STATE_FW_VERSION_READY == SlaveCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, FW_UPDATE_DURATION)){
        SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] fw update time out%d\n",__LINE__);
        return;
      }
        if (Msg) {
            //if receive a valid uart packet
            if (Msg->Type == FW_UPDATE_MSG_TYPE_DATA) {
                memcpy(&RxFrame, Msg->Data, 4);
                memcpy(RxFrame.Payload, Msg->Data + 4, RxFrame.Len);
                //if receive the valid FW version request
                if ((FW_UPDATE_FRAME_TYPE_CMD == RxFrame.Type) &&
                (FW_UPDATE_CMD_ID_VERSION_REQ == RxFrame.Payload[0])) {

                    SlaveCtrl.RetryTimes = 0;
                    /* Cancel the response wait timer*/
                    if (FW_UPDATE_rspWaitTimer) {
                        ev_unon_timer(&FW_UPDATE_rspWaitTimer);
                    }
                    //send the FW version response to master
                    SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_START_READY;
                    state_tick = clock_time() | 1;
                    Len = FW_UPDATE_BuildCmdFrame(&TxFrame, FW_UPDATE_CMD_ID_VERSION_RSP, (unsigned char *)&SlaveCtrl.FwVersion, sizeof(SlaveCtrl.FwVersion));

                    FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
                    /* Start the response wait timer*/
                    FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
                    return;
                }
            }

            if (SlaveCtrl.RetryTimes == FW_UPDATE_RETRY_MAX) {
                SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
                return;
            }
            SlaveCtrl.RetryTimes++;
            /* Start the response wait timer again*/
            if (FW_UPDATE_rspWaitTimer) {
                ev_unon_timer(&FW_UPDATE_rspWaitTimer);
            }
            FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_MASTER_LISTENING_DURATION);

        }
    }
    else if (FW_UPDATE_SLAVE_STATE_START_READY == SlaveCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, FW_UPDATE_DURATION)){
        SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] fw update time out%d\n",__LINE__);
        return;
      }
        if (Msg) {
            //if receive a valid rf packet
            if (Msg->Type == FW_UPDATE_MSG_TYPE_DATA) {
            	debug_step = 1;
                memcpy(&RxFrame, Msg->Data, 4);
                memcpy(RxFrame.Payload, Msg->Data + 4, RxFrame.Len);
                if (FW_UPDATE_FRAME_TYPE_CMD == RxFrame.Type) {
                	debug_step = 2;
                    //if receive the FW version request again
                    if (FW_UPDATE_CMD_ID_VERSION_REQ == RxFrame.Payload[0]) {
                        SlaveCtrl.RetryTimes = 0;
                        /* Cancel the response wait timer*/
                        if (FW_UPDATE_rspWaitTimer) {
                            ev_unon_timer(&FW_UPDATE_rspWaitTimer);
                        }
                        //send the FW version response again to master
                        FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
                        /* Start the response wait timer*/
                        FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
                        return;
                    }
                    //if receive the FW_UPDATE start request
                    if (FW_UPDATE_CMD_ID_START_REQ == RxFrame.Payload[0]) {
                    	debug_step = 3;
                        SlaveCtrl.RetryTimes = 0;
                        /* Cancel the response wait timer*/
                        if (FW_UPDATE_rspWaitTimer) {
                            ev_unon_timer(&FW_UPDATE_rspWaitTimer);
                        }
                        memcpy(&SlaveCtrl.MaxBlockNum, &RxFrame.Payload[1], sizeof(SlaveCtrl.MaxBlockNum));
                        //send the FW_UPDATE start response to master
                        SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_DATA_READY;
                        state_tick = clock_time() | 1;
                        Len = FW_UPDATE_BuildCmdFrame(&TxFrame, FW_UPDATE_CMD_ID_START_RSP, 0, 0);
                        FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
                        /* Start the response wait timer*/
                        FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
                        return;
                    }
                }
            }

            if (SlaveCtrl.RetryTimes == FW_UPDATE_RETRY_MAX) {
                SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
                return;
            }
            SlaveCtrl.RetryTimes++;
            /* Start the response wait timer again*/
            if (FW_UPDATE_rspWaitTimer) {
                ev_unon_timer(&FW_UPDATE_rspWaitTimer);
            }
            FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
        }
    }
    else if (FW_UPDATE_SLAVE_STATE_DATA_READY == SlaveCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, FW_UPDATE_DURATION)){
        SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] fw update time out%d\n",__LINE__);
        return;
      }
        if (Msg) {
            //if receive a valid rf packet
            if (Msg->Type == FW_UPDATE_MSG_TYPE_DATA) {
                memcpy(&RxFrame, Msg->Data, 4);
                memcpy(RxFrame.Payload, Msg->Data + 4, RxFrame.Len);
                //if receive the FW_UPDATE start request again
                if (FW_UPDATE_FRAME_TYPE_CMD == RxFrame.Type) {
                    if (FW_UPDATE_CMD_ID_START_REQ == RxFrame.Payload[0]) {
                        SlaveCtrl.RetryTimes = 0;
                        /* Cancel the response wait timer*/
                        if (FW_UPDATE_rspWaitTimer) {
                            ev_unon_timer(&FW_UPDATE_rspWaitTimer);
                        }
                        //send the FW_UPDATE start response again to master
                        FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
                        /* Start the response wait timer*/
                        FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
                        return;
                    }
                }
                //if receive the FW_UPDATE data frame
                if (FW_UPDATE_FRAME_TYPE_DATA == RxFrame.Type) {
                    //check the block number included in the incoming frame
                    unsigned short BlockNum = RxFrame.Payload[1];
                    BlockNum <<= 8;
                    BlockNum += RxFrame.Payload[0];
                    //if receive the same FW_UPDATE data frame again, just respond with the same ACK
                    if (BlockNum == SlaveCtrl.BlockNum) {
                        SlaveCtrl.RetryTimes = 0;
                        /* Cancel the response wait timer*/
                        if (FW_UPDATE_rspWaitTimer) {
                            ev_unon_timer(&FW_UPDATE_rspWaitTimer);
                        }
                        //send the FW_UPDATE data ack again to master
                        FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
                        /* Start the response wait timer*/
                        FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
                        return;
                    }
                    //if receive the next FW_UPDATE data frame, just respond with an ACK
                    if (BlockNum == SlaveCtrl.BlockNum + 1) {
                        SlaveCtrl.RetryTimes = 0;
                        /* Cancel the response wait timer*/
                        if (FW_UPDATE_rspWaitTimer) {
                            ev_unon_timer(&FW_UPDATE_rspWaitTimer);
                        }
//                        printf("block_num:%d, len:%d, PktCRC:%2x\n", BlockNum, RxFrame.Len - 2, SlaveCtrl.PktCRC);

                        /*
                         * write received data to flash,
                         * and avoid first block data writing boot flag as head of time.
                         */

                        if (1 == BlockNum)
                        {
                            // unfill boot flag in ota procedure
                            flash_write_page(SlaveCtrl.FlashAddr, 8, &RxFrame.Payload[2]);
                            flash_write_page(SlaveCtrl.FlashAddr + 12, RxFrame.Len - 2 - 12, &RxFrame.Payload[2 + 12]);
                        }
                        else
                        {
                            flash_write_page(SlaveCtrl.FlashAddr + SlaveCtrl.TotalBinSize, RxFrame.Len - 2, &RxFrame.Payload[2]);
                        }

                        SlaveCtrl.BlockNum = BlockNum;
                        SlaveCtrl.TotalBinSize += (RxFrame.Len - 2);

                        if (SlaveCtrl.MaxBlockNum == BlockNum) {
                        	SlaveCtrl.PktCRC = FW_CRC16_Cal(SlaveCtrl.PktCRC, &RxFrame.Payload[2], RxFrame.Len - 2 - FW_APPEND_INFO_LEN);
                            SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_END_READY;
                            state_tick = clock_time() | 1;
                        } else {
                        	SlaveCtrl.PktCRC = FW_CRC16_Cal(SlaveCtrl.PktCRC, &RxFrame.Payload[2], RxFrame.Len - 2);
                        }

                        //send the FW_UPDATE data ack to master
                        Len = FW_UPDATE_BuildAckFrame(&TxFrame, BlockNum);
                        FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
                        /* Start the response wait timer again*/
                        FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
                        return;
                    }
                }
            }

            if (SlaveCtrl.RetryTimes == FW_UPDATE_RETRY_MAX) {
                SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
                return;
            }
            SlaveCtrl.RetryTimes++;
            /* Start the response wait timer again*/
            if (FW_UPDATE_rspWaitTimer) {
                ev_unon_timer(&FW_UPDATE_rspWaitTimer);
            }
            FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
        }
    }
    else if (FW_UPDATE_SLAVE_STATE_END_READY == SlaveCtrl.State) {
      if(state_tick && clock_time_exceed(state_tick, FW_UPDATE_DURATION)){
        SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
        tlkapi_printf((stkLog_mask & STK_LOG_OTA_FLOW), "[OTA][FLW] fw update time out%d\n",__LINE__);
        return;
      }
        if (Msg) {
            //if receive a valid rf packet
            if (Msg->Type == FW_UPDATE_MSG_TYPE_DATA) {
                memcpy(&RxFrame, Msg->Data, 4);
                memcpy(RxFrame.Payload, Msg->Data + 4, RxFrame.Len);
                //if receive the last FW_UPDATE data frame again
                if (FW_UPDATE_FRAME_TYPE_DATA == RxFrame.Type) {
                    //check the block number included in the incoming frame
                    unsigned short BlockNum = RxFrame.Payload[1];
                    BlockNum <<= 8;
                    BlockNum += RxFrame.Payload[0];
                    //if receive the same FW_UPDATE data frame again, just respond with the same ACK
                    if (BlockNum == SlaveCtrl.BlockNum) {
                        SlaveCtrl.RetryTimes = 0;
                        /* Cancel the response wait timer */
                        if (FW_UPDATE_rspWaitTimer) {
                            ev_unon_timer(&FW_UPDATE_rspWaitTimer);
                        }
                        //send the FW_UPDATE data ack again to master
                        FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
                        /* Start the response wait timer */
                        FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
                        return;
                    }
                }
                //if receive the FW_UPDATE end request
                if (FW_UPDATE_FRAME_TYPE_CMD == RxFrame.Type) {
                    if (FW_UPDATE_CMD_ID_END_REQ == RxFrame.Payload[0]) {
                        SlaveCtrl.RetryTimes = 0;
                        /* Cancel the response wait timer */
                        if (FW_UPDATE_rspWaitTimer) {
                            ev_unon_timer(&FW_UPDATE_rspWaitTimer);
                        }
                        unsigned int BinSize = 0;
                        memcpy(&BinSize, &RxFrame.Payload[1], sizeof(BinSize));
                        if (SlaveCtrl.TotalBinSize != BinSize) {
                            SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
                            return;
                        }

                        SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_END;
                        state_tick = clock_time() | 1;
                        //send the FW_UPDATE end response to master
                        Len = FW_UPDATE_BuildCmdFrame(&TxFrame, FW_UPDATE_CMD_ID_END_RSP, 0, 0);
                        FW_UPDATE_PHY_SendData((unsigned char *)&TxFrame, Len);
                        WaitMs(250); //wait for transmission finished
                        return;
                    }
                }
            }

            if (SlaveCtrl.RetryTimes == FW_UPDATE_RETRY_MAX) {
                SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
                return;
            }
            SlaveCtrl.RetryTimes++;
            /* Start the response wait timer again*/
            if (FW_UPDATE_rspWaitTimer) {
                ev_unon_timer(&FW_UPDATE_rspWaitTimer);
            }
            FW_UPDATE_rspWaitTimer = ev_on_timer(FW_UPDATE_rspWaitTimerCb, NULL, FW_UPDATE_RESPONSE_WAIT_TIME);
        }
    }
    else if (FW_UPDATE_SLAVE_STATE_END == SlaveCtrl.State) {

        // 1. todo FW crc check
        unsigned char bin_buf[64] = {0};
        int block_idx = 0;
        int len = 0;
//        SlaveCtrl.TotalBinSize -= FW_APPEND_INFO_LEN;
        flash_read_page((unsigned long)SlaveCtrl.FlashAddr + SlaveCtrl.TotalBinSize - FW_APPEND_INFO_LEN,
                2, (unsigned char *)&SlaveCtrl.TargetFwCRC);
        while (1)
        {
            if (SlaveCtrl.TotalBinSize - block_idx * (FW_UPDATE_FRAME_PAYLOAD_MAX -2) > (FW_UPDATE_FRAME_PAYLOAD_MAX - 2))
            {
                len = FW_UPDATE_FRAME_PAYLOAD_MAX - 2;
                flash_read_page((unsigned long)SlaveCtrl.FlashAddr + block_idx * (FW_UPDATE_FRAME_PAYLOAD_MAX - 2),
                        len, &bin_buf[0]);
                if (0 == block_idx)
                {
                    // fill the boot flag mannually
                    bin_buf[8] = 0x4b;
                    bin_buf[9] = 0x4e;
                    bin_buf[10] = 0x4c;
                    bin_buf[11] = 0x54;
                }
                SlaveCtrl.FwCRC = FW_CRC16_Cal(SlaveCtrl.FwCRC, &bin_buf[0], len);
            }
            else
            {
                len = SlaveCtrl.TotalBinSize - (block_idx * (FW_UPDATE_FRAME_PAYLOAD_MAX - 2)) - FW_APPEND_INFO_LEN;
                flash_read_page((unsigned long)SlaveCtrl.FlashAddr + block_idx * (FW_UPDATE_FRAME_PAYLOAD_MAX - 2),
                          len, &bin_buf[0]);
                  SlaveCtrl.FwCRC = FW_CRC16_Cal(SlaveCtrl.FwCRC, &bin_buf[0], len);
                  break;
            }
            block_idx++;
//            printf("fw block idx:%d, len:%d, FwCRC:%2x\r\n", block_idx, len, SlaveCtrl.FwCRC);
        }
//        printf("fw block idx:%d, len:%d, FwCRC:%2x  \r\n", block_idx + 1, len, SlaveCtrl.FwCRC);
//        printf("pkt_crc:%2x, fw_crc:%2x, target_fw_crc:%2x\r\n", SlaveCtrl.PktCRC, SlaveCtrl.FwCRC, SlaveCtrl.TargetFwCRC);
        if (SlaveCtrl.FwCRC != SlaveCtrl.PktCRC || SlaveCtrl.TargetFwCRC != SlaveCtrl.FwCRC)
        {
//            printf("Crc Check Error\r\n");
            SlaveCtrl.State = FW_UPDATE_SLAVE_STATE_ERROR;
            return;
        }

        // 2. set next boot flag
        unsigned int utmp = 0x544C4E4B;
        flash_write_page(SlaveCtrl.FlashAddr + 8, 4, (unsigned char *)&utmp);

        // 3. clear current boot flag
        unsigned char tmp = 0x00;
#if (FW_UPDATE_SLAVE_BIN_ADDR == FW_UPDATE_BIN_ADDR_20000)
        flash_write_page(SlaveCtrl.FlashAddr ? 0x08 : 0x20008, 1, &tmp);
#else
        flash_write_page(0x20008, 1, &tmp);
        flash_write_page(SlaveCtrl.FlashAddr ? 0x08 : 0x40008, 1, &tmp);
#endif
//        if (SlaveCtrl.FlashAddr == 0x00)
//        {
//            printf("cur_boot_addr:%4x, next_boot_addr:%4x\r\n", FW_UPDATE_SLAVE_BIN_ADDR, 0x0000);

//        }
//        else if (SlaveCtrl.FlashAddr == FW_UPDATE_SLAVE_BIN_ADDR_20000)
//        {
//        	printf("cur_boot_addr:%4x, next_boot_addr:%4x\r\n", 0x0000, FW_UPDATE_SLAVE_BIN_ADDR_20000);
//        }
//        else
//        {
//        	printf("cur_boot_addr:%4x, next_boot_addr:%4x\r\n", 0x0000, FW_UPDATE_SLAVE_BIN_ADDR_40000);
//        }
        tlkapi_printf(APP_FW_UPDATE_LOG_EN, "[OTA][FLW] slave fw update success\n");
#if (APP_FLASH_PROTECTION_ENABLE)
      flash_prot_op_cb(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END, 0, 0);
#endif
        //reboot
        irq_disable();
        gpio_set_output_en(GPIO_LED_WHITE, 1); //enable output
		gpio_set_input_en(GPIO_LED_WHITE, 0); //disable input
		gpio_write(GPIO_LED_WHITE, 0); //LED Off
		unsigned char cnt = 20;
		while(cnt --){
			gpio_toggle(GPIO_LED_WHITE);//green
			WaitMs(50);
		}
//		start_reboot();
#if(APP_PM_ENABLE)
        cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_TIMER, ClockTime() + FW_UPDATE_REBOOT_WAIT * 16);
#else
        start_reboot();
#endif
        while(1);
    }
    else if (FW_UPDATE_SLAVE_STATE_ERROR == SlaveCtrl.State) {
      tlkapi_printf(APP_FW_UPDATE_LOG_EN, "[OTA][FLW] slave fw update failed\n");
        //erase the FW_UPDATE write area
        FW_UPDATE_FlashErase();
        //reboot
        irq_disable();
        gpio_set_output_en(GPIO_LED_RED, 1); //enable output
		gpio_set_input_en(GPIO_LED_RED, 0); //disable input
		gpio_write(GPIO_LED_RED, 0); //LED Off
		unsigned char cnt = 20;
		while(cnt --) {
			gpio_toggle(GPIO_LED_RED);//red
			WaitMs(60);
		}
#if (APP_FLASH_PROTECTION_ENABLE)
      flash_prot_op_cb(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END, 0, 0);
#endif
//		start_reboot();
#if(APP_PM_ENABLE)
        cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_TIMER, ClockTime() + FW_UPDATE_REBOOT_WAIT * 16);
#else
        start_reboot();
#endif
		while (1);
    }
}

#endif /*FW_UPDATE_MASTER_EN*/
#endif

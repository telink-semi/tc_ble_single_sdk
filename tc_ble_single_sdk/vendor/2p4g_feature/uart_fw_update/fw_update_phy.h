/********************************************************************************************************
 * @file    fw_update_phy.h
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
#ifndef _FW_UPDATE_PHY_H_
#define _FW_UPDATE_PHY_H_

typedef void (*PHY_Cb_t)(unsigned char *Data);

extern void FW_UPDATE_PHY_Init(const PHY_Cb_t RxCb);

extern int FW_UPDATE_PHY_SendData(const unsigned char *Payload, const int PayloadLen);

extern void FW_UPDATE_PHY_RxIrqHandler(void);

extern void FW_UPDATE_PHY_TxIrqHandler(void);

#endif /*_FW_UPDATE_PHY_H_*/

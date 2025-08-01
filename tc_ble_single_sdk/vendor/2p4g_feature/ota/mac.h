/********************************************************************************************************
 * @file    mac.h
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
#ifndef _MAC_H_
#define _MAC_H_
#if (FEATURE_TEST_MODE == OTA)
typedef void (*MAC_Cb)(unsigned char *Data);

extern void MAC_Init(const unsigned short Channel,
                     const MAC_Cb RxCb,
                     const MAC_Cb RxTimeoutCb,
                     const MAC_Cb RxFirstTimeoutCb);

extern void MAC_SendData(const unsigned char *Payload,
                        const int PayloadLen);

extern void MAC_RecvData(unsigned int TimeUs);
extern void MAC_RxIrqHandler(void);
extern void MAC_RxTimeOutHandler(void);
extern void MAC_RxFirstTimeOutHandler(void);

#endif /* _MAC_H_ */
#endif

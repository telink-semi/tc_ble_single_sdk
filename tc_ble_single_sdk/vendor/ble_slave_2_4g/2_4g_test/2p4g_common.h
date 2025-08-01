/********************************************************************************************************
 * @file    2p4g_common.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    12,2021
 *
 * @par     Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#ifndef _2P4G_COMMON_H
#define _2P4G_COMMON_H
extern volatile unsigned char irq_bleModeFlag;

int app_mainloop_2p4g(unsigned int  wakeup_tick);
void concurrent_irq_handler(void);
void app_exit_2p4gMode();
void app_idle_loop_2p4g();
#endif

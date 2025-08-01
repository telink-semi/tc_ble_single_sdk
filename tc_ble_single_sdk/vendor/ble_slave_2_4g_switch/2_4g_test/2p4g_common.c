/********************************************************************************************************
 * @file    2p4g_common.c
 *
 * @brief   This is the source file for B80
 *
 * @author  2.4G Group
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
#include "2p4g_common.h"
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "../app_ui.h"
#include "../app_config.h"
#include "driver.h"
#if(TEST_2P4G_MODE)
void app_idle_loop_2p4g()
{
#if (UI_LED_ENABLE)
    gpio_write(GPIO_LED_GREEN,0);
#endif
#if (UI_KEYBOARD_ENABLE)
    proc_keyboard (0, 0, 0);

    if(reboot_flag_t){//(blc_ll_getCurrentState() != BLS_LINK_STATE_CONN) &&
        start_reboot();
        while(1);
    }
#endif
}
#endif


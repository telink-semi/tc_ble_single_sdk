/********************************************************************************************************
 * @file    tl_common.h
 *
 * @brief   This is the header file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#pragma once


#include "common/types.h"
#include "common/bit.h"
#include "common/utility.h"
#include "common/static_assert.h"
#include "common/assert.h"
#include "common/sdk_version.h"

#include "application/print/u_printf.h"

#include "vendor/common/user_config.h"
#include "config.h"
#include "common/string.h"


#include "vendor/common/app_common.h"
#include "vendor/common/ble_flash.h"
#include "vendor/common/blt_fw_sign.h"
#include "vendor/common/blt_led.h"
#include "vendor/common/blt_soft_timer.h"
#include "vendor/common/flash_fw_check.h"
#include "vendor/common/flash_prot.h"
#include "vendor/common/tlkapi_debug.h"
#include "vendor/common/app_buffer.h"
#include "vendor/common/simple_sdp.h"
#include "vendor/common/custom_pair.h"
#include "vendor/common/sync_word_check.h"

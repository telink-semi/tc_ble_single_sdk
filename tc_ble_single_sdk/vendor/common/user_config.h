/********************************************************************************************************
 * @file    user_config.h
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


#if (__PROJECT_8258_BLE_REMOTE__ || __PROJECT_8278_BLE_REMOTE__|| __PROJECT_TC321X_BLE_REMOTE__)
	#include "vendor/ble_remote/app_config.h"
#elif (__PROJECT_8258_BLE_SAMPLE__ || __PROJECT_8278_BLE_SAMPLE__ || __PROJECT_TC321X_BLE_SAMPLE__)
	#include "vendor/ble_sample/app_config.h"
#elif (__PROJECT_8258_MODULE__ || __PROJECT_8278_MODULE__ || __PROJECT_TC321X_MODULE__)
	#include "vendor/ble_module/app_config.h"
#elif (__PROJECT_8258_HCI__ || __PROJECT_8278_HCI__ || __PROJECT_TC321X_HCI__)
	#include "vendor/ble_hci/app_config.h"
#elif (__PROJECT_8258_FEATURE_TEST__ || __PROJECT_8278_FEATURE_TEST__ || __PROJECT_TC321X_FEATURE_TEST__)
	#include "vendor/ble_feature_test/app_config.h"
#elif(__PROJECT_8258_MASTER_KMA_DONGLE__ || __PROJECT_8278_MASTER_KMA_DONGLE__ )
	#include "vendor/ble_master_kma_dongle/app_config.h"
#elif(__PROJECT_8258_INTERNAL_TEST__ ||  __PROJECT_8278_INTERNAL_TEST__)
	#include "vendor/internal_test/app_config.h"
#elif (__PROJECT_TC321X_2P4G_GENFSK_LL__ || __PROJECT_8258_2P4G_GENFSK_LL__ || __PROJECT_8278_2P4G_GENFSK_LL__)
    #include "vendor/2p4g_genfsk_ll/app_config.h"
#elif (__PROJECT_TC321X_2P4G_TPLL__ || __PROJECT_8258_2P4G_TPLL__ || __PROJECT_8278_2P4G_TPLL__)
    #include "vendor/2p4g_tpll/app_config.h"
#elif (__PROJECT_TC321X_2P4G_TPSLL__ || __PROJECT_8258_2P4G_TPSLL__ || __PROJECT_8278_2P4G_TPSLL__)
    #include "vendor/2p4g_tpsll/app_config.h"
#elif (__PROJECT_TC321X_2P4G_FEATURE__ || __PROJECT_8258_2P4G_FEATURE__ || __PROJECT_8278_2P4G_FEATURE__)
    #include "vendor/2p4g_feature/app_config.h"
#elif (__PROJECT_TC321X_BLE_SLAVE_2P4G__ || __PROJECT_8258_BLE_SLAVE_2P4G__ || __PROJECT_8278_BLE_SLAVE_2P4G__)
    #include "vendor/ble_slave_2_4g/app_config.h"
#elif (__PROJECT_TC321X_BLE_SLAVE_2P4G_SWITCH__ || __PROJECT_8258_BLE_SLAVE_2P4G_SWITCH__ || __PROJECT_8278_BLE_SLAVE_2P4G_SWITCH__)
    #include "vendor/ble_slave_2_4g_switch/app_config.h"
#else
	#include "vendor/common/default_config.h"
#endif



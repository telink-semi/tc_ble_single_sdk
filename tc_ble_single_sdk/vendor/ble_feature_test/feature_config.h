/********************************************************************************************************
 * @file    feature_config.h
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
#ifndef FEATURE_CONFIG_H_
#define FEATURE_CONFIG_H_



/////////////////// TEST FEATURE SELECTION /////////////////////////////////
//ble link layer test
#define	TEST_ADVERTISING_ONLY							1
#define TEST_SCANNING_ONLY								2
#define TEST_ADVERTISING_IN_CONN_SLAVE_ROLE				3
#define TEST_SCANNING_IN_ADV_AND_CONN_SLAVE_ROLE        4
#define TEST_ADVERTISING_SCANNING_IN_CONN_SLAVE_ROLE	5



//power test
#define TEST_POWER_ADV									10 //ADV power

//smp test
#define TEST_SMP_SECURITY								20 //If testing SECURITY, such as Passkey Entry or Numric_Comparison, we use the remote control board for testing

//gatt secure test
#define TEST_GATT_SECURITY								21 //If testing SECURITY, such as Passkey Entry or Numric_Comparison, we use the remote control board for testing

//slave LL data length exchange test
#define TEST_LL_DATA_LENGTH_EXTENSION_SLAVE				22 //Slave LL DLE test

//master LL data length exchange test
#define TEST_LL_DATA_LENGTH_EXTENSION_MASTER			23 //Master LL DLE test

 //phy test
#define TEST_BLE_PHY									32 // BQB PHY_TEST demo

#define TEST_MD_MASTER									33 //Master more data test

#define TEST_MD_SLAVE									34 //Slave more data test

#define TEST_EXTENDED_ADVERTISING						40 //Extended ADV test

#define TEST_2M_CODED_PHY_CONNECTION					60 //  2M/Coded PHY used on Legacy_ADV and Connection

#define	TEST_OTA_BIG_PDU								70 // OTA for big PDU

#define TEST_LL_PRIVACY_SLAVE                           80 //Privacy Slave local RPA
#define	TEST_LL_PRIVACY_MASTER							81 //Privacy Master local RPA

#define	TEST_OTA_HID									90 //OTA on HID channel

//some simple function test
#define TEST_USER_BLT_SOFT_TIMER						100 //software timer
#define TEST_WHITELIST									110 //whitelist
#define	TEST_CSA2										120 //CSA #2

#define	TEST_MULTIPLE_LOCAL_DEVICE						125 //multiple local device

#define TEST_USER_BLT_SOFT_UART							130 //software UART, TC321X don't support
#define TEST_L2CAP_COC								    140 //L2CAP COC
#define TEST_USB_CDC									150 //USB CDC, TC321X don't support

#define TEST_FEATURE_BACKUP								200




#define FEATURE_TEST_MODE								TEST_FEATURE_BACKUP






#endif /* FEATURE_CONFIG_H_ */

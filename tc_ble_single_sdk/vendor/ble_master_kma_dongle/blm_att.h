/********************************************************************************************************
 * @file    blm_att.h
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
#ifndef BLM_ATT_H_
#define BLM_ATT_H_



/**
 * @brief       this function serves to set current ReadRequest attribute handle
 * @param[in]	handle - connect handle
 * @return      none
 */
void 	app_setCurrentReadReq_attHandle(u16 handle);

/**
 * @brief       call this function when report keyboard
 * @param[in]	conn - connect handle
 * @param[in]	p - pointer of l2cap data packet
 * @return      none
 */
void	att_keyboard (u16 conn, u8 *p);

/**
 * @brief       call this function when report consumer key
 * @param[in]	conn - connect handle
 * @param[in]	p - pointer of l2cap data packet
 * @return      none
 */
void	att_keyboard_media (u16 conn, u8 *p);

/**
 * @brief       call this function when attribute handle:HID_HANDLE_MOUSE_REPORT
 * @param[in]	conn - connect handle
 * @param[in]	p - pointer of l2cap data packet
 * @return      none
 */
void 	att_mouse(u16 conn, u8 *p);

/**
 * @brief       this function serves to clear host attribute data
 * @param[in]	none
 * @return      none
 */
void 	host_att_data_clear(void);


/**
 * @brief      callback function of google voice service discovery
 * @param[in]  db16 -
 * @param[in]  db128 -
 * @return     none
 */
void app_google_voice_service_discovery(att_db_uuid16_t *dbUUid16, att_db_uuid128_t *dbUUid128);


#endif /* BLM_ATT_H_ */

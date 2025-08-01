/********************************************************************************************************
 * @file    app_ui.h
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
#ifndef APP_UI_H_
#define APP_UI_H_


void app_ui_init_normal(void);
void app_ui_init_deepRetn(void);


/**
 * @brief      this function is used to detect if key pressed or released.
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void proc_keyboard(u8 e, u8 *p, int n);





#if (BLE_REMOTE_OTA_ENABLE)
	void app_enter_ota_mode(void);
	void app_debug_ota_result(int result);
#endif



extern 	u8 		key_type;
extern	int 	key_not_released;

extern	int 	ir_not_released;
extern	u8 		user_key_mode;
extern	u8      ir_hw_initialed;
extern	u8 		ota_is_working;
extern	int		lowBatt_alarmFlag;











#endif /* APP_UI_H_ */

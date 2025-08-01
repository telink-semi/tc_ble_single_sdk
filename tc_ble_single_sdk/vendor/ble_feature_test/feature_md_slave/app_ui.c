/********************************************************************************************************
 * @file    app_ui.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2022
 *
 * @par     Copyright (c) 2022, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "../feature_config.h"

#if (FEATURE_TEST_MODE == TEST_MD_SLAVE)


#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"
#include "application/keyboard/keyboard.h"
#include "application/usbstd/usbkeycode.h"
#include "application/usbstd/usbkeycode.h"

#include "app.h"
#include "app_att.h"
#include "app_ui.h"

extern u32	latest_user_event_tick;



_attribute_data_retention_	int 	key_not_released;
_attribute_data_retention_	u8 		key_type;
_attribute_data_retention_	static u32 keyScanTick = 0;


#define CONSUMER_KEY   	   		1
#define KEYBOARD_KEY   	   		2



/**
 * @brief		this function is used to process keyboard matrix status change.
 * @param[in]	none
 * @return      none
 */
void key_change_proc(void)
{

	latest_user_event_tick = clock_time();  //record latest key change time


	//u8 key0 = kb_event.keycode[0];
	//u8 key_buf[8] = {0,0,0,0,0,0,0,0};

	key_not_released = 1;
	if (kb_event.cnt == 2)   //two key press, do  not process
	{

	}
	else if(kb_event.cnt == 1)
	{
		tlkapi_printf(APP_LOG_EN, "Key press");
	}
	else   //kb_event.cnt == 0,  key release
	{
		tlkapi_printf(APP_LOG_EN, "Key release");
	}

}



/**
 * @brief      this function is used to detect if key pressed or released.
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void proc_keyboard(u8 e, u8 *p, int n)
{
	if(clock_time_exceed(keyScanTick, 8000)){
		keyScanTick = clock_time();
	}
	else{
		return;
	}

	kb_event.keycode[0] = 0;
	int det_key = kb_scan_key (0, 1);

	if (det_key){
		key_change_proc();
	}
}


#endif  //end of (FEATURE_TEST_MODE == ...)

/********************************************************************************************************
 * @file    blm_att.c
 *
 * @brief   This is the source file for BLE SDK
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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "app.h"
#include "blm_att.h"
#include "blm_host.h"

#include "application/keyboard/keyboard.h"
#include "application/app/usbmouse.h"

#define     TELINK_UNPAIR_KEYVALUE		0xFF  //conn state, unpair


u8 read_by_type_req_uuid[16] = {};
u8 read_by_type_req_uuidLen;

u16 	current_read_req_handle;

/**
 * @brief       host layer set current readByTypeRequest UUID
 * @param[in]	uuid
 * @param[in]	uuid_len - uuid byte number
 * @return      none
 */
void host_att_set_current_readByTypeReq_uuid(u8 *uuid, u8 uuid_len)
{
	read_by_type_req_uuidLen = uuid_len;
	memcpy(read_by_type_req_uuid, uuid, uuid_len);
}


extern void usbmouse_add_frame (mouse_data_t *packet_mouse, int packet_num);

/**
 * @brief       call this function when attribute handle:HID_HANDLE_MOUSE_REPORT
 * @param[in]	conn - connect handle
 * @param[in]	p - pointer of l2cap data packet
 * @return      none
 */
void	att_mouse (u16 conn, u8 *p)
{
	(void)conn;(void)p;
#if (USB_MOUSE_ENABLE)
	mouse_data_t mouse_dat_report;
	mouse_dat_report.btn = *p++;
	mouse_dat_report.x = *p++;
	mouse_dat_report.y = *p++;
	mouse_dat_report.wheel = *p;

	extern void usbmouse_add_frame (mouse_data_t *packet_mouse, int packet_num);
	usbmouse_add_frame(&mouse_dat_report, 1);
#endif

}



extern void usbkb_hid_report(kb_data_t *data);
extern void report_to_KeySimTool(u8 len,u8 * keycode);
extern void usbkb_report_consumer_key(u16 consumer_key);

extern void report_media_key_to_KeySimTool(u16);

/**
 * @brief       call this function when report consumer key
 * @param[in]	conn - connect handle
 * @param[in]	p - pointer of l2cap data packet
 * @return      none
 */
void	att_keyboard_media (u16 conn, u8 *p)
{
	(void)conn;(void)p;
	u16 media_key = p[0] | p[1]<<8;

	usbkb_report_consumer_key(media_key);
}

//////////////// keyboard ///////////////////////////////////////////////////
int Adbg_att_kb_cnt = 0;
kb_data_t		kb_dat_report = {1, 0, {0,0,0,0,0,0} };
int keyboard_not_release = 0;
extern int 	central_unpair_enable;

/**
 * @brief       call this function when report keyboard
 * @param[in]	conn - connect handle
 * @param[in]	p - pointer of l2cap data packet
 * @return      none
 */
void	att_keyboard (u16 conn, u8 *p)
{
	(void)conn;(void)p;
	Adbg_att_kb_cnt ++;

	memcpy(&kb_dat_report, p, sizeof(kb_data_t));

	if(kb_dat_report.keycode[0] == TELINK_UNPAIR_KEYVALUE){ //slave special unpair cmd

		if(!central_unpair_enable){
			central_unpair_enable = 1;
		}

		return;  //TELINK_UNPAIR_KEYVALUE not report
	}


	if (kb_dat_report.keycode[0])  			//keycode[0]
	{
		kb_dat_report.cnt = 1;  //1 key value
		keyboard_not_release = 1;
	}
	else{
		kb_dat_report.cnt = 0;  //key release
		keyboard_not_release = 0;
	}


	usbkb_hid_report((kb_data_t *) &kb_dat_report);
}



/**
 * @brief       call this function when keyboard release
 * @param[in]	none
 * @return      none
 */
void att_keyboard_release(void)
{
	kb_dat_report.cnt = 0;  //key release
//	usbkb_hid_report((kb_data_t *) &kb_dat_report);
}



////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

/**
 * @brief       this function serves to clear host attribute data
 * @param[in]	none
 * @return      none
 */
void host_att_data_clear(void)
{
	if(keyboard_not_release){
		keyboard_not_release = 0;
		att_keyboard_release();
	}
}


/**
 * @brief       this function serves to set current ReadRequest attribute handle
 * @param[in]	handle - connect handle
 * @return      none
 */
void app_setCurrentReadReq_attHandle(u16 handle)
{
	current_read_req_handle = handle;
}

/**
 * @brief       this function serves to get current ReadRequest attribute handle
 * @param[in]	none
 * @return      current ReadRequest attribute handle
 */
u16 app_getCurrentReadReq_attHandle(void)
{
	return current_read_req_handle;
}






// ************************************************************************************

const u8 sAudioGoogleTXUUID[16]   = WRAPPING_BRACES(AUDIO_GOOGL_TX_CHAR_UUID);
const u8 sAudioGoogleRXUUID[16]   = WRAPPING_BRACES(AUDIO_GOOGL_RX_CHAR_UUID);
const u8 sAudioGoogleCTLUUID[16]   = WRAPPING_BRACES(AUDIO_GOOGL_CTL_CHAR_UUID);

/**
 * @brief      callback function of google voice service discovery
 * @param[in]  none
 * @return     none
 */
void app_google_voice_service_discovery(att_db_uuid16_t *dbUUid16, att_db_uuid128_t *dbUUid128)
{

#if(TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE)
	conn_char_handler[8] = blm_att_findHandleOfUuid128 (dbUUid128, sAudioGoogleTXUUID);
	conn_char_handler[9] = blm_att_findHandleOfUuid128 (dbUUid128, sAudioGoogleRXUUID);
	conn_char_handler[10] = blm_att_findHandleOfUuid128 (dbUUid128, sAudioGoogleCTLUUID);
	#if (GOOGLE_VOICE_OVER_BLE_SPCE_VERSION == GOOGLE_VERSION_1_0)
		u8 caps_data[6]={0};
		caps_data[0] = 0x0A; //get caps
		caps_data[1] = 0x00;
		caps_data[2] = 0x01; // version 0x0100;
		caps_data[3] = 0x00;
		caps_data[4] = 0x03; // legacy 0x0003;
		caps_data[5] = GOOGLE_VOICE_MODE;
		if(conn_char_handler[8]){
			blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,conn_char_handler[8],caps_data,6);
		}

		u8 rx_ccc[2] = {0x00, 0x01};		//Write Rx CCC value for PTV use case
		blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,conn_char_handler[9]+1,rx_ccc,2);
	#else
		u8 caps_data[5]={0};
		caps_data[0] = 0x0A; //get caps
		caps_data[1] = 0x00;
		caps_data[2] = 0x04; // version 0x0004;
		caps_data[3] = 0x00;
		caps_data[4] = 0x03; // legacy 0x0003;
		blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,conn_char_handler[8],caps_data,5);
	#endif
#endif

}



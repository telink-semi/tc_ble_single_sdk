/********************************************************************************************************
 * @file    app_audio.c
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
#include "application/audio/tl_audio.h"
#include "application/audio/audio_config.h"
#include "app.h"
#include "app_audio.h"

#if (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_TELINK)			//GATT Telink

u8		att_mic_rcvd = 0;
u32		tick_adpcm;
u8		buff_mic_adpcm[MIC_ADPCM_FRAME_SIZE];

u32		tick_iso_in;
int		mode_iso_in;


/**
 * @brief		usb_endpoints_irq_handler
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{
	u32 t = clock_time ();
	/////////////////////////////////////
	// ISO IN
	/////////////////////////////////////
	if (reg_usb_irq & BIT(7)) {
		mode_iso_in = 1;
		tick_iso_in = t;
		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

		/////// get MIC input data ///////////////////////////////
		//usb_iso_in_1k_square ();
		//usb_iso_in_from_mic ();
		abuf_dec_usb ();
	}

}

/**
 * @brief		call this function to process when attHandle equal to AUDIO_HANDLE_MIC
 * @param[in]	conn - connect handle
 * @param[in]	p - Pointer point to l2cap data packet.
 * @return      none
 */
void	att_mic (u16 conn, u8 *p)
{
	(void)conn;
	att_mic_rcvd = 1;
	memcpy (buff_mic_adpcm, p, MIC_ADPCM_FRAME_SIZE);
	abuf_mic_add ((u32 *)buff_mic_adpcm);
}

/**
 * @brief		audio proc in main loop
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void proc_audio (void)
{
	if (att_mic_rcvd)
	{
		tick_adpcm = clock_time ();
		att_mic_rcvd = 0;
	}
	if (clock_time_exceed (tick_adpcm, 200000))
	{
		tick_adpcm = clock_time ();
		abuf_init ();
	}
	abuf_mic_dec ();
}

#elif  (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE)			//GATT GOOGLE
u8		att_mic_rcvd = 0;
u32		tick_adpcm;
u8		buff_mic_adpcm[MIC_ADPCM_FRAME_SIZE];

u32		tick_iso_in;
int		mode_iso_in;

volatile u8 google_audio_start;

/**
 * @brief		usb_endpoints_irq_handler
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{
	u32 t = clock_time ();
	/////////////////////////////////////
	// ISO IN
	/////////////////////////////////////
	if (reg_usb_irq & BIT(7)) {
		mode_iso_in = 1;
		tick_iso_in = t;
		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

		/////// get MIC input data ///////////////////////////////
		//usb_iso_in_1k_square ();
		//usb_iso_in_from_mic ();
		abuf_dec_usb ();
	}

}

/**
 * @brief		call this function to process when attHandle equal to AUDIO_HANDLE_MIC
 * @param[in]	conn - connect handle
 * @param[in]	p - Pointer point to l2cap data packet
 * @return      none
 */
void	att_mic (u16 conn, u8 *p)
{
	(void)conn;(void)p;
	att_mic_rcvd = 1;
	memcpy (buff_mic_adpcm, p, MIC_ADPCM_FRAME_SIZE);
	abuf_mic_add ((u32 *)buff_mic_adpcm);
}

/**
 * @brief		copy packet data to defined buffer to process
 * @param[in]	data - Pointer point to l2cap data packet
 * @param[in]	length - the data length
 * @return      none
 */
void app_audio_data(u8 * data, u16 length)
{
	static u8 audio_buffer_serial;
	#if (GOOGLE_VOICE_OVER_BLE_SPCE_VERSION == GOOGLE_VERSION_1_0)
		if(length == 20)
		{
			memcpy(buff_mic_adpcm+audio_buffer_serial*20,data,length);
			audio_buffer_serial++;
			if (audio_buffer_serial == 6) {
				abuf_mic_add ((u32 *)buff_mic_adpcm);
				audio_buffer_serial = 0;
				att_mic_rcvd = 1;
			}
		}
		else if(length == 120)
		{
			memcpy(buff_mic_adpcm,data,length);
			abuf_mic_add ((u32 *)buff_mic_adpcm);
			att_mic_rcvd = 1;
		}
	#else
		if(!google_audio_start)
		{
			return ;
		}
		if(audio_buffer_serial < 6 && length == 20)
		{
			memcpy(buff_mic_adpcm+audio_buffer_serial*20,data,length);
			audio_buffer_serial++;
		}
		else if(audio_buffer_serial==6 && length ==14)
		{
			memcpy(buff_mic_adpcm+audio_buffer_serial*20,data,length);
			abuf_mic_add ((u32 *)buff_mic_adpcm);
			audio_buffer_serial = 0;
			att_mic_rcvd = 1;
		}
		else
		{
			audio_buffer_serial = 0;
		}
	#endif
}

/**
 * @brief		audio proc in main loop
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void proc_audio (void)
{
	if (att_mic_rcvd)
	{
		tick_adpcm = clock_time ();
		att_mic_rcvd = 0;
	}
	if (clock_time_exceed (tick_adpcm, 200000))
	{
		tick_adpcm = clock_time ();
		abuf_init ();
	}
	abuf_mic_dec ();
}
#elif  (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_HID_DONGLE_TO_STB)		//HID Service,ADPCM,Dongle to STB,STB decode

#define MIC_BUFF_NUM	32
#define MIC_DATA_LEN	20

unsigned char mic_dat_buff[MIC_BUFF_NUM][MIC_DATA_LEN];

u8 usb_mic_wptr= 0;
u8 usb_mic_rptr= 0;
u8 audio_id = 0;

/**
 * @brief		usb_endpoints_irq_handler
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{

	if (reg_usb_irq & BIT(7)) {

		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

	}

}

/**
 * @brief		usb_report_hid_mic
 * @param[in]	data - Pointer point to l2cap data packet
 * @param[in]	report_id - the data packet of report id
 * @return      0 - usb is busy and forbidden report hid mic
 *              1 - usb allow to report hid mic
 */
unsigned char usb_report_hid_mic(u8* data, u8 report_id)
{
	if(usbhw_is_ep_busy(USB_EDP_AUDIO_IN))
		return 0;

	reg_usb_ep_ptr(USB_EDP_AUDIO_IN) = 0;

	reg_usb_ep_dat(USB_EDP_AUDIO_IN) = report_id;
	foreach(i, 20){
		reg_usb_ep_dat(USB_EDP_AUDIO_IN) = data[i];
	}

	reg_usb_ep_ctrl(USB_EDP_AUDIO_IN) = FLD_EP_DAT_ACK;		// ACK
	return 1;
}

/**
 * @brief		reset mic_packet,reset audio id and writer pointer and read pointer
 * @param[in]	none
 * @return      none
 */
void mic_packet_reset(void)
{
	audio_id = 0;
	usb_mic_wptr = 0;
	usb_mic_rptr = 0;
}

/**
 * @brief		push_mic_packet
 * @param[in]	p - Pointer point to l2cap data packet
 * @return      none
 */
void push_mic_packet(unsigned char *p)
{
	memcpy(mic_dat_buff[usb_mic_wptr], p, MIC_DATA_LEN);

	usb_mic_wptr =  (usb_mic_wptr+1)&(MIC_BUFF_NUM-1);
}

/**
 * @brief		audio proc in main loop
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void proc_audio (void)
{
	if(usb_mic_wptr != usb_mic_rptr)
	{
		static u32 cnt = 0;
		u8 *pdat = mic_dat_buff[usb_mic_rptr];
		if(usb_report_hid_mic(pdat, (audio_id + 10))==1)
		{
			audio_id += 1;
			audio_id = audio_id%3;
			cnt += 1;
			usb_mic_rptr =  (usb_mic_rptr+1)&(MIC_BUFF_NUM-1);
		}
	}
	return;
}

#elif  (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_HID)		//HID Service,ADPCM,Dongle decode

u8		att_mic_rcvd = 0;
u32		tick_adpcm;

u32		tick_iso_in;
int		mode_iso_in;


extern u8 tmp_mic_data[];

/**
 * @brief		usb_endpoints_irq_handler
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{
	u32 t = clock_time ();
	/////////////////////////////////////
	// ISO IN
	/////////////////////////////////////
	if (reg_usb_irq & BIT(7)) {
		mode_iso_in = 1;
		tick_iso_in = t;
		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

		/////// get MIC input data ///////////////////////////////
		//usb_iso_in_1k_square ();
		//usb_iso_in_from_mic ();
		abuf_dec_usb ();
	}

}

/**
 * @brief		call this function to process when attHandle equal to AUDIO_HANDLE_MIC
 * @param[in]	conn - connect handle
 * @param[in]	p - Pointer point to l2cap data packet.
 * @return      none
 */
void	att_mic (u16 conn, u8 *p)
{
	memcpy (tmp_mic_data, p, MIC_ADPCM_FRAME_SIZE);
	abuf_mic_add ((u32 *)tmp_mic_data);
}

/**
 * @brief		audio proc in main loop
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void proc_audio (void)
{
	if (att_mic_rcvd)
	{
		tick_adpcm = clock_time ();
		att_mic_rcvd = 0;
	}
	if (clock_time_exceed (tick_adpcm, 3*1000*1000))
	{
		tick_adpcm = clock_time ();
	}
	abuf_mic_dec ();
}
#elif  (TL_AUDIO_MODE == TL_AUDIO_DONGLE_SBC_HID_DONGLE_TO_STB)		//HID Service,Dongle to STB,STB decode

#define MIC_BUFF_NUM	32
#define MIC_DATA_LEN	20

unsigned char mic_dat_buff[MIC_BUFF_NUM][MIC_DATA_LEN];

u8 usb_mic_wptr= 0;
u8 usb_mic_rptr= 0;
u8 audio_id = 0;

/**
 * @brief		usb_endpoints_irq_handler
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{

	if (reg_usb_irq & BIT(7)) {

		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

	}

}

/**
 * @brief		usb_report_hid_mic
 * @param[in]	data - Pointer point to l2cap data packet
 * @param[in]	report_id - the data packet of report id
 * @return      0 - usb is busy and forbidden report hid mic
 *              1 - usb allow to report hid mic
 */
unsigned char usb_report_hid_mic(u8* data, u8 report_id)
{
	if(usbhw_is_ep_busy(USB_EDP_AUDIO_IN))
		return 0;

	reg_usb_ep_ptr(USB_EDP_AUDIO_IN) = 0;

	reg_usb_ep_dat(USB_EDP_AUDIO_IN) = report_id;
	foreach(i, 20){
		reg_usb_ep_dat(USB_EDP_AUDIO_IN) = data[i];
	}

	reg_usb_ep_ctrl(USB_EDP_AUDIO_IN) = FLD_EP_DAT_ACK;		// ACK
	return 1;
}

/**
 * @brief		reset mic_packet,reset audio id and writer pointer and read pointer
 * @param[in]	none
 * @return      none
 */
void mic_packet_reset(void)
{
	audio_id = 0;
	usb_mic_wptr = 0;
	usb_mic_rptr = 0;
}

/**
 * @brief		push_mic_packet
 * @param[in]	p - Pointer point to l2cap data packet
 * @return      none
 */
void push_mic_packet(unsigned char *p)
{
	memcpy(mic_dat_buff[usb_mic_wptr], p, MIC_DATA_LEN);

	usb_mic_wptr =  (usb_mic_wptr+1)&(MIC_BUFF_NUM-1);
}

/**
 * @brief		audio proc in main loop
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void proc_audio (void)
{
	if(usb_mic_wptr != usb_mic_rptr)
	{
		static u32 cnt = 0;
		u8 *pdat = mic_dat_buff[usb_mic_rptr];
		if(usb_report_hid_mic(pdat, (audio_id + 10))==1)
		{
			audio_id += 1;
			audio_id = audio_id%3;
			cnt += 1;
			usb_mic_rptr =  (usb_mic_rptr+1)&(MIC_BUFF_NUM-1);
		}
	}
	return;
}

#elif  (TL_AUDIO_MODE == TL_AUDIO_DONGLE_SBC_HID)			//HID Service,SBC,Dongle decode

u8		att_mic_rcvd = 0;
u32		tick_adpcm;

u32		tick_iso_in;
int		mode_iso_in;


extern u8 tmp_mic_data[];

/**
 * @brief		usb_endpoints_irq_handler
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{
	u32 t = clock_time ();
	/////////////////////////////////////
	// ISO IN
	/////////////////////////////////////
	if (reg_usb_irq & BIT(7)) {
		mode_iso_in = 1;
		tick_iso_in = t;
		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

		/////// get MIC input data ///////////////////////////////
		//usb_iso_in_1k_square ();
		//usb_iso_in_from_mic ();
		abuf_dec_usb ();
	}

}

/**
 * @brief		call this function to process when attHandle equal to AUDIO_HANDLE_MIC
 * @param[in]	conn - connect handle
 * @param[in]	p - Pointer point to l2cap data packet.
 * @return      none
 */
void	att_mic (u16 conn, u8 *p)
{
	(void)conn;
	att_mic_rcvd = 1;
	memcpy (tmp_mic_data, p, MIC_ADPCM_FRAME_SIZE);
	abuf_mic_add ((u32 *)tmp_mic_data);
}

/**
 * @brief		audio proc in main loop
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void proc_audio (void)
{
	if (att_mic_rcvd)
	{
		tick_adpcm = clock_time ();
		att_mic_rcvd = 0;
	}
	if (clock_time_exceed (tick_adpcm, 500*1000))
	{
		tick_adpcm = clock_time ();
		abuf_init ();
	}
	abuf_mic_dec ();
}
#elif  (TL_AUDIO_MODE == TL_AUDIO_DONGLE_MSBC_HID)				//HID Service, MSBC, Dongle decode

u8		att_mic_rcvd = 0;
u32		tick_adpcm;

u32		tick_iso_in;
int		mode_iso_in;


extern u8 tmp_mic_data[];

/**
 * @brief		usb_endpoints_irq_handler
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void  usb_endpoints_irq_handler (void)
{
	u32 t = clock_time ();
	/////////////////////////////////////
	// ISO IN
	/////////////////////////////////////
	if (reg_usb_irq & BIT(7)) {
		mode_iso_in = 1;
		tick_iso_in = t;
		reg_usb_irq = BIT(7);	//clear interrupt flag of endpoint 7

		/////// get MIC input data ///////////////////////////////
		//usb_iso_in_1k_square ();
		//usb_iso_in_from_mic ();
		abuf_dec_usb ();
	}

}

/**
 * @brief		call this function to process when attHandle equal to AUDIO_HANDLE_MIC
 * @param[in]	conn - connect handle
 * @param[in]	p - Pointer point to l2cap data packet.
 * @return      none
 */
void	att_mic (u16 conn, u8 *p)
{
	(void)conn;
	att_mic_rcvd = 1;
	memcpy (tmp_mic_data, p, MIC_ADPCM_FRAME_SIZE);
	abuf_mic_add ((u32 *)tmp_mic_data);
}

/**
 * @brief		audio proc in main loop
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void proc_audio (void)
{
	if (att_mic_rcvd)
	{
		tick_adpcm = clock_time ();
		att_mic_rcvd = 0;
	}
	if (clock_time_exceed (tick_adpcm, 500*1000))
	{
		tick_adpcm = clock_time ();
		abuf_init ();
	}
	abuf_mic_dec ();
}
#endif


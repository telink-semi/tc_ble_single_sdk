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
#include "app_config.h"
#include "application/audio/tl_audio.h"
#include "application/audio/audio_config.h"
#include "stack/ble/ble.h"
#include "app.h"
#include "app_att.h"
#include "app_audio.h"
#include "vendor/common/blt_led.h"
#include "battery_check.h"






#if (BLE_AUDIO_ENABLE)
_attribute_data_retention_	u8  	audio_start = 0;
extern u8		buffer_mic_pkt_wptr;
extern u8		buffer_mic_pkt_rptr;
_attribute_data_retention_	u8		ui_mic_enable = 0;
_attribute_data_retention_	u8 		key_voice_press = 0;
_attribute_data_retention_	int     ui_mtu_size_exchange_req = 0;

_attribute_data_retention_	u32     audioProcStart_tick = 0;
_attribute_data_retention_	u32     audioProcDelay_us = 0;

_attribute_data_retention_ u32 	key_voice_pressTick = 0;
u32 	audio_stick = 0;
/**
 * @brief      the func serve to init amic
 * @param[in]  none
 * @return     none
 */
void amic_gpio_reset (void)
{
	gpio_set_func(GPIO_AMIC_BIAS, AS_GPIO);
	gpio_set_input_en(GPIO_AMIC_BIAS, 0);
	gpio_set_output_en(GPIO_AMIC_BIAS, 1);
	gpio_write(GPIO_AMIC_BIAS, 0);

	gpio_set_func(GPIO_AMIC_SP, AS_GPIO);
	gpio_set_input_en(GPIO_AMIC_SP, 0);
	gpio_set_output_en(GPIO_AMIC_SP, 1);
	gpio_write(GPIO_AMIC_SP, 0);

#if (MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_TC321X)
	gpio_set_func(GPIO_AMIC_SN, AS_GPIO);
	gpio_set_input_en(GPIO_AMIC_SN, 0);
	gpio_set_output_en(GPIO_AMIC_SN, 1);
	gpio_write(GPIO_AMIC_SN, 0);
#endif

}

/**
 * @brief      This function is the microphone delay function.
 * @param[in]  delay_time: microphone delay duration, unit is us.
 * @return     none
 */
void audio_proc_delay(u32 delay_time_us)
{
	audioProcDelay_us = delay_time_us;
}


#if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TLEINK)					//GATT Telink
/**
 * @brief      for open the audio and mtu size exchange
 * @param[in]  none
 * @return     none
 */
void voice_press_proc(void)
{
	key_voice_press = 0;
	audio_stick = clock_time()|1;
	ui_enable_mic (1);
	if(ui_mtu_size_exchange_req && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){
		ui_mtu_size_exchange_req = 0;
		blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, 0x009e);
	}
}

/**
 * @brief      This function serves to Request ConnParamUpdate
 * @param[in]  none
 * @return     none
 */
void blc_checkConnParamUpdate(void)
{
	extern u32 interval_update_tick;

	if(	 interval_update_tick && clock_time_exceed(interval_update_tick,5*1000*1000) && \
		 blc_ll_getCurrentState() == BLS_LINK_STATE_CONN &&  bls_ll_getConnectionInterval()!= 8 )
	{
		interval_update_tick = clock_time() | 1;
		bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_10MS, CONN_INTERVAL_10MS, 99, CONN_TIMEOUT_4S);
	}
}

#elif (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE) 				// GATT GOOGLE

#include "application/audio/gl_audio.h"
extern u16		app_audio_sync_serial;
extern u32		app_audio_timer;

_attribute_data_retention_ u8	audio_send_index = 0;
#elif ((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB)\
		|| (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB)\
		|| (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID) )	//HID Service,ADPCM

u8		audio_start_status = 0;
u8		audio_end_status = 0;
u8 		audio_send_idx = 0;
u32 	audio_end_tick = 0;

_attribute_data_retention_	u8  audio_bt_status = 0;
/**
 * @brief      this function is call back function of audio measurement from server to client
 * @param[in]  p:data pointer.
 * @return     will always return 0
 */
int server2client_auido_proc(void* p)
{
	rf_packet_att_data_t *pw = (rf_packet_att_data_t *)p;

	if (pw->dat[0] == APP_AUDIO_BT_OPEN){  //audio start
		if(!ui_mic_enable){
			audio_bt_status = APP_AUDIO_BT_OPEN;
		}

	}
	else if(pw->dat[0] == APP_AUDIO_BT_CLOSE){
		audio_bt_status = APP_AUDIO_BT_CLOSE;
	}
	return 0;
}
#endif

/**
 * @brief      for open the audio and mtu size exchange
 * @param[in]  en   0:close the micphone  1:open the micphone
 * @return     none
 */
void ui_enable_mic (int en)
{
#if (APP_BATT_CHECK_ENABLE)
	battery_set_detect_enable(!en);
#endif
	ui_mic_enable = en;

	//AMIC Bias output
	gpio_set_output_en (GPIO_AMIC_BIAS, en);
	#if (MCU_CORE_TYPE == MCU_CORE_827x)
		gpio_set_data_strength (GPIO_AMIC_BIAS, en);
	#endif
	gpio_write (GPIO_AMIC_BIAS, en);

	#if (UI_LED_ENABLE)
		extern const led_cfg_t led_cfg[];
		device_led_setup(led_cfg[en ? 1 : 2]);
	#endif

	if(en){  //audio on
		#if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
			app_audio_sync_serial = 0;
			bls_pm_setManualLatency(0);
			bls_pm_setSuspendMask(SUSPEND_DISABLE);
			app_audio_timer = clock_time() | 1;
			audio_stick = clock_time() | 1;
			extern u16	adpcm_serial_num;
			adpcm_serial_num = 0;
			audio_send_index = 0;
		#elif ((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB)\
				|| (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB)\
				|| (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID))
			audio_mic_param_init();
			audio_send_idx = 0;
			audio_end_tick = 0;
		#endif
		///////////////////// AUDIO initialization///////////////////
		//buffer_mic set must before audio_init !!!
		#if (MCU_CORE_TYPE == MCU_CORE_TC321X)
			audio_init();
		#else
			audio_config_mic_buf (  (u16*)buffer_mic, TL_MIC_BUFFER_SIZE);
		#endif
		buffer_mic_pkt_rptr = buffer_mic_pkt_wptr = 0;
		#if (MCU_CORE_TYPE == MCU_CORE_827x || MCU_CORE_TYPE == MCU_CORE_TC321X)
			//Amic config
			audio_set_mute_pga(0);  ////enable audio need follow this step: 1 enable bias; 2 disable mute_pga;
			gpio_set_output_en(GPIO_AMIC_SP, 0);
			#if (MCU_CORE_TYPE == MCU_CORE_827x)
				audio_amic_init(AUDIO_16K);							  //3 init; 4 delay about 17ms; 5 enable mute_pga.
			#elif (MCU_CORE_TYPE == MCU_CORE_TC321X)
				gpio_set_output_en(GPIO_AMIC_SN, 0);
				audio_codec_stream0_input_t audio_codec_stream0_input =
				{
					.input_src = AMIC_STREAM0_MONO_L,
					.sample_rate = AUDIO_16K,
					.data_width = CODEC_BIT_16_DATA,
					.fifo_chn = FIFO0,
					.data_buf = buffer_mic,
					.data_buf_size = TL_MIC_BUFFER_SIZE,
				};
				audio_codec_stream0_input_init(&audio_codec_stream0_input);
				audio_set_mute_mic(1);
                audio_codec_clr_input_pop(20);
				audio_set_codec_en(1);
				audio_dfifo_config(FIFO0,(u16*)buffer_mic,TL_MIC_BUFFER_SIZE);
			#endif
		#elif (MCU_CORE_TYPE == MCU_CORE_825x)
			gpio_set_output_en(GPIO_AMIC_SP, 0);
			gpio_set_output_en(GPIO_AMIC_SN, 0);
			audio_amic_init(AUDIO_16K);
		#endif
	#if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
		extern	u32 	latest_user_event_tick;
		latest_user_event_tick = clock_time() | 1;
	#endif
	#if (IIR_FILTER_ENABLE)
		//only used for debugging EQ Filter parameters, removed after mass production
		extern void filter_setting();
		filter_setting();
	#endif
		if(audioProcDelay_us)
		{
			audioProcStart_tick = clock_time()|1;
		}
	}
	else{  //audio off
		#if (MCU_CORE_TYPE == MCU_CORE_827x)
			audio_codec_and_pga_disable();	//power off codec and pga
		#elif (MCU_CORE_TYPE == MCU_CORE_825x)
			adc_power_on_sar_adc(0);   //power off sar adc
		#elif (MCU_CORE_TYPE == MCU_CORE_TC321X)
			audio_codec_adc_power_down();
			audio_power_down();
		#endif
		amic_gpio_reset();
		buffer_mic_pkt_rptr = buffer_mic_pkt_wptr = 0;
		#if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
			audio_stick = 0;
			app_audio_timer = 0;
			audio_start = 0;
		#elif ((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB)\
				|| (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB)\
				|| (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID))
			audio_start_status = 0;
			audio_end_status = 0;
			audio_stick = 0;
			audio_end_tick = 0;
			audio_bt_status = APP_AUDIO_BT_CLOSE;
		#endif
	}
}

/**
 * @brief      this function is used to define what to do when voice key is pressed
 * @param[in]  none
 * @return     none
 */
void key_voice_is_press(void)
{
#if(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TLEINK)
	if(ui_mic_enable){  //if voice on, voice off
		ui_enable_mic (0);
	}
	else{ //if voice not on, mark voice key press tick
		key_voice_press = 1;
		key_voice_pressTick = clock_time();
	}
#elif(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
	if(!ui_mic_enable && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){

#if GOOGLE_AUDIO_DLE
		u8 ret = blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, 0x009e);
#endif
		if(app_audio_key_start(1) == APP_AUDIO_ENABLE){
			ui_enable_mic(1);
		}
	}
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB))
	if(!key_voice_press)
	{
		key_voice_pressTick = clock_time() | 1;//first press tick
		key_voice_press = 1;
		if(!ui_mic_enable && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN)
		{
			if(audio_start_status == 0)
			{
				u8 value[20]={0x99, 0x99, 0x99, 0x21, };// AC_SEARCH
				if(blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
					audio_start_status = 2; //push notify fail
				}
				else{
					audio_start_status = 1; //push notify success
				}
			}
		}
	}
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB)\
		|| (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID))
	if(!ui_mic_enable && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){
		if(audio_start_status == 0){
			u8 value[20]={0x99, 0x99, 0x99, 0x31, };// AC_SEARCH
			if(blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
				audio_start_status = 2; //push notify fail
			}
			else{
				audio_start_status = 1; //push notify success
			}
		}
	}
#endif
}
/**
 * @brief      this function is used to define what to do when voice key is released
 * @param[in]  none
 * @return     none
 */
void key_voice_is_release(void)
{
#if(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TLEINK)
	if(ui_mic_enable){
		ui_enable_mic(0);
	}
#elif(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
	extern u8 app_audio_key_flags;
	extern u8 htt_audio_model_key_press_flags;

	if((app_audio_key_flags & APP_AUDIO_KEY_FLAG_PRESS) || htt_audio_model_key_press_flags)
	{
		app_audio_key_flags &= ~APP_AUDIO_KEY_FLAG_PRESS;

		if(ui_mic_enable){
			if(app_audio_key_start(0) == APP_AUDIO_DISABLE){
				ui_enable_mic(0);
			}
		}
	}
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB))
	if(key_voice_press)
	{
		key_voice_pressTick = 0;
		key_voice_press = 0;
		if(ui_mic_enable && (audio_end_status == 0)){
			u8 value[20]={0x99, 0x99, 0x99, 0x24, };// AC_SEARCH
			if (blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
				audio_end_status = 2;	//push notify fail
			}
			else{
				audio_end_status = 1;	//push notify success
			}
			audio_end_tick = clock_time() | 1;
		}
	}
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB)\
		|| (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID))
	if(ui_mic_enable && (audio_end_status == 0)){
		u8 value[20]={0x99, 0x99, 0x99, 0x34, };// AC_SEARCH
		if (blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
			audio_end_status = 2;	//push notify fail
		}
		else{
			audio_end_status = 1;	//push notify success
		}
		audio_end_tick = clock_time() | 1;
	}
#endif
}

/**
 * @brief      this function is used to check audio state
 * @param[in]  none
 * @return     none
 */
void audio_state_check(void)
{
#if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TLEINK)
	 //long press voice 1 second
	if(key_voice_press && !ui_mic_enable && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN && \
		clock_time_exceed(key_voice_pressTick, 1000000)){

		voice_press_proc();
	}
#elif(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
	// Do nothing
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB))
	if (audio_start_status == 2){//press
		u8 value[20]={0x99, 0x99, 0x99, 0x21, };// AC_SEARCH
		if(!blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
			audio_start_status = 2; //push notify success
		}
	}
	if(audio_end_status == 2){//release
		u8 value[20]={0x99, 0x99, 0x99, 0x24, };// AC_SEARCH
		if (!blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
			audio_end_status = 0;	//push notify success
		}
	}
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB)\
		|| (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID))
	if (audio_start_status == 2){//press
		u8 value[20]={0x99, 0x99, 0x99, 0x31, };// AC_SEARCH
		if(!blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
			audio_start_status = 2; //push notify success
		}
	}
	if(audio_end_status == 2){//release
		u8 value[20]={0x99, 0x99, 0x99, 0x34, };// AC_SEARCH
		if (!blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_CONSUME_REPORT_INPUT_DP_H, value, 20)){
			audio_end_status = 0;	//push notify success
		}
	}
#endif
}

/**
 * @brief      audio task in loop for encode and transmit encode data
 * @param[in]  none
 * @return     none
 */
void task_audio (void)
{
#if(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TLEINK)
	static u32 audioProcTick = 0;
	if(clock_time_exceed(audioProcTick, 500)){
		audioProcTick = clock_time();
	}
	else{
		return;
	}
	if(audioProcStart_tick&&clock_time_exceed(audioProcStart_tick, audioProcDelay_us)){
		memset(buffer_mic, 0, TL_MIC_BUFFER_SIZE);
		audioProcStart_tick = 0;
	}
	else if(audioProcStart_tick!=0){
		return;
	}
	///////////////////////////////////////////////////////////////
	proc_mic_encoder ();

	//////////////////////////////////////////////////////////////////
	if (blc_ll_getTxFifoNumber() < 9)
	{
		int *p = mic_encoder_data_buffer ();
		if (p)					//around 3.2 ms @16MHz clock
		{
			if( BLE_SUCCESS == blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,AUDIO_MIC_INPUT_DP_H, (u8*)p, ADPCM_PACKET_LEN) ){
				mic_encoder_data_read_ok();
			}
		}
	}
#elif(TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
	static u32 audioProcTick = 0;
		if(clock_time_exceed(audioProcTick, 500)){
			audioProcTick = clock_time();
		}
		else{
			return;
		}

		if(audioProcStart_tick&&clock_time_exceed(audioProcStart_tick, audioProcDelay_us)){
			memset(buffer_mic, 0, TL_MIC_BUFFER_SIZE);
			audioProcStart_tick = 0;
		}
		else if(audioProcStart_tick!=0){
			return;
		}


		///////////////////////////////////////////////////////////////

		if(app_audio_timeout_proc()){
			return;
		}
		proc_mic_encoder ();
		///////////////////////////////0.4e///////////////////////////////////
	#if (GOOGLE_AUDIO_VERSION == GOOGLE_AUDIO_V0P4)
		u8 audio_send_length;
		if (blc_ll_getTxFifoNumber() < 6)
		{
			int *p = mic_encoder_data_buffer ();
			if(p)
			{
				for(u8 i=0; i<7; i++)
				{
					if(audio_send_index < 6)
					{
						audio_send_length = 20;
					}
					else if(audio_send_index == 6)
					{
						audio_send_length = 14;
					}
					else
					{
						audio_send_length = 0;
					}
					if(BLE_SUCCESS == blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,AUDIO_GOOGLE_RX_DP_H, (u8*)p+audio_send_index*20, audio_send_length))
					{
						audio_send_index++;
					}
					else
					{
						return ;
					}
					if(audio_send_index == 7)
					{
						audio_send_index = 0;
						mic_encoder_data_read_ok();
					}
				}
			}
		}
	#endif

		////////////////////////////v 1.0////////////////////////////////////////////
	#if (GOOGLE_AUDIO_VERSION == GOOGLE_AUDIO_V1P0)

		// can send data 20 bytes or dle 120 bytes
	#if GOOGLE_AUDIO_DLE
		int *p = mic_encoder_data_buffer();
		u8 dataLen = 120;
		if(blc_ll_getTxFifoNumber() < 7)
		{
			if(p)
			{
				u8 ret = blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,AUDIO_GOOGLE_RX_DP_H, (u8*)p, dataLen);
				if(BLE_SUCCESS == ret){
					mic_encoder_data_read_ok();
				} else{
					return;
				}
			}
		}
	#else
		if(blc_ll_getTxFifoNumber() < 7){
			int *p = mic_encoder_data_buffer();
			u8 dataLen = 20;
			if(p)
			{
				for(int i = 0; i < 6; i++)
				{
					if(BLE_SUCCESS == blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,AUDIO_GOOGLE_RX_DP_H, (u8*)p+audio_send_index*dataLen, dataLen))
					{
						audio_send_index++;
					} else {
						return;
					}
					if(audio_send_index == 6)
					{
						audio_send_index = 0;
						mic_encoder_data_read_ok();
					}
				}
			}
		}
	#endif
	#endif
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB))
	static u32 audioProcTick = 0;
	if(clock_time_exceed(audioProcTick, 500)){
		audioProcTick = clock_time();
	}
	else{
		return;
	}
	if(audioProcStart_tick&&clock_time_exceed(audioProcStart_tick, audioProcDelay_us)){
		memset(buffer_mic, 0, TL_MIC_BUFFER_SIZE);
		audioProcStart_tick = 0;
	}
	else if(audioProcStart_tick!=0){
		return;
	}

	///////////////////////////////////////////////////////////////


	proc_mic_encoder ();

	//////////////////////////////////////////////////////////////////
	if (blc_ll_getTxFifoNumber() < 8 + audio_send_idx)//8 + audio_send_idx
	{
		int *p = mic_encoder_data_buffer ();
		if (p)					//around 3.2 ms @16MHz clock
		{
			u8 i =0;
			u8 handle;
			u8 *buff;
			for(i=0;i<12;i++){
				handle = HID_AUDIO_REPORT_INPUT_FIRST_DP_H + (audio_send_idx%3)*4;//+ (audio_send_idx%3)*4
				buff = ((u8*)(p))+20*audio_send_idx;
				if(BLE_SUCCESS == blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,handle, buff, 20)){
					audio_send_idx++;
				}
				if(audio_send_idx == 6){
					audio_send_idx = 0;
					buffer_mic_pkt_rptr++;
					return;
				}
			}
		}
	}
#elif((TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB)\
		|| (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID))
	static u32 audioProcTick = 0;
	if(clock_time_exceed(audioProcTick, 500)){
		audioProcTick = clock_time();
	}
	else{
		return;
	}

	if(audioProcStart_tick&&clock_time_exceed(audioProcStart_tick, audioProcDelay_us)){
		memset(buffer_mic, 0, TL_MIC_BUFFER_SIZE);
		audioProcStart_tick = 0;
	}
	else if(audioProcStart_tick!=0){
		return;
	}

	///////////////////////////////////////////////////////////////
	proc_mic_encoder ();

	//////////////////////////////////////////////////////////////////
	if (blc_ll_getTxFifoNumber() < 12)//8 + audio_send_idx
	{
		int *p = mic_encoder_data_buffer ();
		if (p)					//around 3.2 ms @16MHz clock
		{
			u8 i =0;
			u8 handle;
			for(i=0;i<3;i++){
				handle = HID_AUDIO_REPORT_INPUT_FIRST_DP_H + (audio_send_idx%3)*4;//+ (audio_send_idx%3)*4
				#if((TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB))
				if(BLE_SUCCESS == blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,handle, (u8*)p +3, 20)){
				#elif(TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID)
					if(BLE_SUCCESS == blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,handle, (u8*)p, ADPCM_PACKET_LEN)){
				#endif
					audio_send_idx++;
					buffer_mic_pkt_rptr++;
					if(audio_send_idx == 3){
						audio_send_idx = 0;
					}
					return;
				}
			}
		}
	}
#endif
}
#endif

/**
 * @brief      audio proc in main loop
 * @param[in]  none
 * @return     none
 */
void proc_audio(void){
#if ((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TLEINK) || (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_GOOGLE) )
	if(ui_mic_enable){
		#if (MCU_CORE_TYPE == MCU_CORE_827x)
		if(audio_start || (audio_stick && clock_time_exceed(audio_stick, 17*1000))){// for 8278
			if(!audio_start)
			{
				audio_set_mute_pga(1);
			}
		#elif (MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_TC321X)
		if(audio_start || (audio_stick && clock_time_exceed(audio_stick, 1*1000))){// for 8258
		#endif
			audio_start = 1;
			task_audio();
		}

	}
	else{
		audio_start = 0;
	}
#elif ((TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_HID_DONGLE_TO_STB)\
		|| (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID) || (TL_AUDIO_MODE == TL_AUDIO_RCU_SBC_HID_DONGLE_TO_STB)\
		|| (TL_AUDIO_MODE == TL_AUDIO_RCU_MSBC_HID))
	if(!ui_mic_enable && key_voice_pressTick && clock_time_exceed(key_voice_pressTick,3*1000*1000)){
		key_voice_pressTick = 0;
		audio_start_status = 0;
		audio_end_status = 0;
		return;
	}
	if(ui_mic_enable && audio_stick && clock_time_exceed(audio_stick, 100*1000*1000)){
		ui_enable_mic (0);
		return;
	}
	if(!key_voice_press && ui_mic_enable && audio_end_tick && clock_time_exceed(audio_end_tick, 200*1000)){
		ui_enable_mic (0);
		return;
	}
	if((!ui_mic_enable) && audio_bt_status == APP_AUDIO_BT_OPEN){
		audio_stick = clock_time()|1;
		ui_enable_mic (1);
		if(ui_mtu_size_exchange_req && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){
			ui_mtu_size_exchange_req = 0;
			blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE, 0x009e);
		}
	}
	if(ui_mic_enable){
		#if (MCU_CORE_TYPE == MCU_CORE_827x)
		if(audio_start || (audio_stick && clock_time_exceed(audio_stick, 17*1000))){// for 8278
			if(!audio_start)
			{
				audio_set_mute_pga(1);
			}
		#elif (MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_TC321X)
		if(audio_start || (audio_stick && clock_time_exceed(audio_stick, 1*1000))){// for 8258
		#endif
			audio_start = 1;
			task_audio();
		}
	}
	else{
		audio_start = 0;
	}
#endif
}

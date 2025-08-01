/********************************************************************************************************
 * @file    app_ui.c
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
#include "app_att.h"
#include "app_config.h"

#include "application/keyboard/keyboard.h"
#include "application/usbstd/usbkeycode.h"
#include "../common/blt_led.h"
#include "../common/blt_soft_timer.h"

#include "rc_ir.h"
#include "rc_ir_learn.h"
#include "battery_check.h"
#include "app_audio.h"


_attribute_data_retention_	u8 		key_type;
_attribute_data_retention_	int 	key_not_released;


_attribute_data_retention_	u8 		ota_is_working = 0;




_attribute_data_retention_	int 	ir_not_released;
_attribute_data_retention_	u8 		user_key_mode;
u8      ir_hw_initialed = 0;   //note: can not be retention variable


#if (STUCK_KEY_PROCESS_ENABLE)
	_attribute_data_retention_	u32 	stuckKey_keyPressTime;
#endif


extern	u32 	latest_user_event_tick;


static const u16 vk_consumer_map[16] = {
		MKEY_VOL_UP,
		MKEY_VOL_DN,
		MKEY_MUTE,
		MKEY_CHN_UP,

		MKEY_CHN_DN,
		MKEY_POWER,
		MKEY_AC_SEARCH,
		MKEY_RECORD,

		MKEY_PLAY,
		MKEY_PAUSE,
		MKEY_STOP,
		MKEY_FAST_FORWARD,  //can not find fast_backword in <<HID Usage Tables>>

		MKEY_FAST_FORWARD,
		MKEY_AC_HOME,
		MKEY_AC_BACK,
		MKEY_MENU,
};





/////////////////////////// led management /////////////////////
#if (UI_LED_ENABLE)

	enum{
		LED_POWER_ON = 0,
		LED_AUDIO_ON,	//1
		LED_AUDIO_OFF,	//2
		LED_SHINE_SLOW, //3
		LED_SHINE_FAST, //4
		LED_SHINE_OTA,  //5
		LED_SHINE_IR_LEARN_OK //6
	};

	const led_cfg_t led_cfg[] = {
			{1000,    0,      1,      0x00,	 },    //power-on, 1s on
			{100,	  0 ,	  0xff,	  0x02,  },    //audio on, long on
			{0,	      100 ,   0xff,	  0x02,  },    //audio off, long off
			{500,	  500 ,   2,	  0x04,	 },    //1Hz for 3 seconds
			{250,	  250 ,   4,	  0x04,  },    //2Hz for 3 seconds
			{250,	  250 ,   200,	  0x08,  },    //2Hz for 50 seconds
			{1000,	  1000,	  3,	  0x10,	 },	   //IR learn OK, 3s on
	};

#endif


#if (REMOTE_IR_LEARN_ENABLE)
	extern ir_learn_ctrl_t *g_ir_learn_ctrl;
	ir_learn_send_t ir_learn_result;
#endif



#if (REMOTE_IR_ENABLE)
	//ir key
	#define TYPE_IR_SEND			1
	#define TYPE_IR_RELEASE			2

	///////////////////// key mode //////////////////////
	#define KEY_MODE_BLE	   		0    //ble key
	#define KEY_MODE_IR        		1    //ir  key


	static const u8 kb_map_ble[30] = 	KB_MAP_BLE;
	static const u8 kb_map_ir[30] = 	KB_MAP_IR;


	void ir_dispatch(u8 type, u8 syscode ,u8 ircode){

		if(!ir_hw_initialed){
			ir_hw_initialed = 1;
			rc_ir_init();
		}
		#if(MCU_CORE_TYPE==MCU_CORE_TC321X)
			ir_learn_ana_tx_en();
		#endif
		if(type == TYPE_IR_SEND){
			ir_nec_send(syscode,~(syscode),ircode);

		}
		else if(type == TYPE_IR_RELEASE){
			ir_send_release();
		}
	}


#endif



#if (BLE_REMOTE_OTA_ENABLE)
	/**
	 * @brief      this function is used to register the function for OTA start.
	 * @param[in]  none
	 * @return     none
	 */
	void app_enter_ota_mode(void)
	{
		ota_is_working = 1;
		#if (UI_LED_ENABLE)
			device_led_setup(led_cfg[LED_SHINE_OTA]);
		#endif
		bls_ota_setTimeout(15 * 1000 * 1000); //set OTA timeout  15 seconds
	}


	/**
	 * @brief       no matter whether the OTA result is successful or fail.
	 *              code will run here to tell user the OTA result.
	 * @param[in]   result    OTA result:success or fail(different reason)
	 * @return      none
	 */
	void app_debug_ota_result(int result)
	{
	    (void)result;

		#if(0 && UI_LED_ENABLE)  //this is only for debug

			gpio_set_output_en(GPIO_LED, 1);

			if(result == OTA_SUCCESS){  //led for debug: OTA success
				gpio_write(GPIO_LED, 1);
				sleep_us(500000);
				gpio_write(GPIO_LED, 0);
				sleep_us(500000);
				gpio_write(GPIO_LED, 1);
				sleep_us(500000);
				gpio_write(GPIO_LED, 0);
				sleep_us(500000);
			}
			else{  //OTA fail

				#if 0 //this is only for debug,  can not use this in application code
					irq_disable();
					WATCHDOG_DISABLE;



					while(1){
						gpio_write(GPIO_LED, 1);
						sleep_us(200000);
						gpio_write(GPIO_LED, 0);
						sleep_us(200000);
					}

				#endif

			}

			gpio_set_output_en(GPIO_LED, 0);
		#endif
	}
#endif






/////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////


//This function process ...
void deep_wakeup_proc(void)
{

#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
	//if deepsleep wakeup is wakeup by GPIO(key press), we must quickly scan this
	//press, hold this data to the cache, when connection established OK, send to master
	//deepsleep_wakeup_fast_keyscan
	if(analog_read(USED_DEEP_ANA_REG) & CONN_DEEP_FLG){
		if(kb_scan_key (KB_NUMLOCK_STATUS_POWERON, 1) && kb_event.cnt){
			deepback_key_state = DEEPBACK_KEY_CACHE;
			key_not_released = 1;
			memcpy(&kb_event_cache,&kb_event,sizeof(kb_event));
		}

		analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) & (~CONN_DEEP_FLG));
	}
#endif
}





void deepback_pre_proc(int *det_key)
{
    (void)det_key;
#if (DEEPBACK_FAST_KEYSCAN_ENABLE)
	// to handle deepback key cache
	if(!(*det_key) && deepback_key_state == DEEPBACK_KEY_CACHE && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN \
			&& clock_time_exceed(bls_ll_getConnectionCreateTime(), 25000)){

		memcpy(&kb_event,&kb_event_cache,sizeof(kb_event));
		*det_key = 1;

		if(key_not_released || kb_event_cache.keycode[0] == VOICE){  //no need manual release
			deepback_key_state = DEEPBACK_KEY_IDLE;
		}
		else{  //need manual release
			deepback_key_tick = clock_time();
			deepback_key_state = DEEPBACK_KEY_WAIT_RELEASE;
		}
	}
#endif
}

void deepback_post_proc(void)
{
#if (DEEPBACK_FAST_KEYSCAN_ENABLE)
	//manual key release
	if(deepback_key_state == DEEPBACK_KEY_WAIT_RELEASE && clock_time_exceed(deepback_key_tick,150000)){
		key_not_released = 0;
		u8 		key_buf[8] = {0};
		key_buf[2] = 0;
		blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
		deepback_key_state = DEEPBACK_KEY_IDLE;
	}
#endif
}

/**
 * @brief		this function is used to process keyboard matrix status change.
 * @param[in]	none
 * @return      none
 */
void key_change_proc(void)
{

	latest_user_event_tick = clock_time();  //record latest key change time
#if BLE_AUDIO_ENABLE
	#if (TL_AUDIO_MODE == TL_AUDIO_RCU_ADPCM_GATT_TLEINK)			//GATT Telink
		if(key_voice_press){  //clear voice key press flg
			key_voice_press = 0;
		}
	#endif
#endif


	u8 key0 = kb_event.keycode[0];
	u8 key1 = kb_event.keycode[1];
	u8 key_value;
	u8 key_buf[8] = {0,0,0,0,0,0,0,0};

	key_not_released = 1;
	if (kb_event.cnt == 2)   //two key press, do  not process
	{
#if (BLE_PHYTEST_MODE != PHYTEST_MODE_DISABLE)  //"enter + back" trigger PhyTest
		//notice that if IR enable, trigger keys must be defined in key map
		if ( (key0 == VK_ENTER && key1 == VK_0) || (key0 == VK_0 && key1 == VK_ENTER))
		{
			extern void app_phytest_init(void);
			extern void app_trigger_phytest_mode(void);
			app_phytest_init();
			app_trigger_phytest_mode();

			device_led_setup(led_cfg[LED_SHINE_FAST]);
		}
#endif
	}
	else if(kb_event.cnt == 1)
	{

		if(key0 == KEY_MODE_SWITCH)
		{
			user_key_mode = !user_key_mode;

			#if (REMOTE_IR_ENABLE)
				if(user_key_mode == KEY_MODE_BLE){
					analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG)&(~IR_MODE_DEEP_FLG));
				}
				else{
					analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG)|IR_MODE_DEEP_FLG);
				}
			#endif

			#if (UI_LED_ENABLE)
				device_led_setup(led_cfg[LED_SHINE_SLOW + user_key_mode]);
			#endif
		}
		else if (key0 == VOICE)
		{
#if (BLE_AUDIO_ENABLE)
			key_voice_is_press();
#endif
		}

#if (REMOTE_IR_ENABLE)
		#if (REMOTE_IR_LEARN_ENABLE)
		else if(key0 == IR_POWER)
		{
			if(!get_ir_learn_state())
			{
				ir_learn_send_init();
				ir_learn_copy_result(&ir_learn_result);
				ir_learn_send(&ir_learn_result);
				device_led_setup(led_cfg[LED_SHINE_FAST]);
			}
			else
			{
				ir_learn_start();
				device_led_setup(led_cfg[LED_AUDIO_ON]);
			}
		}
		#endif
		else if(user_key_mode == KEY_MODE_BLE)
		{

			key_value = kb_map_ble[key0];
			if(key_value >= 0xf0 ){
				key_type = CONSUMER_KEY;
				u16 consumer_key = vk_consumer_map[key_value & 0x0f];
				blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
			}
			else
			{
				key_type = KEYBOARD_KEY;
				key_buf[2] = key_value;
				blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
			}

		}
		else if(user_key_mode == KEY_MODE_IR)
		{  //IR mode
			key_value = kb_map_ir[key0];
			key_type = IR_KEY;
			if(!ir_not_released){
				ir_dispatch(TYPE_IR_SEND, 0x88, key_value);
				ir_not_released = 1;
			}
		}
		else
		{
			key_type = IDLE_KEY;
		}
#else
		else
		{
			key_value = key0;
			if(key_value >= 0xf0 ){
				key_type = CONSUMER_KEY;
				u16 consumer_key = vk_consumer_map[key_value & 0x0f];
				blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
			}
			else
			{
				key_type = KEYBOARD_KEY;
				key_buf[2] = key_value;
				blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
			}
		}

#endif

	}
	else   //kb_event.cnt == 0,  key release
	{
		key_not_released = 0;
		if(key_type == CONSUMER_KEY)
		{
			u16 consumer_key = 0;
			blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
		}
		else if(key_type == KEYBOARD_KEY)
		{
			key_buf[2] = 0;
			blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
		}
#if (REMOTE_IR_ENABLE)
		else if(key_type == IR_KEY)
		{
			if(ir_not_released){
				ir_not_released = 0;
				ir_dispatch(TYPE_IR_RELEASE, 0, 0);  //release
			}
		}
#endif
#if (BLE_AUDIO_ENABLE)
		key_voice_is_release();
#endif
	}


}



#define GPIO_WAKEUP_KEYPROC_CNT				3


_attribute_data_retention_	static int gpioWakeup_keyProc_cnt = 0;
_attribute_data_retention_	static u32 keyScanTick = 0;

/**
 * @brief      this function is used to detect if key pressed or released.
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void proc_keyboard(u8 e, u8 *p, int n)
{
    (void)e;(void)p;(void)n;
#if(REMOTE_IR_LEARN_ENABLE)
	static u8	led_open = 1;
	if(!get_ir_learn_state() )
	{
		if(led_open)
		{
			led_open = 0;
			device_led_setup(led_cfg[LED_SHINE_IR_LEARN_OK]);
		}
	}
	else if(get_ir_learn_state() > IR_LEARN_SUCCESS)
	{
		device_led_setup(led_cfg[LED_AUDIO_OFF]);
	}
#endif

	//when key press gpio wakeup suspend, proc keyscan at least GPIO_WAKEUP_KEYPROC_CNT times
	//regardless of 8000 us interval
	if(e == BLT_EV_FLAG_GPIO_EARLY_WAKEUP){
		gpioWakeup_keyProc_cnt = GPIO_WAKEUP_KEYPROC_CNT;
	}
	else if(gpioWakeup_keyProc_cnt){
		gpioWakeup_keyProc_cnt --;
	}


	if(gpioWakeup_keyProc_cnt || clock_time_exceed(keyScanTick, 8000)){
		keyScanTick = clock_time();
	}
	else{
		return;
	}




	kb_event.keycode[0] = 0;
	int det_key = kb_scan_key (0, 1);


#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
	if(deepback_key_state != DEEPBACK_KEY_IDLE){
		deepback_pre_proc(&det_key);
	}
#endif


	if (det_key){
#if (STUCK_KEY_PROCESS_ENABLE)
	 if(kb_event.cnt){//key press
		 stuckKey_keyPressTime = clock_time() | 1;
	 }
#endif
		key_change_proc();
	}

#if(REMOTE_IR_LEARN_ENABLE)
	ir_learn_detect();
#endif

#if (BLE_AUDIO_ENABLE)
	audio_state_check();
#endif


#if(DEEPBACK_FAST_KEYSCAN_ENABLE)
	if(deepback_key_state != DEEPBACK_KEY_IDLE){
		deepback_post_proc();
	}
#endif
}


extern u32	scan_pin_need;




int gpio_test0(void)
{
	//gpio 0 toggle to see the effect
	DBG_CHN4_TOGGLE;

	return 0;
}


int gpio_test1(void)
{
	//gpio 1 toggle to see the effect
	DBG_CHN5_TOGGLE;


	return 0;

}

int gpio_test2(void)
{
	DBG_CHN6_TOGGLE;

	return 0;
}

int gpio_test3(void)
{
	DBG_CHN7_TOGGLE;

	return 0;
}






void app_ui_init_normal(void)
{


	#if (UI_KEYBOARD_ENABLE)
		/////////// keyboard gpio wakeup init ////////
		u32 pin[] = KB_DRIVE_PINS;
		for (unsigned int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
		{
			cpu_set_gpio_wakeup (pin[i], Level_High,1);  //drive pin pad high wakeup deepsleep
		}

		bls_app_registerEventCallback (BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &proc_keyboard);
	#endif

	#if (BLE_AUDIO_ENABLE)
		amic_gpio_reset();
	#endif


#if (UI_LED_ENABLE)
	device_led_init(GPIO_LED, LED_ON_LEVEL);  //LED initialization
	device_led_setup(led_cfg[LED_POWER_ON]);
#endif


#if (REMOTE_IR_ENABLE)
	user_key_mode = analog_read(USED_DEEP_ANA_REG) & IR_MODE_DEEP_FLG ? KEY_MODE_IR : KEY_MODE_BLE;
#endif



#if (BLT_TEST_SOFT_TIMER_ENABLE)
	blt_soft_timer_init();
	blt_soft_timer_add(&gpio_test0, 35000);
	blt_soft_timer_add(&gpio_test1, 23000);
	blt_soft_timer_add(&gpio_test2, 27000);
	blt_soft_timer_add(&gpio_test3, 33000);
#endif

}



void app_ui_init_deepRetn(void)
{
	#if (UI_KEYBOARD_ENABLE)
		/////////// keyboard gpio wakeup init ////////
		u32 pin[] = KB_DRIVE_PINS;
		for (unsigned int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
		{
			cpu_set_gpio_wakeup (pin[i], Level_High, 1);  //drive pin pad high wakeup deepsleep
		}
	#endif

	#if (BLE_AUDIO_ENABLE)
		amic_gpio_reset();
	#endif

#if (UI_LED_ENABLE)
	device_led_init(GPIO_LED, 1);  //LED initialization
#endif

#if (REMOTE_IR_ENABLE)
	user_key_mode = analog_read(USED_DEEP_ANA_REG) & IR_MODE_DEEP_FLG ? KEY_MODE_IR : KEY_MODE_BLE;
#endif
}



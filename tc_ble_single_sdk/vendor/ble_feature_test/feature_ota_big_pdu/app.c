/********************************************************************************************************
 * @file    app.c
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
#include "../default_att.h"

#include "application/keyboard/keyboard.h"
#include "application/usbstd/usbkeycode.h"


#if (FEATURE_TEST_MODE == TEST_OTA_BIG_PDU)

#define		MY_RF_POWER_INDEX					RF_POWER_P3dBm




/* if user set ACL_CONN_MAX_RX_OCTETS in app_config.h, then RX_FIFO_SIZE will auto calculated in vendor/common/app_buffer.h
 * otherwise, user should define it here */

#define RX_FIFO_NUM						8	//user set value

/* if user set ACL_CONN_MAX_TX_OCTETS in app_config.h, then TX_FIFO_SIZE will auto calculated in vendor/common/app_buffer.h
 * otherwise, user should define it here */

#define TX_FIFO_NUM						8	//user set value



_attribute_data_retention_  u8 		 	blt_rxfifo_b[RX_FIFO_SIZE * RX_FIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	blt_rxfifo = {
												RX_FIFO_SIZE,
												RX_FIFO_NUM,
												0,
												0,
												blt_rxfifo_b,};


_attribute_data_retention_  u8 		 	blt_txfifo_b[TX_FIFO_SIZE * TX_FIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	blt_txfifo = {
												TX_FIFO_SIZE,
												TX_FIFO_NUM,
												0,
												0,
												blt_txfifo_b,};



_attribute_data_retention_ 	int  dle_started_flg = 0;
_attribute_data_retention_ u32 dle_started_tick;


/**
 * @brief	Adv Packet data
 */
const u8	tbl_advData[] = {
	 0x08, DT_COMPLETE_LOCAL_NAME, 'f', 'e', 'a', 't', 'u', 'r', 'e',
	 0x02, DT_FLAGS, 0x05, 							// BLE limited discoverable mode and BR/EDR not supported
	 0x03, DT_APPEARANCE, 0x80, 0x01, 					// 384, Generic Remote Control, Generic category
	 0x05, DT_INCOMPLETE_LIST_16BIT_SERVICE_UUID, 0x12, 0x18, 0x0F, 0x18,		// incomplete list of service class UUIDs (0x1812, 0x180F)
};
/**
 * @brief	Scan Response Packet data
 */
const u8	tbl_scanRsp [] = {
	 0x08, DT_COMPLETE_LOCAL_NAME, 'f', 'e', 'a', 't', 'u', 'r', 'e',
};


_attribute_data_retention_	int device_in_connection_state;


#if (UI_KEYBOARD_ENABLE)
	#define CONSUMER_KEY   	   		1
	#define KEYBOARD_KEY   	   		2
	_attribute_data_retention_	int 	key_not_released;
	_attribute_data_retention_	u8 		key_type;
	_attribute_data_retention_		static u32 keyScanTick = 0;
	extern u32	scan_pin_need;

	/**
	 * @brief   Check changed key value.
	 * @param   none.
	 * @return  none.
	 */
	void key_change_proc(void)
	{
		u8 key0 = kb_event.keycode[0];
		u8 key_buf[8] = {0,0,0,0,0,0,0,0};

		key_not_released = 1;
		if (kb_event.cnt == 2)   //two key press, do  not process
		{

		}
		else if(kb_event.cnt == 1)
		{
			if(key0 >= CR_VOL_UP )  //volume up/down
			{
				key_type = CONSUMER_KEY;
				u16 consumer_key;
				if(key0 == CR_VOL_UP){  	//volume up
					consumer_key = MKEY_VOL_UP;
				}
				else if(key0 == CR_VOL_DN){ //volume down
					consumer_key = MKEY_VOL_DN;
				}
				blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE, HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
			}
			else
			{
				key_type = KEYBOARD_KEY;
				key_buf[2] = key0;
				blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE, HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
			}

		}
		else   //kb_event.cnt == 0,  key release
		{
			key_not_released = 0;
			if(key_type == CONSUMER_KEY)
			{
				u16 consumer_key = 0;
				blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE, HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
			}
			else if(key_type == KEYBOARD_KEY)
			{
				key_buf[2] = 0;
				blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE, HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
			}
		}


	}





	/**
	 * @brief      keyboard task handler
	 * @param[in]  e    - event type
	 * @param[in]  p    - Pointer point to event parameter.
	 * @param[in]  n    - the length of event parameter.
	 * @return     none.
	 */
	void proc_keyboard(u8 e, u8 *p, int n)
	{
	    (void)e;(void)p;(void)n;
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

	/**
	 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_SUSPEND_ENTER"
	 * @param[in]  e - LinkLayer Event type
	 * @param[in]  p - data pointer of event
	 * @param[in]  n - data length of event
	 * @return     none
	 */
	void  task_suspend_enter (u8 e, u8 *p, int n)
	{
	    (void)e;(void)p;(void)n;
		if( blc_ll_getCurrentState() == BLS_LINK_STATE_CONN && ((u32)(bls_pm_getSystemWakeupTick() - clock_time())) > 80 * SYSTEM_TIMER_TICK_1MS){  //suspend time > 30ms.add gpio wakeup
			bls_pm_setWakeupSource(PM_WAKEUP_PAD);  //gpio pad wakeup suspend/deepsleep
		}
	}

#endif

/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_CONNECT"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void	task_connect (u8 e, u8 *p, int n)
{
    (void)e;(void)p;(void)n;
    tlk_contr_evt_connect_t *pConnEvt = (tlk_contr_evt_connect_t *)p;
	tlkapi_send_string_data(APP_LOG_EN, "[APP][EVT] connect, intA & advA:", pConnEvt->initA, 12);
	bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_10MS, CONN_INTERVAL_10MS, 99, CONN_TIMEOUT_4S);  // 1 S

	blc_ll_exchangeDataLength(LL_LENGTH_REQ, ACL_CONN_MAX_TX_OCTETS);

	device_in_connection_state = 1;//

	#if (UI_LED_ENABLE)
		gpio_write(GPIO_LED_RED, LED_ON_LEVEL);  // light on
	#endif
}



/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_TERMINATE"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void 	task_terminate(u8 e,u8 *p, int n) //*p is terminate reason
{
    (void)e;(void)p;(void)n;
    tlk_contr_evt_terminate_t *pEvt = (tlk_contr_evt_terminate_t *)p;
	device_in_connection_state = 0;

	//MTU size exchange and data length exchange procedure must be executed on every new connection,
	//so when connection terminate, relative flags must be cleared
	dle_started_flg = 0;

#if (UI_LED_ENABLE)
	gpio_write(GPIO_LED_RED, !LED_ON_LEVEL);  // light off
#endif


}

/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_DATA_LENGTH_EXCHANGE"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void	task_dle_exchange (u8 e, u8 *p, int n)
{
    (void)e;(void)p;(void)n;

    tlk_contr_evt_dataLenExg_t* dle_param = (tlk_contr_evt_dataLenExg_t*)p;
	tlkapi_printf(APP_LOG_EN, "----- DLE exchange: -----\n");
	tlkapi_printf(APP_LOG_EN, "connEffectiveMaxRxOctets: %d\n", dle_param->connEffectiveMaxRxOctets);
	tlkapi_printf(APP_LOG_EN, "connEffectiveMaxTxOctets: %d\n", dle_param->connEffectiveMaxTxOctets);


	dle_started_flg = 1;

	dle_started_tick = clock_time();
}


/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_SUSPEND_EXIT"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void	task_suspend_exit (u8 e, u8 *p, int n)
{
    (void)e;(void)p;(void)n;
	rf_set_power_level_index (MY_RF_POWER_INDEX);
}

/**
 * @brief      callback function of Host Event
 * @param[in]  h - Host Event type
 * @param[in]  para - data pointer of event
 * @param[in]  n - data length of event
 * @return     0
 */
int app_host_event_callback (u32 h, u8 *para, int n)
{
    (void)h;(void)para;(void)n;
	u8 event = h & 0xFF;

	switch(event)
	{
		case GAP_EVT_SMP_PAIRING_BEGIN:
		{
			gap_smp_pairingBeginEvt_t *pEvt = (gap_smp_pairingBeginEvt_t *)para;
			tlkapi_send_string_data(APP_HOST_EVENT_LOG_EN, "[APP][SMP] paring begin:", pEvt, sizeof(gap_smp_pairingBeginEvt_t));
		}
		break;

		case GAP_EVT_SMP_PAIRING_SUCCESS:
		{
			gap_smp_pairingSuccessEvt_t *pEvt = (gap_smp_pairingSuccessEvt_t *)para;
			tlkapi_send_string_data(APP_HOST_EVENT_LOG_EN, "[APP][SMP] paring success:", pEvt, sizeof(gap_smp_pairingSuccessEvt_t));
		}
		break;

		case GAP_EVT_SMP_PAIRING_FAIL:
		{
			gap_smp_pairingFailEvt_t *pEvt = (gap_smp_pairingFailEvt_t *)para;
			tlkapi_send_string_data(APP_HOST_EVENT_LOG_EN, "[APP][SMP] paring fail:", pEvt, sizeof(gap_smp_pairingFailEvt_t));
		}
		break;

		case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
		{
			gap_smp_connEncDoneEvt_t* pEvt = (gap_smp_connEncDoneEvt_t*)para;
			tlkapi_send_string_data(APP_HOST_EVENT_LOG_EN, "[APP][SMP] paring fail:", pEvt, sizeof(gap_smp_pairingFailEvt_t));

			if(pEvt->re_connect == SMP_STANDARD_PAIR){  //first pairing

			}
			else if(pEvt->re_connect == SMP_FAST_CONNECT){  //auto connect

			}
		}
		break;

		case GAP_EVT_SMP_SECURITY_PROCESS_DONE:
		{
			gap_smp_securityProcessDoneEvt_t *pEvt = (gap_smp_securityProcessDoneEvt_t *)para;
			tlkapi_send_string_data(APP_HOST_EVENT_LOG_EN, "[APP][SMP] security process done:", pEvt, sizeof(gap_smp_securityProcessDoneEvt_t));
		}
		break;

		case GAP_EVT_ATT_EXCHANGE_MTU:
		{
			gap_gatt_mtuSizeExchangeEvt_t *pEvt = (gap_gatt_mtuSizeExchangeEvt_t *)para;
			tlkapi_send_string_data(APP_HOST_EVENT_LOG_EN, "[APP][MTU] mtu exchange", pEvt, sizeof(gap_gatt_mtuSizeExchangeEvt_t));
		}
		break;


		default:
		break;
	}

	return 0;
}

/**
 * @brief      power management code for application
 * @param	   none
 * @return     none
 */
void blt_pm_proc(void)
{
#if(BLE_APP_PM_ENABLE)
	#if (PM_DEEPSLEEP_RETENTION_ENABLE)
		bls_pm_setSuspendMask (SUSPEND_ADV  | SUSPEND_CONN );
//		bls_pm_setSuspendMask (SUSPEND_DISABLE);
	#else
		bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
	#endif

	#if (UI_KEYBOARD_ENABLE)
		if(scan_pin_need || key_not_released){
			bls_pm_setSuspendMask (SUSPEND_DISABLE);
		}
	#endif
#endif  //end of BLE_APP_PM_ENABLE
}






/**
 * @brief		user initialization when MCU power on or wake_up from deepSleep mode
 * @param[in]	none
 * @return      none
 */
void user_init_normal(void)
{
	//random number generator must be initiated here( in the beginning of user_init_nromal)
	//when deepSleep retention wakeUp, no need initialize again
#if(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
	random_generator_init();  //this is must
#endif

	//	debug init
	#if(UART_PRINT_DEBUG_ENABLE)
		tlkapi_debug_init();
		blc_debug_enableStackLog(STK_LOG_DISABLE);
	#endif

	blc_readFlashSize_autoConfigCustomFlashSector();

	/* attention that this function must be called after "blc_readFlashSize_autoConfigCustomFlashSector" !!!*/
	blc_app_loadCustomizedParameters_normal();

////////////////// BLE stack initialization ////////////////////////////////////
	u8  mac_public[6];
	u8  mac_random_static[6];
	//for 512K Flash, flash_sector_mac_address equals to 0x76000
	//for 1M  Flash, flash_sector_mac_address equals to 0xFF000
	blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);
	tlkapi_send_string_data(APP_LOG_EN,"[APP][INI]Public Address", mac_public, 6);


	////// Controller Initialization  //////////
	blc_ll_initBasicMCU();                      //mandatory
	blc_ll_initStandby_module(mac_public);				//mandatory
	blc_ll_initAdvertising_module(mac_public); 	//ADV module: 		 mandatory for BLE slave,
	blc_ll_initConnection_module();				//connection module  mandatory for BLE slave/master
	blc_ll_initSlaveRole_module();				//slave module: 	 mandatory for BLE slave,



	////// Host Initialization  //////////
	blc_gap_peripheral_init();    //gap initialization
	my_att_init(); //gatt initialization
	blc_att_setRxMtuSize(MTU_SIZE_SETTING); //set MTU size, default MTU is 23 if not call this API

	blc_l2cap_register_handler (blc_l2cap_packet_receive);  	//l2cap initialization

	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	blc_smp_peripheral_init();

	// if not set, default is:  send "security request" immediately after link layer connection
	// established(regardless of new connection or reconnection )
	blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000);

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE 	|  \
						  GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE    |  \
						  GAP_EVT_MASK_ATT_EXCHANGE_MTU);



///////////////////// USER application initialization ///////////////////
	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));


	////////////////// config ADV packet /////////////////////
	u8 adv_param_status = BLE_SUCCESS;
	adv_param_status = bls_ll_setAdvParam(  ADV_INTERVAL_30MS, ADV_INTERVAL_35MS,
									 ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
									 0,  NULL,
									 BLT_ENABLE_ADV_ALL,
									 ADV_FP_NONE);
	if(adv_param_status != BLE_SUCCESS) {  	//debug: ADV setting err
		tlkapi_printf(APP_LOG_EN, "[APP][INI] ADV parameters error 0x%x!!!\n", adv_param_status);
		while(1);
	}

	bls_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable



	//set rf power index, user must set it after every suspend wakeup, cause relative setting will be reset in suspend
	rf_set_power_level_index (MY_RF_POWER_INDEX);

	bls_app_registerEventCallback (BLT_EV_FLAG_CONNECT, &task_connect);
	bls_app_registerEventCallback (BLT_EV_FLAG_TERMINATE, &task_terminate);
	bls_app_registerEventCallback (BLT_EV_FLAG_DATA_LENGTH_EXCHANGE, &task_dle_exchange);

	///////////////////// Power Management initialization///////////////////
#if(BLE_APP_PM_ENABLE)
	blc_ll_initPowerManagement_module();        //pm module:      	 optional
	bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_EXIT, &task_suspend_exit);

	#if (PM_DEEPSLEEP_RETENTION_ENABLE)
    	blc_app_setDeepsleepRetentionSramSize(); //select DEEPSLEEP_MODE_RET_SRAM_LOW16K or DEEPSLEEP_MODE_RET_SRAM_LOW32K
		bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
		blc_pm_setDeepsleepRetentionThreshold(95, 95);

		#if(MCU_CORE_TYPE == MCU_CORE_825x)
			blc_pm_setDeepsleepRetentionEarlyWakeupTiming(260);
		#elif((MCU_CORE_TYPE == MCU_CORE_827x))
			blc_pm_setDeepsleepRetentionEarlyWakeupTiming(270);
		#endif
	#else
		bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
	#endif

	#if (UI_KEYBOARD_ENABLE)
		/////////// keyboard gpio wakeup init ////////
		u32 pin[] = KB_DRIVE_PINS;
		for (unsigned int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
		{
			cpu_set_gpio_wakeup (pin[i], Level_High,1);  //drive pin pad high wakeup deepsleep
		}

		bls_app_registerEventCallback (BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &proc_keyboard);
		bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_ENTER, &task_suspend_enter);
	#endif

#else
	bls_pm_setSuspendMask (SUSPEND_DISABLE);
#endif


	/* OTA module initialization must be called before any other OTA API.*/
	blc_ota_initOtaServer_module();

	/* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done.
	 * attention that code will stuck in "while(1)" if any error detected in initialization, user need find what error happens and then fix it */
	blc_app_checkControllerHostInitialization();

	tlkapi_printf(APP_LOG_EN, "[APP][INI] feature_ota_big_pdu init \n");
}



/**
 * @brief		user initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void user_init_deepRetn(void)
{
#if (PM_DEEPSLEEP_RETENTION_ENABLE)
	blc_app_loadCustomizedParameters_deepRetn();
	blc_ll_initBasicMCU();   //mandatory
	rf_set_power_level_index (MY_RF_POWER_INDEX);

	blc_ll_recoverDeepRetention();

	irq_enable();

	#if (UI_KEYBOARD_ENABLE)
		/////////// keyboard gpio wakeup init ////////
		u32 pin[] = KB_DRIVE_PINS;
		for (unsigned int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
		{
			cpu_set_gpio_wakeup (pin[i], Level_High,1);  //drive pin pad high wakeup deepsleep
		}
	#endif

#endif
}

/**
 * @brief     BLE main loop
 * @param[in]  none.
 * @return     none.
 */
void main_loop(void)
{

	////////////////////////////////////// BLE entry /////////////////////////////////
	blt_sdk_main_loop();

	////////////////////////////////////// UI entry /////////////////////////////////
	#if (UI_KEYBOARD_ENABLE)
		proc_keyboard(0, 0, 0);
	#endif

	if(dle_started_flg && clock_time_exceed(dle_started_tick,2000000))
	{
		blc_att_requestMtuSizeExchange(BLS_CONN_HANDLE,MTU_SIZE_SETTING);
		dle_started_flg = 0;
	}
	////////////////////////////////////// PM Process /////////////////////////////////
	blt_pm_proc();
}


#endif  //end of (FEATURE_TEST_MODE == ...)

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


#if (FEATURE_TEST_MODE == TEST_2M_CODED_PHY_CONNECTION)

#define		MY_RF_POWER_INDEX					RF_POWER_P3dBm


#define RX_FIFO_SIZE	64
#define RX_FIFO_NUM		8

#define TX_FIFO_SIZE	40
#define TX_FIFO_NUM		8

//if pm is closed, the follow not need place in retention code
#if BLE_APP_PM_ENABLE
_attribute_data_retention_
#endif
u8 		 	blt_rxfifo_b[RX_FIFO_SIZE * RX_FIFO_NUM] = {0};

#if BLE_APP_PM_ENABLE
_attribute_data_retention_
#endif
my_fifo_t	blt_rxfifo = {
												RX_FIFO_SIZE,
												RX_FIFO_NUM,
												0,
												0,
												blt_rxfifo_b,};

#if BLE_APP_PM_ENABLE
_attribute_data_retention_
#endif
u8 		 	blt_txfifo_b[TX_FIFO_SIZE * TX_FIFO_NUM] = {0};

#if	BLE_APP_PM_ENABLE
_attribute_data_retention_
#endif
my_fifo_t	blt_txfifo = {
												TX_FIFO_SIZE,
												TX_FIFO_NUM,
												0,
												0,
												blt_txfifo_b,};



//////////////////////////////////////////////////////////////////////////////
//	 Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////
const u8	tbl_advData[] = {
	 0x08, DT_COMPLETE_LOCAL_NAME, 'f', 'e', 'a', 't', 'u', 'r', 'e',
	 0x02, DT_FLAGS, 0x05, 							// BLE limited discoverable mode and BR/EDR not supported
	 0x03, DT_APPEARANCE, 0x80, 0x01, 					// 384, Generic Remote Control, Generic category
	 0x05, DT_INCOMPLETE_LIST_16BIT_SERVICE_UUID, 0x12, 0x18, 0x0F, 0x18,		// incomplete list of service class UUIDs (0x1812, 0x180F)
};

const u8	tbl_scanRsp [] = {
	 0x08, DT_COMPLETE_LOCAL_NAME, 'f', 'e', 'a', 't', 'u', 'r', 'e',
};

#if (CONNECTABLE_MODE == EXTENDED_ADV_CONNECTABLE_UNDIRECTED)

	#define	APP_ADV_SETS_NUMBER						1			// Number of Supported Advertising Sets
	#define APP_MAX_LENGTH_ADV_DATA					318			// Maximum Advertising Data Length,   (if legacy ADV, max length 31 bytes is enough)
	#define APP_MAX_LENGTH_SCAN_RESPONSE_DATA		318			// Maximum Scan Response Data Length, (if legacy ADV, max length 31 bytes is enough)

#if	BLE_APP_PM_ENABLE
	_attribute_data_retention_
#endif
	u8  app_adv_set_param[ADV_SET_PARAM_LENGTH * APP_ADV_SETS_NUMBER];
#if	BLE_APP_PM_ENABLE
	_attribute_data_retention_
#endif
	u8	app_primary_adv_pkt[MAX_LENGTH_PRIMARY_ADV_PKT * APP_ADV_SETS_NUMBER];
#if BLE_APP_PM_ENABLE
	_attribute_data_retention_
#endif
	u8	app_secondary_adv_pkt[MAX_LENGTH_SECOND_ADV_PKT * APP_ADV_SETS_NUMBER];
#if	BLE_APP_PM_ENABLE
	_attribute_data_retention_
#endif
	u8 	app_advData[APP_MAX_LENGTH_ADV_DATA	* APP_ADV_SETS_NUMBER];
#if BLE_APP_PM_ENABLE
	_attribute_data_retention_
#endif
	u8 	app_scanRspData[APP_MAX_LENGTH_SCAN_RESPONSE_DATA * APP_ADV_SETS_NUMBER];

#endif//end of (CONNECTABLE_MODE == ...)

_attribute_data_retention_	u32 device_connection_tick;
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

	device_in_connection_state = 1;//

	device_connection_tick = clock_time() | 1;

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

	device_connection_tick = 0;

	if(pEvt->terminate_reason == HCI_ERR_CONN_TIMEOUT){

	}
	else if(pEvt->terminate_reason == HCI_ERR_REMOTE_USER_TERM_CONN){

	}
	else if(pEvt->terminate_reason == HCI_ERR_CONN_TERM_MIC_FAILURE){

	}
	else{

	}

	tlkapi_printf(APP_LOG_EN, "[APP][EVT] disconnect, reason 0x%x\n", pEvt->terminate_reason);

#if (UI_LED_ENABLE)
	gpio_write(GPIO_LED_RED, !LED_ON_LEVEL);  // light off
#endif


}

/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_PHY_UPDATE"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void 	callback_phy_update_complete_event(u8 e,u8 *p, int n)
{
    (void)e;(void)p;(void)n;
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
 * @brief      power management code for application
 * @param	   none
 * @return     none
 */
void blt_pm_proc(void)
{
#if(BLE_APP_PM_ENABLE)
	#if (PM_DEEPSLEEP_RETENTION_ENABLE)
		bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
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

#if (CONNECTABLE_MODE == LEGACY_ADV_CONNECTABLE_UNDIRECTED)
	blc_ll_initAdvertising_module(mac_public); 	//ADV module: 		 mandatory for BLE slave,
#elif (CONNECTABLE_MODE == EXTENDED_ADV_CONNECTABLE_UNDIRECTED)
//Extended ADV module:
	blc_ll_initExtendedAdvertising_module(app_adv_set_param, app_primary_adv_pkt, APP_ADV_SETS_NUMBER);
	blc_ll_initExtSecondaryAdvPacketBuffer(app_secondary_adv_pkt, MAX_LENGTH_SECOND_ADV_PKT);
	blc_ll_initExtAdvDataBuffer(app_advData, APP_MAX_LENGTH_ADV_DATA);
	blc_ll_initExtScanRspDataBuffer(app_scanRspData, APP_MAX_LENGTH_SCAN_RESPONSE_DATA);

	blc_ll_initChannelSelectionAlgorithm_2_feature();
#endif

	blc_ll_initConnection_module();				//connection module  mandatory for BLE slave/master
	blc_ll_initSlaveRole_module();				//slave module: 	 mandatory for BLE slave,

	blc_ll_init2MPhyCodedPhy_feature();			// mandatory for 2M/Coded PHY

	////// Host Initialization  //////////
	blc_gap_peripheral_init();    //gap initialization
	extern void my_att_init();
	my_att_init(); //gatt initialization
	blc_l2cap_register_handler (blc_l2cap_packet_receive);  	//l2cap initialization

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	blc_smp_peripheral_init();

	//set rf power index, user must set it after every suspend wakeup, cause relative setting will be reset in suspend
	rf_set_power_level_index (MY_RF_POWER_INDEX);

	bls_app_registerEventCallback (BLT_EV_FLAG_CONNECT, &task_connect);
	bls_app_registerEventCallback (BLT_EV_FLAG_TERMINATE, &task_terminate);
	bls_app_registerEventCallback (BLT_EV_FLAG_PHY_UPDATE, &callback_phy_update_complete_event);



	///////////////////// USER application initialization ///////////////////
#if (CONNECTABLE_MODE == LEGACY_ADV_CONNECTABLE_UNDIRECTED)
	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));

	u8 adv_param_status = BLE_SUCCESS;
	adv_param_status = bls_ll_setAdvParam(  ADV_INTERVAL_50MS, ADV_INTERVAL_50MS,
									 ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
									 0,  NULL,
									 BLT_ENABLE_ADV_ALL,
									 ADV_FP_NONE);
	if(adv_param_status != BLE_SUCCESS){
		tlkapi_printf(APP_LOG_EN, "[APP][INI] ADV parameters error 0x%x!!!\n", adv_param_status);
		while(1);
	}


	bls_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable
#elif (CONNECTABLE_MODE == EXTENDED_ADV_CONNECTABLE_UNDIRECTED)
	u32 my_adv_interval_min = ADV_INTERVAL_50MS;
	u32 my_adv_interval_max = ADV_INTERVAL_50MS;

	le_phy_type_t  user_primary_adv_phy;
	le_phy_type_t  user_secondary_adv_phy;

	#if 1      // ADV_EXT_IND: 1M PHY;  		AUX_ADV_IND/AUX_CHAIN_IND: 1M PHY
	user_primary_adv_phy   = BLE_PHY_1M;
	user_secondary_adv_phy = BLE_PHY_1M;
	#elif 1      // ADV_EXT_IND: 1M PHY;  		AUX_ADV_IND/AUX_CHAIN_IND: Coded PHY(S8)
	user_primary_adv_phy   = BLE_PHY_1M;
	user_secondary_adv_phy = BLE_PHY_CODED;
	blc_ll_setDefaultExtAdvCodingIndication(ADV_HANDLE0, CODED_PHY_PREFER_S8);
	#elif 0      // ADV_EXT_IND: 1M PHY;  		AUX_ADV_IND/AUX_CHAIN_IND: Coded PHY(S2)
	user_primary_adv_phy   = BLE_PHY_1M;
	user_secondary_adv_phy = BLE_PHY_CODED;
	blc_ll_setDefaultExtAdvCodingIndication(ADV_HANDLE0, CODED_PHY_PREFER_S2);
	#else
	user_primary_adv_phy   = BLE_PHY_1M;
	user_secondary_adv_phy = BLE_PHY_2M;
	#endif

	blc_ll_setExtAdvParam( ADV_HANDLE0, 		ADV_EVT_PROP_EXTENDED_CONNECTABLE_UNDIRECTED, 					my_adv_interval_min, 			my_adv_interval_max,
						   BLT_ENABLE_ADV_ALL,	OWN_ADDRESS_PUBLIC, 										    BLE_ADDR_PUBLIC, 				NULL,
						   ADV_FP_NONE,  		TX_POWER_8dBm,												   	user_primary_adv_phy, 			0,
						   user_secondary_adv_phy, 	ADV_SID_0, 													0);

	blc_ll_setExtAdvData( ADV_HANDLE0, DATA_OPER_COMPLETE, DATA_FRAGM_ALLOWED, sizeof(tbl_advData),   (u8*)tbl_advData);
	blc_ll_setExtScanRspData( ADV_HANDLE0, DATA_OPER_COMPLETE, DATA_FRAGM_ALLOWED, sizeof(tbl_scanRsp) , (u8 *)tbl_scanRsp);

	blc_ll_setExtAdvEnable( BLC_ADV_ENABLE, 1, ADV_HANDLE0, 0 , 0);
#endif

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

	/* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done.
	 * attention that code will stuck in "while(1)" if any error detected in initialization, user need find what error happens and then fix it */
	blc_app_checkControllerHostInitialization();

	tlkapi_printf(APP_LOG_EN, "[APP][INI] feature_phy_conn init \n");
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

_attribute_data_retention_ u32 phy_update_test_tick = 0;
_attribute_data_retention_ u32 phy_update_test_seq = 0;
_attribute_data_retention_	int AAA_update = 0;

/**
 * @brief		2m_coded_phy_connection test in mainloop
 * @param[in]	none
 * @return      none
 */
void feature_2m_coded_phy_conn_mainloop(void)
{
	if(device_connection_tick && clock_time_exceed(device_connection_tick, 10000000)){
		device_connection_tick = 0;

		phy_update_test_tick = clock_time() | 1;
		phy_update_test_seq = 0;  //reset

	}


	if(phy_update_test_tick && clock_time_exceed(phy_update_test_tick, 2000000)){
		phy_update_test_tick = clock_time() | 1;

		int AAA = phy_update_test_seq%4;
		if(AAA == 0){
			#if(MCU_CORE_TYPE != MCU_CORE_TC321X)
				blc_ll_setPhy(BLS_CONN_HANDLE, PHY_TRX_PREFER, PHY_PREFER_CODED, PHY_PREFER_CODED, CODED_PHY_PREFER_S2);
			#endif
		}
		else if(AAA == 1){
			blc_ll_setPhy(BLS_CONN_HANDLE, PHY_TRX_PREFER, PHY_PREFER_2M, 	 PHY_PREFER_2M,    CODED_PHY_PREFER_NONE);
		}
		else if(AAA == 2){
			#if(MCU_CORE_TYPE != MCU_CORE_TC321X)
				blc_ll_setPhy(BLS_CONN_HANDLE, PHY_TRX_PREFER, PHY_PREFER_CODED, PHY_PREFER_CODED, CODED_PHY_PREFER_S8);
			#endif
		}
		else{
			blc_ll_setPhy(BLS_CONN_HANDLE, PHY_TRX_PREFER, PHY_PREFER_1M, 	 PHY_PREFER_1M,    CODED_PHY_PREFER_NONE);
		}

		phy_update_test_seq ++;
	}

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

	feature_2m_coded_phy_conn_mainloop();//todo

	////////////////////////////////////// UI entry /////////////////////////////////
	#if (UI_KEYBOARD_ENABLE)
		proc_keyboard(0, 0, 0);
	#endif

	////////////////////////////////////// PM Process /////////////////////////////////
	blt_pm_proc();
}


#endif  //end of (FEATURE_TEST_MODE == ...)

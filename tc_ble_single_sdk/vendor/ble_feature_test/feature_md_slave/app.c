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
#include "../feature_config.h"

#if (FEATURE_TEST_MODE == TEST_MD_SLAVE)


#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "app_config.h"
#include "app.h"
#include "app_ui.h"
#include "app_att.h"


#define		MY_RF_POWER_INDEX					RF_POWER_P3dBm


/**
 * @brief      LinkLayer RX & TX FIFO configuration
 */
/* CAL_LL_ACL_RX_BUF_SIZE(maxRxOct): maxRxOct + 22, then 16 byte align */
#define RX_FIFO_SIZE	64
/* must be: 2^n, (power of 2);at least 4; recommended value: 4, 8, 16 */
#define RX_FIFO_NUM		16


/* CAL_LL_ACL_TX_BUF_SIZE(maxTxOct):  maxTxOct + 10, then 4 byte align */
#define TX_FIFO_SIZE	40
/* must be: (2^n), (power of 2); at least 8; recommended value: 8, 16, 32, other value not allowed. */
#define TX_FIFO_NUM		16


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



/**
 * @brief	BLE Advertising data
 */
const u8	tbl_advData[] = {
	 0x08, DT_COMPLETE_LOCAL_NAME, 'f', 'e', 'a', 't', 'u', 'r', 'e',
	 0x02, DT_FLAGS, 0x05, 							// BLE limited discoverable mode and BR/EDR not supported
	 0x05, DT_INCOMPLETE_LIST_16BIT_SERVICE_UUID, 0x12, 0x18, 0x0F, 0x18,		// incomplete list of service class UUIDs (0x1812, 0x180F)
};

/**
 * @brief	BLE Scan Response Packet data
 */
const u8	tbl_scanRsp [] = {
	 0x08, DT_COMPLETE_LOCAL_NAME, 'f', 'e', 'a', 't', 'u', 'r', 'e',
};


#define TEST_DATA_LEN		20

_attribute_data_retention_	int device_in_connection_state;

_attribute_data_retention_	u32 advertise_begin_tick;

_attribute_data_retention_	u32	latest_user_event_tick;

_attribute_data_retention_	u32 device_connection_tick;

_attribute_data_retention_  int write_data_test_tick;

_attribute_data_retention_	u8  app_test_data[TEST_DATA_LEN];

_attribute_data_retention_  u8 enc_done_flag;


/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_SUSPEND_ENTER"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void  task_sleep_enter (u8 e, u8 *p, int n)
{
	(void)e;(void)p;(void)n;
	if( blc_ll_getCurrentState() == BLS_LINK_STATE_CONN && ((u32)(bls_pm_getSystemWakeupTick() - clock_time())) > 80 * SYSTEM_TIMER_TICK_1MS){  //suspend time > 30ms.add gpio wakeup
		bls_pm_setWakeupSource(PM_WAKEUP_PAD);  //gpio pad wakeup suspend/deepsleep
	}
}


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
	tlkapi_send_string_data(APP_CONTR_EVENT_LOG_EN, "[APP][EVT] connect, intA & advA:", pConnEvt->initA, 12);

	//bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_10MS, CONN_INTERVAL_10MS, 99, CONN_TIMEOUT_4S);  // 1 S

	latest_user_event_tick = clock_time();

	device_in_connection_state = 1;

	device_connection_tick = clock_time() | 1;

	enc_done_flag = 0;

	#if (UI_LED_ENABLE)
		gpio_write(GPIO_LED_RED, LED_ON_LEVEL);  //red led on
	#endif
}



/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_TERMINATE"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void 	task_terminate(u8 e, u8 *p, int n) //*p is terminate reason
{
	(void)e;(void)n;
	tlk_contr_evt_terminate_t *pEvt = (tlk_contr_evt_terminate_t *)p;

	device_in_connection_state = 0;

	device_connection_tick = 0;

	write_data_test_tick = 0;

	app_test_data[0] = 0;

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
		gpio_write(GPIO_LED_RED, !LED_ON_LEVEL);  //red led off
	#endif

	advertise_begin_tick = clock_time();
}




/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_SUSPEND_EXIT"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
_attribute_ram_code_ void	task_suspend_exit (u8 e, u8 *p, int n)
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

	u8 event = h & 0xFF;

	switch(event)
	{
		case GAP_EVT_SMP_PAIRING_BEGIN:
		{
			gap_smp_pairingBeginEvt_t *pEvt = (gap_smp_pairingBeginEvt_t *)para;
			tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] paring begin:", pEvt, sizeof(gap_smp_pairingBeginEvt_t));
		}
		break;

		case GAP_EVT_SMP_PAIRING_SUCCESS:
		{
			gap_smp_pairingSuccessEvt_t *pEvt = (gap_smp_pairingSuccessEvt_t *)para;
			tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] paring success:", pEvt, sizeof(gap_smp_pairingSuccessEvt_t));
		}
		break;

		case GAP_EVT_SMP_PAIRING_FAIL:
		{
			gap_smp_pairingFailEvt_t *pEvt = (gap_smp_pairingFailEvt_t *)para;
			tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] paring fail:", pEvt, sizeof(gap_smp_pairingFailEvt_t));
		}
		break;

		case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
		{
			gap_smp_connEncDoneEvt_t *pEvt = (gap_smp_connEncDoneEvt_t *)para;
			tlkapi_send_string_data(APP_SMP_LOG_EN, "[APP][SMP] encryption done:", pEvt, sizeof(gap_smp_connEncDoneEvt_t));
			enc_done_flag = 1;
		}
		break;

		case GAP_EVT_SMP_SECURITY_PROCESS_DONE:
		{
			//gap_smp_securityProcessDoneEvt_t *pEvt = (gap_smp_securityProcessDoneEvt_t *)para;
		}
		break;


		case GAP_EVT_SMP_TK_DISPLAY:
		{
			//u32 pinCode = MAKE_U32(para[3], para[2], para[1], para[0]);
		}
		break;

		case GAP_EVT_SMP_TK_REQUEST_PASSKEY:
		{
			//for this event, no data, "para" is NULL
		}
		break;

		case GAP_EVT_SMP_TK_REQUEST_OOB:
		{
			//for this event, no data, "para" is NULL
		}
		break;

		case GAP_EVT_SMP_TK_NUMERIC_COMPARE:
		{
			//u32 pinCode = MAKE_U32(para[3], para[2], para[1], para[0]);
		}
		break;

		case GAP_EVT_ATT_EXCHANGE_MTU:
		{
			gap_gatt_mtuSizeExchangeEvt_t *pEvt = (gap_gatt_mtuSizeExchangeEvt_t *)para;
			tlkapi_send_string_data(APP_HOST_EVENT_LOG_EN, "[APP][MTU] mtu exchange", pEvt, sizeof(gap_gatt_mtuSizeExchangeEvt_t));
		}
		break;

		case GAP_EVT_GATT_HANDLE_VALUE_CONFIRM:
		{
			//for this event, no data, "para" is NULL
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
	if(blc_ll_getCurrentState() == BLS_LINK_STATE_IDLE){ //PM module can not manage Idle state low power.
		/* user manage BLE Idle state sleep with API "cpu_sleep_wakeup" */
	}
	else{ //PM module manage advertising and ACL connection Slave role low power only
		#if (PM_DEEPSLEEP_RETENTION_ENABLE)
			bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
		#else
			bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
		#endif


		//do not care about keyScan/button_detect power here, if you care about this, please refer to "ble_remote" demo
		#if (UI_KEYBOARD_ENABLE)
			if(scan_pin_need || key_not_released){
				bls_pm_setSuspendMask (SUSPEND_DISABLE);
			}
		#endif
	}
#endif  //end of BLE_APP_PM_ENABLE
}


/**
 * @brief		user initialization when MCU power on or wake_up from deepSleep mode
 * @param[in]	none
 * @return      none
 */
_attribute_no_inline_ void user_init_normal(void)
{

//////////////////////////// basic hardware Initialization  Begin //////////////////////////////////

	/* random number generator must be initiated before any BLE stack initialization.
	 * When deepSleep retention wakeUp, no need initialize again */
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


//////////////////////////// basic hardware Initialization  End //////////////////////////////////




//////////////////////////// BLE stack Initialization  Begin //////////////////////////////////
	//////////// Controller Initialization  Begin /////////////////////////
	u8  mac_public[6];
	u8  mac_random_static[6];
	/* for 512K Flash, flash_sector_mac_address equals to 0x76000, for 1M  Flash, flash_sector_mac_address equals to 0xFF000 */
	blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);
	tlkapi_send_string_data(APP_LOG_EN,"[APP][INI]Public Address", mac_public, 6);


	blc_ll_initBasicMCU();                      //mandatory
	blc_ll_initStandby_module(mac_public);		//mandatory
	blc_ll_initAdvertising_module(mac_public); 	//legacy advertising module: mandatory for BLE slave
	blc_ll_initConnection_module();				//connection module  mandatory for BLE slave/master
	blc_ll_initSlaveRole_module();				//slave module: 	 mandatory for BLE slave,
	//////////// Controller Initialization  End /////////////////////////


	//////////// Host Initialization  Begin /////////////////////////
	/* Host Initialization */
	/* GAP initialization must be done before any other host feature initialization !!! */
	blc_gap_peripheral_init();    //gap initialization
	blc_l2cap_register_handler (blc_l2cap_packet_receive);  	//l2cap initialization
	my_att_init(); //gatt initialization
	blc_att_setRxMtuSize(MTU_SIZE_SETTING); //set MTU size, default MTU is 23 if not call this API

	/* SMP Initialization may involve flash write/erase(when one sector stores too much information,
	 *   is about to exceed the sector threshold, this sector must be erased, and all useful information
	 *   should re_stored) , so it must be done after battery check */
	#if (BLE_APP_SECURITY_ENABLE)
		/* attention: If this API is used, must be called before "blc smp_peripheral_init" when initialization !!! */
		bls_smp_configPairingSecurityInfoStorageAddr(flash_sector_smp_storage);
		blc_smp_peripheral_init();

		/* Hid device on android7.0/7.1 or later version
		 * New paring: send security_request immediately after connection complete
		 * reConnect:  send security_request 1000mS after connection complete. If master start paring or encryption before 1000mS timeout, slave do not send security_request. */
		blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection)
	#else
		blc_smp_setSecurityLevel(No_Security);
	#endif


	/* host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask */
	blc_gap_registerHostEventHandler(app_host_event_callback);
	/* enable some frequently-used host event by default, user can add more host event */
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PAIRING_BEGIN 			|  \
						  GAP_EVT_MASK_SMP_PAIRING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PAIRING_FAIL				|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE		|  \
						  GAP_EVT_MASK_ATT_EXCHANGE_MTU);
	//////////// Host Initialization  End /////////////////////////

//////////////////////////// BLE stack Initialization  End //////////////////////////////////


//////////////////////////// User Configuration for BLE application ////////////////////////////
	////////////////// config ADV packet /////////////////////
	bls_ll_setAdvParam(  ADV_INTERVAL_30MS, ADV_INTERVAL_35MS,
						 ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
						 0,  NULL,
						 BLT_ENABLE_ADV_ALL,
						 ADV_FP_NONE);
	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));
	bls_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable

	/* set RF power index, user must set it after every suspend wake_up, because relative setting will be reset in suspend */
	rf_set_power_level_index (MY_RF_POWER_INDEX);

	bls_app_registerEventCallback (BLT_EV_FLAG_CONNECT, &task_connect);
	bls_app_registerEventCallback (BLT_EV_FLAG_TERMINATE, &task_terminate);
	bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_EXIT, &task_suspend_exit);

	///////////////////// Power Management initialization///////////////////
	#if(BLE_APP_PM_ENABLE)
		blc_ll_initPowerManagement_module();

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

		bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_ENTER, &task_sleep_enter);
	#else
		#if (PM_DEEPSLEEP_RETENTION_ENABLE)
			#error "can not use deep retention when PM disable !!!"
		#endif

		bls_pm_setSuspendMask (SUSPEND_DISABLE);
	#endif


	#if (UI_KEYBOARD_ENABLE)
		/////////// keyboard gpio wakeup init ////////
		u32 pin[] = KB_DRIVE_PINS;
		for (int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
		{
			cpu_set_gpio_wakeup (pin[i], Level_High,1);  //drive pin pad high wakeup deepsleep
		}

		bls_app_registerEventCallback (BLT_EV_FLAG_GPIO_EARLY_WAKEUP, &proc_keyboard);
	#endif
////////////////////////////////////////////////////////////////////////////////////////////////

	/* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done.
	 * attention that code will stuck in "while(1)" if any error detected in initialization, user need find what error happens and then fix it */
	blc_app_checkControllerHostInitialization();

	advertise_begin_tick = clock_time();

	tlkapi_printf(APP_LOG_EN, "[APP][INI] BLE feature more data slave init \n");

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
		/////////// keyboard GPIO wake_up initialization ////////
		u32 pin[] = KB_DRIVE_PINS;
		for (int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
		{
			cpu_set_gpio_wakeup (pin[i], Level_High,1);  //drive pin high level wake_up deepsleep
		}
	#endif
#endif
}



/////////////////////////////////////////////////////////////////////s
// main loop flow
/////////////////////////////////////////////////////////////////////

/**
 * @brief		This is main_loop function
 * @param[in]	none
 * @return      none
 */
_attribute_no_inline_ void main_loop(void)
{
	////////////////////////////////////// BLE entry /////////////////////////////////
	blt_sdk_main_loop();


	////////////////////////////////////// UI entry /////////////////////////////////
	#if (UI_KEYBOARD_ENABLE)
		proc_keyboard(0, 0, 0);
	#endif

	if(device_connection_tick && clock_time_exceed(device_connection_tick, 2000000)){
		device_connection_tick = 0;
		write_data_test_tick = clock_time() | 1;
	}

#if (BLE_APP_SECURITY_ENABLE)
	if(enc_done_flag)
#endif
	{
		if(write_data_test_tick && clock_time_exceed(write_data_test_tick, 100)){ // >100us
			write_data_test_tick = clock_time() | 1;

			while(1)
			{
				if( BLE_SUCCESS == blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE, SPP_SERVER_TO_CLIENT_DP_H, app_test_data, TEST_DATA_LEN)){
					app_test_data[0] ++;
				}
				else{
					break;
				}
			}
		}
	}

	////////////////////////////////////// PM Process /////////////////////////////////
	blt_pm_proc();
}





#endif  //end of (FEATURE_TEST_MODE == ...)

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
#include "app_att.h"

#include "application/keyboard/keyboard.h"
#include "application/usbstd/usbkeycode.h"


#if (FEATURE_TEST_MODE == TEST_GATT_SECURITY)


#define		MY_RF_POWER_INDEX					RF_POWER_P3dBm

#define RX_FIFO_SIZE	64
#define RX_FIFO_NUM		8

#define TX_FIFO_SIZE	40
#define TX_FIFO_NUM		8



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



//////////////////////////////////////////////////////////////////////////////
//	 Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////
const u8	tbl_advData[] = {
	 0x08, DT_COMPLETE_LOCAL_NAME, 'f', 'e', 'a', 't', 'u', 'r', 'e',
};

const u8	tbl_scanRsp [] = {
	 0x08, DT_COMPLETE_LOCAL_NAME, 'f', 'e', 'a', 't', 'u', 'r', 'e',
};


_attribute_data_retention_	int device_in_connection_state;
_attribute_data_retention_	u32 advertise_begin_tick;


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
	bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_10MS, CONN_INTERVAL_10MS, 99, CONN_TIMEOUT_4S);  // 1 S

	device_in_connection_state = 1;//

	#if (UI_LED_ENABLE)
		gpio_write(GPIO_LED_RED, LED_ON_LEVEL);  //RED light on
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
	device_in_connection_state = 0;

	tlk_contr_evt_terminate_t *pEvt = (tlk_contr_evt_terminate_t *)p;
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
	gpio_write(GPIO_LED_RED, !LED_ON_LEVEL);  //RED light off
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

#endif  //end of BLE_APP_PM_ENABLE
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
			tlkapi_printf(APP_HOST_EVENT_LOG_EN, "Pairing begin\n");

			#if (SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MDSI || SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MISI || \
				 SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MISI || SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MDSI)
				digital_key_cnt = 0;//clr
			#endif
		}
		break;

		case GAP_EVT_SMP_PAIRING_SUCCESS:
		{
			gap_smp_pairingSuccessEvt_t* p = (gap_smp_pairingSuccessEvt_t*)para;
			tlkapi_printf(APP_HOST_EVENT_LOG_EN, "Pairing success:bond flg %s\n", p->bonding ?"true":"false");

			if(p->bonding_result){
				tlkapi_printf(APP_HOST_EVENT_LOG_EN, "save smp key succ\n");
			}
			else{
				tlkapi_printf(APP_HOST_EVENT_LOG_EN, "save smp key failed\n");
			}
		}
		break;

		case GAP_EVT_SMP_PAIRING_FAIL:
		{
			gap_smp_pairingFailEvt_t* p = (gap_smp_pairingFailEvt_t*)para;
			tlkapi_printf(APP_HOST_EVENT_LOG_EN, "Pairing failed:rsn:0x%x\n", p->reason);
		}
		break;

		case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
		{
			gap_smp_connEncDoneEvt_t* p = (gap_smp_connEncDoneEvt_t*)para;
			tlkapi_printf(APP_HOST_EVENT_LOG_EN, "Connection encryption done\n");

			if(p->re_connect == SMP_STANDARD_PAIR){  //first pairing

			}
			else if(p->re_connect == SMP_FAST_CONNECT){  //auto connect

			}
		}
		break;

		case GAP_EVT_SMP_TK_DISPLAY:
		{
			char pc[7];
			u32 pinCode = *(u32*)para;
			sprintf(pc, "%d", pinCode);
			tlkapi_printf(APP_HOST_EVENT_LOG_EN,"[APP][SMP]TK display:%s\n", pc);
		}
		break;

		case GAP_EVT_SMP_TK_REQUEST_PASSKEY:
		{
			tlkapi_printf(APP_HOST_EVENT_LOG_EN, "TK Request passkey\n");
		}
		break;

		case GAP_EVT_SMP_TK_REQUEST_OOB:
		{
			tlkapi_printf(APP_HOST_EVENT_LOG_EN, "TK Request OOB\n");
		}
		break;

		case GAP_EVT_SMP_TK_NUMERIC_COMPARE:
		{
			char pc[7];
			u32 pinCode = *(u32*)para;
			sprintf(pc, "%d", pinCode);
			tlkapi_printf(APP_HOST_EVENT_LOG_EN, "TK numeric comparison:%s\n", pc);
		}
		break;

		default:
		break;
	}

	return 0;
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
	extern void my_att_init();
	my_att_init(); //gatt initialization
	blc_l2cap_register_handler (blc_l2cap_packet_receive);  	//l2cap initialization




#if ( SMP_TEST_MODE	== LE_SECURITY_MODE_1_LEVEL_1)

	blc_smp_setSecurityLevel(No_Authentication_No_Encryption);  // LE_Security_Mode_1_Level_1	//Smp Initialization may involve flash write/erase(when one sector stores too much information,

#elif ( SMP_TEST_MODE == LE_SECURITY_MODE_1_LEVEL_2  )
	blc_att_setRxMtuSize(65); //set MTU size to 65
	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//set security level: "LE_Security_Mode_1_Level_2"
	blc_smp_setSecurityLevel(Unauthenticated_Pairing_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Pairing_with_Encryption)
	blc_smp_setBondingMode(Bondable_Mode);	// if not set, default is : Bondable_Mode
	blc_smp_setIoCapability(IO_CAPABILITY_NO_IN_NO_OUT);	// if not set, default is : IO_CAPABILITY_NO_INPUT_NO_OUTPUT

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	//Notice:if user set smp parameters: it should be called after usr smp settings
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE );


#elif ( SMP_TEST_MODE == LE_SECURITY_MODE_1_LEVEL_3  )

	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//set security level: "LE_Security_Mode_1_Level_3"
	blc_smp_setSecurityLevel(Authenticated_Pairing_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Pairing_with_Encryption)
	blc_smp_enableAuthMITM(1);
	blc_smp_setBondingMode(Bondable_Mode);	// if not set, default is : Bondable_Mode
	blc_smp_setIoCapability(IO_CAPABILITY_DISPLAY_ONLY);	// if not set, default is : IO_CAPABILITY_NO_INPUT_NO_OUTPUT

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	//Notice:if user set smp parameters: it should be called after usr smp settings
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_TK_DISPLAY				|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE     |  \
						  GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE);

#elif ( SMP_TEST_MODE == LE_SECURITY_MODE_1_LEVEL_4  )
	blc_att_setRxMtuSize(65); //set MTU size to 65
	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//set security level: "LE_Security_Mode_1_Level_4"
	blc_smp_setSecurityLevel(Authenticated_LE_Secure_Connection_Pairing_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Pairing_with_Encryption)
	blc_smp_enableSecureConnections(1);
	blc_smp_setSecurityParameters(Bondable_Mode, 1, 0, 0, IO_CAPABILITY_DISPLAY_ONLY);
	blc_smp_setEcdhDebugMode(debug_mode); //use debug mode for sniffer decryption

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	//Notice:if user set smp parameters: it should be called after usr smp settings
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_TK_DISPLAY				|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE     |  \
						  GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE);

#endif


///////////////////// USER application initialization ///////////////////
	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));




	////////////////// config ADV packet /////////////////////
	u8 adv_param_status = BLE_SUCCESS;
	adv_param_status = bls_ll_setAdvParam(  ADV_INTERVAL_30MS, ADV_INTERVAL_30MS,
									 ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
									 0,  NULL,
									 BLT_ENABLE_ADV_37,
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

#else
	bls_pm_setSuspendMask (SUSPEND_DISABLE);
#endif

	/* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done.
	 * attention that code will stuck in "while(1)" if any error detected in initialization, user need find what error happens and then fix it */
	blc_app_checkControllerHostInitialization();

	advertise_begin_tick = clock_time();
	tlkapi_printf(APP_LOG_EN, "[APP][INI] feature_gatt_security init \n");

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
		for (int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
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

	////////////////////////////////////// PM Process /////////////////////////////////
	blt_pm_proc();
}


#endif  //end of (FEATURE_TEST_MODE == ...)

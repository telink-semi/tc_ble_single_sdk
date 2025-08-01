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
#include "app_config.h"
#include "../default_att.h"

#include "application/keyboard/keyboard.h"
#include "application/usbstd/usbkeycode.h"


#if (FEATURE_TEST_MODE == TEST_L2CAP_COC)


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

typedef struct{
	u16 connHandle;
	u16 mtu;
	u16 srcCid;
	u16 dstCid;
} app_cocCid_t;

app_cocCid_t app_cocCid[COC_CID_COUNT];

static u8 cocBuffer[COC_MODULE_BUFFER_SIZE(CREATE_COC_CONNECT_ACL_COUNT, COC_CID_COUNT, 0, COC_MTU_SIZE)];

//////////////////////////////////////////////////////////////////////////////
//	 Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////

const u8	tbl_advData[] = {
	 0x08,  DT_COMPLETE_LOCAL_NAME, 				'f', 'e', 'a', 't', 'u', 'r', 'e',
	 0x02,	DT_FLAGS, 								0x05, 					// BLE limited discoverable mode and BR/EDR not supported
	 0x03,  DT_APPEARANCE, 							0x80, 0x01, 			// 384, Generic Remote Control, Generic category
	 0x05,  DT_INCOMPLETE_LIST_16BIT_SERVICE_UUID,	0x12, 0x18, 0x0F, 0x18,	// incomplete list of service class UUIDs (0x1812, 0x180F)
};

const u8	tbl_scanRsp [] = {
	 0x08,  DT_COMPLETE_LOCAL_NAME, 				 'f', 'e', 'a', 't', 'u', 'r', 'e',
};



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
				if(key0 == CR_VOL_UP){  	//coc create le credit based connection.
					consumer_key = MKEY_VOL_UP;
					gpio_write(GPIO_LED_WHITE,1);
					app_createLeCreditBasedConnect();
					tlkapi_printf(APP_COC_LOG_EN,"app_createLeCreditBasedConnect!\n");
				}
				else if(key0 == CR_VOL_DN){ //coc create credit based connection.
					consumer_key = MKEY_VOL_DN;
					gpio_write(GPIO_LED_RED,1);
					app_sendCocData();
				}
			}
			else
			{
				key_type = KEYBOARD_KEY;
				if (key0 == VK_1) {
					gpio_write(GPIO_LED_BLUE,1);
					app_disconnCocConnect();
					tlkapi_printf(APP_COC_LOG_EN,"app_disconnCocConnect!\n");
				} else if (key0 == VK_2) {
					gpio_write(GPIO_LED_GREEN,1);
					bls_l2cap_requestConnParamUpdate(8, 8, 99, 400);
					tlkapi_printf(APP_COC_LOG_EN,"bls_l2cap_requestConnParamUpdate!\n");
				}
			}

		}
		else   //kb_event.cnt == 0,  key release
		{
			key_not_released = 0;

			gpio_write(GPIO_LED_WHITE,0);
			gpio_write(GPIO_LED_GREEN,0);
			gpio_write(GPIO_LED_BLUE,0);
			gpio_write(GPIO_LED_RED,0);
			key_not_released = 0;
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
	tlkapi_printf(APP_COC_LOG_EN,"----- connected -----\n");

	//bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_10MS, CONN_INTERVAL_10MS, 99, CONN_TIMEOUT_4S);  // 1 S

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
	tlkapi_printf(APP_COC_LOG_EN,"----- terminate rsn: 0x%x -----\n", *p);

	tlk_contr_evt_terminate_t *pEvt = (tlk_contr_evt_terminate_t *)p;
	if(pEvt->terminate_reason == HCI_ERR_CONN_TIMEOUT){

	}
	else if(pEvt->terminate_reason == HCI_ERR_REMOTE_USER_TERM_CONN){

	}
	else if(pEvt->terminate_reason == HCI_ERR_CONN_TERM_MIC_FAILURE){

	}
	else{

	}



#if (UI_LED_ENABLE)
	gpio_write(GPIO_LED_RED, !LED_ON_LEVEL);  // light off
#endif


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
	app_host_coc_event_callback(h, para, n);
	u8 event = h & 0xFF;

	switch(event)
	{
		case GAP_EVT_SMP_PAIRING_BEGIN:
		{
			tlkapi_printf(APP_COC_LOG_EN,"----- Pairing begin -----\n");
		}
		break;

		case GAP_EVT_SMP_PAIRING_SUCCESS:
		{
			gap_smp_pairingSuccessEvt_t* p = (gap_smp_pairingSuccessEvt_t*)para;
			tlkapi_printf(APP_COC_LOG_EN,"Pairing success:bond flg %s\n", p->bonding ?"true":"false");

			if(p->bonding_result){
				tlkapi_printf(APP_COC_LOG_EN,"save smp key succ\n");
			}
			else{
				tlkapi_printf(APP_COC_LOG_EN,"save smp key failed\n");
			}
		}
		break;

		case GAP_EVT_SMP_PAIRING_FAIL:
		{
			gap_smp_pairingFailEvt_t* p = (gap_smp_pairingFailEvt_t*)para;
			tlkapi_printf(APP_COC_LOG_EN,"----- Pairing failed:rsn:0x%x -----\n", p->reason);
		}
		break;

		case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
		{
			gap_smp_connEncDoneEvt_t* p = (gap_smp_connEncDoneEvt_t*)para;
			tlkapi_printf(APP_COC_LOG_EN,"----- Connection encryption done -----\n");

			if(p->re_connect == SMP_STANDARD_PAIR){  //first pairing

			}
			else if(p->re_connect == SMP_FAST_CONNECT){  //auto connect

			}
		}
		break;

		case GAP_EVT_SMP_SECURITY_PROCESS_DONE:
		{
			gap_smp_connEncDoneEvt_t *p = (gap_smp_connEncDoneEvt_t*)para;
			if(p->re_connect == SMP_FAST_CONNECT)//first pairing
			{
				tlkapi_printf(APP_COC_LOG_EN,"Pairing process done--->SMP_FAST_CONNECT");
			}
			else if(p->re_connect == SMP_STANDARD_PAIR)//auto connect
			{
				tlkapi_printf(APP_COC_LOG_EN,"Pairing process done--->SMP_STABDARD_PAIR");
			}

		}
		break;

		case GAP_EVT_ATT_EXCHANGE_MTU:
		{
		}
		break;


		default:
		break;
	}

	return 0;
}


//////////////////////////////// COC /////////////////////////
int myC2SWrite(void * p)
{
    (void)p;
	return 0;
}

/**
 * @brief	Initialize the L2CAP CoC channel, configure parameters such as MTU and SPSM.
 * @param[in]  none.
 * @return     none.
 */

void app_l2cap_coc_init(void)
{
	blc_coc_initParam_t regParam = {
		.MTU = COC_MTU_SIZE,
		.SPSM = 0x0080,
		.createConnCnt = 1,
		.cocCidCnt = COC_CID_COUNT,
	};
	int state = blc_l2cap_registerCocModule(&regParam, cocBuffer, sizeof(cocBuffer));
	if(state){}
}

/**
 * @brief      Handle  events related to CoC channel such as connection and disconnection.
 * @param[in]  h       event type
 * @param[in]  para    Pointer point to event parameter buffer.
 * @param[in]  n       the length of event parameter.
 * @return
 */

int app_host_coc_event_callback (u32 h, u8 *para, int n)
{
    (void)h;(void)para;(void)n;
	u8 event = h & 0xFF;

	switch(event)
	{
		case GAP_EVT_L2CAP_COC_CONNECT:
		{
			gap_l2cap_cocConnectEvt_t* cocConnEvt = (gap_l2cap_cocConnectEvt_t*)para;
			for(unsigned int i=0; i<ARRAY_SIZE(app_cocCid); i++)
			{
				if(!app_cocCid[i].connHandle)
				{
					app_cocCid[i].connHandle = cocConnEvt->connHandle;
					app_cocCid[i].mtu = cocConnEvt->mtu;
					app_cocCid[i].srcCid = cocConnEvt->srcCid;
					app_cocCid[i].dstCid = cocConnEvt->dstCid;
					break;
				}
			}
		}
		break;

		case GAP_EVT_L2CAP_COC_DISCONNECT:
		{
			gap_l2cap_cocDisconnectEvt_t* cocDisconnEvt = (gap_l2cap_cocDisconnectEvt_t*)para;
			for(unsigned int i=0; i<ARRAY_SIZE(app_cocCid); i++)
			{
				if(app_cocCid[i].connHandle == cocDisconnEvt->connHandle &&
						app_cocCid[i].srcCid == cocDisconnEvt->srcCid &&
						app_cocCid[i].dstCid == cocDisconnEvt->dstCid
				)
				{
					app_cocCid[i].connHandle = 0;
					app_cocCid[i].srcCid = 0;
					app_cocCid[i].dstCid = 0;
					break;
				}
			}
		}
		break;

		case GAP_EVT_L2CAP_COC_RECONFIGURE:
		{
//			gap_l2cap_cocReconfigureEvt_t* cocRecfgEvt = (gap_l2cap_cocReconfigureEvt_t*)para;
		}
		break;

		case GAP_EVT_L2CAP_COC_RECV_DATA:
		{
//			gap_l2cap_cocRecvDataEvt_t* cocRecvDataEvt = (gap_l2cap_cocRecvDataEvt_t*)para;
		}
		break;

		case GAP_EVT_L2CAP_COC_SEND_DATA_FINISH:
		{
//			gap_l2cap_cocSendDataFinishEvt_t* cocSendDataFinishEvt = (gap_l2cap_cocSendDataFinishEvt_t*)para;
		}
		break;

		case GAP_EVT_L2CAP_COC_CREATE_CONNECT_FINISH:
		{
//			gap_l2cap_cocCreateConnectFinishEvt_t* cocCreateConnFinishEvt = (gap_l2cap_cocCreateConnectFinishEvt_t*)para;
		}
		break;

		default:
		break;
	}

	return 0;
}

/**
 * @brief	Establish a connection for the CoC channel.
 * @param[in]  none.
 * @return     none.
 */

void app_createLeCreditBasedConnect(void)
{
	ble_sts_t state = blc_l2cap_createLeCreditBasedConnect(BLS_CONN_HANDLE);
	if(state){}
}

void app_createCreditBasedConnect(void)
{
	ble_sts_t state = blc_l2cap_createCreditBasedConnect(BLS_CONN_HANDLE, 5);
	if(state){}
}

_attribute_data_retention_	u8 	coc_test_Data[] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
		0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,};

/**
 * @brief	Send data to all connections on the CoC channel.
 * @param[in]  none.
 * @return     none.
 */
void app_sendCocData(void)
{
	for(unsigned int i=0; i<ARRAY_SIZE(app_cocCid); i++)
	{
		if(app_cocCid[i].connHandle)
		{
			ble_sts_t state = blc_l2cap_sendCocData(app_cocCid[i].connHandle, app_cocCid[i].srcCid, coc_test_Data, min(sizeof(coc_test_Data), app_cocCid[i].mtu));
			if(state){}
		}
	}
}

/**
 * @brief	Disconnect the CoC channel connection.
 * @param[in]  none.
 * @return     none.
 */
void app_disconnCocConnect(void)
{
	for(unsigned int i=0; i<ARRAY_SIZE(app_cocCid); i++)
	{
		if(app_cocCid[i].connHandle)
		{
			ble_sts_t state = blc_l2cap_disconnectCocChannel(app_cocCid[i].connHandle, app_cocCid[i].srcCid);
			if(state){}
		}
	}
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

	blc_l2cap_register_handler (blc_l2cap_packet_receive);  	//l2cap initialization

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_NOT_SEND, SecReq_NOT_SEND, 1000);  //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_L2CAP_COC_CONNECT			|  \
						  GAP_EVT_MASK_L2CAP_COC_DISCONNECT			|  \
						  GAP_EVT_MASK_L2CAP_COC_RECONFIGURE		|  \
						  GAP_EVT_MASK_L2CAP_COC_RECV_DATA			|  \
						  GAP_EVT_MASK_L2CAP_COC_SEND_DATA_FINISH	|  \
						  GAP_EVT_MASK_L2CAP_COC_CREATE_CONNECT_FINISH
						  );
//	blc_gap_setEventMask( -1 );
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


	///////////////////// Power Management initialization///////////////////
	bls_pm_setSuspendMask (SUSPEND_DISABLE);

	app_l2cap_coc_init();

	/* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done.
	 * attention that code will stuck in "while(1)" if any error detected in initialization, user need find what error happens and then fix it */
	blc_app_checkControllerHostInitialization();

	tlkapi_printf(APP_LOG_EN, "[APP][INI] feature_l2cap_coc init \n");
}



/**
 * @brief		user initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void user_init_deepRetn(void)
{
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
}


#endif  //end of (FEATURE_TEST_MODE == ...)

/********************************************************************************************************
 * @file    app.c
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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "app.h"
#include "../default_att.h"

#include "application/keyboard/keyboard.h"
#include "application/usbstd/usbkeycode.h"


#if (FEATURE_TEST_MODE == TEST_LL_PRIVACY_SLAVE)



#define 	MY_DIRECT_ADV_TIME					30000000


#define     MY_APP_ADV_CHANNEL					BLT_ENABLE_ADV_ALL
#define 	MY_ADV_INTERVAL_MIN					ADV_INTERVAL_30MS
#define 	MY_ADV_INTERVAL_MAX					ADV_INTERVAL_35MS

#define		MY_RF_POWER_INDEX					RF_POWER_P3dBm
#define		BLE_DEVICE_ADDRESS_TYPE 			BLE_DEVICE_ADDRESS_PUBLIC

#define 	GATT_UUID_CENTRAL_ADDR_RES       	0x2AA6


_attribute_data_retention_	own_addr_type_t 	app_own_address_type = OWN_ADDRESS_PUBLIC;

_attribute_data_retention_	int device_in_connection_state;


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
	    (void)e;(void)p;void(n);
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
	    (void)e;(void)p;void(n);
		if( blc_ll_getCurrentState() == BLS_LINK_STATE_CONN && ((u32)(bls_pm_getSystemWakeupTick() - clock_time())) > 80 * SYSTEM_TIMER_TICK_1MS){  //suspend time > 30ms.add gpio wakeup
			bls_pm_setWakeupSource(PM_WAKEUP_PAD);  //gpio pad wakeup suspend/deepsleep
		}
	}

#endif




void 	app_switch_to_undirected_adv(u8 e, u8 *p, int n)
{
    (void)e;(void)p;(void)n;

	bls_ll_setAdvEnable(BLC_ADV_DISABLE);

	app_own_address_type = OWN_ADDRESS_PUBLIC;
	u8 status = bls_ll_setAdvParam(  MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
									 ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
									 0,  NULL,
									 MY_APP_ADV_CHANNEL,
									 ADV_FP_NONE);
	if(status != BLE_SUCCESS) { 	while(1);}  //debug: ADV setting err
	status = blc_ll_setAddressResolutionEnable(0);

	bls_ll_setAdvEnable(BLC_ADV_ENABLE);  //must: set ADV enable
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
	tlkapi_send_string_data(APP_LOG_EN, "[APP][EVT] connect, intA & advA:", pConnEvt->initA, 12);

#if 0
	/* sample code for get peer Central(Master) IDA(identity address) if RPA(resolved private address) is used in packet "CONNECT_IND" */
	u8 RxAdd = GET_RXADD_FROM_CONNECT_EVT_DATA(p);
	if(IS_RESOLVABLE_PRIVATE_ADDR(RxAdd, pConnEvt->initA)){
		smp_param_save_t  bondInfo;
		if(bls_smp_param_loadByAddr(RxAdd, pConnEvt->initA, &bondInfo)){
			tlkapi_send_string_data(APP_LOG_EN, "[APP][EVT] peer identity type and address", (u8*)&bondInfo.peer_id_adrType, 7);
		}
	}
#endif

	bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_10MS, CONN_INTERVAL_10MS, 99, CONN_TIMEOUT_4S);  // 1 S

	device_in_connection_state = 1;//

	#if (UI_LED_ENABLE)
		gpio_write(GPIO_LED_RED, LED_ON_LEVEL);
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

	app_configAdvParam();

#if (UI_LED_ENABLE)
	gpio_write(GPIO_LED_RED, !LED_ON_LEVEL);  //RED light off
#endif

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


_attribute_data_retention_ u32 bondingFlashAddr;
_attribute_data_retention_ u16 centralAddrResHdlReq = 0;
u8 * l2cap_matt_handler(u16 connHandle, u8 * p)
{
	(void)connHandle;
	rf_packet_l2cap_req_t * req = (rf_packet_l2cap_req_t *)p;
	switch(req->opcode){
		case ATT_OP_READ_BY_TYPE_RSP: {
			if(centralAddrResHdlReq == 1){
				centralAddrResHdlReq = 0;
				//rf_pkt_att_readByTypeRsp_t *ptr = (rf_pkt_att_readByTypeRsp_t *)&req->type;
				//u16 centralAddrResHdl = ptr->data[0] | (u16)ptr->data[1]<<8;
			}
		}
		break;

		case ATT_OP_ERROR_RSP: {
			if(centralAddrResHdlReq == 1){
				centralAddrResHdlReq = 0;
				//rf_packet_att_errRsp_t * errRsp = (rf_packet_att_errRsp_t*)p;
			}
		}
		break;
	}

	return NULL;
}

int app_host_event_callback (u32 h, u8 *para, int n)
{

    (void)h;(void)para;(void)n;
	u8 event = h & 0xFF;

	switch(event)
	{
		case GAP_EVT_SMP_PAIRING_BEGIN:
		{
			tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] Pairing Begin",0,0);
		}
		break;

		case GAP_EVT_SMP_PAIRING_SUCCESS:
		{
			gap_smp_pairingSuccessEvt_t* p = (gap_smp_pairingSuccessEvt_t*)para;
			tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] Pairing success,bond flg", &(p->bonding), 1);

			if(p->bonding_result){
				tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] save smp key succ",0,0);

			    u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
			    smp_param_save_t  bondInfo;

			    bondingFlashAddr = bls_smp_param_loadByIndex(bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )
			    tlkapi_printf(APP_LOG_EN,"[APP][SMP] bondingFlashAddr", bondingFlashAddr);

				u16 my_centralAddrResUUID = GATT_UUID_CENTRAL_ADDR_RES;
			    if(blc_gatt_pushReadByTypeRequest(BLS_CONN_HANDLE, 0x0001, 0xffff, (u8*)&my_centralAddrResUUID, 2)){
			    	DBG_CHN3_TOGGLE;
			    	centralAddrResHdlReq = 1;
			    	tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] read by type req: UUID: 0x%x\n", &my_centralAddrResUUID,2);
			    }
			}
			else{
				tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] save smp key failed",0,0);
			}
		}
		break;

		case GAP_EVT_SMP_PAIRING_FAIL:
		{
			gap_smp_pairingFailEvt_t* p = (gap_smp_pairingFailEvt_t*)para;
			tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] Pairing failed", &(p->reason), 1);
		}
		break;

		case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
		{
			gap_smp_connEncDoneEvt_t* p = (gap_smp_connEncDoneEvt_t*)para;

			if(p->re_connect == SMP_STANDARD_PAIR){  //first pairing
				tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] First pairing",0,0);
			}
			else if(p->re_connect == SMP_FAST_CONNECT){  //auto connect
				tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] Auto reconnect",0,0);
			}
		}
		break;

		default:
		break;
	}

	return 0;
}

//////////////////////////////////////////////////////////
// event call back
//////////////////////////////////////////////////////////
int controller_event_callback (u32 h, u8 *p, int n)
{
    (void)h;(void)p;(void)n;
	if (h &HCI_FLAG_EVENT_BT_STD)		//ble controller hci event
	{
		u8 evtCode = h & 0xff;

		//------------ disconnect -------------------------------------
		if(evtCode == HCI_EVT_DISCONNECTION_COMPLETE)  //connection terminate
		{
			tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] le connection terminate event",0,0);
		}
		else if(evtCode == HCI_EVT_LE_META)
		{
			u8 subEvt_code = p[0];


			//------hci le event: le connection complete event---------------------------------
			if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_COMPLETE)	// connection complete
			{
				tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] le conn complete event",0,0);
			}
			else if (subEvt_code == HCI_SUB_EVT_LE_ENHANCED_CONNECTION_COMPLETE)
			{
				hci_le_enhancedConnCompleteEvt_t *pEvt = (hci_le_enhancedConnCompleteEvt_t *)p;
				tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] le enhanced conn complete event",0,0);
				tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] peerAddrType,peerAddr",&pEvt->PeerAddrType,7);
				tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] localRpa",pEvt->localRslvPrivAddr, 6);
				tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] peerRpa",pEvt->Peer_RslvPrivAddr, 6);
			}
		}
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

void app_configAdvParam(void){

	blc_ll_initPrivacyLocalRpa(); //must call this API if user need local RPA for privacy

	bls_ll_setAdvEnable(BLC_ADV_DISABLE);  //ADV disable
	////////////////// config ADV packet /////////////////////
	u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
	tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] bond_number", &bond_number, 1);
	smp_param_save_t  bondInfo;
	u8 adv_param_status = BLE_SUCCESS;
	ble_sts_t status;
	if(bond_number)   //at least 1 bonding device exist
	{

		u32 current_addr = bls_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )
		tlkapi_printf(APP_LOG_EN,"[APP][SMP] smpAddr", current_addr);

		tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] central bondInfo.flag", &bondInfo.flag,1);

		u8 own_use_rpa = 1;
		u8 empty_16_ff[16] = {0xFF, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF,  0xFF, 0xFF, 0xFF, 0xFF};
		if(!memcmp(bondInfo.peer_irk, empty_16_ff, 16)){ //all 0xff
			memset(bondInfo.peer_irk, 0, 16);
		}
		if(!memcmp(bondInfo.local_irk, empty_16_ff, 16)){ //all 0xff
			own_use_rpa = 0;
			memset(bondInfo.local_irk, 0, 16);
		}

		status = blc_ll_addDeviceToResolvingList(bondInfo.peer_id_adrType, bondInfo.peer_id_addr, bondInfo.peer_irk, bondInfo.local_irk);
		tlkapi_send_string_data(APP_LOG_EN,"[APP][RPA] LL resolving list add status", &status,1);

		status = blc_ll_setAddressResolutionEnable(1);
		tlkapi_send_string_data(APP_LOG_EN,"[APP][RPA] LL add resolution enable status", &status,1);

		tlkapi_send_string_data(APP_LOG_EN,"[APP][SMP] central bondInfo.flag", &bondInfo.flag,1);

		app_own_address_type = own_use_rpa ? OWN_ADDRESS_RESOLVE_PRIVATE_PUBLIC : OWN_ADDRESS_PUBLIC;
		if(app_own_address_type == OWN_ADDRESS_RESOLVE_PRIVATE_PUBLIC){
			tlkapi_send_string_data(APP_LOG_EN,"[APP][RPA] RPA",0,0);
			DBG_CHN3_TOGGLE;;
		}else{
			tlkapi_send_string_data(APP_LOG_EN,"[APP][RPA] PUB",0,0);
			DBG_CHN4_TOGGLE;
		}

		u8* peerAddr;
		u8  peerAddrType;
		if(app_own_address_type < OWN_ADDRESS_RESOLVE_PRIVATE_PUBLIC){
			peerAddr = bondInfo.peer_addr;
			peerAddrType = bondInfo.peer_addr_type;
			tlkapi_send_string_data(APP_LOG_EN,"[APP][RPA] AdvA: pub",bondInfo.peer_addr,6);
		}
		else{
			peerAddr = bondInfo.peer_id_addr;
			peerAddrType = bondInfo.peer_id_adrType;
			tlkapi_send_string_data(APP_LOG_EN,"[APP][RPA] AdvA: rpa",bondInfo.peer_id_addr,6);
		}
		adv_param_status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
									ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, app_own_address_type,
									 peerAddrType,  peerAddr,
									 MY_APP_ADV_CHANNEL,
									 ADV_FP_NONE);//  ADV_FP_NONE  ADV_FP_ALLOW_SCAN_ANY_ALLOW_CONN_WL
		if(adv_param_status != BLE_SUCCESS) { 	//debug: ADV setting err
			tlkapi_printf(APP_LOG_EN, "[APP][INI] ADV parameters error 0x%x!!!\n", adv_param_status);
			while(1);
		}

		bls_ll_setAdvDuration(MY_DIRECT_ADV_TIME>>1, 1);
		bls_app_registerEventCallback (BLT_EV_FLAG_ADV_DURATION_TIMEOUT, &app_switch_to_undirected_adv);
	}
	else{
		#if 1 //If enable, ADV used PUBLIC ADDRESS when no bonded device.
			app_own_address_type = OWN_ADDRESS_PUBLIC;
			adv_param_status = bls_ll_setAdvParam(  MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
											 ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
											 0,  NULL,
											 MY_APP_ADV_CHANNEL,
											 ADV_FP_NONE);
			if(adv_param_status != BLE_SUCCESS) { 	//debug: ADV setting err
				tlkapi_printf(APP_LOG_EN, "[APP][INI] ADV parameters error 0x%x!!!\n", adv_param_status);
				while(1);
			}
			status = blc_ll_setAddressResolutionEnable(0);
			tlkapi_send_string_data(APP_LOG_EN,"[APP][RPA] LL add resolution disable status",&(status), 1);
		#else //If enable, ADV used RPA when no bonded device.

			u8	tmp_peer_irk[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
			u8	tmp_local_irk[16];
			u8	tmp_peer_addr[6];
			generateRandomNum(16,tmp_local_irk);
			generateRandomNum(6,tmp_peer_addr);

			u8	tmp_peer_addr_type = OWN_ADDRESS_PUBLIC;

			tlkapi_send_string_data(APP_LOG_EN,"[APP][RPA] First Connection,made Irk and related message", tmp_local_irk,16);

			u8 status =blc_ll_addDeviceToResolvingList(tmp_peer_addr_type,tmp_peer_addr,tmp_peer_irk,tmp_local_irk);
			tlkapi_send_string_data(APP_LOG_EN,"[APP][RPA] LL resolving list add status", &status,1);

			status = blc_ll_setAddressResolutionEnable(1);
			tlkapi_send_string_data(APP_LOG_EN,"[APP][RPA] LL add resolution enable status", &status, 1);

			status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
										 ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_RESOLVE_PRIVATE_PUBLIC,
										 tmp_peer_addr_type,  tmp_peer_addr,
										 MY_APP_ADV_CHANNEL,
										 ADV_FP_NONE);//  ADV_FP_NONE  ADV_FP_ALLOW_SCAN_ANY_ALLOW_CONN_WL
			if(status != BLE_SUCCESS) { 	while(1);}  //debug: ADV setting err

		#endif

	}

		bls_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable
}



/**
 * @brief		user initialization when MCU power on or wake_up from deepSleep mode
 * @param[in]	none
 * @return      none
 */
_attribute_no_inline_ void user_init_normal(void)
{
	//random number generator must be initiated here( in the beginning of user_init_nromal)
	//when deepSleep retention wakeUp, no need initialize again
#if(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
	random_generator_init();  //this is must
#endif

	#if (UART_PRINT_DEBUG_ENABLE)
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
	//for 1M   Flash, flash_sector_mac_address equals to 0xFF000
	blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);
	tlkapi_send_string_data(APP_LOG_EN,"[APP][INI]Public Address", mac_public, 6);

	#if(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_PUBLIC)
		app_own_address_type = OWN_ADDRESS_PUBLIC;
	#elif(BLE_DEVICE_ADDRESS_TYPE == BLE_DEVICE_ADDRESS_RANDOM_STATIC)
		app_own_address_type = OWN_ADDRESS_RANDOM;
		blc_ll_setRandomAddr(mac_random_static);
	#endif

	////// Controller Initialization  //////////
	blc_ll_initBasicMCU();                      //mandatory
	blc_ll_initStandby_module(mac_public);				//mandatory
	blc_ll_initAdvertising_module(mac_public); 	//ADV module: 		 mandatory for BLE slave,
	blc_ll_initConnection_module();				//connection module  mandatory for BLE slave/master
	blc_ll_initSlaveRole_module();				//slave module: 	 mandatory for BLE slave,
	blc_ll_initPowerManagement_module();        //pm module:      	 optional


	////// Host Initialization  //////////
	blc_gap_peripheral_init();    //gap initialization
	my_att_init(); //gatt initialization
	blc_l2cap_register_handler (blc_l2cap_packet_receive);  	//l2cap initialization

#if (1)
	extern void blc_l2cap_reg_att_cli_handler(void *p);
	blc_l2cap_reg_att_cli_handler(l2cap_matt_handler);

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE );

	blc_hci_registerControllerEventHandler(controller_event_callback); //controller hci event to host all processed in this func

	//bluetooth event
	blc_hci_setEventMask_cmd (HCI_EVT_MASK_DISCONNECTION_COMPLETE);

	//bluetooth low energy(LE) event
	blc_hci_le_setEventMask_cmd(		HCI_LE_EVT_MASK_CONNECTION_COMPLETE 		\
									|	HCI_LE_EVT_MASK_ENHANCED_CONNECTION_COMPLETE \
									|   HCI_LE_EVT_MASK_CONNECTION_ESTABLISH );         //connection establish: telink private event
#endif

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	#if (1)
		blc_smp_peripheral_init();

		//Hid device on android7.0/7.1 or later version
		// New pairing: send security_request immediately after connection complete
		// reConnect:  send security_request 1000mS after connection complete. If master start pairing or encryption before 1000mS timeout, slave do not send security_request.
		blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )
	#else
		blc_smp_setSecurityLevel(No_Security);
	#endif



	///////////////// USER application initialization ///////////////
	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));




	////////////////// config ADV packet /////////////////////

	app_configAdvParam(); ///note: this API set resolve list.



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

	tlkapi_printf(APP_LOG_EN, "[APP][INI] feature_privacy_slave init \n");
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
_attribute_no_inline_ void main_loop(void)
{

	////////////////////////////////////// BLE entry /////////////////////////////////
	blt_sdk_main_loop();


	////////////////////////////////////// UI entry /////////////////////////////////
	#if (UI_KEYBOARD_ENABLE)
		proc_keyboard(0, 0, 0);
	#endif

	////////////////////////////////////// PM Process /////////////////////////////////
	blt_pm_proc();
}


#endif  //end of (FEATURE_TEST_MODE == ...)

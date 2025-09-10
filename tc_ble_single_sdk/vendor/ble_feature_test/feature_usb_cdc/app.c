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

#include "application/usbstd/usb.h"
#include "application/app/usbcdc.h"


#if (FEATURE_TEST_MODE == TEST_USB_CDC)


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



int	central_smp_pending = 0; 		// SMP: security & encryption;
static u32 master_connected_led_on;
static u32 host_update_conn_param_req;
static u16 host_update_conn_min;
static u16 host_update_conn_latency;
static u16 host_update_conn_timeout;
static u32 central_pairing_enable;
static u32 central_unpair_enable;


#if (UI_KEYBOARD_ENABLE)
	#define CONSUMER_KEY   	   		1
	#define KEYBOARD_KEY   	   		2
	_attribute_data_retention_	int 	key_not_released;
	_attribute_data_retention_	u8 		key_type;
	_attribute_data_retention_		static u32 keyScanTick = 0;
	extern u32	scan_pin_need;

	#define KEY_PAIR				VK_1
	#define KEY_UNPAIR				VK_2


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

			tlkapi_printf(APP_LOG_EN,"Key value��%x\n",key0 );
			if(key0 == KEY_SW2)
			{
				central_pairing_enable = 1;

				tlkapi_printf(APP_LOG_EN,"enter pair state\n");
			}
			else if(key0 == KEY_SW3)
			{
				central_unpair_enable = 1;
				tlkapi_printf(APP_LOG_EN,"exit pair state\n");
			}

		}
		else   //kb_event.cnt == 0,  key release
		{
			key_not_released = 0;
			if(central_pairing_enable)
			{
				central_pairing_enable = 0;
			}

			if(central_unpair_enable)
			{
				central_unpair_enable = 0;
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
#endif





#if (BLE_HOST_SMP_ENABLE)
/**
 * @brief      callback function of smp finish
 * @param[in]  none
 * @return     0
 */
int app_host_smp_finish (void)  //smp finish callback
{
	central_smp_pending = 0;

	return 0;
}
#endif




/**
 * @brief      callback function of L2CAP layer handle packet data
 * @param[in]  conn_handle - connect handle
 * @param[in]  raw_pkt - Pointer point to l2cap data packet
 * @return     0
 */
int app_l2cap_handler (u16 conn_handle, u8 *raw_pkt)
{
	//l2cap data packeted, make sure that user see complete l2cap data
	rf_packet_l2cap_t *ptrL2cap = blm_l2cap_packet_pack (conn_handle, raw_pkt);
	if (!ptrL2cap)
		return 0;

	//l2cap data channel id, 4 for att, 5 for signal, 6 for smp
	if(ptrL2cap->chanId == L2CAP_CID_ATTR_PROTOCOL)  //att data
	{
		rf_packet_att_t *pAtt = (rf_packet_att_t*)ptrL2cap;

		if(pAtt->opcode == ATT_OP_EXCHANGE_MTU_REQ || pAtt->opcode == ATT_OP_EXCHANGE_MTU_RSP)
		{

		}
		else if(pAtt->opcode == ATT_OP_HANDLE_VALUE_NOTI)  //slave handle notify
		{

		}
	}
	else if(ptrL2cap->chanId == L2CAP_CID_SIG_CHANNEL)  //signal
	{
		if(ptrL2cap->opcode == L2CAP_CMD_CONN_UPD_PARA_REQ)  //slave send conn param update req on l2cap
		{
			rf_packet_l2cap_connParaUpReq_t  * req = (rf_packet_l2cap_connParaUpReq_t *)ptrL2cap;

			u32 interval_us = req->min_interval*1250;  //1.25ms unit
			u32 timeout_us = req->timeout*10000; //10ms unit
			u32 long_suspend_us = interval_us * (req->latency+1);

			//interval < 200ms, long suspend < 11S, interval * (latency +1)*2 <= timeout
			if( interval_us < 200000 && long_suspend_us < 20000000 && (long_suspend_us*2<=timeout_us) )
			{
				//when master host accept slave's conn param update req, should send a conn param update response on l2cap
				//with CONN_PARAM_UPDATE_ACCEPT; if not accept,should send  CONN_PARAM_UPDATE_REJECT
				blc_l2cap_SendConnParamUpdateResponse(conn_handle, req->id, CONN_PARAM_UPDATE_ACCEPT);  //send SIG Connection Param Update Response

				tlkapi_printf(APP_LOG_EN,"send Connection Param Update accept\n");

				//if accept, master host should mark this, add will send  update conn param req on link layer later set a flag here, then send update conn param req in mainloop
				host_update_conn_param_req = clock_time() | 1 ; //in case zero value
				host_update_conn_min = req->min_interval;  //backup update param
				host_update_conn_latency = req->latency;
				host_update_conn_timeout = req->timeout;
			}
			else
			{
				blc_l2cap_SendConnParamUpdateResponse(conn_handle, req->id, CONN_PARAM_UPDATE_REJECT);  //send SIG Connection Param Update Response
				tlkapi_printf(APP_LOG_EN,"send Connection Param Update reject\n");
			}
		}
	}
	else if(ptrL2cap->chanId == L2CAP_CID_SMP) //smp
	{
		#if (BLE_HOST_SMP_ENABLE)
			if(central_smp_pending)
			{
				blm_host_smp_handler(conn_handle, (u8 *)ptrL2cap);
			}
		#endif
	}
	return 0;
}


/**
 * @brief      call this function when  HCI Controller Event :HCI_SUB_EVT_LE_ADVERTISING_REPORT
 *     		   after controller is set to scan state, it will report all the ADV packet it received by this event
 * @param[in]  p - data pointer of event
 * @return     0
 */
int blm_le_adv_report_event_handle(u8 *p)
{
	//after controller is set to scan state, it will report all the ADV packet it received by this event
	event_adv_report_t *pa = (event_adv_report_t *)p;
	s8 rssi = pa->data[pa->len];

	 //if previous connection smp&sdp not finish, can not create a new connection
	#if (BLE_HOST_SMP_ENABLE)
		if(central_smp_pending){ 	 //if previous connection SMP not finish, can not create a new connection
			return 1;
		}
	#endif


	int master_auto_connect = 0;
	int user_manual_pairing = central_pairing_enable && (rssi > -56);  //key press trigger pairing(rssi threshold, short distance)

	#if (BLE_HOST_SMP_ENABLE)
		if(blc_ll_getCurrentState() != BLS_LINK_STATE_INIT){
			master_auto_connect = tbl_bond_slave_search(pa->adr_type, pa->mac);
		}
	#endif

	if(user_manual_pairing || master_auto_connect)
	{
		//send create connection cmd to controller, trigger it switch to initiating state, after this cmd,
		//controller will scan all the ADV packets it received but not report to host, to find the specified
		//device(adr_type & mac), then send a connection request packet after 150us, enter to connection state
		// and send a connection complete event(HCI_SUB_EVT_LE_CONNECTION_COMPLETE)
		blc_ll_createConnection( SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS, INITIATE_FP_ADV_SPECIFY,  \
								 pa->adr_type, pa->mac, BLE_ADDR_PUBLIC, \
								 CONN_INTERVAL_10MS, CONN_INTERVAL_10MS, 0, CONN_TIMEOUT_4S, \
								 0, 0xFFFF);
	}

	return 0;
}

/**
 * @brief		this function serves to connect terminate
 * @param[in]	p - data pointer of event
 * @return      none
 */
int 	blm_disconnect_event_handle(u8 *p)
{
	hci_disconnectionCompleteEvt_t	*pDisConn = (hci_disconnectionCompleteEvt_t *)p;

	//terminate reason//connection timeout
	if(pDisConn->reason == HCI_ERR_CONN_TIMEOUT){
	}
	//peer device(slave) send terminate cmd on link layer
	else if(pDisConn->reason == HCI_ERR_REMOTE_USER_TERM_CONN){
	}
	//master host disconnect( blm_ll_disconnect(BLM_CONN_HANDLE, HCI_ERR_REMOTE_USER_TERM_CONN) )
	else if(pDisConn->reason == HCI_ERR_CONN_TERM_BY_LOCAL_HOST){
	}
	 //master create connection, send conn_req, but did not received acked packet in 6 connection event
	else if(pDisConn->reason == HCI_ERR_CONN_FAILED_TO_ESTABLISH){ //send connection establish event to host(telink defined event)
	}
	else{
	}

	tlkapi_printf(APP_LOG_EN,"----- terminate rsn: 0x%x -----\n", pDisConn->reason);

	//if previous connection SMP & SDP not finished, clear flag
	#if (BLE_HOST_SMP_ENABLE)
		if(central_smp_pending){
			central_smp_pending = 0;
		}
	#endif

	//led show none connection state
	if(master_connected_led_on){
		master_connected_led_on = 0;
		gpio_write(GPIO_LED_WHITE, LED_ON_LEVEL);   //white on
		gpio_write(GPIO_LED_RED, !LED_ON_LEVEL);    //red off
	}

	host_update_conn_param_req = 0; //when disconnect, clear update conn flag

	//MTU size exchange and data length exchange procedure must be executed on every new connection,
	//so when connection terminate, relative flags must be cleared

	//MTU size reset to default 23 bytes when connection terminated
	blc_att_resetEffectiveMtuSize(pDisConn->connHandle);  //important

	//should set scan mode again to scan slave ADV packet
	blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS, OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
	blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);


	return 0;
}


//////////////////////////////////////////////////////////
// event call back
//////////////////////////////////////////////////////////


/**
 * @brief      callback function of HCI Controller Event
 * @param[in]  h - HCI Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */

int controller_event_callback (u32 h, u8 *p, int n)
{
    (void)h;(void)p;(void)n;
	if (h &HCI_FLAG_EVENT_BT_STD)		//ble controller hci event
	{
		u8 evtCode = h & 0xff;

		//------------ disconnect -------------------------------------
		if(evtCode == HCI_EVT_DISCONNECTION_COMPLETE)  //connection terminate
		{
			blm_disconnect_event_handle(p);
		}
#if (BLE_HOST_SMP_ENABLE)
		else if(evtCode == HCI_EVT_ENCRYPTION_CHANGE)
		{
			event_enc_change_t *pe = (event_enc_change_t *)p;
			blm_smp_encChangeEvt(pe->status, pe->handle, pe->enc_enable);
		}
		else if(evtCode == HCI_EVT_ENCRYPTION_KEY_REFRESH)
		{
			event_enc_refresh_t *pe = (event_enc_refresh_t *)p;
			blm_smp_encChangeEvt(pe->status, pe->handle, 1);
		}
#endif
		else if(evtCode == HCI_EVT_LE_META)
		{
			u8 subEvt_code = p[0];

			//------hci le event: le connection establish event---------------------------------
			if(subEvt_code == HCI_SUB_EVT_LE_CONNECTION_ESTABLISH)  //connection establish(telink private event)
			{
				event_connection_complete_t *pCon = (event_connection_complete_t *)p;

				if (pCon->status == BLE_SUCCESS)	// status OK
				{
					tlkapi_printf(APP_LOG_EN,"----- connected -----\n");

					//led show connection state
					master_connected_led_on = 1;
					gpio_write(GPIO_LED_RED, LED_ON_LEVEL);     //red on
					gpio_write(GPIO_LED_WHITE, !LED_ON_LEVEL);  //white off

					#if (BLE_HOST_SMP_ENABLE)
						central_smp_pending = 1; //pair & security first
					#endif
				}
			}
			//--------hci le event: le ADV report event ----------------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_ADVERTISING_REPORT)	// ADV packet
			{
				blm_le_adv_report_event_handle(p);
			}
			//--------hci le event: le data length change event ----------------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_DATA_LENGTH_CHANGE)
			{

			}
		}
	}
	return 0;
}


extern unsigned char usb_cdc_data[CDC_TXRX_EPSIZE];
extern unsigned short usb_cdc_data_len;

void usb_cdc_init(void){
	usb_set_pin_en();
	usb_init_interrupt();
}



/**
 * @brief		user initialization
 * @param[in]	none
 * @return      none
 */
void user_init(void)
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

	//set USB ID
	REG_ADDR8(0x74) = 0x62;
	REG_ADDR16(0x7e) = 0x08d0;
	REG_ADDR8(0x74) = 0x00;

	reg_usb_mask = BIT(7);
	reg_irq_mask |= FLD_IRQ_IRQ4_EN;
	reg_usb_ep6_buf_addr = 0x80;
	reg_usb_ep7_buf_addr = 0x60;
	reg_usb_ep_max_size = (256 >> 3);

	usb_dp_pullup_en (1);  //open USB enum

	usb_cdc_init();

	u8  mac_public[6];
	u8  mac_random_static[6];
	//for 512K Flash, flash_sector_mac_address equals to 0x76000
	//for 1M  Flash, flash_sector_mac_address equals to 0xFF000
	blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);
	tlkapi_send_string_data(APP_LOG_EN,"[APP][INI]Public Address", mac_public, 6);

	////// Controller Initialization  //////////
	blc_ll_initBasicMCU();
	blc_ll_initStandby_module(mac_public);				//mandatory
	blc_ll_initScanning_module(mac_public); 			//scan module: 		 mandatory for BLE master,
	blc_ll_initInitiating_module();						//initiate module: 	 mandatory for BLE master,
	blc_ll_initConnection_module();						//connection module  mandatory for BLE slave/master
	blc_ll_initMasterRoleSingleConn_module();			//master module: 	 mandatory for BLE master,

	rf_set_power_level_index (MY_RF_POWER_INDEX);

	////// Host Initialization  //////////
	blc_gap_central_init();										//gap initialization
	blc_l2cap_register_handler (app_l2cap_handler);    			//l2cap initialization
	blc_hci_registerControllerEventHandler(controller_event_callback); //controller hci event to host all processed in this func

	//bluetooth event
	blc_hci_setEventMask_cmd (HCI_EVT_MASK_DISCONNECTION_COMPLETE | HCI_EVT_MASK_ENCRYPTION_CHANGE);

	//bluetooth low energy(LE) event
	blc_hci_le_setEventMask_cmd(  HCI_LE_EVT_MASK_CONNECTION_COMPLETE  \
							    | HCI_LE_EVT_MASK_ADVERTISING_REPORT \
							    | HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE \
							    | HCI_LE_EVT_MASK_DATA_LENGTH_CHANGE \
							    | HCI_LE_EVT_MASK_CONNECTION_ESTABLISH ); //connection establish: telink private event

	//ATT initialization
	blc_att_setRxMtuSize(MTU_SIZE_SETTING); //set MTU size, default MTU is 23 if not call this API

	#if (MTU_SIZE_SETTING > ATT_MTU_MAX_SDK_DFT_BUF)
		blc_l2cap_initMtuBuffer(app_l2cap_rx_fifo, ACL_L2CAP_BUFF_SIZE, app_l2cap_tx_fifo, ACL_L2CAP_BUFF_SIZE);
	#endif

	#if (BLE_HOST_SMP_ENABLE)
		blm_smp_configPairingSecurityInfoStorageAddr(flash_sector_master_pairing);
		blm_smp_registerSmpFinishCb(app_host_smp_finish);

		blc_smp_central_init();

		//SMP trigger by master
		blm_host_smp_setSecurityTrigger(MASTER_TRIGGER_SMP_FIRST_PAIRING | MASTER_TRIGGER_SMP_AUTO_CONNECT);
	#else
		blc_smp_setSecurityLevel(No_Security);
	#endif

	/* set scan parameter and scan enable */
	blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS, OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
	blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);


	/* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done.
	 * attention that code will stuck in "while(1)" if any error detected in initialization, user need find what error happens and then fix it */
	blc_app_checkControllerHostInitialization();

	//central or peripheral initiate Data Length Update procedure
	tlkapi_printf(APP_LOG_EN, "[APP][INI] feature DLE master init \n");

}

/**
 * @brief		master dle test in mainloop
 * @param[in]	none
 * @return      none
 */
void usb_cdc_mainloop(void)
{

	////////////////////////////////////// UI entry /////////////////////////////////
#if (UI_KEYBOARD_ENABLE)
	proc_keyboard(0,0,0);
#endif

	////////////////////////////////////// host connection update /////////////////////////////////
	if( host_update_conn_param_req && clock_time_exceed(host_update_conn_param_req, 50000))
	{
		host_update_conn_param_req = 0;
		if(blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){  //still in connection state
			blm_ll_updateConnection (BLM_CONN_HANDLE, host_update_conn_min, host_update_conn_min, host_update_conn_latency,  host_update_conn_timeout, 0, 0 );
		}
	}

	////////////////////////////////////// terminate unpair proc /////////////////////////////////
	static int master_disconnect_flag;
	if(central_unpair_enable){
		if(!master_disconnect_flag && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){
			if( blm_ll_disconnect(BLM_CONN_HANDLE, HCI_ERR_REMOTE_USER_TERM_CONN) == BLE_SUCCESS){
				master_disconnect_flag = 1;
				central_unpair_enable = 0;
			}
		}
	}
	if(master_disconnect_flag && blc_ll_getCurrentState() != BLS_LINK_STATE_CONN){
		master_disconnect_flag = 0;
	}

	////////////////////////////////////// USB CDC Proc /////////////////////////////////
	usb_handle_irq();
	if(usb_cdc_data_len!=0)
	{
		usb_cdc_tx_data_to_host(usb_cdc_data,usb_cdc_data_len);
		usb_cdc_data_len = 0;
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

	usb_cdc_mainloop();
}


#endif  //end of (FEATURE_TEST_MODE == ...)

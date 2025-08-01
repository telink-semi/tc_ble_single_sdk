/********************************************************************************************************
 * @file    blm_host.c
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

#if (FEATURE_TEST_MODE == TEST_MD_MASTER)

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "app.h"
#include "blm_host.h"




////////////////////////////////////////////////////////////////////
u32 host_update_conn_param_req = 0;
u16 host_update_conn_min;
u16 host_update_conn_latency;
u16 host_update_conn_timeout;

u16 final_MTU_size = 23;

int master_connected_led_on = 0;


int	central_smp_pending = 0; 		// SMP: security & encryption;

int master_auto_connect = 0;
int user_manual_pairing;


int	central_pairing_enable = 0;
int central_unpair_enable = 0;

int device_in_connection_state;

u32 device_connection_tick;

extern u8 app_test_data[];
extern int write_data_test_tick;
static u8 seq_num_next = 0;

//no service discovery, use agreed ATT handle value with peer Slave device
//need define att handle same with slave
#define 		HID_HANDLE_MOUSE_REPORT
#define			HID_HANDLE_CONSUME_REPORT			25
#define			HID_HANDLE_KEYBOARD_REPORT			29
#define			AUDIO_HANDLE_MIC					52





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
 * @brief      call this function when  HCI Controller Event :HCI_SUB_EVT_LE_ADVERTISING_REPORT
 *     		   after controller is set to scan state, it will report all the ADV packet it received by this event
 * @param[in]  p - data pointer of event
 * @return     0
 */
int blm_le_adv_report_event_handle(u8 *p)
{
	event_adv_report_t *pa = (event_adv_report_t *)p;
	s8 rssi = pa->data[pa->len];

	 //if previous connection smp&sdp not finish, can not create a new connection
	#if (BLE_HOST_SMP_ENABLE)
		if(central_smp_pending){ 	 //if previous connection SMP not finish, can not create a new connection
			return 1;
		}
	#endif


	/****************** Button press or Adv pair packet triggers pair ***********************/
	int master_auto_connect = 0;
	int user_manual_pairing = 0;

	//manual pairing methods 1: button triggers
	user_manual_pairing = central_pairing_enable && (rssi > -56);  //button trigger pairing(rssi threshold, short distance)


	#if (BLE_HOST_SMP_ENABLE)
		if(blc_ll_getCurrentState() != BLS_LINK_STATE_INIT){
			master_auto_connect = tbl_bond_slave_search(pa->adr_type, pa->mac);
		}
	#endif

	if(master_auto_connect || user_manual_pairing)
	{
		//send create connection cmd to controller, trigger it switch to initiating state, after this cmd,
		//controller will scan all the ADV packets it received but not report to host, to find the specified
		//device(adr_type & mac), then send a connection request packet after 150us, enter to connection state
		// and send a connection complete event(HCI_SUB_EVT_LE_CONNECTION_COMPLETE)
		u8 status = blc_ll_createConnection( SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS, INITIATE_FP_ADV_SPECIFY,  \
								 pa->adr_type, pa->mac, BLE_ADDR_PUBLIC, \
								 CONN_INTERVAL_10MS, CONN_INTERVAL_10MS, 0, CONN_TIMEOUT_4S, \
								 0, 0xFFFF);
		if(status == BLE_SUCCESS)   //create connection success
		{

		}

	}


	return 0;
}


/**
 * @brief		this connection event is defined by telink, not a standard ble controller event
 * 				after master controller send connection request packet to slave, when slave received this packet
 * 				and enter to connection state, send a ack packet within 6 connection event, master will send
 *              connection establish event to host(HCI_SUB_EVT_LE_CONNECTION_ESTABLISH)
 * @param[in]	p - data pointer of event
 * @return      none
 */
int blm_le_connection_establish_event_handle(u8 *p)
{

	hci_le_connectionCompleteEvt_t *pConnEvt = (hci_le_connectionCompleteEvt_t *)p;
	if (pConnEvt->status == BLE_SUCCESS)	// status OK
	{
		#if (UI_LED_ENABLE)
			//led show connection state
			master_connected_led_on = 1;
			gpio_write(GPIO_LED_RED, LED_ON_LEVEL);     //red on
			gpio_write(GPIO_LED_WHITE, !LED_ON_LEVEL);  //white off
		#endif


		cur_conn_device.conn_handle = pConnEvt->connHandle;   //mark conn handle, in fact this equals to BLM_CONN_HANDLE

		//save current connect address type and address
		cur_conn_device.mac_adrType = pConnEvt->peerAddrType;
		memcpy(cur_conn_device.mac_addr, pConnEvt->peerAddr, 6);


		#if (BLE_HOST_SMP_ENABLE)
			central_smp_pending = 1; //pair & security first
		#endif

		seq_num_next = 0; //clear

		app_test_data[0] = 0;  //clear

		device_in_connection_state = 1;

		device_connection_tick = clock_time() | 1;
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

	//terminate reason
	//connection timeout
	if(pDisConn->reason == HCI_ERR_CONN_TIMEOUT){

	}
	//peer device(slave) send terminate cmd on link layer
	else if(pDisConn->reason == HCI_ERR_REMOTE_USER_TERM_CONN){

	}
	//master host disconnect( blm_ll_disconnect(BLM_CONN_HANDLE, HCI_ERR_REMOTE_USER_TERM_CONN) )
	else if(pDisConn->reason == HCI_ERR_CONN_TERM_BY_LOCAL_HOST){

	}
	 //master create connection, send conn_req, but did not received acked packet in 6 connection event
	else if(pDisConn->reason == HCI_ERR_CONN_FAILED_TO_ESTABLISH){
		//when controller is in initiating state, find the specified device, send connection request to slave,
		//but slave lost this rf packet, there will no ack packet from slave, after 6 connection events, master
		//controller send a disconnect event with reason HCI_ERR_CONN_FAILED_TO_ESTABLISH
		//if slave got the connection request packet and send ack within 6 connection events, controller
		//send connection establish event to host(telink defined event)


	}
	else{

	}

	#if (UI_LED_ENABLE)
		//led show none connection state
		if(master_connected_led_on){
			master_connected_led_on = 0;
			gpio_write(GPIO_LED_WHITE, LED_ON_LEVEL);   //white on
			gpio_write(GPIO_LED_RED, !LED_ON_LEVEL);    //red off
		}
	#endif


	cur_conn_device.conn_handle = 0;  //when disconnect, clear conn handle


	//if previous connection SMP & SDP not finished, clear flag
	#if (BLE_HOST_SMP_ENABLE)
		if(central_smp_pending){
			central_smp_pending = 0;
		}
	#endif

	host_update_conn_param_req = 0; //when disconnect, clear update conn flag

	device_in_connection_state = 0;

	device_connection_tick = 0;
	write_data_test_tick = 0;

	//MTU size reset to default 23 bytes when connection terminated
	blc_att_resetEffectiveMtuSize(BLM_CONN_HANDLE);	 //important

	final_MTU_size = 23;

	//should set scan mode again to scan slave ADV packet
	blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS,
							OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
	blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);


	return 0;
}


/**
 * @brief      call this function when  HCI Controller Event :HCI_SUB_EVT_LE_CONNECTION_UPDATE_COMPLETE
 * @param[in]  p - data pointer of event
 * @return     0
 */
int blm_le_conn_update_event_proc(u8 *p)
{


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
 * @return     0
 */
int controller_event_callback (u32 h, u8 *p, int n)
{

	(void)n;
	static u32 event_cb_num;
	event_cb_num++;

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

			//------hci le event: le connection complete event---------------------------------
			if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_COMPLETE)	// connection complete
			{
				//after controller is set to initiating state by host (blc_ll_createConnection(...) )
				//it will scan the specified device(adr_type & mac), when find this ADV packet, send a connection request packet to slave
				//and enter to connection state, send connection complete event. but notice that connection complete not
				//equals to connection establish. connection complete means that master controller set all the ble timing
				//get ready, but has not received any slave packet, if slave rf lost the connection request packet, it will
				//not send any packet to master controller
				printf("conn_success");

			}
			//------hci le event: le connection establish event---------------------------------
			else if(subEvt_code == HCI_SUB_EVT_LE_CONNECTION_ESTABLISH)  //connection establish(telink private event)
			{
				//notice that: this connection event is defined by telink, not a standard ble controller event
				//after master controller send connection request packet to slave, when slave received this packet
				//and enter to connection state, send a ack packet within 6 connection event, master will send
				//connection establish event to host(HCI_SUB_EVT_LE_CONNECTION_ESTABLISH)

				blm_le_connection_establish_event_handle(p);
			}
			//--------hci le event: le ADV report event ----------------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_ADVERTISING_REPORT)	// ADV packet
			{
				//after controller is set to scan state, it will report all the ADV packet it received by this event

				blm_le_adv_report_event_handle(p);
			}
			//------hci le event: le connection update complete event-------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_CONNECTION_UPDATE_COMPLETE)	// connection update
			{
				//after master host send update conn param req cmd to controller( blm_ll_updateConnection(...) ),
				//when update take effect, controller send update complete event to host
				blm_le_conn_update_event_proc(p);
			}
			//------hci le event: le phy update complete event-------------------------------
			else if (subEvt_code == HCI_SUB_EVT_LE_PHY_UPDATE_COMPLETE)	// connection update
			{
				//2 situation can trigger this event:
				//   1) master host trigger: by calling API  blc_ll_setPhy(...)
				//   2) peer slave device trigger: send "LL_PHY_REQ" on linklayer
				//when update take effect, controller send update complete event to host

			}

		}
	}


	return 0;

}


volatile int app_l2cap_handle_cnt = 0;

/**
 * @brief      callback function of L2CAP layer handle packet data
 * @param[in]  conn_handle - connect handle
 * @param[in]  raw_pkt - Pointer point to l2cap data packet
 * @return     0
 */
int app_l2cap_handler (u16 conn_handle, u8 *raw_pkt)
{
	app_l2cap_handle_cnt ++;  //debug




	//l2cap data packeted, make sure that user see complete l2cap data
	rf_packet_l2cap_t *ptrL2cap = blm_l2cap_packet_pack (conn_handle, raw_pkt);
	if (!ptrL2cap)
	{
		return 0;
	}



	//l2cap data channel id, 4 for ATT, 5 for Signal, 6 for SMP
	if(ptrL2cap->chanId == L2CAP_CID_ATTR_PROTOCOL)  //att data
	{

		rf_packet_att_t *pAtt = (rf_packet_att_t*)ptrL2cap;
		u16 attHandle = pAtt->handle0 | pAtt->handle1<<8;


		if(pAtt->opcode == ATT_OP_EXCHANGE_MTU_REQ || pAtt->opcode == ATT_OP_EXCHANGE_MTU_RSP)
		{
			rf_packet_att_mtu_exchange_t *pMtu = (rf_packet_att_mtu_exchange_t*)ptrL2cap;

			if(pAtt->opcode ==  ATT_OP_EXCHANGE_MTU_REQ){
				blc_att_responseMtuSizeExchange(conn_handle, ATT_MTU_MAX_SDK_DFT_BUF);
			}

			u16 peer_mtu_size = (pMtu->mtu[0] | pMtu->mtu[1]<<8);
			final_MTU_size = min(ATT_MTU_MAX_SDK_DFT_BUF, peer_mtu_size);
			blc_att_setEffectiveMtuSize(conn_handle , final_MTU_size);  //important
		}
		else if(pAtt->opcode == ATT_OP_READ_BY_TYPE_RSP)  //slave ack ATT_OP_READ_BY_TYPE_REQ data
		{

		}
		else if(pAtt->opcode == ATT_OP_HANDLE_VALUE_NOTI)  //slave handle notify
		{
			if(attHandle == SPP_HANDLE_DATA_S2C)
			{
				u8 len = pAtt->l2capLen - 3;
				if(len > 0)
				{
					u8 seq_num = pAtt->dat[0];

					if(seq_num == seq_num_next){
						if(BLE_SUCCESS == blc_gatt_pushWriteCommand(BLM_CONN_HANDLE, SPP_HANDLE_DATA_C2S, app_test_data, 20)){
							app_test_data[0]++;
						}
					}
					else{
						write_reg8(0x40000, 0x66);
						irq_disable();
						while(1);
						write_reg8(0x40000, 0x22);
					}

					seq_num_next = seq_num + 1;
				}
			}
		}
		else if (pAtt->opcode == ATT_OP_HANDLE_VALUE_IND)
		{
			blc_gatt_pushConfirm(conn_handle);
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

			//interval < 200ms
			//long suspend < 11S
			// interval * (latency +1)*2 <= timeout
			if( interval_us < 200000 && long_suspend_us < 20000000 && (long_suspend_us*2<=timeout_us) )
			{
				//when master host accept slave's conn param update req, should send a conn param update response on l2cap
				//with CONN_PARAM_UPDATE_ACCEPT; if not accept,should send  CONN_PARAM_UPDATE_REJECT
				blc_l2cap_SendConnParamUpdateResponse(conn_handle, req->id, CONN_PARAM_UPDATE_ACCEPT);  //send SIG Connection Param Update Response


				//if accept, master host should mark this, add will send  update conn param req on link layer later
				//set a flag here, then send update conn param req in mainloop
				host_update_conn_param_req = clock_time() | 1 ; //in case zero value
				host_update_conn_min = req->min_interval;  //backup update param
				host_update_conn_latency = req->latency;
				host_update_conn_timeout = req->timeout;
			}
			else
			{
				blc_l2cap_SendConnParamUpdateResponse(conn_handle, req->id, CONN_PARAM_UPDATE_REJECT);  //send SIG Connection Param Update Response
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
	else
	{

	}


	return 0;
}


#endif  //end of (FEATURE_TEST_MODE == ...)

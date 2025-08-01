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
#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "app.h"
#include "blm_att.h"
#include "blm_host.h"
#include "blm_ota.h"

#include "application/audio/audio_config.h"
#include "application/audio/tl_audio.h"
#include "app_audio.h"


extern u8 read_by_type_req_uuidLen;
extern u8 read_by_type_req_uuid[16];


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

const u8 	telink_adv_trigger_pairing[] = {5, 0xFF, 0x11, 0x02, 0x01, 0x00};
const u8 	telink_adv_trigger_unpair[] = {5, 0xFF, 0x11, 0x02, 0x01, 0x01};

const u8 	telink_adv_trigger_pairing_8258[] = {7, 0xFF, 0x11, 0x02, 0x01, 0x00, 0x58, 0x82};
const u8 	telink_adv_trigger_unpair_8258[] = {7, 0xFF, 0x11, 0x02, 0x01, 0x01, 0x58, 0x82};


#if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)

extern void host_att_service_disccovery_clear(void);

#if(TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE)
u8 google_voice_model = 0;
#endif

#define			HID_HANDLE_CONSUME_REPORT			conn_char_handler[3]
#define			HID_HANDLE_KEYBOARD_REPORT			conn_char_handler[4]
#define			HID_HANDLE_MOUSE_REPORT				conn_char_handler[5]
#define			AUDIO_HANDLE_MIC					conn_char_handler[0]
#define			HID_HANDLE_KEYBOARD_REPORT_OUT		conn_char_handler[6]
#define			AUDIO_FIRST_REPORT					conn_char_handler[7]

#if(TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE)

#define 		GOOGLE_AUDIO_HANDLE_MIC_CMD			conn_char_handler[8]//52
#define 		GOOGLE_AUDIO_HANDLE_MIC_DATA		conn_char_handler[9]//54
#define 		GOOGLE_AUDIO_HANDLE_MIC_RSP			conn_char_handler[10]//57
#else
#endif

#else  //no service discovery, use agreed ATT handle value with peer Slave device

//need define att handle same with slave
#define 		HID_HANDLE_MOUSE_REPORT
#define			HID_HANDLE_CONSUME_REPORT			25
#define			HID_HANDLE_KEYBOARD_REPORT			29
#define			AUDIO_HANDLE_MIC					52


#endif




#if (BLE_HOST_SMP_ENABLE)
/**
 * @brief      callback function of smp finish
 * @param[in]  none
 * @return     0
 */
int app_host_smp_finish (void)  //smp finish callback
{
	#if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)  //smp finish, start sdp
		if(central_smp_pending)
		{
			//new slave device, should do service discovery again
			if (cur_conn_device.mac_adrType != serviceDiscovery_adr_type || \
				memcmp(cur_conn_device.mac_addr, serviceDiscovery_address, 6))
			{
				app_register_service(&app_service_discovery);
				central_sdp_pending = 1; //service discovery busy
			}
			else
			{
				central_sdp_pending = 0;  //no need simple SDP
#if(TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE)
			#if (GOOGLE_VOICE_OVER_BLE_SPCE_VERSION == GOOGLE_VERSION_1_0)
				u8 caps_data[6]={0};
				caps_data[0] = 0x0A; //get caps
				caps_data[1] = 0x01;
				caps_data[2] = 0x00; // version 0x0100;
				caps_data[3] = 0x00;
				caps_data[4] = 0x03; // legacy 0x0003;
				caps_data[5] = GOOGLE_VOICE_MODE;
				blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,conn_char_handler[8],caps_data,6);

				u8 rx_ccc[2] = {0x00, 0x01};		//Write Rx CCC value for PTV use case
				blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,conn_char_handler[9]+1,rx_ccc,2);
			#else
				u8 caps_data[5]={0};
				caps_data[0] = 0x0A; //get caps
				caps_data[1] = 0x00;
				caps_data[2] = 0x04; // version 0x0004;
				caps_data[3] = 0x00;
				caps_data[4] = 0x03; // legacy 0x0003;
				blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,conn_char_handler[8],caps_data,5);
			#endif
#endif
			}
		}
	#endif

	#if (BLE_HOST_SMP_ENABLE)
		central_smp_pending = 0;
	#endif


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

	#if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
		if(central_sdp_pending){ 	 //if previous connection SDP not finish, can not create a new connection
			return 1;
		}
	#endif


	/****************** Button press or Adv pair packet triggers pair ***********************/
	int master_auto_connect = 0;
	int user_manual_pairing = 0;

	//manual pairing methods 1: button triggers
	user_manual_pairing = central_pairing_enable && (rssi > -56);  //button trigger pairing(rssi threshold, short distance)

	//manual pairing methods 2: special pairing ADV data
	if(!user_manual_pairing){  //special ADV pair data can also trigger pairing
		user_manual_pairing = (memcmp(pa->data, telink_adv_trigger_pairing_8258, sizeof(telink_adv_trigger_pairing_8258)) == 0) && (rssi > -56);
	}


	#if (BLE_HOST_SMP_ENABLE)
		if(blc_ll_getCurrentState() != BLS_LINK_STATE_INIT){
			master_auto_connect = tbl_bond_slave_search(pa->adr_type, pa->mac);
		}
	#elif (ACL_CENTRAL_CUSTOM_PAIR_ENABLE)
		//search in slave mac table to find whether this device is an old device which has already paired with master
		master_auto_connect = user_tbl_slave_mac_search(pa->adr_type, pa->mac);
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
			#if (ACL_CENTRAL_CUSTOM_PAIR_ENABLE)
				if(user_manual_pairing && !master_auto_connect){  //manual pair
					blm_manPair.manual_pair = 1;
					blm_manPair.mac_type = pa->adr_type;
					memcpy(blm_manPair.mac, pa->mac, 6);
					blm_manPair.pair_tick = clock_time();
				}
			#endif
		}

	}


	/****************** Adv unpair packet triggers unpair ***********************/
	int adv_unpair_en = !memcmp(pa->data, telink_adv_trigger_unpair_8258, sizeof(telink_adv_trigger_unpair_8258));
	if(adv_unpair_en)
	{
		int device_is_bond;

		#if (BLE_HOST_SMP_ENABLE)
			device_is_bond = tbl_bond_slave_search(pa->adr_type, pa->mac);
			if(device_is_bond){ //this ADV mac is bonded in master
				tbl_bond_slave_delete_by_adr(pa->adr_type, pa->mac);  //by telink stack host smp
			}
		#elif (ACL_CENTRAL_CUSTOM_PAIR_ENABLE)
			device_is_bond = user_tbl_slave_mac_search(pa->adr_type, pa->mac);
			if(device_is_bond){ //this ADV mac is bonded in master
				user_tbl_slave_mac_delete_by_adr(pa->adr_type, pa->mac);  //by user application code
			}
		#endif
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
		#elif (ACL_CENTRAL_CUSTOM_PAIR_ENABLE)
			//manual pairing, device match, add this device to slave mac table
			if(blm_manPair.manual_pair && blm_manPair.mac_type == pConnEvt->peerAddrType && !memcmp(blm_manPair.mac, pConnEvt->peerAddr, 6)){
				blm_manPair.manual_pair = 0;

				user_tbl_slave_mac_add(pCon->peerAddrType, pCon->peerAddr);
			}
		#endif

		#if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
			//new slave device, should do service discovery again
			if (pConnEvt->peerAddrType != serviceDiscovery_adr_type || memcmp(pConnEvt->peerAddr, serviceDiscovery_address, 6)){
				app_register_service(&app_service_discovery);
				central_sdp_pending = 1;  //service discovery busy
			}
			else{
				central_sdp_pending = 0;  //no need sdp
			}
		#endif
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
	#if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
		if(central_sdp_pending){
			central_sdp_pending = 0;
		}
	#endif


	host_update_conn_param_req = 0; //when disconnect, clear update conn flag

	host_att_data_clear();


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

	#if (BLE_MASTER_OTA_ENABLE)
		hci_le_connectionUpdateCompleteEvt_t *pCon = (hci_le_connectionUpdateCompleteEvt_t *)p;

		extern void host_ota_update_conn_complete(u16, u16, u16);
		host_ota_update_conn_complete( pCon->connInterval, pCon->connLatency, pCon->supervisionTimeout );
	#endif

	return 0;
}

/**
 * @brief      call this function when  HCI Controller Event :HCI_SUB_EVT_LE_PHY_UPDATE_COMPLETE
 * @param[in]  p - data pointer of event
 * @return     0
 */
int blm_le_phy_update_complete_event_proc(u8 *p)
{
	(void)p;
//	hci_le_phyUpdateCompleteEvt_t *pPhyUpt = (hci_le_phyUpdateCompleteEvt_t *)p;



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
				blm_le_phy_update_complete_event_proc(p);
			}

		}
	}


	return 0;

}



/**
 * @brief      update connection parameter in mainloop
 * @param[in]  none
 * @return     none
 */
_attribute_ram_code_
void host_update_conn_proc(void)
{
	//at least 50ms later and make sure SMP/SDP is finished
	#if (BLE_HOST_SMP_ENABLE)
		if(central_smp_pending){
			return;
		}
	#endif

	#if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
		if(central_sdp_pending){
			return;
		}
	#endif

	if( host_update_conn_param_req && clock_time_exceed(host_update_conn_param_req, 50000))
	{
		host_update_conn_param_req = 0;

		if(blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){  //still in connection state
			blm_ll_updateConnection (cur_conn_device.conn_handle,
					host_update_conn_min, host_update_conn_min, host_update_conn_latency,  host_update_conn_timeout,
											  0, 0 );
		}
	}
}

#if (TL_AUDIO_MODE & TL_AUDIO_MASK_HID_SERVICE_CHANNEL)
	u8 audio_start = 0x01;
	u8 adpcm_hid_audio_stop  = 0x00;
	u8 audio_config  = 0x02;
	extern u8 mic_cnt;
	extern u8 att_mic_rcvd;
	u8  host_write_dat[32] = {0};
	u8		tmp_mic_data[MIC_ADPCM_FRAME_SIZE];
#endif


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

		#if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
			if(central_sdp_pending)  //ATT service discovery is ongoing
			{
				//when service discovery function is running, all the ATT data from slave
				//will be processed by it,  user can only send your own att cmd after  service discovery is over
				host_att_client_handler (conn_handle, (u8 *)ptrL2cap); //handle this ATT data by service discovery process
			}
		#endif


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
			#if (BLE_MASTER_OTA_ENABLE)
				//when use ATT_OP_READ_BY_TYPE_REQ to find ota atthandle, should get the result
				extern void host_find_slave_ota_attHandle(u8 *p);
				host_find_slave_ota_attHandle( (u8 *)pAtt );
			#endif
			//u16 slave_ota_handle;
		}
		else if(pAtt->opcode == ATT_OP_HANDLE_VALUE_NOTI)  //slave handle notify
		{

			if(attHandle == HID_HANDLE_CONSUME_REPORT)
			{
				static u32 app_key;
				app_key++;

				#if (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_HID_DONGLE_TO_STB)
					static u32 rcu_cmd = 0;
					rcu_cmd = (pAtt->dat[3]<<24)|(pAtt->dat[2]<<16) | (pAtt->dat[1]<<8)|(pAtt->dat[0]);

					if(rcu_cmd == MIC_OPEN_FROM_RCU){//open mic
						if(blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,HID_HANDLE_KEYBOARD_REPORT_OUT,(u8 *)&audio_start, 1)){
							// fail
							// while(1);
						}
						u8 key[20] = {0};
						u32 mic_open_to_stb = MIC_OPEN_TO_STB;
						memcpy(key,(u8 *)&mic_open_to_stb,4);
						mic_packet_reset();
						usb_report_hid_mic(key, 2);
					}
					else if(rcu_cmd == MIC_CLOSE_FROM_RCU){ //close mic

						if(blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,HID_HANDLE_KEYBOARD_REPORT_OUT,(u8 *)&adpcm_hid_audio_stop, 1)){
							// fail
							// while(1);
						}
						u8 key[20] = {0};
						u32 mic_close_to_stb = MIC_CLOSE_TO_STB;
						memcpy(key,(u8 *)&mic_close_to_stb,4);
						usb_report_hid_mic(key, 2);
					}
					else
					{
						att_keyboard_media (conn_handle, pAtt->dat);
					}

				#elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_HID)
					static u16 tem_data = 0;
					tem_data = pAtt->dat[3];

					if(tem_data == 0x21){

						if(blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,HID_HANDLE_KEYBOARD_REPORT_OUT,(u8 *)&audio_start, 1)){  //open mic
							// fail
							 while(1);
						}
						abuf_init ();
					}
					else if(tem_data == 0x24){
						if(blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,HID_HANDLE_KEYBOARD_REPORT_OUT,(u8 *)&adpcm_hid_audio_stop, 1)){  //close mic
							// fail
							 while(1);
						}
					}
					else{
						att_keyboard_media (conn_handle, pAtt->dat);
					}
				#elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_SBC_HID_DONGLE_TO_STB)
					static u32 rcu_cmd = 0;
					rcu_cmd = (pAtt->dat[3]<<24)|(pAtt->dat[2]<<16) | (pAtt->dat[1]<<8)|(pAtt->dat[0]);

					if(rcu_cmd == MIC_OPEN_FROM_RCU){//open mic

						if(blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,HID_HANDLE_KEYBOARD_REPORT_OUT,(u8 *)&audio_start, 1)){
							// fail
							// while(1);
						}
						u8 key[20] = {0};
						u32 mic_open_to_stb = MIC_OPEN_TO_STB;
						memcpy(key,(u8 *)&mic_open_to_stb,4);
						mic_packet_reset();
						usb_report_hid_mic(key, 2);
					}
					else if(rcu_cmd == MIC_CLOSE_FROM_RCU){ //close mic

						if(blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,HID_HANDLE_KEYBOARD_REPORT_OUT,(u8 *)&adpcm_hid_audio_stop, 1)){
							// fail
							// while(1);
						}
						u8 key[20] = {0};
						u32 mic_close_to_stb = MIC_CLOSE_TO_STB;
						memcpy(key,(u8 *)&mic_close_to_stb,4);
						usb_report_hid_mic(key, 2);
					}
					else
					{
						att_keyboard_media (conn_handle, pAtt->dat);
					}

				#elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_SBC_HID)
					static u16 tem_data = 0;
					tem_data = pAtt->dat[3];

					if(tem_data == 0x31){

						if(blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,HID_HANDLE_KEYBOARD_REPORT_OUT,(u8 *)&audio_start, 1)){
							// fail
							while(1);
						}
					}
					else if(tem_data == 0x34){
						if(blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,HID_HANDLE_KEYBOARD_REPORT_OUT,(u8 *)&adpcm_hid_audio_stop, 1)){
							// fail
							while(1);
						}
					}
					else{
						att_keyboard_media (conn_handle, pAtt->dat);
					}
				#elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_MSBC_HID)
					static u16 tem_data = 0;
					tem_data = pAtt->dat[3];

					if(tem_data == 0x31){

						if(blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,HID_HANDLE_KEYBOARD_REPORT_OUT,(u8 *)&audio_start, 1)){
							// fail
							while(1);
						}
					}
					else if(tem_data == 0x34){

						if(blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,HID_HANDLE_KEYBOARD_REPORT_OUT,(u8 *)&adpcm_hid_audio_stop, 1)){
							// fail
							while(1);
						}
					}
					else{
						att_keyboard_media (conn_handle, pAtt->dat);
					}
				#else
					att_keyboard_media (conn_handle, pAtt->dat);
				#endif
			}
			else if(attHandle == HID_HANDLE_KEYBOARD_REPORT)
			{
				static u32 app_key;
				app_key++;
				att_keyboard (conn_handle, pAtt->dat);

			}
//			else if(HID_HANDLE_MOUSE_REPORT){
//				static u32 app_mouse_dat;
//				att_mouse(conn_handle,pAtt->dat);
//			}

#if (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_TELINK)
			else if(attHandle == AUDIO_HANDLE_MIC)
			{
				static u32 app_mic;
				app_mic	++;
				att_mic (conn_handle, pAtt->dat);
			}
#elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_HID_DONGLE_TO_STB)
			else if(attHandle == AUDIO_FIRST_REPORT || attHandle == (AUDIO_FIRST_REPORT + 4) || attHandle == (AUDIO_FIRST_REPORT + 8))
			{
				push_mic_packet(pAtt->dat);
			}
#elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_HID)
			else if(attHandle == AUDIO_FIRST_REPORT || attHandle == (AUDIO_FIRST_REPORT + 4) || attHandle == (AUDIO_FIRST_REPORT + 8))//HIDdongle
			{
				att_mic_rcvd = 1;

				static u32 app_mic;
				app_mic	++;

				mic_cnt++;

				if(mic_cnt <=6 ){
					memcpy(tmp_mic_data+(mic_cnt-1)*20, pAtt->dat, 20);
				}

				if(mic_cnt == 6){
					mic_cnt = 0;
					att_mic (conn_handle, tmp_mic_data);
				}
			}
#elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_SBC_HID_DONGLE_TO_STB)
			else if(attHandle == AUDIO_FIRST_REPORT || attHandle == (AUDIO_FIRST_REPORT + 4) || attHandle == (AUDIO_FIRST_REPORT + 8))
			{
				push_mic_packet(pAtt->dat);
			}
#elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_SBC_HID)
			else if(attHandle == AUDIO_FIRST_REPORT || attHandle == (AUDIO_FIRST_REPORT + 4) || attHandle == (AUDIO_FIRST_REPORT + 8))
			{
				static u32 app_mic;
				app_mic	++;
				att_mic (conn_handle, pAtt->dat);
			}
#elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_MSBC_HID)
			else if(attHandle == AUDIO_FIRST_REPORT || attHandle == (AUDIO_FIRST_REPORT + 4) || attHandle == (AUDIO_FIRST_REPORT + 8))
			{
				static u32 app_mic;
				app_mic	++;
				att_mic (conn_handle, pAtt->dat);
			}
#elif (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE)
			else if(attHandle == GOOGLE_AUDIO_HANDLE_MIC_DATA)
			{
				app_audio_data(pAtt->dat,pAtt->l2capLen-3);
			}
			else if(attHandle == GOOGLE_AUDIO_HANDLE_MIC_RSP)
			{
				extern u8 google_audio_start;
				//if(pAtt->dat[0] == (google_voice_model ? 0x04 : 0x08))
				if(pAtt->dat[0] == 0x08) //google voice 0.4
				{
					#if (MIC_SAMPLE_RATE == 16000)
					u8 data[10] = {0x0C, 0x00, 0x02, 0x00, 0x03};//16000
					#else
					u8 data[10] = {0x0C, 0x00, 0x01, 0x00, 0x01};//8000
					#endif

					if(BLE_SUCCESS == blc_gatt_pushWriteRequest(BLM_CONN_HANDLE,GOOGLE_AUDIO_HANDLE_MIC_CMD,data,5)){

					}

					//printf("audio start");
					google_audio_start = true;//app_audio_start();

				}
				else if(pAtt->dat[0] == 0x00)
				{
					google_audio_start = false;//app_audio_stop();
				}
				else if(pAtt->dat[0] == 0x0A)
				{
					//printf("AUDIO_HANDLE_MIC_RSP: sync\r\n");
				}
				else if(pAtt->dat[0] == 0x0B)
				{
					//get caps rsp
					google_voice_model = pAtt->dat[4];

				}
			}
#endif
			else
			{

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

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
#include "blm_host.h"
#include "application/audio/tl_audio.h"
#include "application/audio/audio_config.h"
#include "application/keyboard/keyboard.h"
#include "application/usbstd/usbkeycode.h"

#if (FEATURE_TEST_MODE == TEST_LL_PRIVACY_MASTER)

#define		MY_RF_POWER_INDEX					RF_POWER_P3dBm

MYFIFO_INIT(blt_rxfifo, 64, 16);
MYFIFO_INIT(blt_txfifo, 40, 8);

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
			if(key0 == KEY_PAIR)
			{
				central_pairing_enable = 1;

			}
			else if(key0 == KEY_UNPAIR)
			{
				central_unpair_enable = 1;

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
		case GAP_EVT_SMP_PAIRING_SUCCESS:
		{
			gap_smp_pairingSuccessEvt_t* p = (gap_smp_pairingSuccessEvt_t*)para;

			if(p->bonding_result){

			}
			else{
				tlkapi_printf(APP_LOG_EN,"save smp key failed %x,handle %x\n",p->bonding_result,p->connHandle);
			}
		}
		break;
	}
	return 0;
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

	#if(UART_PRINT_DEBUG_ENABLE)
		tlkapi_debug_init();
		blc_debug_enableStackLog(STK_LOG_DISABLE);
	#endif

	blc_readFlashSize_autoConfigCustomFlashSector();

	/* attention that this function must be called after "blc_readFlashSize_autoConfigCustomFlashSector" !!!*/
	blc_app_loadCustomizedParameters_normal();






///////////// BLE stack Initialization ////////////////
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
	blc_hci_le_setEventMask_cmd(		HCI_LE_EVT_MASK_CONNECTION_COMPLETE 		\
									|	HCI_LE_EVT_MASK_ADVERTISING_REPORT 			\
									|   HCI_LE_EVT_MASK_CONNECTION_UPDATE_COMPLETE  \
									|	HCI_LE_EVT_MASK_PHY_UPDATE_COMPLETE			\
									|   HCI_LE_EVT_MASK_CONNECTION_ESTABLISH );         //connection establish: telink private event



	blm_smp_configPairingSecurityInfoStorageAddr(flash_sector_master_pairing);
	blm_smp_registerSmpFinishCb(app_host_smp_finish);

	blc_smp_central_init();

	//SMP trigger by master
	blm_host_smp_setSecurityTrigger(MASTER_TRIGGER_SMP_FIRST_PAIRING | MASTER_TRIGGER_SMP_AUTO_CONNECT);

	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PAIRING_SUCCESS );
	blc_gap_registerHostEventHandler( app_host_event_callback );


	blc_ll_initPrivacyLocalRpa(); //must call this API if user need local RPA for privacy

	app_configScanParameter();

	/* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done.
	 * attention that code will stuck in "while(1)" if any error detected in initialization, user need find what error happens and then fix it */
	blc_app_checkControllerHostInitialization();

	tlkapi_printf(APP_LOG_EN, "[APP][INI] feature_privacy_master init \n");
}


/**
 * @brief		host pair or upair proc in main loop
 * @param[in]	none
 * @return      none
 */
void host_pair_unpair_proc(void)
{

	//terminate and unpair proc
	static int master_disconnect_flag;
	if(central_unpair_enable){
		if(!master_disconnect_flag && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){
			if( blm_ll_disconnect(cur_conn_device.conn_handle, HCI_ERR_REMOTE_USER_TERM_CONN) == BLE_SUCCESS){
				master_disconnect_flag = 1;
				central_unpair_enable = 0;

				tbl_bond_slave_unpair_proc(cur_conn_device.mac_adrType, cur_conn_device.mac_addr); //by telink stack host smp

			}
		}
	}
	if(master_disconnect_flag && blc_ll_getCurrentState() != BLS_LINK_STATE_CONN){
		master_disconnect_flag = 0;
	}
}

int main_idle_loop (void)
{

	////////////////////////////////////// BLE entry /////////////////////////////////
	blt_sdk_main_loop();


#if (UI_KEYBOARD_ENABLE)
	proc_keyboard(0, 0, 0);
#endif
	host_pair_unpair_proc();

#if 1
	//proc master update
	if(host_update_conn_param_req){
		host_update_conn_proc();
	}
#endif

	return 0;
}


/**
 * @brief     BLE main loop
 * @param[in]  none.
 * @return     none.
 */
void main_loop(void)
{
	main_idle_loop();
}


#endif



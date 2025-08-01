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
#include "blm_att.h"
#include "blm_host.h"
#include "blm_ota.h"
#include "application/audio/tl_audio.h"
#include "application/audio/audio_config.h"
#include "application/usbstd/usb.h"
#include "app_audio.h"


#if (TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE) && (GOOGLE_VOICE_OVER_BLE_SPCE_VERSION == GOOGLE_VERSION_1_0)
MYFIFO_INIT(blt_rxfifo, 256, 8);
#else
MYFIFO_INIT(blt_rxfifo, 64, 16);
#endif
MYFIFO_INIT(blt_txfifo, 40, 8);







#if (APP_FLASH_PROTECTION_ENABLE)

/**
 * @brief      flash protection operation, including all locking & unlocking for application
 * 			   handle all flash write & erase action for this demo code. use should add more more if they have more flash operation.
 * @param[in]  flash_op_evt - flash operation event, including application layer action and stack layer action event(OTA write & erase)
 * 			   attention 1: if you have more flash write or erase action, you should should add more type and process them
 * 			   attention 2: for "end" event, no need to pay attention on op_addr_begin & op_addr_end, we set them to 0 for
 * 			   			    stack event, such as stack OTA write new firmware end event
 * @param[in]  op_addr_begin - operating flash address range begin value
 * @param[in]  op_addr_end - operating flash address range end value
 * 			   attention that, we use: [op_addr_begin, op_addr_end)
 * 			   e.g. if we write flash sector from 0x10000 to 0x20000, actual operating flash address is 0x10000 ~ 0x1FFFF
 * 			   		but we use [0x10000, 0x20000):  op_addr_begin = 0x10000, op_addr_end = 0x20000
 * @return     none
 */
_attribute_data_retention_ u16  flash_lockBlock_cmd = 0;
void app_flash_protection_operation(u8 flash_op_evt, u32 op_addr_begin, u32 op_addr_end)
{
	if(flash_op_evt == FLASH_OP_EVT_APP_INITIALIZATION)
	{
		/* ignore "op addr_begin" and "op addr_end" for initialization event
		 * must call "flash protection_init" first, will choose correct flash protection relative API according to current internal flash type in MCU */
		flash_protection_init();

		/* just sample code here, protect all flash area for firmware, do not protect system data area.
		 * user can change this design if have other consideration */
		u32  app_lockBlock = app_lockBlock = FLASH_LOCK_FW_LOW_256K; //just demo value, user can change this value according to application

		flash_lockBlock_cmd = flash_change_app_lock_block_to_flash_lock_block(app_lockBlock);

		if(blc_flashProt.init_err){
			tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash protection initialization error!!!\n");
		}

		tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] initialization, lock flash\n");
		flash_lock(flash_lockBlock_cmd);
	}
	/* add more flash protection operation for your application if needed */
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
	u8 event = h & 0xFF;
	switch(event)
	{
		case GAP_EVT_SMP_PAIRING_FAIL:
		{
			central_smp_pending=0;
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
	random_generator_init();  //this is must

	//	debug init
	#if(UART_PRINT_DEBUG_ENABLE || USB_PRINT_DEBUG_ENABLE)
		tlkapi_debug_init();
		blc_debug_enableStackLog(STK_LOG_DISABLE);
	#endif

	blc_readFlashSize_autoConfigCustomFlashSector();

	/* attention that this function must be called after "blc_readFlashSize_autoConfigCustomFlashSector" !!!*/
	blc_app_loadCustomizedParameters_normal();


	#if (APP_FLASH_PROTECTION_ENABLE)
		app_flash_protection_operation(FLASH_OP_EVT_APP_INITIALIZATION, 0, 0);
		blc_appRegisterStackFlashOperationCallback(app_flash_protection_operation); //register flash operation callback for stack
	#endif



	//set USB ID
	REG_ADDR8(0x74) = 0x62;
	REG_ADDR16(0x7e) = 0x08d0;
	REG_ADDR8(0x74) = 0x00;

	//////////////// config USB ISO IN/OUT interrupt /////////////////
	reg_usb_mask = BIT(7);			//audio in interrupt enable
	reg_irq_mask |= FLD_IRQ_IRQ4_EN;
	reg_usb_ep6_buf_addr = 0x80;
	reg_usb_ep7_buf_addr = 0x60;
	reg_usb_ep_max_size = (256 >> 3);

	usb_dp_pullup_en (1);  //open USB enum


	///////////////// SDM /////////////////////////////////
#if (AUDIO_SDM_ENABLE)
	u16 sdm_step = config_sdm  (buffer_sdm, TL_SDM_BUFFER_SIZE, 16000, 4);
#endif


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


//	blc_ll_init2MPhyCodedPhy_feature();
//	blc_ll_setDefaultConnCodingIndication(CODED_PHY_PREFER_S2);
//	blc_ll_initChannelSelectionAlgorithm_2_feature();

	rf_set_power_level_index (RF_POWER_P3dBm);



	////// Host Initialization  //////////
	blc_gap_central_init();										//gap initialization
	blc_gap_registerHostEventHandler (app_host_event_callback);
	blc_gap_setEventMask(GAP_EVT_MASK_SMP_PARING_FAIL);
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



	#if (BLE_HOST_SMP_ENABLE)
		blm_smp_configPairingSecurityInfoStorageAddr(flash_sector_master_pairing);
		blm_smp_registerSmpFinishCb(app_host_smp_finish);

		blc_smp_central_init();

		//SMP trigger by master
		blm_host_smp_setSecurityTrigger(MASTER_TRIGGER_SMP_FIRST_PAIRING | MASTER_TRIGGER_SMP_AUTO_CONNECT);
	#else
		blc_smp_setSecurityLevel(No_Security);

		#if (ACL_CENTRAL_CUSTOM_PAIR_ENABLE)
			user_master_host_pairing_management_init();
		#endif
	#endif

	#if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
		host_att_register_idle_func (main_idle_loop);

		#if(TL_AUDIO_MODE == TL_AUDIO_DONGLE_ADPCM_GATT_GOOGLE)
			app_sdp_register_get_att_handle_callback(&app_google_voice_service_discovery);
		#endif
	#endif


	/* set scan parameter and scan enable */
	blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS,	\
							OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
	blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);

	/* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done.
	 * attention that code will stuck in "while(1)" if any error detected in initialization, user need find what error happens and then fix it */
	blc_app_checkControllerHostInitialization();

	tlkapi_printf(APP_LOG_EN, "[APP][INI] BLE kma dongle init \n");
}



/**
 * @brief		host pair or upair proc in main loop
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_
void host_pair_unpair_proc(void)
{

	#if (ACL_CENTRAL_CUSTOM_PAIR_ENABLE)
		if(blm_manPair.manual_pair && clock_time_exceed(blm_manPair.pair_tick, 2000000)){  //@@
			blm_manPair.manual_pair = 0;
		}
	#endif


	//terminate and unpair proc
	static int master_disconnect_flag;
	if(central_unpair_enable){
		if(!master_disconnect_flag && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){
			if( blm_ll_disconnect(cur_conn_device.conn_handle, HCI_ERR_REMOTE_USER_TERM_CONN) == BLE_SUCCESS){
				master_disconnect_flag = 1;
				central_unpair_enable = 0;

				#if (BLE_HOST_SMP_ENABLE)
					tbl_bond_slave_unpair_proc(cur_conn_device.mac_adrType, cur_conn_device.mac_addr); //by telink stack host smp
				#elif (ACL_CENTRAL_CUSTOM_PAIR_ENABLE)
					user_tbl_slave_mac_delete_by_adr(cur_conn_device.mac_adrType, cur_conn_device.mac_addr);
				#endif
			}
		}
	}
	if(master_disconnect_flag && blc_ll_getCurrentState() != BLS_LINK_STATE_CONN){
		master_disconnect_flag = 0;
	}
}



/**
 * @brief     BLE main idle loop
 * @param[in]  none.
 * @return     none.
 */
int main_idle_loop (void)
{


	////////////////////////////////////// BLE entry /////////////////////////////////
	blt_sdk_main_loop();


	///////////////////////////////////// proc usb cmd from host /////////////////////
	usb_handle_irq();


	////////////////////////////////////// UI entry /////////////////////////////////
#if (UI_BUTTON_ENABLE)
	static u8 button_detect_en = 0;
	if(!button_detect_en && clock_time_exceed(0, 1000000)){// proc button 1 second later after power on
		button_detect_en = 1;
	}
	static u32 button_detect_tick = 0;
	if(button_detect_en && clock_time_exceed(button_detect_tick, 5000))
	{
		button_detect_tick = clock_time();
		proc_button();  //button triggers pair & unpair  and OTA
	}
#endif


	////////////////////////////////////// proc audio ////////////////////////////////
#if (UI_AUDIO_ENABLE)
	proc_audio();

	static u32 tick_bo;
	if (REG_ADDR8(0x125) & BIT(0))
	{
		tick_bo = clock_time ();
	}
	else if (clock_time_exceed (tick_bo, 200000))
	{
		REG_ADDR8(0x125) = BIT(0);
	}
#endif


	host_pair_unpair_proc();


#if(BLE_MASTER_OTA_ENABLE)
	proc_ota();
#endif

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
	main_idle_loop ();

	#if (ACL_CENTRAL_SIMPLE_SDP_ENABLE)
		simple_sdp_loop();
	#endif
}






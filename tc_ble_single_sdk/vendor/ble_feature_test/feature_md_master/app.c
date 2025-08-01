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

#if (FEATURE_TEST_MODE == TEST_MD_MASTER)

#include "tl_common.h"
#include "drivers.h"
#include "stack/ble/ble.h"

#include "app.h"
#include "blm_host.h"
#include "app_ui.h"


MYFIFO_INIT(blt_rxfifo, 64, 16);

MYFIFO_INIT(blt_txfifo, 40, 8);


extern u32 device_connection_tick;


#define TEST_DATA_LEN		20
u8	app_test_data[TEST_DATA_LEN];
int write_data_test_tick;



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
	#if(UART_PRINT_DEBUG_ENABLE || USB_PRINT_DEBUG_ENABLE)
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


	rf_set_power_level_index (RF_POWER_P3dBm);


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
	blc_ll_setScanParameter(SCAN_TYPE_PASSIVE, SCAN_INTERVAL_100MS, SCAN_INTERVAL_100MS,	\
							OWN_ADDRESS_PUBLIC, SCAN_FP_ALLOW_ADV_ANY);
	blc_ll_setScanEnable (BLC_SCAN_ENABLE, DUP_FILTER_DISABLE);

	/* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done.
	 * attention that code will stuck in "while(1)" if any error detected in initialization, user need find what error happens and then fix it */
	blc_app_checkControllerHostInitialization();

	tlkapi_printf(APP_LOG_EN, "[APP][INI] BLE feature more data master init \n");
}



/**
 * @brief		host pair or upair proc in main loop
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_
void host_pair_unpair_proc(void)
{

	//terminate and unpair proc
	static int master_disconnect_flag;
	if(central_unpair_enable){
		if(!master_disconnect_flag && blc_ll_getCurrentState() == BLS_LINK_STATE_CONN){
			if( blm_ll_disconnect(cur_conn_device.conn_handle, HCI_ERR_REMOTE_USER_TERM_CONN) == BLE_SUCCESS){
				master_disconnect_flag = 1;
				central_unpair_enable = 0;

				#if (BLE_HOST_SMP_ENABLE)
					tbl_bond_slave_unpair_proc(cur_conn_device.mac_adrType, cur_conn_device.mac_addr); //by telink stack host smp
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

	////////////////////////////////////// UI entry /////////////////////////////////
	host_pair_unpair_proc();

	#if (UI_KEYBOARD_ENABLE)
		proc_keyboard(0, 0, 0);
	#endif

	if(device_connection_tick && clock_time_exceed(device_connection_tick, 2000000)){
		device_connection_tick = 0;
		write_data_test_tick = clock_time() | 1;
	}

#if (0) //not used, demo uses loopback test by slave ATT notify, master ATT write command
	if(write_data_test_tick && clock_time_exceed(write_data_test_tick, 15000)){
		write_data_test_tick = clock_time() | 1;
		if(BLE_SUCCESS == blc_gatt_pushWriteCommand(BLM_CONN_HANDLE,  SPP_HANDLE_DATA_C2S, app_test_data, TEST_DATA_LEN)){
			app_test_data[0] ++;
		}
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
}




#endif  //end of (FEATURE_TEST_MODE == ...)

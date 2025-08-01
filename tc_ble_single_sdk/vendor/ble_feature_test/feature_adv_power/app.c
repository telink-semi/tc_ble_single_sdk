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

#include "app_config.h"
#include "app.h"
#include "../default_att.h"

#if (FEATURE_TEST_MODE == TEST_POWER_ADV)

//TEST_POWER_ADV config start
#define CONNECT_12B_1S_1CHANNEL      	0
#define CONNECT_12B_1S_3CHANNEL      	1
#define CONNECT_12B_500MS_3CHANNEL   	2
#define CONNECT_12B_30MS_3CHANNEL    	3

#define UNCONNECTED_16B_1S_3CHANNEL       4
#define UNCONNECTED_16B_1_5S_3CHANNEL     5
#define UNCONNECTED_16B_2S_3CHANNEL   	6

#define UNCONNECTED_31B_1S_3CHANNEL      	7
#define UNCONNECTED_31B_1_5S_3CHANNEL     8
#define UNCONNECTED_31B_2S_3CHANNEL   	9

#define APP_ADV_POWER_TEST_TYPE        UNCONNECTED_31B_2S_3CHANNEL





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

#define		MY_RF_POWER_INDEX					RF_POWER_P0dBm






/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_SUSPEND_EXIT"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void task_suspend_exit (u8 e, u8 *p, int n)
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

	//	debug init
	#if(UART_PRINT_DEBUG_ENABLE)
		tlkapi_debug_init();
		blc_debug_enableStackLog(STK_LOG_DISABLE);
	#endif


	blc_readFlashSize_autoConfigCustomFlashSector();

	/* attention that this function must be called after "blc readFlashSize_autoConfigCustomFlashSector" !!!*/
	blc_app_loadCustomizedParameters_normal();



//////////////////////////// basic hardware Initialization  End //////////////////////////////////




//////////////////////////// BLE stack Initialization  Begin //////////////////////////////////
	//////////// Controller Initialization  Begin /////////////////////////
	u8  mac_public[6];
	u8  mac_random_static[6];
	//for 512K Flash, flash_sector_mac_address equals to 0x76000
	//for 1M  Flash, flash_sector_mac_address equals to 0xFF000
	blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);
	tlkapi_send_string_data(APP_LOG_EN,"[APP][INI]Public Address", mac_public, 6);


	blc_ll_initBasicMCU();                      //mandatory
	blc_ll_initStandby_module(mac_public);		//mandatory
	blc_ll_initAdvertising_module(mac_public); 	//legacy advertising module: mandatory for BLE slave


	//////////// Controller Initialization  End /////////////////////////
	//when debugging, if long time deepSleep retention or suspend happens quickly after power on, it will make "ResetMCU" very hard, so add some time here
	sleep_us(2000000);  //only for debug



	//////////// HCI Initialization  Begin /////////////////////////

	//////////// HCI Initialization  End   /////////////////////////


	//////////// Host Initialization  Begin /////////////////////////
	/******************************************************************************************************
	 * Here are just some ADV power example
	 * The actual measured power is affected by several ADV parameters, such as:
	 * 1. ADV data length: long ADV data means bigger power
	 *
	 * 2. ADV type:   non_connectable undirected: ADV power is small, cause only data sending involved, no
	 *                                           need receiving any packet from master
	 *                connectable ADV: must try to receive scan_req/scan_conn from master after sending adv
	 *                                           data, so power is bigger.
	 *                                               And if needing send scan_rsp to master's scan_req,
	 *                                           power will increase. Here we can use whiteList to disable scan_rsp.
	 *											     With connectable ADV, user should test power under a clean
	 *											 and shielded environment to avoid receiving scan_req/conn_req
	 *
	 * 3. ADV power index: We use 0dBm in examples, higher power index will cause poser to increase
	 *
	 * 4. ADV interval: Bigger ADV interval lead to smaller power, cause more timing for suspend/deepSleep retention
	 *
	 * 5. ADV channel: Power with 3 channel is bigger than power with 1 or 2 channel
	 *
	 *
	 * If you want test ADV power with different ADV parameters from our examples, you should modify these
	 *      parameters in code, and re_test by yourself.
	 *****************************************************************************************************/

		//set to special ADV channel can avoid master's scan_req to get a very clean power,
		// but remember that special channel ADV packet can not be scanned by BLE master and captured by BLE sniffer
	//	blc_ll_setAdvCustomizedChannel(33,34,35);
	u8 adv_param_status = BLE_SUCCESS;
	#if APP_ADV_POWER_TEST_TYPE < UNCONNECTED_16B_1S_3CHANNEL  // connectable undirected ADV
		/**
		 * @brief	Adv Packet data
		 */
		//ADV data length: 12 byte
		u8 tbl_advData[12] = {
			 0x08, DT_COMPLETE_LOCAL_NAME, 't', 'e', 's', 't', 'a', 'd', 'v',
			 0x02, DT_FLAGS, 0x05,
			};

		/**
		 * @brief	Scan Response Packet data
		 */
		u8	tbl_scanRsp [] = {
				 0x08, DT_COMPLETE_LOCAL_NAME, 'T', 'E', 'S', 'T', 'A', 'D', 'V',	//scan name
			};

		bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
		bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));

		#if APP_ADV_POWER_TEST_TYPE == CONNECT_12B_1S_1CHANNEL
			// ADV data length:	12 byte
			// ADV type: 		connectable undirected ADV
			// ADV power index: 3 dBm
			// ADV interval: 	1S
			// ADV channel: 	1 channel
			adv_param_status = bls_ll_setAdvParam( ADV_INTERVAL_1S, ADV_INTERVAL_1S,ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0,  NULL,  BLT_ENABLE_ADV_37, ADV_FP_ALLOW_SCAN_WL_ALLOW_CONN_WL);  //no scan, no connect

		#elif APP_ADV_POWER_TEST_TYPE == CONNECT_12B_1S_3CHANNEL
			// ADV data length:	12 byte
			// ADV type: 		connectable undirected ADV
			// ADV power index: 3 dBm
			// ADV interval: 	1S
			// ADV channel: 	3 channel
			adv_param_status = bls_ll_setAdvParam( ADV_INTERVAL_1S, ADV_INTERVAL_1S, \
											ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, \
											 0,  NULL,  BLT_ENABLE_ADV_ALL, ADV_FP_ALLOW_SCAN_WL_ALLOW_CONN_WL);  //no scan, no connect

		#elif APP_ADV_POWER_TEST_TYPE == CONNECT_12B_500MS_3CHANNEL
			// ADV data length:	12 byte
			// ADV type: 		connectable undirected ADV
			// ADV power index: 3 dBm
			// ADV interval: 	500 mS
			// ADV channel: 	3 channel
			adv_param_status = bls_ll_setAdvParam( ADV_INTERVAL_500MS, ADV_INTERVAL_500MS, ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0,  NULL,  BLT_ENABLE_ADV_ALL, ADV_FP_ALLOW_SCAN_WL_ALLOW_CONN_WL);  //no scan, no connect

		#elif APP_ADV_POWER_TEST_TYPE == CONNECT_12B_30MS_3CHANNEL
			// ADV data length:	12 byte
			// ADV type: 		connectable undirected ADV
			// ADV power index: 3 dBm
			// ADV interval: 	30 mS
			// ADV channel: 	3 channel
			adv_param_status = bls_ll_setAdvParam( ADV_INTERVAL_30MS, ADV_INTERVAL_30MS, ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0,  NULL,  BLT_ENABLE_ADV_ALL, ADV_FP_ALLOW_SCAN_WL_ALLOW_CONN_WL);  //no scan, no connect
		#endif

	#else  // non_connectable undirected ADV, no need scanRsp

		/**
		 * @brief	Adv Packet data
		 */
		#if APP_ADV_POWER_TEST_TYPE < UNCONNECTED_31B_1S_3CHANNEL
			//ADV data length: 16 byte
			u8 tbl_advData[] = {
				 0x0F, DT_COMPLETE_LOCAL_NAME, 't', 'e', 's', 't', 'a', 'd', 'v', '8', '9', 'A', 'B', 'C', 'D', 'E',
				};
		#else  	//ADV data length: max 31 byte
			u8 tbl_advData[] = {
				 0x1E, DT_COMPLETE_LOCAL_NAME, 't', 'e', 's', 't', 'a', 'd', 'v', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D'
			};
		#endif


		bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );

		#if APP_ADV_POWER_TEST_TYPE == UNCONNECTED_16B_1S_3CHANNEL || APP_ADV_POWER_TEST_TYPE == UNCONNECTED_31B_1S_3CHANNEL
			// ADV type: non_connectable undirected ADV
			// ADV power index: 3 dBm
			// ADV interval: 1S
			// ADV channel: 3 channel
			adv_param_status = bls_ll_setAdvParam( ADV_INTERVAL_1S, ADV_INTERVAL_1S, ADV_TYPE_NONCONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0,  NULL,  BLT_ENABLE_ADV_ALL, ADV_FP_NONE);

		#elif APP_ADV_POWER_TEST_TYPE == UNCONNECTED_16B_1_5S_3CHANNEL || APP_ADV_POWER_TEST_TYPE == UNCONNECTED_31B_1_5S_3CHANNEL
			// ADV type: non_connectable undirected ADV
			// ADV power index: 3 dBm
			// ADV interval: 1.5S
			// ADV channel: 3 channel
			adv_param_status = bls_ll_setAdvParam( ADV_INTERVAL_1S5, ADV_INTERVAL_1S5, ADV_TYPE_NONCONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0,  NULL,  BLT_ENABLE_ADV_ALL, ADV_FP_NONE);

		#elif APP_ADV_POWER_TEST_TYPE == UNCONNECTED_16B_2S_3CHANNEL || APP_ADV_POWER_TEST_TYPE == UNCONNECTED_31B_2S_3CHANNEL
			// ADV type: non_connectable undirected ADV
			// ADV power index: 3 dBm
			// ADV interval: 2S
			// ADV channel: 3 channel
			adv_param_status = bls_ll_setAdvParam( ADV_INTERVAL_2S, ADV_INTERVAL_2S, ADV_TYPE_NONCONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC, 0,  NULL,  BLT_ENABLE_ADV_ALL, ADV_FP_NONE);
		#endif
	#endif


	if(adv_param_status != BLE_SUCCESS) {
		tlkapi_printf(APP_LOG_EN, "[APP][INI] ADV parameters error 0x%x!!!\n", adv_param_status);
		while(1);
	}

	bls_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable

	/* set RF power index, user must set it after every suspend wake_up, because relative setting will be reset in suspend */
	rf_set_power_level_index (MY_RF_POWER_INDEX);



#if(BLE_APP_PM_ENABLE)
	blc_ll_initPowerManagement_module();

	#if (PM_DEEPSLEEP_RETENTION_ENABLE)
    	blc_app_setDeepsleepRetentionSramSize(); //select DEEPSLEEP_MODE_RET_SRAM_LOW16K or DEEPSLEEP_MODE_RET_SRAM_LOW32K
		bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
		blc_pm_setDeepsleepRetentionThreshold(50, 50);
		blc_pm_setDeepsleepRetentionEarlyWakeupTiming(200);
	#else
		bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
	#endif

	bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_EXIT, &task_suspend_exit);
#else
	bls_pm_setSuspendMask (SUSPEND_DISABLE);
#endif

	/* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done.
	 * attention that code will stuck in "while(1)" if any error detected in initialization, user need find what error happens and then fix it */
	blc_app_checkControllerHostInitialization();

	tlkapi_printf(APP_LOG_EN, "[APP][INI] feature_adv_power init \n");

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

	DBG_CHN0_HIGH;    //debug
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


	////////////////////////////////////// PM Process /////////////////////////////////
	blt_pm_proc();



}


#endif  //end of (FEATURE_TEST_MODE == ...)

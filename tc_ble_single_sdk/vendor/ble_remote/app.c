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

#include "application/keyboard/keyboard.h"

#include "app.h"
#include "app_att.h"
#include "app_ui.h"
#include "battery_check.h"
#include "rc_ir.h"
#include "rc_ir_learn.h"
#include "app_audio.h"
#include "application/audio/gl_audio.h"



#define 	ADV_IDLE_ENTER_DEEP_TIME			60  //60 s
#define 	CONN_IDLE_ENTER_DEEP_TIME			60  //60 s

#define 	MY_DIRECT_ADV_TIME					2000000


#define     MY_APP_ADV_CHANNEL					BLT_ENABLE_ADV_ALL
#define 	MY_ADV_INTERVAL_MIN					ADV_INTERVAL_30MS
#define 	MY_ADV_INTERVAL_MAX					ADV_INTERVAL_35MS

#define		MY_RF_POWER_INDEX					RF_POWER_P3dBm

#define		BLE_DEVICE_ADDRESS_TYPE 			BLE_DEVICE_ADDRESS_PUBLIC

_attribute_data_retention_	own_addr_type_t 	app_own_address_type = OWN_ADDRESS_PUBLIC;





/* CAL_LL_ACL_RX_BUF_SIZE(maxRxOct): maxRxOct + 22, then 16 byte align */
#define RX_FIFO_SIZE	64
/* must be: 2^n, (power of 2);at least 4; recommended value: 4, 8, 16 */
#define RX_FIFO_NUM		8


/* CAL_LL_ACL_TX_BUF_SIZE(maxTxOct):  maxTxOct + 10, then 4 byte align */
#define TX_FIFO_SIZE	40
/* must be: (2^n), (power of 2); at least 8; recommended value: 8, 16, 32, other value not allowed. */
#define TX_FIFO_NUM		16


#if 0
	MYFIFO_INIT(blt_rxfifo, RX_FIFO_SIZE, RX_FIFO_NUM);
#else
_attribute_data_retention_  u8 		 	blt_rxfifo_b[RX_FIFO_SIZE * RX_FIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	blt_rxfifo = {
												RX_FIFO_SIZE,
												RX_FIFO_NUM,
												0,
												0,
												blt_rxfifo_b,};
#endif


#if 0
	MYFIFO_INIT(blt_txfifo, TX_FIFO_SIZE, TX_FIFO_NUM);
#else
	_attribute_data_retention_  u8 		 	blt_txfifo_b[TX_FIFO_SIZE * TX_FIFO_NUM] = {0};
	_attribute_data_retention_	my_fifo_t	blt_txfifo = {
													TX_FIFO_SIZE,
													TX_FIFO_NUM,
													0,
													0,
													blt_txfifo_b,};
#endif




/**
 * @brief	Adv Packet data
 */
const u8	tbl_advData[] = {
	 0x05, 0x09, 'v', 'h', 'i', 'd',
	 0x02, 0x01, 0x05, 							// BLE limited discoverable mode and BR/EDR not supported
	 0x03, 0x19, 0x80, 0x01, 					// 384, Generic Remote Control, Generic category
	 0x05, 0x02, 0x12, 0x18, 0x0F, 0x18,		// incomplete list of service class UUIDs (0x1812, 0x180F)
};
/**
 * @brief	Scan Response Packet data
 */
const u8	tbl_scanRsp [] = {
		 0x08, 0x09, 'V', 'R', 'e', 'm', 'o', 't', 'e',
	};

#if(REMOTE_IR_LEARN_ENABLE)
	extern ir_learn_ctrl_t *g_ir_learn_ctrl;
#endif

_attribute_data_retention_	int device_in_connection_state;

_attribute_data_retention_	u32 advertise_begin_tick;

_attribute_data_retention_	u32	interval_update_tick;

_attribute_data_retention_	u8	sendTerminate_before_enterDeep = 0;

_attribute_data_retention_	u32	latest_user_event_tick;

_attribute_data_retention_	u32	lowBattDet_tick   = 0;




/**
 * @brief      callback function of LinkLayer Event "BLT_EV_FLAG_ADV_DURATION_TIMEOUT"
 * @param[in]  e - LinkLayer Event type
 * @param[in]  p - data pointer of event
 * @param[in]  n - data length of event
 * @return     none
 */
void 	app_switch_to_undirected_adv(u8 e, u8 *p, int n)
{
    (void)e;(void)p;(void)n;
	bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
						ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
						0,  NULL,
						MY_APP_ADV_CHANNEL,
						ADV_FP_NONE);

	bls_ll_setAdvEnable(BLC_ADV_ENABLE);  //must: set ADV enable
}



void 	ble_remote_terminate(u8 e,u8 *p, int n) //*p is terminate reason
{
    (void)e;(void)p;(void)n;
	tlkapi_printf(APP_CONTR_EVENT_LOG_EN, "[APP][EVT] remote terminate, reason 0x%x\n", *p);
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



#if (BLE_APP_PM_ENABLE)
	 //user has push terminate pkt to ble TX buffer before deepsleep
	if(sendTerminate_before_enterDeep == 1){
		sendTerminate_before_enterDeep = 2;
	}
#endif


#if (BLE_AUDIO_ENABLE)
	if(ui_mic_enable){
		ui_enable_mic (0);
	}
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
void task_suspend_exit (u8 e, u8 *p, int n)
{
    (void)e;(void)p;(void)n;
	rf_set_power_level_index (MY_RF_POWER_INDEX);
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
	bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_10MS, CONN_INTERVAL_10MS, 99, CONN_TIMEOUT_4S);
	bls_l2cap_setMinimalUpdateReqSendingTime_after_connCreate(1000);


	latest_user_event_tick = clock_time();
#if(BLE_AUDIO_ENABLE)
	ui_mtu_size_exchange_req = 1;
#endif
	device_in_connection_state = 1;//

	interval_update_tick = clock_time() | 1; //none zero
}


void	task_conn_update_req (u8 e, u8 *p, int n)
{
    (void)e;(void)p;(void)n;
}

void	task_conn_update_done (u8 e, u8 *p, int n)
{
    (void)e;(void)p;(void)n;
}


int app_conn_param_update_response(u8 id, u16  result)
{
    (void)id;(void)result;
	if(result == CONN_PARAM_UPDATE_ACCEPT){

	}
	else if(result == CONN_PARAM_UPDATE_REJECT){

	}

	return 0;
}

#if (AUDIO_TRANS_USE_2M_PHY_ENABLE)
int app_host_event_callback (u32 h, u8 *para, int n)
{
	u8 event = h & 0xFF;
	if(event==GAP_EVT_ATT_EXCHANGE_MTU)
	{
		blc_ll_setPhy(BLS_CONN_HANDLE, PHY_TRX_PREFER, PHY_PREFER_2M, 	 PHY_PREFER_2M,    CODED_PHY_PREFER_NONE);
	}
	return 0;
}
void 	app_phy_update_complete_event(u8 e,u8 *p, int n)
{
}
#endif

#if (STUCK_KEY_PROCESS_ENABLE)
	extern u32 	stuckKey_keyPressTime;
#endif


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

    int user_task_flg = ota_is_working || scan_pin_need || key_not_released || DEVICE_LED_BUSY;

    if(user_task_flg){

        bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);

        #if (LONG_PRESS_KEY_POWER_OPTIMIZE)
            extern int key_matrix_same_as_last_cnt;
            if(!ota_is_working && key_matrix_same_as_last_cnt > 5){  //key matrix stable can optize
                bls_pm_setManualLatency(3);
            }
            else{
                bls_pm_setManualLatency(0);  //latency off: 0
            }
        #else
            bls_pm_setManualLatency(0);
        #endif
    }

    //If press any key during 1 minute, RCU enter sleep mode.
    #if (STUCK_KEY_PROCESS_ENABLE)
        if (key_not_released && clock_time_exceed(stuckKey_keyPressTime, STUCK_KEY_ENTERDEEP_TIME * 1000000))
        {
            u32 pin[] = KB_DRIVE_PINS;
            for (u8 i = 0; i < ARRAY_SIZE(pin); i ++){
                extern u8 stuckKeyPress[];
                if(!stuckKeyPress[i])
                    continue;
                cpu_set_gpio_wakeup (pin[i], 0, 1);  // reverse stuck key pad wakeup level
            }
            bls_pm_setWakeupSource(PM_WAKEUP_PAD);  //gpio PAD wakeup deesleep

            if( blc_ll_getCurrentState() == BLS_LINK_STATE_CONN)
            {
                //DEBUG("Disable adv\n");
                bls_ll_setAdvEnable(BLC_ADV_DISABLE);   //disable adv
                advertise_begin_tick = 0;
                /*
                 * when RCU connected state, if press key during 1 minute, RCU transmit release code.
                 */

                if(key_type == CONSUMER_KEY)
                {
                    //DEBUG("RCU transmit consumer release code.\n");
                    u16 consumer_key = 0;
                    blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
                }
                else if(key_type == KEYBOARD_KEY)
                {
                    //DEBUG("RCU transmit keyboard release code.\n");
                    u8 key_buf[2] = {0};
                    blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE,HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8); //release
                }

                bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN); //push terminate cmd into ble TX buffer
                sendTerminate_before_enterDeep = 1;
            }
            else
            {
                sendTerminate_before_enterDeep = 2;
            }

        }
#endif //END of  STUCK_KEY_PROCESS_ENABLE

#if 1 //deepsleep
    if(sendTerminate_before_enterDeep == 1){ //sending Terminate and wait for ack before enter deepsleep
        if(user_task_flg){  //detect key Press again,  can not enter deep now
            sendTerminate_before_enterDeep = 0;
            bls_ll_setAdvEnable(BLC_ADV_ENABLE);   //enable ADV again
        }
    }
    else if(sendTerminate_before_enterDeep == 2){  //Terminate OK
        analog_write(USED_DEEP_ANA_REG, analog_read(USED_DEEP_ANA_REG) | CONN_DEEP_FLG);
        cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);  //deepsleep
    }


    if(  !blc_ll_isControllerEventPending() ){  //no controller event pending
        //ADV 60s, deepsleep
        if( blc_ll_getCurrentState() == BLS_LINK_STATE_ADV && !sendTerminate_before_enterDeep && \
            clock_time_exceed(advertise_begin_tick , ADV_IDLE_ENTER_DEEP_TIME * 1000000))
        {
            cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);  //deepsleep
        }
        //conn 60s no event(key/voice/led), enter deepsleep
        else if( device_in_connection_state && !user_task_flg && \
                clock_time_exceed(latest_user_event_tick, CONN_IDLE_ENTER_DEEP_TIME * 1000000) )
        {

            bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN); //push terminate cmd into ble TX buffer
            bls_ll_setAdvEnable(BLC_ADV_DISABLE);   //disable adv
            sendTerminate_before_enterDeep = 1;
        }
    }

#endif

#if(BLE_AUDIO_ENABLE)
    if(ui_mic_enable)
#else
    if(0)
#endif
    {
        bls_pm_setSuspendMask (SUSPEND_DISABLE);
    }
#if(REMOTE_IR_ENABLE)
    else if( ir_send_ctrl.is_sending){
        bls_pm_setSuspendMask(SUSPEND_DISABLE);
        //If press any key during 1 minute, RCU enter sleep mode.
        #if (STUCK_KEY_PROCESS_ENABLE )
            if (key_not_released && clock_time_exceed(stuckKey_keyPressTime, STUCK_KEY_ENTERDEEP_TIME * 1000000))
            {
                u32 pin[] = KB_DRIVE_PINS;
                for (u8 i = 0; i < ARRAY_SIZE(pin); i ++){
                    extern u8 stuckKeyPress[];
                    if(!stuckKeyPress[i])
                        continue;
                    cpu_set_gpio_wakeup (pin[i], 0, 1);  // reverse stuck key pad wakeup level
                }

                bls_pm_setWakeupSource(PM_WAKEUP_PAD);  //gpio PAD wakeup deesleep

                cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);  //deepsleep
            }
        #endif// END of  STUCK_KEY_PROCESS_ENABLE
    }
#endif

#if(REMOTE_IR_LEARN_ENABLE)
    if( get_ir_learn_state() == 1){
        bls_pm_setSuspendMask(SUSPEND_DISABLE);
    }
#endif

#if (BLE_PHYTEST_MODE != PHYTEST_MODE_DISABLE)
    else if( blc_phy_isPhyTestEnable() )
    {
        bls_pm_setSuspendMask(SUSPEND_DISABLE);  //phy test can not enter suspend
    }
#endif


#endif  //END of  BLE_APP_PM_ENABLE
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

#if (APP_BATT_CHECK_ENABLE)

/**
 * @brief		this function is used to process battery power.
 * 				The low voltage protection threshold 2.0V is an example and reference value. Customers should
 * 				evaluate and modify these thresholds according to the actual situation. If users have unreasonable designs
 * 				in the hardware circuit, which leads to a decrease in the stability of the power supply network, the
 * 				safety thresholds must be increased as appropriate.
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void user_battery_power_check(u16 alarm_vol_mv)
{
	/*For battery-powered products, as the battery power will gradually drop, when the voltage is low to a certain
	  value, it will cause many problems.
		a) When the voltage is lower than operating voltage range of chip, chip can no longer guarantee stable operation.
		b) When the battery voltage is low, due to the unstable power supply, the write and erase operations
			of Flash may have the risk of error, causing the program firmware and user data to be modified abnormally,
			and eventually causing the product to fail. */
	u8 battery_check_returnValue = 0;
	if(analog_read(USED_DEEP_ANA_REG) & LOW_BATT_FLG){
		battery_check_returnValue = app_battery_power_check(alarm_vol_mv + 200);
	}
	else{
		battery_check_returnValue = app_battery_power_check(alarm_vol_mv);
	}

	if (battery_check_returnValue)
	{
		analog_write(USED_DEEP_ANA_REG,  analog_read(USED_DEEP_ANA_REG) & (~LOW_BATT_FLG));  //clr
	}
	else
	{
		#if (UI_LED_ENABLE)  //led indicate
			gpio_set_output_en(GPIO_LED, 1);  //output enable
			for(int k = 0; k < 3; k++){
				gpio_write(GPIO_LED, LED_ON_LEVEL);
				sleep_us(200000);
				gpio_write(GPIO_LED, !LED_ON_LEVEL);
				sleep_us(200000);
			}
		#endif

		if(analog_read(USED_DEEP_ANA_REG) & LOW_BATT_FLG){
			tlkapi_printf(APP_BATT_CHECK_LOG_EN, "[APP][BAT] The battery voltage is lower than %dmV, shut down!!!\n", (alarm_vol_mv + 200));
		} else {
			tlkapi_printf(APP_BATT_CHECK_LOG_EN, "[APP][BAT] The battery voltage is lower than %dmV, shut down!!!\n", alarm_vol_mv);
		}

		analog_write(USED_DEEP_ANA_REG,  analog_read(USED_DEEP_ANA_REG) | LOW_BATT_FLG);  //mark

		u32 pin[] = KB_DRIVE_PINS;
		for (unsigned int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
		{
			cpu_set_gpio_wakeup (pin[i], Level_High, 1);  //drive pin pad high wakeup deepsleep
		}
		cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);  //deepsleep
	}
}
#endif

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

#if(UART_PRINT_DEBUG_ENABLE)
	tlkapi_debug_init();
	blc_debug_enableStackLog(STK_LOG_DISABLE);
#endif


	blc_readFlashSize_autoConfigCustomFlashSector();

	/* attention that this function must be called after "blc readFlashSize_autoConfigCustomFlashSector" !!!*/
	blc_app_loadCustomizedParameters_normal();


	/* attention that this function must be called after "blc_app_loadCustomizedParameters_normal" !!!
	   The reason is that the low battery check need the ADC calibration parameter, and this parameter
	   is loaded in blc_app_loadCustomizedParameters_normal.
	 */
	#if (APP_BATT_CHECK_ENABLE)
	/*The SDK must do a quick low battery detect during user initialization instead of waiting
	  until the main_loop. The reason for this process is to avoid application errors that the device
	  has already working at low power.
	  Considering the working voltage of MCU and the working voltage of flash, if the Demo is set below 2.0V,
	  the chip will alarm and deep sleep (Due to PM does not work in the current version of B92, it does not go
	  into deepsleep), and once the chip is detected to be lower than 2.0V, it needs to wait until the voltage rises to 2.2V,
	  the chip will resume normal operation. Consider the following points in this design:
		At 2.0V, when other modules are operated, the voltage may be pulled down and the flash will not
		work normally. Therefore, it is necessary to enter deepsleep below 2.0V to ensure that the chip no
		longer runs related modules;
		When there is a low voltage situation, need to restore to 2.2V in order to make other functions normal,
		this is to ensure that the power supply voltage is confirmed in the charge and has a certain amount of
		power, then start to restore the function can be safer.*/
		user_battery_power_check(VBAT_DEEP_THRES_MV);
	#endif


	#if (APP_FLASH_PROTECTION_ENABLE)
		app_flash_protection_operation(FLASH_OP_EVT_APP_INITIALIZATION, 0, 0);
		blc_appRegisterStackFlashOperationCallback(app_flash_protection_operation); //register flash operation callback for stack
	#endif

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

	#if(AUDIO_TRANS_USE_2M_PHY_ENABLE)
		blc_ll_init2MPhyCodedPhy_feature();			//enable 2M/coded PHY
	#endif


	////// Host Initialization  //////////
	blc_gap_peripheral_init();    //gap initialization
	blc_l2cap_register_handler (blc_l2cap_packet_receive);  	//l2cap initialization
	my_att_init(); //gatt initialization
	blc_att_setRxMtuSize(MTU_SIZE_SETTING); //set MTU size, default MTU is 23 if not call this API

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	#if (BLE_REMOTE_SECURITY_ENABLE)
		/* attention: If this API is used, must be called before "blc smp_peripheral_init" when initialization !!! */
		bls_smp_configPairingSecurityInfoStorageAddr(flash_sector_smp_storage);
		blc_smp_peripheral_init();

		//Hid device on android7.0/7.1 or later version
		// New pairing: send security_request immediately after connection complete
		// reConnect:  send security_request 1000mS after connection complete. If master start pairing or encryption before 1000mS timeout, slave do not send security_request.
		blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )
	#else
		blc_smp_setSecurityLevel(No_Security);
	#endif





///////////////////// USER application initialization ///////////////////
	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));




	////////////////// config ADV packet /////////////////////
	u8 adv_param_status = BLE_SUCCESS;
#if (BLE_REMOTE_SECURITY_ENABLE)
	u8 bond_number = blc_smp_param_getCurrentBondingDeviceNumber();  //get bonded device number
	smp_param_save_t  bondInfo;
	if(bond_number)   //at least 1 bonding device exist
	{
		bls_smp_param_loadByIndex( bond_number - 1, &bondInfo);  //get the latest bonding device (index: bond_number-1 )

	}

	if(bond_number)   //set direct ADV
	{
		/* set direct ADV
		 * bondInfo.peer_addr_type & bondInfo.peer_addr is the address in the air packet of "CONNECT_IND" PDU stored in Flash.
		 * if peer address is IDA(identity address), bondInfo.peer_addr is OK used here.
		 * if peer address is RPA(resolved private address), bondInfo.peer_addr is one RPA peer device has used, it has a correct relation
		 * with peer IRK, so it can match to peer device at any time even peer device changes it's RPA. */
		adv_param_status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
										ADV_TYPE_CONNECTABLE_DIRECTED_LOW_DUTY, app_own_address_type,
										bondInfo.peer_addr_type,  bondInfo.peer_addr,
										MY_APP_ADV_CHANNEL,
										ADV_FP_NONE);

		/* If IRK distributed by peer device is valid, peer device may use RPA(resolved private address) at any time,
		 * even if it used IDA(identity address) in first pairing phase.
		 * So here must add peer IRK to resolving list and enable address resolution, since local device should check if
		 * "CONNECT_IND" PDU is sent by the device directed to.
		 * attention: local RPA not used, so parameter "local_irk" set to NULL */
		if(blc_app_isIrkValid(bondInfo.peer_irk)){
			blc_ll_addDeviceToResolvingList(bondInfo.peer_id_adrType, bondInfo.peer_id_addr, bondInfo.peer_irk, NULL);
			blc_ll_setAddressResolutionEnable(1);
		}

		//it is recommended that direct ADV only last for several seconds, then switch to undirected adv
		bls_ll_setAdvDuration(MY_DIRECT_ADV_TIME, 1);
		bls_app_registerEventCallback (BLT_EV_FLAG_ADV_DURATION_TIMEOUT, &app_switch_to_undirected_adv);

	}
	else   //set indirect adv
#endif
	{
		adv_param_status = bls_ll_setAdvParam(  MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
										 ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
										 0,  NULL,
										 MY_APP_ADV_CHANNEL,
										 ADV_FP_NONE);
		if(adv_param_status != BLE_SUCCESS) { //debug: ADV setting err
			tlkapi_printf(APP_LOG_EN, "[APP][INI] ADV parameters error 0x%x!!!\n", adv_param_status);
			while(1);
		}
	}

	bls_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable


	//set rf power index, user must set it after every suspend wakeup, cause relative setting will be reset in suspend
	rf_set_power_level_index (MY_RF_POWER_INDEX);



	//ble event call back
	bls_app_registerEventCallback (BLT_EV_FLAG_CONNECT, &task_connect);
	bls_app_registerEventCallback (BLT_EV_FLAG_TERMINATE, &ble_remote_terminate);


	bls_app_registerEventCallback (BLT_EV_FLAG_CONN_PARA_REQ, &task_conn_update_req);
	bls_app_registerEventCallback (BLT_EV_FLAG_CONN_PARA_UPDATE, &task_conn_update_done);

	blc_l2cap_registerConnUpdateRspCb(app_conn_param_update_response);
	#if(AUDIO_TRANS_USE_2M_PHY_ENABLE)
		blc_gap_setEventMask(GAP_EVT_MASK_ATT_EXCHANGE_MTU);
		blc_gap_registerHostEventHandler( app_host_event_callback );
		bls_app_registerEventCallback (BLT_EV_FLAG_PHY_UPDATE, &app_phy_update_complete_event);
	#endif

#if (REMOTE_IR_LEARN_ENABLE)
	ir_learn_init();
#endif
#if(TL_AUDIO_MODE==TL_AUDIO_RCU_ADPCM_GATT_GOOGLE)
	google_handle_init(AUDIO_GOOGLE_CTL_DP_H,HID_CONSUME_REPORT_INPUT_DP_H);
#endif
	///////////////////// Power Management initialization///////////////////
#if(BLE_APP_PM_ENABLE)
	blc_ll_initPowerManagement_module();
	bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_EXIT, &task_suspend_exit);
	/*audio config*/
	#if	(TL_AUDIO_MODE & (TL_AUDIO_MASK_SBC_MODE | TL_AUDIO_MASK_MSBC_MODE))
		blc_pm_setDeepsleepRetentionType(DEEPSLEEP_MODE_RET_SRAM_LOW32K);
	#endif

	#if (PM_DEEPSLEEP_RETENTION_ENABLE)
		blc_app_setDeepsleepRetentionSramSize(); //select DEEPSLEEP_MODE_RET_SRAM_LOW16K or DEEPSLEEP_MODE_RET_SRAM_LOW32K
		bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
		blc_pm_setDeepsleepRetentionThreshold(50, 30);
		#if (__PROJECT_8278_BLE_REMOTE__)
			blc_pm_setDeepsleepRetentionEarlyWakeupTiming(480);
		#else
			blc_pm_setDeepsleepRetentionEarlyWakeupTiming(400);
		#endif
	#else
		bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
	#endif

	bls_app_registerEventCallback (BLT_EV_FLAG_SUSPEND_ENTER, &task_suspend_enter);
#else
	bls_pm_setSuspendMask (SUSPEND_DISABLE);
#endif



#if (BLE_REMOTE_OTA_ENABLE)
	////////////////// OTA relative ////////////////////////
	blc_ota_initOtaServer_module(); //must
	bls_ota_registerStartCmdCb(app_enter_ota_mode);
	//bls_ota_registerResultIndicateCb(app_debug_ota_result);  //debug
#endif



	app_ui_init_normal();

	/* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done.
	 * attention that code will stuck in "while(1)" if any error detected in initialization, user need find what error happens and then fix it */
	blc_app_checkControllerHostInitialization();


	advertise_begin_tick = clock_time();

	tlkapi_printf(APP_LOG_EN, "[APP][INI] BLE Remote init \n");
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

	#if (APP_BATT_CHECK_ENABLE)
		/* ADC settings will lost during deepsleep retention mode, so here need clear flag */
		battery_clear_adc_setting_flag();
	#endif

	DBG_CHN0_HIGH;    //debug

	irq_enable();


	app_ui_init_deepRetn();
#endif
}

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

		/* just sample code here, protect all flash area for old firmware and OTA new firmware.
		 * user can change this design if have other consideration */
		u32  app_lockBlock = 0;
		#if (BLE_REMOTE_OTA_ENABLE)
			u32 multiBootAddress = blc_ota_getCurrentUsedMultipleBootAddress();
			if(multiBootAddress == MULTI_BOOT_ADDR_0x20000){
				app_lockBlock = FLASH_LOCK_FW_LOW_256K;
			}
			else if(multiBootAddress == MULTI_BOOT_ADDR_0x40000){
				/* attention that 512K capacity flash can not lock all 512K area, should leave some upper sector
				 * for system data(SMP storage data & calibration data & MAC address) and user data
				 * will use a approximate value */
				app_lockBlock = FLASH_LOCK_FW_LOW_512K;
			}
			#if(MCU_CORE_TYPE == MCU_CORE_827x)
			else if(multiBootAddress == MULTI_BOOT_ADDR_0x80000){
				if(blc_flash_capacity < FLASH_SIZE_1M){ //for flash capacity smaller than 1M, OTA can not use 512K as multiple boot address
					blc_flashProt.init_err = 1;
				}
				else{
					/* attention that 1M capacity flash can not lock all 1M area, should leave some upper sector for
					 * system data(SMP storage data & calibration data & MAC address) and user data
					 * will use a approximate value */
					app_lockBlock = FLASH_LOCK_FW_LOW_1M;
				}
			}
			#endif
		#else
			app_lockBlock = FLASH_LOCK_FW_LOW_256K; //just demo value, user can change this value according to application
		#endif


		flash_lockBlock_cmd = flash_change_app_lock_block_to_flash_lock_block(app_lockBlock);

		if(blc_flashProt.init_err){
			tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash protection initialization error!!!\n"); //print log here, tell user initialization error
		}

		tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] initialization, lock flash\n");
		flash_lock(flash_lockBlock_cmd);
	}
#if (BLE_REMOTE_OTA_ENABLE)
	else if(flash_op_evt == FLASH_OP_EVT_STACK_OTA_CLEAR_OLD_FW_BEGIN)
	{
		/* OTA clear old firmware begin event is triggered by stack, in "blc ota_initOtaServer_module", rebooting from a successful OTA.
		 * Software will erase whole old firmware for potential next new OTA, need unlock flash if any part of flash address from
		 * "op addr_begin" to "op addr_end" is in locking block area.
		 * In this sample code, we protect whole flash area for old and new firmware, so here we do not need judge "op addr_begin" and "op addr_end",
		 * must unlock flash */
		tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA clear old FW begin, unlock flash\n");
		flash_unlock();
	}
	else if(flash_op_evt == FLASH_OP_EVT_STACK_OTA_CLEAR_OLD_FW_END)
	{
		/* ignore "op addr_begin" and "op addr_end" for END event
		 * OTA clear old firmware end event is triggered by stack, in "blc ota_initOtaServer_module", erasing old firmware data finished.
		 * In this sample code, we need lock flash again, because we have unlocked it at the begin event of clear old firmware */
		tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA clear old FW end, restore flash locking\n");
		flash_lock(flash_lockBlock_cmd);
	}
	else if(flash_op_evt == FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_BEGIN)
	{
		/* OTA write new firmware begin event is triggered by stack, when receive first OTA data PDU.
		 * Software will write data to flash on new firmware area,  need unlock flash if any part of flash address from
		 * "op addr_begin" to "op addr_end" is in locking block area.
		 * In this sample code, we protect whole flash area for old and new firmware, so here we do not need judge "op addr_begin" and "op addr_end",
		 * must unlock flash */
		tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA write new FW begin, unlock flash\n");
		flash_unlock();
	}
	else if(flash_op_evt == FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END)
	{
		/* ignore "op addr_begin" and "op addr_end" for END event
		 * OTA write new firmware end event is triggered by stack, after OTA end or an OTA error happens, writing new firmware data finished.
		 * In this sample code, we need lock flash again, because we have unlocked it at the begin event of write new firmware */
		tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA write new FW end, restore flash locking\n");
		flash_lock(flash_lockBlock_cmd);
	}
#endif
	/* add more flash protection operation for your application if needed */
}

#endif





u32 tick_loop;


/**
 * @brief     BLE main loop
 * @param[in]  none.
 * @return     none.
 */
void main_loop(void)
{
	tick_loop ++;

#if (BLT_TEST_SOFT_TIMER_ENABLE)
	blt_soft_timer_process(MAINLOOP_ENTRY);
#endif
	////////////////////////////////////// BLE entry /////////////////////////////////
	blt_sdk_main_loop();


	////////////////////////////////////// UI entry /////////////////////////////////
	#if (BLE_AUDIO_ENABLE)
		proc_audio();
	#endif

	#if (APP_BATT_CHECK_ENABLE)
		/*The frequency of low battery detect is controlled by the variable lowBattDet_tick, which is executed every
		 500ms in the demo. Users can modify this time according to their needs.*/
		if(battery_get_detect_enable() && clock_time_exceed(lowBattDet_tick, 500000) ){
			lowBattDet_tick = clock_time();
			#if(MCU_CORE_TYPE == MCU_CORE_TC321X && APP_BATT_CHECK_ENABLE)	// AMIC and DC module cannot work at the same time in TC321X
			if(!ui_mic_enable)
			#endif
			{
				user_battery_power_check(VBAT_DEEP_THRES_MV);
			}
		}
	#endif

	#if (UI_KEYBOARD_ENABLE)
		proc_keyboard(0, 0, 0);
	#endif



	#if (UI_LED_ENABLE)
		device_led_process();
	#endif

	blt_pm_proc();
}


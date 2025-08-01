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


#if (FEATURE_TEST_MODE == TEST_SMP_SECURITY)

#define		MY_RF_POWER_INDEX					RF_POWER_P3dBm


#define RX_FIFO_SIZE	64
#define RX_FIFO_NUM		8

#define TX_FIFO_SIZE	40
#define TX_FIFO_NUM		8

#if (SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_OOB||SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_OOB||\
		SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MDSI || SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MISI || \
		SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MISI || SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MDSI)

#if (SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_OOB)

typedef struct{
	smp_sc_oob_data_t scoob_local;
	smp_sc_oob_key_t scoob_local_key;
	smp_sc_oob_data_t scoob_remote;

	u8 scoob_local_used;
	u8 scoob_remote_used;
	u8 scoob_remote_geted;
}ui_sc_oob_data_t;

_attribute_ble_data_retention_	ui_sc_oob_data_t	sc_oob_data_cb;

#elif (SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_OOB)
_attribute_ble_data_retention_ unsigned char OOB_TK_key[16]={0};

#endif

#define BUFF_DATA_LEN     256
volatile unsigned char rec_buff[BUFF_DATA_LEN] __attribute__((aligned(4))) = {0};


#endif

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



//////////////////////////////////////////////////////////////////////////////
//	 Adv Packet, Response Packet
//////////////////////////////////////////////////////////////////////////////
const u8	tbl_advData[] = {
	 0x08, DT_COMPLETE_LOCAL_NAME, 'f', 'e', 'a', 't', 'u', 'r', 'e',
	 0x02, DT_FLAGS, 0x05, 							// BLE limited discoverable mode and BR/EDR not supported
	 0x03, DT_APPEARANCE, 0x80, 0x01, 					// 384, Generic Remote Control, Generic category
	 0x05, DT_INCOMPLETE_LIST_16BIT_SERVICE_UUID, 0x12, 0x18, 0x0F, 0x18,		// incomplete list of service class UUIDs (0x1812, 0x180F)
};

const u8	tbl_scanRsp [] = {
	 0x08, DT_COMPLETE_LOCAL_NAME, 'f', 'e', 'a', 't', 'u', 'r', 'e',
};


_attribute_data_retention_	int device_in_connection_state;
_attribute_data_retention_	u32 advertise_begin_tick;


#if (UI_KEYBOARD_ENABLE)
	#define CONSUMER_KEY   	   		1
	#define KEYBOARD_KEY   	   		2


	_attribute_data_retention_	int 	key_not_released;
	_attribute_data_retention_	u8 		key_type;
	_attribute_data_retention_		static u32 keyScanTick = 0;
	extern u32	scan_pin_need;

	_attribute_data_retention_  u8      tk_num_cnt;
	_attribute_data_retention_  u8      tk_input[6];

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
		if (kb_event.cnt >= 2){   //two or more key press, do  not process

		}
		else if(kb_event.cnt == 1){
			if(key0 >= CR_VOL_UP ){
				key_type = CONSUMER_KEY;
				u16 consumer_key;
				if(key0 == CR_VOL_UP){  	//volume up
					consumer_key = MKEY_VOL_UP;
				}
				else if(key0 == CR_VOL_DN){ //volume down
					consumer_key = MKEY_VOL_DN;
				}
				blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE, HID_CONSUME_REPORT_INPUT_DP_H, (u8 *)&consumer_key, 2);
				tlkapi_printf(APP_KEYBOARD_LOG_EN, "[APP][KEY] CK:0x%x pressed\n", consumer_key);

				#if (SMP_TEST_MODE == SMP_TEST_SC_NUMERIC_COMPARISON)
					//NOTICE:smp NC confirm (vol-)"YES" or (vol+)"NO"
					if(blc_smp_isWaitingToCfmNumericComparison()){
							if(consumer_key == MKEY_VOL_DN){
								blc_smp_setNumericComparisonResult(1);// YES
								tlkapi_printf(APP_KEYBOARD_LOG_EN, "[APP][KEY] confirmed YES\n");
							}
							else if(consumer_key == MKEY_VOL_UP){
								blc_smp_setNumericComparisonResult(0);// NO
								tlkapi_printf(APP_KEYBOARD_LOG_EN, "[APP][KEY] confirmed NO\n");
							}
							#if (UI_LED_ENABLE)
								gpio_write(GPIO_LED_RED, LED_ON_LEVEL);  // light on
							#endif
						}
				#endif
			}
			else
			{
				key_type = KEYBOARD_KEY;
				key_buf[2] = key0;
				blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE, HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
				tlkapi_printf(APP_KEYBOARD_LOG_EN, "[APP][KEY] KK:0x%x pressed\n", key0);


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
				blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE, HID_NORMAL_KB_REPORT_INPUT_DP_H, key_buf, 8);
			}

			#if (UI_LED_ENABLE)
				gpio_write(GPIO_LED_RED, !LED_ON_LEVEL);  // light off
			#endif
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

    tlk_contr_evt_connect_t *pConnEvt = (tlk_contr_evt_connect_t *)p;
	tlkapi_send_string_data(APP_LOG_EN, "[APP][EVT] connect, intA & advA:", pConnEvt->initA, 12);

	bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_10MS, CONN_INTERVAL_10MS, 99, CONN_TIMEOUT_4S);  // 1 S

	device_in_connection_state = 1;//

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

	#if (UI_LED_ENABLE)
		gpio_write(GPIO_LED_RED, LED_ON_LEVEL);  // light on
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
void	task_suspend_exit (u8 e, u8 *p, int n)
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

	#if (UI_KEYBOARD_ENABLE)
		if(scan_pin_need || key_not_released){
			bls_pm_setSuspendMask (SUSPEND_DISABLE);
		}
	#endif
#endif  //end of BLE_APP_PM_ENABLE
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
	u8 event = h & 0xFF;

	switch(event)
	{
		case GAP_EVT_SMP_PAIRING_BEGIN:
		{
			tlkapi_printf(APP_LOG_EN, "Pairing begin\n");

			#if (SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MDSI || SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MISI || \
				 SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MISI || SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MDSI)
			tk_num_cnt = 0;//clr
			#endif
		}
		break;

		case GAP_EVT_SMP_PAIRING_SUCCESS:
		{
			gap_smp_pairingSuccessEvt_t* p = (gap_smp_pairingSuccessEvt_t*)para;
			tlkapi_printf(APP_LOG_EN, "Pairing success:bond flg %s\n", p->bonding ?"true":"false");

			if(p->bonding_result){
				tlkapi_printf(APP_LOG_EN, "save smp key succ\n");
			}
			else{
				tlkapi_printf(APP_LOG_EN, "save smp key failed\n");
			}
		}
		break;

		case GAP_EVT_SMP_PAIRING_FAIL:
		{
			gap_smp_pairingFailEvt_t* p = (gap_smp_pairingFailEvt_t*)para;
			tlkapi_printf(APP_LOG_EN, "Pairing failed:rsn:0x%x\n", p->reason);
		}
		break;

		case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
		{
			gap_smp_connEncDoneEvt_t* p = (gap_smp_connEncDoneEvt_t*)para;
			tlkapi_printf(APP_LOG_EN, "Connection encryption done\n");

			if(p->re_connect == SMP_STANDARD_PAIR){  //first pairing

			}
			else if(p->re_connect == SMP_FAST_CONNECT){  //auto connect

			}
		}
		break;

		case GAP_EVT_SMP_TK_DISPLAY:
		{
			char pc[7];
			u32 pinCode = *(u32*)para;
			sprintf(pc, "%d", pinCode);
			tlkapi_printf(APP_LOG_EN, "TK display:%s\n", pc);
		}
		break;

		case GAP_EVT_SMP_TK_REQUEST_PASSKEY:
		{
			tlkapi_printf(APP_LOG_EN, "TK Request passkey\n");
			blc_smp_sendKeypressNotify(BLS_CONN_HANDLE, KEYPRESS_NTF_PKE_START);
		}
		break;
#if (SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_OOB)
		case GAP_EVT_SMP_TK_REQUEST_OOB:
		{
			tlkapi_printf(APP_LOG_EN, "TK Request OOB\n");

			blc_smp_setTK_by_OOB(OOB_TK_key);
			tlkapi_printf(APP_LOG_EN, "[APP][SMP]set TK data %s\n", hex_to_str(OOB_TK_key, 16));

		}
		break;
#endif
		case GAP_EVT_SMP_TK_NUMERIC_COMPARE:
		{
			char pc[7];
			u32 pinCode = *(u32*)para;
			sprintf(pc, "%d", pinCode);
			tlkapi_printf(APP_LOG_EN, "TK numeric comparison:%s\n", pc);
		}
		break;
#if (SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_OOB)
		case GAP_EVT_SMP_TK_SEND_SC_OOB_DATA:
		{
			gap_smp_TkSendScOobDataEvt_t* pEvt = (gap_smp_TkSendScOobDataEvt_t*)para;

			if(pEvt->sendScOobData2RemoteFlag){
				tlkapi_printf(APP_LOG_EN, "[APP][SMP] need to send SC OOB data to remote device\n");

				tlkapi_printf(APP_LOG_EN, "           Local SC OOB data-c(be) (by UART)      %s\n", hex_to_str(sc_oob_data_cb.scoob_local.confirm, 16));
				tlkapi_printf(APP_LOG_EN, "[APP][SMP] Send Local SC OOB data-r(be) (by UART) %s\n", hex_to_str(sc_oob_data_cb.scoob_local.random, 16));
			}
			else{
				tlkapi_printf(APP_LOG_EN, "[APP][SMP] not need to send SC OOB data to remote device\n");
			}
		}
		break;

		case GAP_EVT_SMP_TK_REQUEST_SC_OOB:
		{
			gap_smp_TkRequestScOobDataEvt_t* pEvt = (gap_smp_TkRequestScOobDataEvt_t*)para;

			tlkapi_printf(APP_LOG_EN, "[APP][SMP] SC OOB scOobLocalUsed %d\n", pEvt->scOobLocalUsed);
			tlkapi_printf(APP_LOG_EN, "[APP][SMP] SC OOB scOobRemoteUsed %d\n", pEvt->scOobRemoteUsed);

			sc_oob_data_cb.scoob_remote_used = pEvt->scOobRemoteUsed;
			sc_oob_data_cb.scoob_local_used = pEvt->scOobLocalUsed;

			if(pEvt->scOobRemoteUsed || pEvt->scOobLocalUsed){
				blc_smp_setScOobData(pEvt->connHandle, &sc_oob_data_cb.scoob_local, &sc_oob_data_cb.scoob_remote);
			}
		}
		break;
#endif
		default:
		break;
	}

	return 0;
}



#if (SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_OOB||SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_OOB||\
		SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MDSI || SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MISI || \
		SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MISI || SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MDSI)


void uart_irq_handler()
{
	unsigned char irqS = dma_chn_irq_status_get();
    if(irqS & FLD_DMA_CHN_UART_RX)	//rx
    {
    	dma_chn_irq_status_clr(FLD_DMA_CHN_UART_RX);
    	unsigned char rev_data_len = rec_buff[0];//The first four Bytes are header of DMA


#if (SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_OOB)
    	if(rev_data_len >= 32&&(!sc_oob_data_cb.scoob_remote_geted))
    	{
			for(int i=0; i<16; i++) {
				sc_oob_data_cb.scoob_remote.confirm[i] = rec_buff[i+4];
				sc_oob_data_cb.scoob_remote.random[i] = rec_buff[i+20];
			}
			sc_oob_data_cb.scoob_remote_geted=1;
			tlkapi_printf(APP_LOG_EN, "[APP][SMP]Get Remote SC OOB data-c(be)(by UART) %s\n", hex_to_str(sc_oob_data_cb.scoob_remote.confirm, 16));
			tlkapi_printf(APP_LOG_EN, "[APP][SMP]Get Remote SC OOB data-r(be)(by UART) %s\n", hex_to_str(sc_oob_data_cb.scoob_remote.random, 16));
    	}
#elif (SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_OOB)
    	if(rev_data_len >=16 )
    	{
    		for(int i=0; i<16; i++) {
    			OOB_TK_key[i] = rec_buff[i+4];
			}
    	}
		tlkapi_printf(APP_LOG_EN, "[APP][SMP]Get Remote TK data(by UART) %s\n", hex_to_str(OOB_TK_key, 16));
#elif (SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MDSI || SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MISI || \
					 SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MISI || SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MDSI)

		if(blc_smp_isWaitingToSetPasskeyEntry()){
			if(rev_data_len)
			{
				for(int i = 0; i<rev_data_len; i++){
					if(rec_buff[i+4]>='0' && rec_buff[i+4]<='9' && tk_num_cnt<6)
						tk_input[tk_num_cnt++] = rec_buff[i+4]-'0';
						blc_smp_sendKeypressNotify(BLS_CONN_HANDLE, KEYPRESS_NTF_PKE_DIGIT_ENTERED);
				}
			}
			if(tk_num_cnt==6)
			{
				tlkapi_printf(APP_LOG_EN, "[APP][SMP]Get Remote TK data(by UART) %s\n", hex_to_str(tk_input, 6));

				u32 pincode = tk_input[0]*100000 + tk_input[1]*10000 + \
						  tk_input[2]*1000 + tk_input[3]*100 + tk_input[4]*10 + tk_input[5];
				tlkapi_printf(APP_KEYBOARD_LOG_EN, "[APP][KEY] TK 6-dig value:%d\n", pincode);
				blc_smp_setTK_by_PasskeyEntry(pincode);

				blc_smp_sendKeypressNotify(BLS_CONN_HANDLE, KEYPRESS_NTF_PKE_COMPLETED);
				tk_num_cnt=0;
			}
		}

#endif
    }


}



void UART_Init(void)
{
	//note: dma addr must be set first before any other uart initialization!
	uart_recbuff_init( (unsigned char *)rec_buff, BUFF_DATA_LEN);

	uart_gpio_set(UART_TX_PB1, UART_RX_PB0);

	uart_reset();  //will reset uart digital registers from 0x90 ~ 0x9f, so uart setting must set after this reset

	//baud rate: 115200
	#if (CLOCK_SYS_CLOCK_HZ == 16000000)
		uart_init(9, 13, PARITY_NONE, STOP_BIT_ONE);
	#elif (CLOCK_SYS_CLOCK_HZ == 24000000)
		uart_init(12, 15, PARITY_NONE, STOP_BIT_ONE);
	#elif (CLOCK_SYS_CLOCK_HZ == 32000000)
		uart_init(30, 8, PARITY_NONE, STOP_BIT_ONE);
	#elif (CLOCK_SYS_CLOCK_HZ == 48000000)
		uart_init(25, 15, PARITY_NONE, STOP_BIT_ONE);
	#endif

	uart_dma_enable(1, 1); 	//uart data in hardware buffer moved by dma, so we need enable them first

	irq_set_mask(FLD_IRQ_DMA_EN);
	dma_chn_irq_enable(FLD_DMA_CHN_UART_RX | FLD_DMA_CHN_UART_TX, 1);   	//uart Rx/Tx dma irq enable

	uart_irq_enable(0, 0);  	//uart Rx/Tx irq no need, disable them



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
	extern void my_att_init();
	my_att_init(); //gatt initialization
	blc_l2cap_register_handler (blc_l2cap_packet_receive);  	//l2cap initialization




#if ( SMP_TEST_MODE	== SMP_TEST_NO_SECURITY)

	blc_smp_setSecurityLevel(No_Authentication_No_Encryption);  // LE_Security_Mode_1_Level_1	//Smp Initialization may involve flash write/erase(when one sector stores too much information,

#elif ( SMP_TEST_MODE == SMP_TEST_LEGACY_PAIRING_JUST_WORKS  )

	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//set security level: "LE_Security_Mode_1_Level_2"
	blc_smp_setSecurityLevel(Unauthenticated_Pairing_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Pairing_with_Encryption)
	blc_smp_setBondingMode(Bondable_Mode);	// if not set, default is : Bondable_Mode
	blc_smp_setIoCapability(IO_CAPABILITY_NO_IN_NO_OUT);	// if not set, default is : IO_CAPABILITY_NO_INPUT_NO_OUTPUT

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	//Notice:if user set smp parameters: it should be called after usr smp settings
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE );

#elif ( SMP_TEST_MODE == SMP_TEST_SC_PAIRING_JUST_WORKS )
	blc_att_setRxMtuSize(65); //set MTU size to 65
	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//set security level: "LE_Security_Mode_1_Level_2"
	blc_smp_setSecurityLevel(Unauthenticated_Pairing_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Pairing_with_Encryption)
	blc_smp_enableSecureConnections(1);
	blc_smp_setSecurityParameters(Bondable_Mode, 1, 0, 0, IO_CAPABILITY_NO_IN_NO_OUT);
	//use debug mode for sniffer decryption
	blc_smp_setEcdhDebugMode(debug_mode);

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	//Notice:if user set smp parameters: it should be called after usr smp settings
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE     |  \
						  GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE);

#elif ( SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_SDMI  )

	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//set security level: "LE_Security_Mode_1_Level_3"
	///
	blc_smp_setSecurityLevel(Authenticated_Pairing_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Pairing_with_Encryption)
	blc_smp_enableAuthMITM(1);
	blc_smp_setBondingMode(Bondable_Mode);	// if not set, default is : Bondable_Mode
	blc_smp_setIoCapability(IO_CAPABILITY_DISPLAY_ONLY);	// if not set, default is : IO_CAPABILITY_NO_INPUT_NO_OUTPUT

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	//Notice:if user set smp parameters: it should be called after usr smp settings
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_TK_DISPLAY				|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE     |  \
						  GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE);

#elif ( SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MDSI|| SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MISI)

	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//set security level: "LE_Security_Mode_1_Level_3"
	blc_smp_setSecurityLevel(Authenticated_Pairing_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Pairing_with_Encryption)
	blc_smp_enableAuthMITM(1);
	blc_smp_setBondingMode(Bondable_Mode);	// if not set, default is : Bondable_Mode
	blc_smp_setIoCapability(IO_CAPABILITY_KEYBOARD_ONLY);	// if not set, default is : IO_CAPABILITY_NO_INPUT_NO_OUTPUT

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	//Notice:if user set smp parameters: it should be called after usr smp settings
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_TK_DISPLAY				|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE     |  \
						  GAP_EVT_MASK_SMP_TK_REQUEST_PASSKEY       |  \
						  GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE);
#elif ( SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_OOB )

	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//set security level: "LE_Security_Mode_1_Level_3"
	///
	blc_smp_setSecurityLevel(Authenticated_Pairing_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Pairing_with_Encryption)
	blc_smp_enableAuthMITM(1);
	blc_smp_setBondingMode(Bondable_Mode);	// if not set, default is : Bondable_Mode
	blc_smp_setIoCapability(IO_CAPABILITY_KEYBOARD_ONLY);	// if not set, default is : IO_CAPABILITY_NO_INPUT_NO_OUTPUT
	blc_smp_enableOobAuthentication(1);

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	//Notice:if user set smp parameters: it should be called after usr smp settings
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_TK_REQUEST_OOB			|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE     |  \
						  GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE);
#elif ( SMP_TEST_MODE == SMP_TEST_SC_NUMERIC_COMPARISON  )
	blc_att_setRxMtuSize(65); //set MTU size to 65
	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//set security level: "LE_Security_Mode_1_Level_4"
	blc_smp_setSecurityLevel(Authenticated_LE_Secure_Connection_Pairing_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Pairing_with_Encryption)
	blc_smp_enableSecureConnections(1);
	blc_smp_setSecurityParameters(Bondable_Mode, 1, 0, 0, IO_CAPABILITY_DISPLAY_YESNO);

	//use debug mode for sniffer decryption
	blc_smp_setEcdhDebugMode(debug_mode);

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	//Notice:if user set smp parameters: it should be called after usr smp settings
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_TK_NUMERIC_COMPARE		|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE );


#elif ( SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_SDMI  )
	blc_att_setRxMtuSize(65); //set MTU size to 65
	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//set security level: "LE_Security_Mode_1_Level_4"
	blc_smp_setSecurityLevel(Authenticated_LE_Secure_Connection_Pairing_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Pairing_with_Encryption)
	blc_smp_enableSecureConnections(1);
	blc_smp_setSecurityParameters(Bondable_Mode, 1, 0, 0, IO_CAPABILITY_DISPLAY_ONLY);
	//use debug mode for sniffer decryption
	blc_smp_setEcdhDebugMode(debug_mode);

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	//Notice:if user set smp parameters: it should be called after usr smp settings
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_TK_DISPLAY				|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE     |  \
						  GAP_EVT_MASK_SMP_SECURITY_PROCESS_DONE);

#elif ( SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MDSI || SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MISI)
	blc_att_setRxMtuSize(65); //set MTU size to 65
	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//set security level: "LE_Security_Mode_1_Level_4"
	blc_smp_setSecurityLevel(Authenticated_LE_Secure_Connection_Pairing_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Pairing_with_Encryption)
	blc_smp_enableSecureConnections(1);
	blc_smp_setSecurityParameters(Bondable_Mode, 1, 0, 1, IO_CAPABILITY_KEYBOARD_ONLY);
	//use debug mode for sniffer decryption
	blc_smp_setEcdhDebugMode(debug_mode);

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	//Notice:if user set smp parameters: it should be called after usr smp settings
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_IMM_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_TK_REQUEST_PASSKEY		|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE );

#elif ( SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_OOB  )
	blc_att_setRxMtuSize(65); //set MTU size to 65
	blc_smp_param_setBondingDeviceMaxNumber(4);    //if not set, default is : SMP_BONDING_DEVICE_MAX_NUM

	//set security level: "LE_Security_Mode_1_Level_4"
	blc_smp_setSecurityLevel(Authenticated_LE_Secure_Connection_Pairing_with_Encryption);  //if not set, default is : LE_Security_Mode_1_Level_2(Unauthenticated_Pairing_with_Encryption)
	blc_smp_enableSecureConnections(1);
	blc_smp_setSecurityParameters(Bondable_Mode, 1, 1, 0, IO_CAPABILITY_NO_INPUT_NO_OUTPUT);

	blc_smp_generateScOobData(&sc_oob_data_cb.scoob_local,&sc_oob_data_cb.scoob_local_key);
    tlkapi_printf(APP_LOG_EN, "SC OOB data-confirm (be) %s\n", hex_to_str(sc_oob_data_cb.scoob_local.confirm, 16));
    tlkapi_printf(APP_LOG_EN, "SC OOB data-random  (be) %s\n", hex_to_str(sc_oob_data_cb.scoob_local.random, 16));
	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	//Notice:if user set smp parameters: it should be called after usr smp settings
	blc_smp_peripheral_init();

	blc_smp_configSecurityRequestSending(SecReq_PEND_SEND, SecReq_PEND_SEND, 1000); //if not set, default is:  send "security request" immediately after link layer connection established(regardless of new connection or reconnection )

	//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
	blc_gap_registerHostEventHandler( app_host_event_callback );
	blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
						  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
						  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
						  GAP_EVT_MASK_SMP_TK_REQUEST_PASSKEY		|  \
						  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE |  \
						  GAP_EVT_MASK_SMP_TK_SEND_SC_OOB_DATA |  \
						  GAP_EVT_MASK_SMP_TK_REQUEST_SC_OOB);
#endif

///////////////////// UART initialization ///////////////////
#if (SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_OOB||SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_OOB||\
		SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MDSI || SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MISI || \
		SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MISI || SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MDSI)
	UART_Init();
#endif

///////////////////// USER application initialization ///////////////////
	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));




	////////////////// config ADV packet /////////////////////
	u8 adv_param_status = BLE_SUCCESS;
	adv_param_status = bls_ll_setAdvParam(  ADV_INTERVAL_30MS, ADV_INTERVAL_30MS,
									 ADV_TYPE_CONNECTABLE_UNDIRECTED, OWN_ADDRESS_PUBLIC,
									 0,  NULL,
									 BLT_ENABLE_ADV_ALL,
									 ADV_FP_NONE);
	if(adv_param_status != BLE_SUCCESS){
		tlkapi_printf(APP_LOG_EN, "[APP][INI] ADV parameters error 0x%x!!!\n", adv_param_status);
		while(1);
	}

	bls_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable



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

	tlkapi_printf(APP_LOG_EN, "[APP][INI] feature_smp_security init \n");
	advertise_begin_tick = clock_time();
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

#if (SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_OOB||SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_OOB||\
		SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MDSI || SMP_TEST_MODE == SMP_TEST_SC_PASSKEY_ENTRY_MISI || \
		SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MISI || SMP_TEST_MODE == SMP_TEST_LEGACY_PASSKEY_ENTRY_MDSI)
	UART_Init();
#endif

#endif
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

	////////////////////////////////////// PM Process /////////////////////////////////
	blt_pm_proc();
}


#endif  //end of (FEATURE_TEST_MODE == ...)

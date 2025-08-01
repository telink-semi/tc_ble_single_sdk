/********************************************************************************************************
 * @file    spp.c
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
#include "app_att.h"
#include "spp.h"


extern int	module_uart_data_flg;
extern u32 module_wakeup_module_tick;

extern my_fifo_t spp_rx_fifo;
extern my_fifo_t spp_tx_fifo;
extern void app_suspend_exit ();
u8  pm_ctrl_flg;
u8  pairing_end_status;
volatile u8 isUartTxDone = 1;
///////////the code below is just for demonstration of the event callback only////////////

_attribute_data_retention_	u32 spp_cmd_restart_flag;

/**
 * @brief      callback function of LinkLayer Event
 * @param[in]  h     - LinkLayer Event type
 * @param[in]  param - data pointer of event
 * @param[in]  n     - data length of event
 * @return     none
 */
int controller_event_handler(u32 h, u8 *para, int n)
{
    (void)h;(void)para;(void)n;
	if((h&HCI_FLAG_EVENT_TLK_MODULE)!= 0)			//module event
	{

		u8 event = (u8)(h&0xff);

		u8	retPara[8] = {0};
		spp_event_t *pEvt =  (spp_event_t *)&retPara;
		pEvt->token = 0xFF;
		pEvt->paramLen = 2; //default 2(eventID), will change in specific cmd process
		pEvt->eventId = 0x0780 + event;
		pEvt->param[0] = BLE_SUCCESS;  //default all success, will change in specific cmd process


		switch(event)
		{
			case BLT_EV_FLAG_SCAN_RSP:
			break;


			case BLT_EV_FLAG_CONNECT:
			{
				bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_10MS, CONN_INTERVAL_15MS, 99, CONN_TIMEOUT_4S);

				spp_send_data(HCI_FLAG_EVENT_TLK_MODULE, pEvt);
			}
			break;


			case BLT_EV_FLAG_TERMINATE:
			{
				spp_send_data(HCI_FLAG_EVENT_TLK_MODULE, pEvt);
			}
			break;

			case BLT_EV_FLAG_GPIO_EARLY_WAKEUP:
			break;

			case BLT_EV_FLAG_CHN_MAP_REQ:
			break;


			case BLT_EV_FLAG_CONN_PARA_REQ:
			break;


			case BLT_EV_FLAG_CHN_MAP_UPDATE:
			{
				spp_send_data(HCI_FLAG_EVENT_TLK_MODULE, pEvt);
			}
			break;


			case BLT_EV_FLAG_CONN_PARA_UPDATE:
			{
				spp_send_data(HCI_FLAG_EVENT_TLK_MODULE, pEvt);

				//Master send SIG_Connection_Param_Update_Rsp pkt,and the reply result is 0x0000. When connection event counter value is equal
				//to the instant, a callback event BLT_EV_FLAG_CONN_PARA_UPDATE will generate. The connection interval at this time should be the
				//currently updated and valid connection interval!
//				printf("Update param event occur.\n");
//				printf("Current Connection interval:%dus.\n", bls_ll_getConnectionInterval() * 1250);
			}
			break;


			case BLT_EV_FLAG_ADV_DURATION_TIMEOUT:
			break;


			case BLT_EV_FLAG_SUSPEND_ENTER:
			break;


			case BLT_EV_FLAG_SUSPEND_EXIT:
				app_suspend_exit ();
			break;


			default:
			break;
		}
	}

	return 0;
}

/**
 * @brief      BLE host event handler call-back.
 * @param[in]  h       event type
 * @param[in]  para    Pointer point to event parameter buffer.
 * @param[in]  n       the length of event parameter.
 * @return
 */
int app_host_event_callback (u32 h, u8 *para, int n)
{
	u8 event = h & 0xFF;
    (void)h;(void)para;(void)n;
	switch(event)
	{
		case GAP_EVT_SMP_PAIRING_BEGIN:
		{

		}
		break;

		case GAP_EVT_SMP_PAIRING_SUCCESS:
		{
			gap_smp_pairingSuccessEvt_t* p = (gap_smp_pairingSuccessEvt_t*)para;

			if(p->bonding_result){

			}
			else{

			}
		}
		break;

		case GAP_EVT_SMP_PAIRING_FAIL:
		{
//			gap_smp_pairingFailEvt_t* p = (gap_smp_pairingFailEvt_t*)para;
		}
		break;

		case GAP_EVT_SMP_CONN_ENCRYPTION_DONE:
		{
			gap_smp_connEncDoneEvt_t* p = (gap_smp_connEncDoneEvt_t*)para;

			if(p->re_connect == SMP_STANDARD_PAIR){  //first pairing

			}
			else if(p->re_connect == SMP_FAST_CONNECT){  //auto connect

			}
		}
		break;

		case GAP_EVT_SMP_TK_DISPLAY:
		{
//			char pc[7];
//			u32 pinCode = *(u32*)para;
		}
		break;

		case GAP_EVT_SMP_TK_REQUEST_PASSKEY:
		{

		}
		break;

		case GAP_EVT_SMP_TK_REQUEST_OOB:
		{

		}
		break;

		case GAP_EVT_SMP_TK_NUMERIC_COMPARE:
		{
//			char pc[7];
//			u32 pinCode = *(u32*)para;
		}
		break;

		default:
		break;
	}

	return 0;
}

/////////////////////////////////////blc_register_hci_handler for spp////////////////////////////
/**
 * @brief		this function is used to process rx uart data.
 * @param[in]	none
 * @return      0 is ok
 */
int rx_from_uart_cb (void)//UART data send to Master,we will handler the data as CMD or DATA
{
	if(my_fifo_get(&spp_rx_fifo) == 0)
	{
		return 0;
	}

	u8* p = my_fifo_get(&spp_rx_fifo);
	u32 rx_len = p[1]<<8 | p[0]; //usually <= 65536 so 2 byte should be sufficient

	if (rx_len)
	{
		bls_uart_handler(&p[4], rx_len - 4);
		my_fifo_pop(&spp_rx_fifo);
	}

	return 0;
}

/**
 * @brief		this function is used to process rx uart data to remote device.
 * @param[in]   p - data pointer
 * @param[in]   n - data length
 * @return      0 is ok
 */
///////////////////////////////////////////the default bls_uart_handler///////////////////////////////
int bls_uart_handler (u8 *p, int n)
{
    (void)p;(void)n;
	spp_cmd_t *pCmd =  (spp_cmd_t *)p;
	u16 spp_cmd = pCmd->cmdId;
	u8 *cmdPara = pCmd->param;


	u8	retPara[20] = {0};
	spp_event_t *pEvt =  (spp_event_t *)&retPara;
	pEvt->token = 0xFF;
	pEvt->paramLen = 3; //default 2(eventID) + 1(status), will change in specific cmd process
	pEvt->eventId = ((spp_cmd & 0x3ff) | 0x400);
	pEvt->param[0] = BLE_SUCCESS;  //default all success, will change in specific cmd process



	// set advertising interval: 01 ff 02 00 50 00: 80 *0.625ms
	if (spp_cmd == SPP_CMD_SET_ADV_INTV)
	{
		u8 interval = cmdPara[0] ;
		pEvt->param[0] = bls_ll_setAdvInterval(interval, interval);
	}
	// set advertising data: 02 ff 06 00 01 02 03 04 05 06
	else if (spp_cmd == SPP_CMD_SET_ADV_DATA)
	{
		pEvt->param[0] = (u8)bls_ll_setAdvData(cmdPara, pCmd->paramLen);
	}
	// enable/disable advertising: 0a ff 01 00  01
	else if (spp_cmd == SPP_CMD_SET_ADV_ENABLE)
	{
		pEvt->param[0] = (u8)bls_ll_setAdvEnable(cmdPara[0]);
	}
	// send data: 0b ff 05 00  01 02 03 04 05
	//change format to 0b ff 07 handle(2bytes) 00 01 02 03 04 05
	else if (spp_cmd == 0xFF0B)
	{

	}
	// get module available data buffer: 0c ff 00  00
	else if (spp_cmd == SPP_CMD_GET_BUF_SIZE)
	{
		u8 r[4];
		pEvt->param[0] = (u8)blc_hci_le_readBufferSize_cmd( (u8 *)(r) );
		pEvt->param[1] = r[2];
		pEvt->paramLen = 4;  //eventID + param
	}
	// set advertising type: 0d ff 01 00  00
	else if (spp_cmd == SPP_CMD_SET_ADV_TYPE)
	{
		pEvt->param[0] = bls_ll_setAdvType(cmdPara[0]);
	}
	// set advertising address type: 0e ff 01 00  00(public; 1 for random) 01 02 03 04 05 06
	else if (spp_cmd == SPP_CMD_SET_ADV_ADDR_TYPE)
	{
		pEvt->param[0] = blt_set_adv_addrtype(cmdPara);
	}
	// set advertising direct initiator address type: 19 ff 07 00  00(public; 1 for random) 01 02 03 04 05 06
	else if (spp_cmd == SPP_CMD_SET_ADV_DIRECT_ADDR)
	{
		pEvt->param[0] = blt_set_adv_direct_init_addrtype(cmdPara);
	}
	// add white list entry: 0f ff 07 00 01 02 03 04 05 06
	else if (spp_cmd == SPP_CMD_ADD_WHITE_LST_ENTRY)
	{
		pEvt->param[0] = (u8)ll_whiteList_add(cmdPara[0], cmdPara + 1);
	}
	// delete white list entry: 10  ff 07 00 01 02 03 04 05 06
	else if (spp_cmd == SPP_CMD_DEL_WHITE_LST_ENTRY)
	{
		pEvt->param[0] = (u8)ll_whiteList_delete(cmdPara[0], cmdPara + 1);
	}
	// reset white list entry: 11 ff 00 00
	else if (spp_cmd == SPP_CMD_RST_WHITE_LST)
	{
		pEvt->param[0] = (u8)ll_whiteList_reset();
	}
	// set filter policy: 12 ff 10 00 00(bit0: scan WL enable; bit1: connect WL enable)
	else if (spp_cmd == SPP_CMD_SET_FLT_POLICY)
	{
		pEvt->param[0] = bls_ll_setAdvFilterPolicy(cmdPara[0]);
	}
	// set device name: 13 ff 0a 00  01 02 03 04 05 06 07 08 09 0a
	else if (spp_cmd == SPP_CMD_SET_DEV_NAME)
	{
		pEvt->param[0] = bls_att_setDeviceName(cmdPara,p[2]);
	}
	// get connection parameter: 14 ff 00 00
	else if (spp_cmd == SPP_CMD_GET_CONN_PARA)
	{
		u16 interval = bls_ll_getConnectionInterval();
		u16 latency =  bls_ll_getConnectionLatency();
		u16 timeout =  bls_ll_getConnectionTimeout();

		if(interval){
			pEvt->param[0] = BLE_SUCCESS;
			pEvt->param[1] = interval&0xff;
			pEvt->param[2] = interval>>8;
			pEvt->param[3] = latency&0xff;
			pEvt->param[4] = latency>>8;
			pEvt->param[5] = timeout&0xff;
			pEvt->param[6] = timeout>>8;
		}
		else{  //no connection
			pEvt->param[0] = LL_ERR_CONNECTION_NOT_ESTABLISH;
		}

		pEvt->paramLen = 9;  //eventID + param
	}
	// set connection parameter: 15 ff 08 00 a0 00 a2 00 00 00 2c 01 (min, max, latency, timeout)
	else if (spp_cmd == SPP_CMD_SET_CONN_PARA)
	{
		u16 interval_min = cmdPara[0] | cmdPara[1]<<8;
		u16 interval_max = cmdPara[2] | cmdPara[3]<<8;
		u16 latency 	 = cmdPara[4] | cmdPara[5]<<8;
		u16 timeout 	 = cmdPara[6] | cmdPara[7]<<8;

		bls_l2cap_requestConnParamUpdate(interval_min, interval_max, latency, timeout);
	}
	// get module current work state: 16 ff 00 00
	else if (spp_cmd == SPP_CMD_GET_CUR_STATE)
	{
		pEvt->param[1] = blc_ll_getCurrentState();
		pEvt->paramLen = 4;  //eventID + param
	}
	// terminate connection: 17 ff 00 00
	else if (spp_cmd == SPP_CMD_TERMINATE)
	{
		bls_ll_terminateConnection(HCI_ERR_REMOTE_USER_TERM_CONN);
	}
	// restart module: 18 ff 00 00
	else if (spp_cmd == SPP_CMD_RESTART_MOD)
	{
		spp_cmd_restart_flag = clock_time() | 1;
	}
	// enable/disable MAC binding function: 1a ff 01 00 00(disable, 01 enable)
	else if (spp_cmd == 0xFF1A)
	{

	}
	// add MAC address to binding table: 1b ff 06 00 01 02 03 04 05 06
	else if (spp_cmd == 0xFF1B)
	{

	}
	// delete MAC address from binding table: 1d ff 06 00 01 02 03 04 05 06
	else if (spp_cmd == 0xFF1D)
	{

	}
	//send notify data: 1c ff 07 00 11 00 01 02 03 04 05
	else if (spp_cmd == SPP_CMD_SEND_NOTIFY_DATA)
	{
		if (pCmd->paramLen > 42)
		{
			pEvt->param[0] = HCI_ERR_INVALID_HCI_CMD_PARAMS;			//data too long
		}
		else
		{
			pEvt->param[0] = blc_gatt_pushHandleValueNotify (BLS_CONN_HANDLE, cmdPara[0] | (cmdPara[1]<<8), cmdPara + 2,  pCmd->paramLen - 2);
		}
	}


	spp_send_data (HCI_FLAG_EVENT_TLK_MODULE, pEvt);
	return 0;
}


/**
 * @brief		this function is used to process tx uart data to remote device.
 * @param[in]   header - hci event type
 * @param[in]   pEvent - event data
 * @return      0 is ok
 */
int spp_send_data (u32 header, spp_event_t * pEvt)
{

	u8 *p = my_fifo_wptr (&spp_tx_fifo);
	if (!p || (pEvt->paramLen+4) >= spp_tx_fifo.size)
	{
		return -1;
	}

#if (BLE_MODULE_INDICATE_DATA_TO_MCU)
	if(!module_uart_data_flg){ //UART idle, new data is sent
		GPIO_WAKEUP_MCU_HIGH;  //Notify MCU that there is data here
		module_wakeup_module_tick = clock_time() | 1;
		module_uart_data_flg = 1;
	}
#endif


	int sppEvt_len = pEvt->paramLen + 3;
	if (header & HCI_FLAG_EVENT_TLK_MODULE)
	{
		*p++ = sppEvt_len;
		*p++ = sppEvt_len >> 8;
		#if 1
			memcpy (p, (u8 *)pEvt, pEvt->paramLen + 3);
			p += pEvt->paramLen + 3;
		#else
			*p++ = pEvt->token;
			*p++ = pEvt->paramLen;
			*p++ = pEvt->eventId;
			*p++ = pEvt->eventId>>8;
			if(pEvt->paramLen - 2 > 0){
				memcpy (p, pEvt->param, pEvt->paramLen - 2);
				p += pEvt->paramLen - 2;
			}
		#endif
	}

	my_fifo_next (&spp_tx_fifo);
	return 0;
}

uart_data_t T_txdata_buf;

/**
 * @brief		this function is used to process tx uart data.
 * @param[in]	none
 * @return      0 is ok
 */
int tx_to_uart_cb (void)
{
	if(spp_tx_fifo.wptr == spp_tx_fifo.rptr){
		return 0;//have no data
	}
	if (Tr_isUartTxDone()) {
		return 0;
	}
	u8 *p = my_fifo_get (&spp_tx_fifo);
	memcpy(&T_txdata_buf.data, p + 2, p[0] | (p[1] << 8));

	T_txdata_buf.len = p[0] | (p[1] << 8);
#if (BLE_MODULE_INDICATE_DATA_TO_MCU)
		//If the MCU side is designed to have low power consumption and the module has data to pull up
		//the GPIO_WAKEUP_MCU will only wake up the MCU, then you need to consider whether MCU needs a
		//reply time T from wakeup to a stable receive UART data. If you need a response time of T, ch-
		//ange the following 100US to the actual time required by user.
		if(module_wakeup_module_tick){
			while( !clock_time_exceed(module_wakeup_module_tick, 100) );
		}
#endif
	Tr_clrUartTxDone();
	uart_send_dma(UART_CONVERT((unsigned char*) (&T_txdata_buf)));
	my_fifo_pop(&spp_tx_fifo);
	return 1;
}

/**
 * @brief		this function is used to restart module.
 * @param[in]	none
 * @return      none
 */
void spp_restart_proc(void)
{
	//when received SPP_CMD_RESTART_MOD, leave 500ms(you can change this time) for module to send uart ack to host, then restart.
	if(spp_cmd_restart_flag && clock_time_exceed(spp_cmd_restart_flag, 500000)){
		cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_TIMER, clock_time() + 10000 * SYSTEM_TIMER_TICK_1US);
	}
}

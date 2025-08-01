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
#include "vendor/common/ble_flash.h"



#define EXTENDED_ADV_ENABLE							0

#if (EXTENDED_ADV_ENABLE)


	#define	APP_ADV_SETS_NUMBER						1			// Number of Supported Advertising Sets
	#define APP_MAX_LENGTH_ADV_DATA					200		// Maximum Advertising Data Length,   (if legacy ADV, max length 31 bytes is enough)
	#define APP_MAX_LENGTH_SCAN_RESPONSE_DATA		31		// Maximum Scan Response Data Length, (if legacy ADV, max length 31 bytes is enough)



	_attribute_data_retention_
								u8  app_adv_set_param[ADV_SET_PARAM_LENGTH * APP_ADV_SETS_NUMBER];

	_attribute_data_retention_
								u8	app_primary_adv_pkt[MAX_LENGTH_PRIMARY_ADV_PKT * APP_ADV_SETS_NUMBER];

	_attribute_data_retention_
								u8	app_secondary_adv_pkt[MAX_LENGTH_SECOND_ADV_PKT * APP_ADV_SETS_NUMBER];

	_attribute_data_retention_
								u8 	app_advData[APP_MAX_LENGTH_ADV_DATA	* APP_ADV_SETS_NUMBER];
	_attribute_data_retention_
								u8 	app_scanRspData[APP_MAX_LENGTH_SCAN_RESPONSE_DATA * APP_ADV_SETS_NUMBER];


#endif


//_attribute_data_retention_
							u8 		 	hci_rx_fifo_b[HCI_RXFIFO_SIZE * HCI_RXFIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	hci_rx_fifo = {
												HCI_RXFIFO_SIZE,
												HCI_RXFIFO_NUM,
												0,
												0,
												hci_rx_fifo_b,};

//_attribute_data_retention_
							u8 		 	hci_tx_fifo_b[HCI_TXFIFO_SIZE * HCI_TXFIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	hci_tx_fifo = {
												HCI_TXFIFO_SIZE,
												HCI_TXFIFO_NUM,
												0,
												0,
												hci_tx_fifo_b,};

//RF Tx / Rx fifo
/* CAL_LL_ACL_RX_BUF_SIZE(maxRxOct): maxRxOct + 22, then 16 byte align */
#define RX_FIFO_SIZE	64
/* must be: 2^n, (power of 2);at least 4; recommended value: 4, 8, 16 */
#define RX_FIFO_NUM		8


/* CAL_LL_ACL_TX_BUF_SIZE(maxTxOct):  maxTxOct + 10, then 4 byte align */
#define TX_FIFO_SIZE	40
/* must be: (2^n), (power of 2); at least 8; recommended value: 8, 16, 32, other value not allowed. */
#define TX_FIFO_NUM		16

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


#define     MY_APP_ADV_CHANNEL					BLT_ENABLE_ADV_ALL
#define 	MY_ADV_INTERVAL_MIN					ADV_INTERVAL_30MS
#define 	MY_ADV_INTERVAL_MAX					ADV_INTERVAL_40MS



#define MAX_INTERVAL_VAL		16




u32 tick_wakeup;
int	mcu_uart_working;			//depends on the wakeup scheme, attention to the use
int	module_uart_working;
int module_task_busy;


int	module_uart_data_flg;
u32 module_wakeup_module_tick;

#define UART_TX_BUSY			( (hci_tx_fifo.rptr != hci_tx_fifo.wptr) || uart_tx_is_busy(UART_NUM) )
#define UART_RX_BUSY			(hci_rx_fifo.rptr != hci_rx_fifo.wptr)


uart_data_t Tr_txdata_buf;
volatile u8 isUartTxDone = 1;
#define   Tr_clrUartTxDone()    (isUartTxDone = 0)
#define   Tr_SetUartTxDone()    (isUartTxDone = 1)
#define   Tr_isUartTxDone()     (!isUartTxDone)

/**
 * @brief		obtain uart working status
 * @param[in]	none
 * @return      0 for idle  else for busy
 */
int app_module_busy ()
{
	mcu_uart_working = gpio_read(GPIO_WAKEUP_MODULE);  //mcu use GPIO_WAKEUP_MODULE to indicate the UART data transmission or receiving state
	module_uart_working = UART_TX_BUSY || UART_RX_BUSY; //module checks to see if UART rx and tX are all processed
	module_task_busy = mcu_uart_working || module_uart_working;
	return module_task_busy;
}

/**
 * @brief		exit suspend mode
 * @param[in]	none
 * @return      none
 */
void app_suspend_exit ()
{
	GPIO_WAKEUP_MODULE_HIGH;  //module enter working state
	bls_pm_setSuspendMask(SUSPEND_DISABLE);
	tick_wakeup = clock_time () | 1;
}

/**
 * @brief		enter suspend mode
 * @param[in]	none
 * @return      0 - forbidden enter suspend mode
 *              1 - allow enter suspend mode
 */
int app_suspend_enter ()
{
	if (app_module_busy ())
	{
		app_suspend_exit ();
		return 0;
	}
	return 1;
}

/**
 * @brief      power management code for application
 * @param[in]  none
 * @return     none
 */
void app_power_management ()
{
#if (BLE_APP_PM_ENABLE)

	if (!app_module_busy() && !tick_wakeup)
	{
		#if (PM_DEEPSLEEP_RETENTION_ENABLE)
			bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
		#else
			bls_pm_setSuspendMask(SUSPEND_ADV | SUSPEND_CONN);
		#endif

		bls_pm_setWakeupSource(PM_WAKEUP_PAD);  // GPIO_WAKEUP_MODULE needs to be wakened
	}

	if (tick_wakeup && clock_time_exceed (tick_wakeup, 500))
	{
		GPIO_WAKEUP_MODULE_LOW;
		tick_wakeup = 0;
	}

#endif
}

/////////////////////////////////////blc_register_hci_handler for spp////////////////////////////

/**
 * @brief		this function is used to process rx uart data.
 * @param[in]	none
 * @return      0 is ok
 */
int rx_from_uart_cb (void)//UART data send to Master,we will handler the data as CMD or DATA
{
	if(my_fifo_get(&hci_rx_fifo) == 0)
	{
		return 0;
	}

	u8* p = my_fifo_get(&hci_rx_fifo);
	u32 rx_len = p[0]; //usually <= 255 so 1 byte should be sufficient

	if (rx_len)
	{
		blc_hci_handler(&p[4], rx_len - 4);
		my_fifo_pop(&hci_rx_fifo);
	}

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
	if(hci_tx_fifo.wptr == hci_tx_fifo.rptr){
		return 0;//have no data
	}
	if (Tr_isUartTxDone()) {
		return 0;
	}
	u8 *p =  my_fifo_get(&hci_tx_fifo);
	memcpy(&T_txdata_buf.data, p + 2, p[0] | (p[1] << 8));

	T_txdata_buf.len = p[0] | (p[1] << 8);
	Tr_clrUartTxDone();
	uart_send_dma(UART_CONVERT((unsigned char*) (&T_txdata_buf)));
	my_fifo_pop(&hci_tx_fifo);
	return 1;
}


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
	//	debug init
	#if(UART_PRINT_DEBUG_ENABLE)
		tlkapi_debug_init();
		blc_debug_enableStackLog(STK_LOG_DISABLE);
	#endif

	blc_readFlashSize_autoConfigCustomFlashSector();

	/* attention that this function must be called after "blc_readFlashSize_autoConfigCustomFlashSector" !!!*/
	blc_app_loadCustomizedParameters_normal();

////////////////// BLE stack initialization ////////////////////////////////////
	u8  mac_public[6];
	u8  mac_random_static[6];   //this is not available for HCI controller, cause host will set random address to it
	//for 512K Flash, flash_sector_mac_address equals to 0x76000
	//for 1M  Flash, flash_sector_mac_address equals to 0xFF000
	blc_initMacAddress(flash_sector_mac_address, mac_public, mac_random_static);

	////// Controller Initialization  //////////
	blc_ll_initBasicMCU();                      //mandatory
	blc_ll_initStandby_module(mac_public);				//mandatory

	#if (EXTENDED_ADV_ENABLE)
		blc_ll_initExtendedAdvertising_module(app_adv_set_param, app_primary_adv_pkt, APP_ADV_SETS_NUMBER);
		blc_ll_initExtSecondaryAdvPacketBuffer(app_secondary_adv_pkt, MAX_LENGTH_SECOND_ADV_PKT);
		blc_ll_initExtAdvDataBuffer(app_advData, APP_MAX_LENGTH_ADV_DATA);
		blc_ll_initExtScanRspDataBuffer(app_scanRspData, APP_MAX_LENGTH_SCAN_RESPONSE_DATA);
	#else
		blc_ll_initAdvertising_module(mac_public); 	//ADV module: 		 mandatory for BLE slave,
	#endif
	blc_ll_initConnection_module();					//connection module  mandatory for BLE slave/master
	blc_ll_initSlaveRole_module();					//slave module: 	 mandatory for BLE slave,

	////// Host Initialization  //////////
	blc_l2cap_register_handler (blc_hci_sendACLData2Host);  	//l2cap initialization


	///////////////////// USER application initialization ///////////////////
	bls_ll_setAdvEnable(BLC_ADV_DISABLE);  //ADV enable


	//blc_ll_initChannelSelectionAlgorithm_2_feature();

	rf_set_power_level_index (RF_POWER_P3dBm);

	////////////////// SPP initialization ///////////////////////////////////
	#if (HCI_ACCESS==HCI_USE_USB)
		usb_bulk_drv_init (0);
		blc_register_hci_handler (blc_hci_rx_from_usb, blc_hci_tx_to_usb);

		usb_dp_pullup_en (1);  //open USB enum
	#else	//uart
		//note: dma addr must be set first before any other uart initialization!
		uart_recbuff_init(UART_CONVERT((unsigned char *)hci_rx_fifo_b, hci_rx_fifo.size));
		uart_gpio_set(UART_CONVERT(UART_TX_PIN, UART_RX_PIN));
		uart_reset(UART_NUM);  //will reset uart digital registers from 0x90 ~ 0x9f, so uart setting must set after this reset

		//baud rate: 115200
		uart_init_baudrate(UART_CONVERT(UART__BAUDRATE, CLOCK_SYS_CLOCK_HZ, PARITY_NONE, STOP_BIT_ONE));

		uart_dma_enable(UART_CONVERT(1, 1)); 	//uart data in hardware buffer moved by dma, so we need enable them first
		irq_set_mask(FLD_IRQ_DMA_EN);
		dma_chn_irq_enable(FLD_DMA_CHN_UART_RX, 1); //UART RX DMA IRQ enable
		uart_mask_tx_done_irq_enable(UART_NUM);
		uart_mask_error_irq_enable(UART_NUM);// open uart_error_mask,when stop bit error or parity error,it will enter error_interrupt.
		irq_enable_type(FLD_IRQ_UART_EN);// uart_tx use uart_txdone irq
		irq_enable();

		#if (MCU_CORE_TYPE != MCU_CORE_TC321X)
			uart_clr_tx_done();
		#endif

		isUartTxDone = 1;

		extern int rx_from_uart_cb (void);
		extern int tx_to_uart_cb (void);
		blc_register_hci_handler(rx_from_uart_cb, tx_to_uart_cb);				//customized uart handler
	#endif

	extern int event_handler(u32 h, u8 *para, int n);
	blc_hci_registerControllerEventHandler(blc_hci_send_data);		//register event callback


#if (BLE_APP_PM_ENABLE)
	blc_ll_initPowerManagement_module();        //pm module:      	 optional

	bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);


	//mcu can wake up module from suspend or deepsleep by pulling up GPIO_WAKEUP_MODULE
	cpu_set_gpio_wakeup (GPIO_WAKEUP_MODULE, Level_High, 1);  // pad high wakeup deepsleep

	GPIO_WAKEUP_MODULE_LOW;

	bls_pm_registerFuncBeforeSuspend( &app_suspend_enter );
#else
	bls_pm_setSuspendMask (SUSPEND_DISABLE);
#endif

	/* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done!!! */
	u32 error_code1 = blc_contr_checkControllerInitialization();
	if(error_code1 != INIT_SUCCESS){
		/* It's recommended that user set some UI alarm to know the exact error, e.g. LED shine, print log */
		#if (UART_PRINT_DEBUG_ENABLE)
			tlkapi_printf(APP_LOG_EN, "[APP][INI] Stack INIT ERROR 0x%04x", error_code1);
		#endif

		#if (UI_LED_ENABLE)
			gpio_write(GPIO_LED, LED_ON_LEVEL);
		#endif
		while(1);
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

	//  add spp UI task
	app_power_management ();
}

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
#include "app_att.h"
#include "spp.h"
#include "battery_check.h"

//module spp Tx / Rx fifo
//MYFIFO_INIT(spp_rx_fifo, 72, 2);
//MYFIFO_INIT(spp_tx_fifo, 72, 8);


_attribute_data_retention_  u8 		 	spp_rx_fifo_b[SPP_RXFIFO_SIZE * SPP_RXFIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	spp_rx_fifo = {
												SPP_RXFIFO_SIZE,
												SPP_RXFIFO_NUM,
												0,
												0,
												spp_rx_fifo_b,};

_attribute_data_retention_  u8 		 	spp_tx_fifo_b[SPP_TXFIFO_SIZE * SPP_TXFIFO_NUM] = {0};
_attribute_data_retention_	my_fifo_t	spp_tx_fifo = {
												SPP_TXFIFO_SIZE,
												SPP_TXFIFO_NUM,
												0,
												0,
												spp_tx_fifo_b,};



//RF Tx / Rx fifo
//MYFIFO_INIT(blt_rxfifo, 64, 8);
//MYFIFO_INIT(blt_txfifo, 40, 16);


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















#define     MY_APP_ADV_CHANNEL					BLT_ENABLE_ADV_37
#define 	MY_ADV_INTERVAL_MIN					ADV_INTERVAL_30MS
#define 	MY_ADV_INTERVAL_MAX					ADV_INTERVAL_40MS

#define 	MY_DIRECT_ADV_TMIE					2000000  //us

#define		MY_RF_POWER_INDEX					RF_POWER_P3dBm

#define		BLE_DEVICE_ADDRESS_TYPE 			BLE_DEVICE_ADDRESS_PUBLIC

_attribute_data_retention_	own_addr_type_t 	app_own_address_type = OWN_ADDRESS_PUBLIC;




/**
 * @brief	Adv Packet data
 */
const u8	tbl_advData[] = {
	 0x05, 0x09, 'k', 'M', 'o', 'd',
	 0x02, 0x01, 0x05, 							// BLE limited discoverable mode and BR/EDR not supported
};
/**
 * @brief	Scan Response Packet data
 */
const u8	tbl_scanRsp [] = {
		 0x07, 0x09, 'k', 'M', 'o', 'd', 'u', 'l',	//scan name " tmodul"
	};


_attribute_data_retention_	u8 	ui_ota_is_working = 0;



_attribute_data_retention_ u8 conn_update_cnt;

/**
 * @brief		call back function of connect parameter update response
 * @param[in]	id - connect id
 * @param[in]	result - connect parameter update response result
 * @return      0    - success
 *              else - reserved
 */
int app_conn_param_update_response(u8 id, u16  result)
{
    (void)id;(void)result;
#if 0
	if(result == CONN_PARAM_UPDATE_ACCEPT){
		tlkapi_printf("SIG: the LE master Host has accepted the connection parameters.\n");
		conn_update_cnt = 0;
	}
	else if(result == CONN_PARAM_UPDATE_REJECT)
	{
		tlkapi_printf("SIG: the LE master Host has rejected the connection parameters..\n");
		tlkapi_printf("Current Connection interval:%dus.\n", bls_ll_getConnectionInterval() * 1250 );
		conn_update_cnt++;
		if(conn_update_cnt < 4){
			tlkapi_printf("Slave sent update connPara req!\n");
		}
		if(conn_update_cnt == 1){
			bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_10MS, CONN_INTERVAL_20MS, 0, CONN_TIMEOUT_4S); //18.75ms iOS
		}
		else if(conn_update_cnt == 2){
			bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_20MS, CONN_INTERVAL_40MS, 0, CONN_TIMEOUT_4S);
		}
		else if(conn_update_cnt == 3){
			bls_l2cap_requestConnParamUpdate (CONN_INTERVAL_40MS, CONN_INTERVAL_75MS, 0, CONN_TIMEOUT_4S);
		}
		else{
			conn_update_cnt = 0;
			tlkapi_printf("Slave Connection Parameters Update table all tested and failed!\n");
		}
	}
#endif

	return 0;
}



#if (BLE_OTA_ENABLE)

/**
 * @brief		callback function of ota start
 * @param[in]	none
 * @return      none
 */
void entry_ota_mode(void)
{
	bls_ota_setTimeout(15 * 1000 * 1000); //set OTA timeout  15 seconds

	#if(UI_LED_ENABLE)
		gpio_set_output_en(GPIO_LED_RED, 1);  //output enable
		gpio_write(GPIO_LED_RED, 1);  //LED on for indicate OTA mode
	#endif
}

/**
 * @brief		callback function of ota result
 * @param[in]	none
 * @return      none
 */
void show_ota_result(int result)
{

	#if(1 && UI_LED_ENABLE)
		gpio_set_output_en(GPIO_LED_RED, 1);

		if(result == OTA_SUCCESS){  //OTA success
			gpio_write(GPIO_LED_RED, 1);
			sleep_us(500000);
			gpio_write(GPIO_LED_RED, 0);
			sleep_us(500000);
			gpio_write(GPIO_LED_RED, 1);
			sleep_us(500000);
			gpio_write(GPIO_LED_RED, 0);
			sleep_us(500000);
		}
		else{  //OTA fail

			#if 0 //this is only for debug,  can not use this in application code
				irq_disable();
				WATCHDOG_DISABLE;


				while(1){
					gpio_write(GPIO_LED_RED, 1);
					sleep_us(200000);
					gpio_write(GPIO_LED_RED, 0);
					sleep_us(200000);
				}

			#endif

		}

		gpio_set_output_en(GPIO_LED_RED, 0);
	#endif
}
#endif


#define MAX_INTERVAL_VAL		16






_attribute_data_retention_	u32 tick_wakeup;
_attribute_data_retention_	int	mcu_uart_working;
_attribute_data_retention_	int	module_uart_working;
_attribute_data_retention_	int module_task_busy;


_attribute_data_retention_	int	module_uart_data_flg;
_attribute_data_retention_	u32 module_wakeup_module_tick;

#define UART_TX_BUSY			( (spp_tx_fifo.rptr != spp_tx_fifo.wptr) || Tr_isUartTxDone() )
#define UART_RX_BUSY			(spp_rx_fifo.rptr != spp_rx_fifo.wptr)

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
	rf_set_power_level_index (MY_RF_POWER_INDEX);
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

#if (BLE_MODULE_INDICATE_DATA_TO_MCU)
	module_uart_working = UART_TX_BUSY || UART_RX_BUSY;

	//When module's UART data is sent, the GPIO_WAKEUP_MCU is lowered or suspended (depending on how the user is designed)
	if(module_uart_data_flg && !module_uart_working){
		module_uart_data_flg = 0;
		module_wakeup_module_tick = 0;
		GPIO_WAKEUP_MCU_LOW;
	}
#endif

	// pullup GPIO_WAKEUP_MODULE: exit from suspend
	// pulldown GPIO_WAKEUP_MODULE: enter suspend

#if (BLE_APP_PM_ENABLE)

	if (!app_module_busy() && !tick_wakeup)
	{
		#if (PM_DEEPSLEEP_RETENTION_ENABLE)
	    	blc_app_setDeepsleepRetentionSramSize(); //select DEEPSLEEP_MODE_RET_SRAM_LOW16K or DEEPSLEEP_MODE_RET_SRAM_LOW32K
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

#if (APP_BATT_CHECK_ENABLE)  //battery check must do before OTA relative operation
/**
 * @brief		callback function of adjust whether allow enter to pm or not
 * @param[in]	none
 * @return      0 forbidden enter cpu_sleep_wakeup, 1 allow enter cpu_sleep_wakeup
 */
int app_suspend_enter_low_battery (void)
{
	if (!gpio_read(GPIO_WAKEUP_MODULE)) //gpio low level
	{
		analog_write(USED_DEEP_ANA_REG,  analog_read(USED_DEEP_ANA_REG) | LOW_BATT_FLG);  //mark
		return 1;//allow enter cpu_sleep_wakeup
	}

	analog_write(USED_DEEP_ANA_REG,  analog_read(USED_DEEP_ANA_REG) & (~LOW_BATT_FLG));  //clr
	return 0; //forbidden enter cpu_sleep_wakeup
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
	//	debug init
	#if(UART_PRINT_DEBUG_ENABLE)
		tlkapi_debug_init();
		blc_debug_enableStackLog(STK_LOG_DISABLE);
	#endif

	blc_readFlashSize_autoConfigCustomFlashSector();

	/* attention that this function must be called after "blc readFlashSize_autoConfigCustomFlashSector" !!!*/
	blc_app_loadCustomizedParameters_normal();
	/*****************************************************************************************
	 Note: battery check must do before any flash write/erase operation, cause flash write/erase
		   under a low or unstable power supply will lead to error flash operation

		   Some module initialization may involve flash write/erase, include: OTA initialization,
				SMP initialization, ..
				So these initialization must be done after  battery check

	   Attention that this function must be called after "blc_app_loadCustomizedParameters_normal" !!!
	   The reason is that the low battery check need the ADC calibration parameter, and this parameter
	   is loaded in blc_app_loadCustomizedParameters_normal.
	*****************************************************************************************/
	#if (APP_BATT_CHECK_ENABLE)
	/*The SDK must do a quick low battery detect during user initialization instead of waiting
	  until the main_loop. The reason for this process is to avoid application errors that the device
	  has already working at low power.
	  Considering the working voltage of MCU and the working voltage of flash, if the Demo is set below 2.0V,
	  the chip will alarm and deep sleep, and once the chip is detected to be lower than 2.0V, it needs to wait
	  until the voltage rises to 2.2V,
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
	//for 1M  Flash, flash_sector_mac_address equals to 0xFF000
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




	////// Host Initialization  //////////
	blc_gap_peripheral_init();    //gap initialization
	blc_l2cap_register_handler (blc_l2cap_packet_receive);  	//l2cap initialization
	blc_l2cap_registerConnUpdateRspCb(app_conn_param_update_response);         //register sig process handler
	my_att_init(); //gatt initialization
	blc_att_setRxMtuSize(MTU_SIZE_SETTING); //set MTU size, default MTU is 23 if not call this API

	//Smp Initialization may involve flash write/erase(when one sector stores too much information,
	//   is about to exceed the sector threshold, this sector must be erased, and all useful information
	//   should re_stored) , so it must be done after battery check
	#if (BLE_SECURITY_ENABLE)
		/* attention: If this API is used, must be called before "blc smp_peripheral_init" when initialization !!! */
		bls_smp_configPairingSecurityInfoStorageAddr(flash_sector_smp_storage);
		//default smp4.0, just work
		blc_smp_peripheral_init();

		#if (0) //default close
			//host(GAP/SMP/GATT/ATT) event process: register host event callback and set event mask
			blc_gap_registerHostEventHandler( app_host_event_callback );
			blc_gap_setEventMask( GAP_EVT_MASK_SMP_PARING_BEAGIN 			|  \
								  GAP_EVT_MASK_SMP_PARING_SUCCESS   		|  \
								  GAP_EVT_MASK_SMP_PARING_FAIL				|  \
								  GAP_EVT_MASK_SMP_CONN_ENCRYPTION_DONE );
		#endif
	#else
		blc_smp_setSecurityLevel(No_Security);
	#endif



	///////////////////// USER application initialization ///////////////////

	bls_ll_setAdvData( (u8 *)tbl_advData, sizeof(tbl_advData) );
	bls_ll_setScanRspData( (u8 *)tbl_scanRsp, sizeof(tbl_scanRsp));



	////////////////// config ADV packet /////////////////////
	u8 adv_param_status = BLE_SUCCESS;
	adv_param_status = bls_ll_setAdvParam( MY_ADV_INTERVAL_MIN, MY_ADV_INTERVAL_MAX,
								 ADV_TYPE_CONNECTABLE_UNDIRECTED, app_own_address_type,
								 0,  NULL,
								 MY_APP_ADV_CHANNEL,
								 ADV_FP_NONE);

	if(adv_param_status != BLE_SUCCESS) {  //debug: ADV setting err
		tlkapi_printf(APP_LOG_EN, "[APP][INI] ADV parameters error 0x%x!!!\n", adv_param_status);
		while(1);
	}


	bls_ll_setAdvEnable(BLC_ADV_ENABLE);  //ADV enable
	rf_set_power_level_index (MY_RF_POWER_INDEX);






	////////////////// SPP initialization ///////////////////////////////////
	//note: dma addr must be set first before any other uart initialization!
	u8 *uart_rx_addr = (spp_rx_fifo_b + (spp_rx_fifo.wptr & (spp_rx_fifo.num-1)) * spp_rx_fifo.size);
	uart_recbuff_init(UART_CONVERT((unsigned char *)uart_rx_addr, spp_rx_fifo.size));

	uart_gpio_set(UART_CONVERT(UART_TX_PIN, UART_RX_PIN));

	uart_reset(UART_MODULE_SEL);  //will reset uart digital registers from 0x90 ~ 0x9f, so uart setting must set after this reset

	//baud rate: 115200
	uart_init_baudrate(UART_CONVERT(UART_BAUD_RATE, CLOCK_SYS_CLOCK_HZ, PARITY_NONE, STOP_BIT_ONE));

	uart_dma_enable(UART_CONVERT(1, 1)); 	//uart data in hardware buffer moved by dma, so we need enable them first

	irq_set_mask(FLD_IRQ_DMA_EN);
	dma_chn_irq_enable(FLD_DMA_CHN_UART_RX, 1);

	uart_mask_tx_done_irq_enable(UART_MODULE_SEL);
	uart_mask_error_irq_enable(UART_MODULE_SEL);
	irq_enable_type(FLD_IRQ_UART_EN);

#if (MCU_CORE_TYPE != MCU_CORE_TC321X)
	uart_clr_tx_done();
#endif

	Tr_SetUartTxDone();
	extern int rx_from_uart_cb (void);
	extern int tx_to_uart_cb (void);
	blc_register_hci_handler(rx_from_uart_cb, tx_to_uart_cb);				//customized uart handler


	extern int controller_event_handler(u32 h, u8 *para, int n);
	blc_hci_registerControllerEventHandler(controller_event_handler);		//register event callback
	bls_hci_mod_setEventMask_cmd(0xfffff);			//enable all 18 events,event list see ll.h





#if (BLE_APP_PM_ENABLE)
	blc_ll_initPowerManagement_module();        //pm module:      	 optional

	#if (PM_DEEPSLEEP_RETENTION_ENABLE)
    	blc_app_setDeepsleepRetentionSramSize(); //select DEEPSLEEP_MODE_RET_SRAM_LOW16K or DEEPSLEEP_MODE_RET_SRAM_LOW32K
		bls_pm_setSuspendMask (SUSPEND_ADV | DEEPSLEEP_RETENTION_ADV | SUSPEND_CONN | DEEPSLEEP_RETENTION_CONN);
		blc_pm_setDeepsleepRetentionThreshold(95, 95);
		blc_pm_setDeepsleepRetentionEarlyWakeupTiming(250);
	#else
		bls_pm_setSuspendMask (SUSPEND_ADV | SUSPEND_CONN);
	#endif


	//mcu can wake up module from suspend or deepsleep by pulling up GPIO_WAKEUP_MODULE
	cpu_set_gpio_wakeup (GPIO_WAKEUP_MODULE, Level_High, 1);  // pad high wakeup deepsleep

	GPIO_WAKEUP_MODULE_LOW;

	bls_pm_registerFuncBeforeSuspend( &app_suspend_enter );
#else
	bls_pm_setSuspendMask (SUSPEND_DISABLE);
#endif



#if (BLE_OTA_ENABLE)
	// OTA init
	blc_ota_initOtaServer_module(); //must
	bls_ota_registerStartCmdCb(entry_ota_mode);
	bls_ota_registerResultIndicateCb(show_ota_result);
#endif

	/* Check if any Stack(Controller & Host) Initialization error after all BLE initialization done.
	 * attention that code will stuck in "while(1)" if any error detected in initialization, user need find what error happens and then fix it */
	blc_app_checkControllerHostInitialization();

	tlkapi_printf(APP_LOG_EN, "[APP][INI] BLE module init done! \n");

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


	////////////////// SPP initialization ///////////////////////////////////
	//note: dma addr must be set first before any other uart initialization!

	u8 *uart_rx_addr = (spp_rx_fifo_b + (spp_rx_fifo.wptr & (spp_rx_fifo.num-1)) * spp_rx_fifo.size);
	uart_recbuff_init(UART_CONVERT((unsigned char *)uart_rx_addr, spp_rx_fifo.size));


	uart_gpio_set(UART_CONVERT(UART_TX_PIN, UART_RX_PIN));

	uart_reset(UART_MODULE_SEL);  //will reset uart digital registers from 0x90 ~ 0x9f, so uart setting must set after this reset


	DBG_CHN0_LOW;  //debug

	//baud rate: 115200
	uart_init_baudrate(UART_CONVERT(UART_BAUD_RATE, CLOCK_SYS_CLOCK_HZ, PARITY_NONE, STOP_BIT_ONE));

	uart_dma_enable(UART_CONVERT(1, 1)); 	//uart data in hardware buffer moved by dma, so we need enable them first

	irq_set_mask(FLD_IRQ_DMA_EN);
	dma_chn_irq_enable(FLD_DMA_CHN_UART_RX, 1);

	uart_mask_tx_done_irq_enable(UART_MODULE_SEL);
	uart_mask_error_irq_enable(UART_MODULE_SEL);
	irq_enable_type(FLD_IRQ_UART_EN);

#if (MCU_CORE_TYPE != MCU_CORE_TC321X)
	uart_clr_tx_done();
#endif

	Tr_SetUartTxDone();

	//mcu can wake up module from suspend or deepsleep by pulling up GPIO_WAKEUP_MODULE
	cpu_set_gpio_wakeup (GPIO_WAKEUP_MODULE, Level_High, 1);  // pad high wakeup deepsleep

	GPIO_WAKEUP_MODULE_LOW;




	DBG_CHN0_HIGH;   //debug
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
		#if (BLE_OTA_ENABLE)
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
#if (BLE_OTA_ENABLE)
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

#if (APP_BATT_CHECK_ENABLE)
	/*The frequency of low battery detect is controlled by the variable lowBattDet_tick, which is executed every
	 500ms in the demo. Users can modify this time according to their needs.*/
	if(battery_get_detect_enable() && clock_time_exceed(lowBattDet_tick, 500000) ){
		lowBattDet_tick = clock_time();
		user_battery_power_check(VBAT_DEEP_THRES_MV);
	}
#endif


	//  add spp UI task
	app_power_management ();


	spp_restart_proc();
}

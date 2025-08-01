/********************************************************************************************************
 * @file    app_tl_prx.c
 *
 * @brief   This is the source file for 2.4G SDK
 *
 * @author  2.4G GROUP
 * @date    02,2025
 *
 * @par     Copyright (c) 2025, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "stack/2p4g/tl_tpll/tl_tpll.h"
#include "app_config.h"
#include "app.h"
#include "battery_check.h"
#if (TPLL_MODE == TL_TPLL_PRX)
trf_tpll_payload_t ack_payload = TRF_TPLL_CREATE_PAYLOAD(TRF_TPLL_PIPE0,
                                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                                0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20);
trf_tpll_payload_t rx_payload;

volatile unsigned int irq_cnt_invalid_pid = 0;
volatile unsigned int irq_cnt_tx_ds       = 0;
volatile unsigned int irq_cnt_tx          = 0;
volatile unsigned int irq_cnt_rx          = 0;


//extern void trf_tpll_event_handler(trf_tpll_event_id_t evt_id);
static trf_tpll_event_handler_t m_event_handler;
unsigned char err_code          = 0;
unsigned int  evt_tx_finish_cnt = 0;
unsigned int  evt_tx_failed_cnt = 0;
unsigned int  evt_rx_cnt        = 0;


_attribute_ram_code_  void irq_2p4g_sdk_handler(void)
{
  unsigned short src_rf = rf_irq_src_get();
  m_event_handler = trf_tpll_get_event_handler();
  if (src_rf & FLD_RF_IRQ_RX)
  {
      rf_irq_clr_src(FLD_RF_IRQ_RX);
      trf_tpll_rxirq_handler(m_event_handler);
      irq_cnt_rx++;
  }
  if (src_rf & FLD_RF_IRQ_TX)
  {
      rf_irq_clr_src(FLD_RF_IRQ_TX);
      m_event_handler(TRF_TPLL_EVENT_TX_FINISH);
      irq_cnt_tx++;
  }
  if (src_rf & FLD_RF_IRQ_TX_DS)
  {
      rf_irq_clr_src(FLD_RF_IRQ_TX_DS);
      irq_cnt_tx_ds++;
  }
  if (src_rf & FLD_RF_IRQ_INVALID_PID)
  {
      rf_irq_clr_src(FLD_RF_IRQ_INVALID_PID);
      irq_cnt_invalid_pid++;
  }
  irq_clr_src();
  rf_irq_clr_src(FLD_RF_IRQ_ALL);
}

__attribute__((section(".ram_code")))__attribute__((optimize("-Os")))
void trf_tpll_event_handler(trf_tpll_event_id_t evt_id)
{
    switch (evt_id)
    {
    case TRF_TPLL_EVENT_TX_FINISH:
        #if (UI_LED_ENABLE)
        gpio_toggle(GPIO_LED_GREEN);
        #endif
        evt_tx_finish_cnt++;
        break;
    case TRF_TPLL_EVENT_TX_FALIED:
        evt_tx_failed_cnt++;
        break;
    case TRF_TPLL_EVENT_RX_RECEIVED:
        trf_tpll_read_rx_payload(&rx_payload);
        #if (UI_LED_ENABLE)
        gpio_toggle(GPIO_LED_WHITE);
        #endif
        trf_tpll_flush_tx(ack_payload.pipe_id); // it is necessary in this demo
        ack_payload.data[2] = rx_payload.data[2];
        if (TRF_SUCCESS == trf_tpll_write_payload(&ack_payload))
        {

        }
        else
        {
            #if (UI_LED_ENABLE)
            gpio_toggle(GPIO_LED_RED);
            #endif
        }
        evt_rx_cnt++;
        break;
    }
}

unsigned char tpll_config_init(void)
{
    unsigned char err_code = 0;
    unsigned char base_address_0[4] = {0xe7, 0xe7, 0xe7, 0xe7};
    unsigned char base_address_1[4] = {0xc2, 0xc2, 0xc2, 0xc2};
    unsigned char addr_prefix[6] = {0xe7, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6};

    trf_tpll_config_t trf_tpll_config = TRF_TPLL_DEFALT_CONFIG;
    trf_tpll_config.mode             = TRF_TPLL_MODE_PRX;
    trf_tpll_config.bitrate          = TRF_TPLL_BITRATE_2MBPS;
    trf_tpll_config.crc              = TRF_TPLL_CRC_16BIT;
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    trf_tpll_config.tx_power         = RF_POWER_P0p00dBm;
#elif(MCU_CORE_TYPE == MCU_CORE_825x)
    trf_tpll_config.tx_power         = RF_POWER_P0p04dBm;
#elif(MCU_CORE_TYPE == MCU_CORE_827x)
    trf_tpll_config.tx_power         = RF_POWER_P0p52dBm;
#endif
    trf_tpll_config.event_handler    = trf_tpll_event_handler;
    trf_tpll_config.preamble_len     = 2;
    trf_tpll_config.payload_len      = 32;

    err_code = trf_tpll_init(&trf_tpll_config);
    // init irq
    irq_clr_src();
    rf_irq_clr_src(FLD_RF_IRQ_ALL);
    irq_enable_type(FLD_IRQ_ZB_RT_EN);
    rf_irq_disable(FLD_RF_IRQ_ALL);
    rf_irq_enable(FLD_RF_IRQ_TX | FLD_RF_IRQ_TX_DS |
#if (MCU_CORE_TYPE == MCU_CORE_TC321X)
    FLD_RF_IRQ_INVALID_PID | FLD_RF_IRQ_TX_RETRYCNT | FLD_RF_IRQ_RX);
#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
    FLD_RF_IRQ_INVALID_PID | FLD_RF_IRQ_RETRY_HIT | FLD_RF_IRQ_RX);
#endif
    irq_enable();

    TRF_RETVAL_CHECK((err_code == TRF_SUCCESS));

    trf_tpll_set_address_width(TRF_TPLL_ADDRESS_WIDTH_5BYTES);

    err_code = trf_tpll_set_base_address_0(base_address_0);
    TRF_RETVAL_CHECK((err_code == TRF_SUCCESS));

    err_code = trf_tpll_set_base_address_1(base_address_1);
    TRF_RETVAL_CHECK((err_code == TRF_SUCCESS));

    err_code = trf_tpll_set_prefixes(addr_prefix, 6);
    TRF_RETVAL_CHECK((err_code == TRF_SUCCESS));

    trf_tpll_set_txpipe(TRF_TPLL_PIPE0);

    trf_tpll_set_rf_channel(5);
    return err_code;
}

/**
 * @brief      power management code for application
 * @param	   none
 * @return     none
 */
void blt_pm_proc(void)
{


}
/**
 * @brief		user initialization when MCU power on or wake_up from deepSleep mode
 * @param[in]	none
 * @return      none
 */
_attribute_no_inline_ void user_init_normal(void)
{
//////////////////////////// basic hardware Initialization  Begin //////////////////////////////////
	//	debug init
	#if(UART_PRINT_DEBUG_ENABLE)
		tlkapi_debug_init();
	#endif

	blc_readFlashSize_autoConfigCustomFlashSector();

	/* attention that this function must be called after "blc_readFlashSize_autoConfigCustomFlashSector" !!!*/
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
//////////////////////////// basic hardware Initialization  End //////////////////////////////////


//////////////////////////// 2P4G stack Initialization  Begin //////////////////////////////////
    err_code = tpll_config_init();
    TRF_RETVAL_CHECK((err_code == TRF_SUCCESS));
    trf_tpll_enable_crcfilter(1);
//////////////////////////// 2P4G stack Initialization  End //////////////////////////////////


//////////////////////////// User Configuration for 2P4G application ////////////////////////////


	///////////////////// Power Management initialization///////////////////
	#if(APP_PM_ENABLE)

	#else

	#endif
///////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////// User 2P4G application /////////////////////////////////////
    trf_tpll_start_rx();
    ack_payload.data[1] = 0xbc; // for debug
////////////////////////////////////////////////////////////////////////////////////////////////
}

/**
 * @brief		user initialization when MCU wake_up from deepSleep_retention mode
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ void user_init_deepRetn(void)
{
#if (APP_PM_DEEPSLEEP_RETENTION_ENABLE)

	blc_app_loadCustomizedParameters_deepRetn();

	#if (APP_BATT_CHECK_ENABLE)
		/* ADC settings will lost during deepsleep retention mode, so here need clear flag */
		battery_clear_adc_setting_flag();
	#endif

	DBG_CHN0_HIGH;    //debug

	irq_enable();


#endif
}

///////////////////////////////main loop flow//////////////////////////////////////s
_attribute_ram_code_ void sdk_2p4g_main_loop(void)
{

}
/////////////////////////////////////////////////////////////////////

/**
 * @brief		This is main_loop function
 * @param[in]	none
 * @return      none
 */
_attribute_no_inline_ void main_loop(void)
{
////////////////////////////////////// 2P4G entry /////////////////////////////////
    sdk_2p4g_main_loop();

////////////////////////////////////// UI entry ////////////////////////////////////

///////////////////////////////////// Battery Check ////////////////////////////////
    #if (APP_BATT_CHECK_ENABLE)
    /*The frequency of low battery detect is controlled by the variable lowBattDet_tick, which is executed every
     500ms in the demo. Users can modify this time according to their needs.*/
    if(battery_get_detect_enable() && clock_time_exceed(lowBattDet_tick, 500000) ){
        lowBattDet_tick = clock_time();
        user_battery_power_check(VBAT_DEEP_THRES_MV);
    }
    #endif

    #if (UI_KEYBOARD_ENABLE)
    proc_keyboard(0, 0, 0);
    #elif (UI_BUTTON_ENABLE)
    /* process button 1 second later after power on, to avoid power unstable */
    if(!button_detect_en && clock_time_exceed(0, 1000000)){
        button_detect_en = 1;
    }
    if(button_detect_en && clock_time_exceed(button_detect_tick, 5000))
    {
        button_detect_tick = clock_time();
        proc_button(0, 0, 0);  //button triggers OTA
    }
    #endif
////////////////////////////////////// PM Process /////////////////////////////////
	blt_pm_proc();
}
#endif

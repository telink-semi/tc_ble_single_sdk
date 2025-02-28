/********************************************************************************************************
 * @file    app_tx.c
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
#include "stack/2p4g/genfsk_ll/genfsk_ll.h"
#include "app_config.h"
#include "app.h"
#include "battery_check.h"
#if (GENFSK_LL_MODE == GENFSK_TX)
unsigned char tx_payload[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
static unsigned char __attribute__ ((aligned (4)))         tx_buffer[64] = {0};
volatile static unsigned char                                 tx_done_flag = 0;
volatile unsigned int                                               tx_cnt = 0;
static volatile unsigned int                                        irq_cnt_tx;


_attribute_ram_code_  void irq_2p4g_sdk_handler(void)
{
  unsigned int irq_src = irq_get_src();
  unsigned short rf_irq_src = rf_irq_src_get();

  if (irq_src & FLD_IRQ_ZB_RT_EN) // if rf irq occurs
  {
      if (rf_irq_src & FLD_RF_IRQ_TX) // if rf tx irq occurs
      {
          tx_done_flag = 1;
          irq_cnt_tx++;
      }
      rf_irq_clr_src(FLD_RF_IRQ_ALL);
  }
  irq_clr_sel_src(FLD_IRQ_ALL);
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
    unsigned char sync_word[4] = {0x53, 0x78, 0x56, 0x52};
    // it needs to notice that this api is different from vulture / kite
    gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS); //Note that this API must be invoked first before all other APIs
    gen_fsk_preamble_len_set(4);
    gen_fsk_sync_word_len_set(SYNC_WORD_LEN_4BYTE);
    gen_fsk_sync_word_set(GEN_FSK_PIPE0, sync_word); //set pipe0's sync word
    gen_fsk_pipe_open(GEN_FSK_PIPE0); // enable pipe0's reception
    gen_fsk_packet_format_set(GEN_FSK_PACKET_FORMAT_FIXED_PAYLOAD, sizeof(tx_payload));
    gen_fsk_radio_power_set(GEN_FSK_RADIO_POWER_P0p00dBm);
    gen_fsk_channel_set(7);
    gen_fsk_radio_state_set(GEN_FSK_STATE_TX); // set transceiver to basic TX state
    gen_fsk_tx_settle_set(149);
    WaitUs(149); // wait for tx settle

    rf_irq_enable(FLD_RF_IRQ_TX); // enable rf tx irq
    irq_enable_type(FLD_IRQ_ZB_RT_EN); // enable RF irq
    irq_enable(); // enable general irq
//////////////////////////// 2P4G stack Initialization  End //////////////////////////////////


//////////////////////////// User Configuration for 2P4G application ////////////////////////////
	    //fill the dma info in tx buffer
	    tx_buffer[0] = sizeof(tx_payload);
	    tx_buffer[1] = 0x00;
	    tx_buffer[2] = 0x00;
	    tx_buffer[3] = 0x00;
	    memcpy(&tx_buffer[4], tx_payload, sizeof(tx_payload));
	///////////////////// Power Management initialization///////////////////
	#if(APP_PM_ENABLE)

	#else

	#endif

////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////// User 2P4G application /////////////////////////////////////

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
    tx_done_flag = 0;
    gen_fsk_tx_start(tx_buffer); // start the Radio transmission
    u32 pending_tick = clock_time();//200ms
    while (tx_done_flag == 0 && !clock_time_exceed(pending_tick,200*1000));   // tx finish
    tx_cnt++;
    tx_buffer[4]++;
    #if (UI_LED_ENABLE)
    gpio_toggle(GPIO_LED_GREEN);
    #endif
    WaitMs(1000); // delay 500 ms
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

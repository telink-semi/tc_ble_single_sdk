/********************************************************************************************************
 * @file    app_stx.c
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
#include "stack/2p4g/tpsll/tpsll.h"
#include "app_config.h"
#include "app.h"
#include "battery_check.h"
#if (TPSLL_MODE == TPSLL_STX)
static unsigned char payload[255] __attribute__((aligned(4))) ={
        TPSLL_SYNC_DATA,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x11,0x11,0x11,0x11,0x11,0x11,0x11,0x11,
        0x22,0x22,0x22,0x22,0x22,0x22,0x22,0x22,
        0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33
};

volatile static unsigned char tx_done_flag = 0;
volatile static unsigned char irq_cnt_tx   = 0;
volatile static unsigned char chn          = 10;
unsigned char                 payload_len  = 32;

_attribute_ram_code_  void irq_2p4g_sdk_handler(void)
{
  unsigned int irq_src = irq_get_src();
  unsigned short rf_irq_src = rf_irq_src_get();
  if (irq_src & FLD_IRQ_ZB_RT_EN) {//if rf irq occurs
      if (rf_irq_src & FLD_RF_IRQ_TX) {//if rf tx irq occurs
              rf_irq_clr_src(FLD_RF_IRQ_TX);
              tx_done_flag = 1;
              irq_cnt_tx++;
          }
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
    unsigned char sync_word[4] = {0xDF, 0x56, 0xD9, 0x35};
    //init Link Layer configuratioin
    tpsll_init(TPSLL_DATARATE_2MBPS);
    tpsll_channel_set(chn);
    tpsll_preamble_len_set(2);
    tpsll_sync_word_len_set(TPSLL_SYNC_WORD_LEN_4BYTE);

    if (sync_word_is_invalid(sync_word, 4)) {
        tlk_printf("sync_word invalid!\n");
        while(1);
    }

    tpsll_sync_word_set(TPSLL_PIPE0,sync_word);
    tpsll_pipe_open(TPSLL_PIPE0);
    tpsll_tx_pipe_set(TPSLL_PIPE0);
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    tpsll_radio_power_set(RF_POWER_P0p00dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_825x)
    tpsll_radio_power_set(RF_POWER_P0p04dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_827x)
    tpsll_radio_power_set(RF_POWER_P0p52dBm);
#endif
    tpsll_tx_settle_set(113);

    //irq configuration
    rf_irq_disable(FLD_RF_IRQ_ALL);
    rf_irq_enable(FLD_RF_IRQ_TX); //enable rf rx and rx first timeout irq
    irq_enable_type(FLD_IRQ_ZB_RT_EN); //enable RF irq
    irq_enable(); //enable general irq
//////////////////////////// 2P4G stack Initialization  End //////////////////////////////////


//////////////////////////// User Configuration for 2P4G application ////////////////////////////
	///////////////////// Power Management initialization///////////////////
	#if(APP_PM_ENABLE)

	#else

	#endif
////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////// User 2P4G application /////////////////////////////////////
    ///////////////////// Start stx///////////////////
    tpsll_tx_write_payload(payload,payload_len);
    tpsll_stx_start(clock_time()+50*16);
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

/////////////////////////////////////////////////////////////////////s
// main loop flow
_attribute_ram_code_ void sdk_2p4g_main_loop(void)
{
    tx_done_flag = 0;
    payload[4]++;
    tpsll_tx_write_payload(payload,payload_len);
    tpsll_stx_start(clock_time()+100*16);
    u32 pending_tick = clock_time();//200ms
    while (tx_done_flag == 0 && !clock_time_exceed(pending_tick,200*1000));   // tx finish
    #if (UI_LED_ENABLE)
    gpio_write(GPIO_LED_GREEN, 1); //LED On
    #endif
    WaitMs(200);
    #if (UI_LED_ENABLE)
    gpio_write(GPIO_LED_GREEN, 0); //LED Off
    #endif
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

	////////////////////////////////////// UI entry /////////////////////////////////
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

/********************************************************************************************************
 * @file    app_srx2tx.c
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

#if (GENFSK_LL_MODE == GENFSK_SRX2TX)
#define RX_BUF_LEN              64
#define RX_BUF_NUM              4

volatile static unsigned char rx_buf[RX_BUF_LEN * RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
volatile static unsigned char rx_ptr                                                        = 0;
volatile static unsigned char rx_flag                                                       = 0;
volatile static unsigned char rx_first_timeout_flag                                         = 0;
volatile static unsigned char rx_payload_len                                                = 0;
volatile static unsigned int  rx_timestamp                                                  = 0;
volatile static unsigned char rx_rssi                                                       = 0;
volatile static unsigned char *rx_payload                                                   = 0;
volatile static unsigned char *rx_packet                                                    = 0;
volatile static unsigned int irq_cnt_rx_first_timeout                                       = 0;
volatile static unsigned int irq_cnt_rx                                                     = 0;
volatile static unsigned int irq_cnt_crc_ok                                                 = 0;
volatile static unsigned int irq_cnt_rx_sync                                                = 0;
volatile static unsigned int irq_cnt_rx_head_done                                           = 0;

static unsigned char __attribute__ ((aligned (4))) tx_buffer[64] = {0};
unsigned char                                      tx_payload[8] = {0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0xaa, 0xbb};
volatile static unsigned char                      tx_done_flag  = 0;
volatile static unsigned int                       irq_cnt_tx    = 0;

_attribute_ram_code_  void irq_2p4g_sdk_handler(void)
{
    unsigned int irq_src = irq_get_src();
    unsigned short rf_irq_src = rf_irq_src_get();

    if (irq_src & FLD_IRQ_ZB_RT_EN) // rf irq occurs
    {
        if (rf_irq_src & FLD_RF_IRQ_RX) // rf rx irq occurs
        {
            rf_irq_clr_src(FLD_RF_IRQ_RX);
            irq_cnt_rx++;
            rx_packet = rx_buf + rx_ptr * RX_BUF_LEN;
            rx_ptr = (rx_ptr + 1) % RX_BUF_NUM;
            gen_fsk_rx_buffer_set((unsigned char *)(rx_buf + rx_ptr * RX_BUF_LEN), RX_BUF_LEN);

            if (gen_fsk_is_rx_crc_ok((unsigned char *)rx_packet))
            {
              irq_cnt_crc_ok++;
            }
            rx_flag = 1;
        }
        if (rf_irq_src & FLD_RF_IRQ_FIRST_TIMEOUT) // if rf first timeout irq occurs
        {
            rf_irq_clr_src(FLD_RF_IRQ_FIRST_TIMEOUT);
            rx_first_timeout_flag = 1;
            irq_cnt_rx_first_timeout++;
        }

        if (rf_irq_src & FLD_RF_IRQ_TX) // rf tx irq occurs
        {
            rf_irq_clr_src(FLD_RF_IRQ_TX);
            irq_cnt_tx++;
            tx_done_flag = 1;
        }
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
        if (rf_irq_src & FLD_RF_IRQ_HIT_SYNC) //if rf rx irq occurs
        {
          rf_irq_clr_src(FLD_RF_IRQ_HIT_SYNC);
            irq_cnt_rx_sync++;
        }
        if (rf_irq_src & FLD_RF_IRQ_HEADER_DONE) //if rf rx irq occurs
        {
          rf_irq_clr_src(FLD_RF_IRQ_HEADER_DONE);
            irq_cnt_rx_head_done++;
        }
#endif
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

    if (sync_word_is_invalid(sync_word, 4)) {
        tlk_printf("sync_word invalid!\n");
        while(1);
    }

    gen_fsk_sync_word_set(GEN_FSK_PIPE0, sync_word); //set pipe0's sync word
    gen_fsk_pipe_open(GEN_FSK_PIPE0); //enable pipe0's reception
    gen_fsk_tx_pipe_set(GEN_FSK_PIPE0); //set pipe0 as the TX pipe
    gen_fsk_packet_format_set(GEN_FSK_PACKET_FORMAT_FIXED_PAYLOAD, 8);
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    gen_fsk_radio_power_set(RF_POWER_P0p00dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_825x)
    gen_fsk_radio_power_set(RF_POWER_P0p04dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_827x)
    gen_fsk_radio_power_set(RF_POWER_P0p52dBm);
#endif
    gen_fsk_rx_buffer_set((unsigned char *)(rx_buf + rx_ptr * RX_BUF_LEN), RX_BUF_LEN);
    gen_fsk_channel_set(7); //set rf freq as 2403.5MHz
    gen_fsk_radio_state_set(GEN_FSK_STATE_AUTO);
    gen_fsk_tx_settle_set(149);
    gen_fsk_rx_settle_set(89);

    //irq configuration
    rf_irq_disable(FLD_RF_IRQ_ALL);
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    rf_irq_enable(FLD_RF_IRQ_RX | FLD_RF_IRQ_TX | FLD_RF_IRQ_HIT_SYNC |FLD_RF_IRQ_HEADER_DONE | FLD_RF_IRQ_FIRST_TIMEOUT);
#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
    rf_irq_enable(FLD_RF_IRQ_RX | FLD_RF_IRQ_TX | FLD_RF_IRQ_FIRST_TIMEOUT);
#endif
    irq_enable_type(FLD_IRQ_ZB_RT_EN); //enable RF irq
    irq_enable(); //enable general irq
//////////////////////////// 2P4G stack Initialization  End //////////////////////////////////


//////////////////////////// User Configuration for 2P4G application ////////////////////////////
    ///////////////////// Fill the dma info in tx buffer ///////////////////
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
    ///////////////////// Start the SRX2TX ///////////////////
    gen_fsk_srx2tx_start(tx_buffer, clock_time() + 50 * 16, 0);
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
  if (rx_flag)
  {
      rx_flag = 0;
      rx_payload = gen_fsk_rx_payload_get((unsigned char *)rx_packet, (unsigned char *)&rx_payload_len);
      rx_rssi = gen_fsk_rx_packet_rssi_get((unsigned char *)rx_packet) + 110;
      rx_timestamp = gen_fsk_rx_timestamp_get((unsigned char *)rx_packet);
      #if (UI_LED_ENABLE)
      gpio_toggle(GPIO_LED_GREEN);
      #endif
  }

  if (tx_done_flag)
  {
      tx_done_flag = 0;
      //start the SRX2TX
      gen_fsk_srx2tx_start(tx_buffer, clock_time() + 50 * 16, 0);
      #if (UI_LED_ENABLE)
      gpio_toggle(GPIO_LED_WHITE);
      #endif
  }
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

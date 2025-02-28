/********************************************************************************************************
 * @file    tpsll_mode.c
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
#include "app.h"
#include "battery_check.h"

#if(FEATURE_TEST_MODE == PM && RF_MODE == TPSLL)

#define RX_BUF_SIZE                     255
volatile static unsigned char tpsll_rxbuf[RX_BUF_SIZE]  __attribute__((aligned(4)));

unsigned char payload_len = 8;                                  //payload_len best to be 4n-1;
volatile static unsigned char payload[8] = {
        TPSLL_SYNC_DATA,0x11,0x22,0x33,0x44,0x55,0x66,0x77
};

volatile static unsigned char rx_flag                 = 0;
volatile static unsigned char rx_timeout_flag         = 0;
volatile static unsigned char tx_done_flag            = 0;
volatile static unsigned int irq_cnt_rx               = 0;
volatile static unsigned int irq_cnt_tx               = 0;
volatile static unsigned int irq_cnt_rx_crc_ok        = 0;
volatile static unsigned int irq_cnt_rx_timeout       = 0;
volatile static unsigned int irq_cnt_rx_sync          = 0;
volatile static unsigned int irq_cnt_rx_head_done     = 0;

_attribute_ram_code_  void irq_2p4g_sdk_handler(void)
{
  unsigned int irq_src = irq_get_src();
  unsigned short rf_irq_src = rf_irq_src_get();

  if (irq_src & FLD_IRQ_ZB_RT_EN) {//if rf irq occurs
      if (rf_irq_src & FLD_RF_IRQ_RX) {//if rf rx irq occurs
          rf_irq_clr_src(FLD_RF_IRQ_RX);
          irq_cnt_rx++;
          if (tpsll_is_rx_crc_ok((unsigned char *)tpsll_rxbuf)) {
            irq_cnt_rx_crc_ok++;
              rx_flag = 1;
          }
      }
      if (rf_irq_src & FLD_RF_IRQ_RX_TIMEOUT) {//if rf tx irq occurs
          rf_irq_clr_src(FLD_RF_IRQ_RX_TIMEOUT);
          rx_timeout_flag = 1;
          irq_cnt_rx_timeout++;
      }
      if (rf_irq_src & FLD_RF_IRQ_TX) {//if rf tx irq occurs
              rf_irq_clr_src(FLD_RF_IRQ_TX);
              tx_done_flag = 1;
              irq_cnt_tx++;
          }
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
  }
  irq_clr_sel_src(FLD_IRQ_ALL);
}

_attribute_ram_code_ void rf_init()
{
    unsigned char sync_word[4] = {0x11, 0x22, 0x33, 0x44};
    //init Link Layer configuratioin
    tpsll_init(TPSLL_DATARATE_1MBPS);
    tpsll_channel_set(7);
    tpsll_preamble_len_set(2);
    tpsll_sync_word_len_set(TPSLL_SYNC_WORD_LEN_4BYTE);
    tpsll_sync_word_set(TPSLL_PIPE0,sync_word);
    tpsll_pipe_open(TPSLL_PIPE0);
    tpsll_rx_buffer_set((unsigned char *)tpsll_rxbuf,RX_BUF_SIZE);
    tpsll_radio_power_set(TPSLL_RADIO_POWER_P0p00dBm);
    tpsll_tx_settle_set(113);
    //irq configuration
    rf_irq_disable(FLD_RF_IRQ_ALL);
    rf_irq_enable(FLD_RF_IRQ_TX|FLD_RF_IRQ_RX | FLD_RF_IRQ_RX_TIMEOUT | FLD_RF_IRQ_HIT_SYNC |FLD_RF_IRQ_HEADER_DONE); //enable rf rx and rx first timeout irq
    irq_enable_type(FLD_IRQ_ZB_RT_EN); //enable RF irq
    irq_enable(); //enable general irq
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

	tlkapi_printf(APP_LOG_EN, "---TPSLL_user_init_normal---!\r\n");
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
	rf_init();

//////////////////////////// 2P4G stack Initialization  End //////////////////////////////////


//////////////////////////// User Configuration for 2P4G application ////////////////////////////
	///////////////////// Power Management initialization///////////////////
	#if(APP_PM_ENABLE)

	#else

	#endif
////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////// User 2P4G application /////////////////////////////////////
    ///////////////////// Start stx2rx ///////////////////
    tpsll_tx_write_payload((unsigned char *)payload,payload_len);
    tpsll_stx2rx_start(clock_time()+50*16,250);
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

	tlkapi_printf(APP_LOG_EN, "---TPSLL_user_init_deepRetn---!\r\n");
	blc_app_loadCustomizedParameters_deepRetn();

	#if (APP_BATT_CHECK_ENABLE)
		/* ADC settings will lost during deepsleep retention mode, so here need clear flag */
		battery_clear_adc_setting_flag();
	#endif

	DBG_CHN0_HIGH;    //debug


//////////////////////////// 2P4G stack Initialization  Begin //////////////////////////////////
	rf_init();

//////////////////////////////////// User 2P4G application /////////////////////////////////////
	///////////////////// Start stx2rx ///////////////////
	tpsll_tx_write_payload((unsigned char *)payload,payload_len);
	tpsll_stx2rx_start(clock_time()+50*16,250);

#endif
}

/////////////////////////////////////////////////////////////////////s
// main loop flow
_attribute_ram_code_ void sdk_2p4g_main_loop(void)
{
    if (rx_timeout_flag) {
        rx_timeout_flag = 0;
        //sleep
        unsigned char pm_cnt = analog_read(0x3b);
        if(pm_cnt < 10){
			pm_cnt++;
			analog_write(0x3b, pm_cnt);
			gpio_shutdown(GPIO_ALL);
			cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_TIMER, (clock_time() + 1000*CLOCK_SYS_TIMER_CLK_1MS));
        }
		else if(pm_cnt < 20){
			pm_cnt++;
			analog_write(0x3b, pm_cnt);
			gpio_shutdown(GPIO_ALL);
			cpu_sleep_wakeup(DEEPSLEEP_MODE_RET_SRAM_LOW32K, PM_WAKEUP_TIMER, (clock_time() + 1000*CLOCK_SYS_TIMER_CLK_1MS));
		}
		else if(pm_cnt < 30){
			pm_cnt++;
			analog_write(0x3b, pm_cnt);
			cpu_sleep_wakeup(SUSPEND_MODE, PM_WAKEUP_TIMER, clock_time() + 1000*CLOCK_SYS_TIMER_CLK_1MS);
			rf_init();
			tpsll_tx_write_payload((unsigned char *)payload,payload_len);
			tpsll_stx2rx_start(clock_time()+50*16,250);
		}
		else{
			while(1);
		}
    }
    if (rx_flag) {
        rx_flag = 0;
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
	////////////////////////////////////// PM Process /////////////////////////////////
	blt_pm_proc();
}

#endif

/********************************************************************************************************
 * @file    app.c
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
#include "app_config.h"
#include "app.h"
#include "battery_check.h"
#include "fw_update_phy.h"
#include "fw_update.h"
#include "timer_event.h"
#include "../../stack/ble/debug/debug.h"
#if (FEATURE_TEST_MODE == UART_FW_UPDATE)
#if (UART_FW_UPDATE_ROLE == MASTER)
#define FW_UPDATE_FW_VERSION        0x0001
#elif(FEATURE_TEST_MODE == SLAVE)
#define FW_UPDATE_FW_VERSION         0x0000
#define Flash_Addr              0x08
#define Flash_Buff_Len          1
volatile unsigned char Flash_Read_Buff[Flash_Buff_Len]={0};
#endif

#define OTA_CHANNEL       70
volatile unsigned char    FW_UPDATE_Trig = 0;

volatile unsigned char uart_dmairq_tx_cnt;
volatile unsigned char uart_dmairq_rx_cnt;
volatile unsigned char uart_dmairq_err_cnt;
volatile unsigned char uart_dma_send_flag = 1;

#if(INTERNAL_TEST)
#define FW_UPDATE_TEST_TIMES      100
_attribute_data_retention_sec_  volatile unsigned int master_test_success_cnt = 0;
_attribute_data_retention_sec_  volatile unsigned int master_test_fail_cnt = 0;
_attribute_data_retention_sec_  volatile unsigned int fw_update_test_cnt = 0;
#endif
_attribute_ram_code_  void irq_2p4g_sdk_handler(void)
{
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
  if(reg_uart_status1(UART_MODULE_SEL) & FLD_UART_TX_DONE)
  {
      uart_dmairq_tx_cnt++;
      uart_dma_send_flag = 1;
      uart_clr_tx_done(UART_MODULE_SEL);
      FW_UPDATE_PHY_TxIrqHandler();
  }
  if(dma_chn_irq_status_get(FLD_DMA_CHN_UART_RX))
  {
      dma_chn_irq_status_clr(FLD_DMA_CHN_UART_RX);
      uart_dmairq_rx_cnt++;
      FW_UPDATE_PHY_RxIrqHandler();
  }

  if(uart_is_parity_error(UART_MODULE_SEL))//when stop bit error or parity error.
  {
      uart_clear_parity_error(UART_MODULE_SEL);
      uart_dmairq_err_cnt++;
  }

irq_clr_src();

#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)

    unsigned char uart_dma_irqsrc= dma_chn_irq_status_get();
    //1. UART irq
    if(uart_dma_irqsrc & FLD_DMA_CHN_UART_RX)
    {
        dma_chn_irq_status_clr(FLD_DMA_CHN_UART_RX);
        FW_UPDATE_PHY_RxIrqHandler();
    }
    if(uart_dma_irqsrc & FLD_DMA_CHN_UART_TX)
    {
        dma_chn_irq_status_clr(FLD_DMA_CHN_UART_TX);

        FW_UPDATE_PHY_TxIrqHandler();
    }


#endif
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


//////////////////////////// 2P4G stack Initialization  End //////////////////////////////////


//////////////////////////// User Configuration for 2P4G application ////////////////////////////

    ////////////////// OTA relative ////////////////////////
    #if (UART_PRINT_DEBUG_ENABLE)
        blc_debug_addStackLog(STK_LOG_OTA_FLOW);
    #endif
    #if(UART_FW_UPDATE_ROLE == SLAVE)
        flash_read_page(Flash_Addr,Flash_Buff_Len, (unsigned char *)Flash_Read_Buff);
          if (Flash_Read_Buff[0] == 0x4b)
          {
            tlkapi_printf((APP_FW_UPDATE_LOG_EN), "[OTA][FLW] slave start at 0 address\n");
          }
          else
          {
            tlkapi_printf((APP_FW_UPDATE_LOG_EN), "[OTA][FLW] slave start at %4x address\n",FW_UPDATE_SLAVE_BIN_ADDR);
          }
    #endif

    #if(INTERNAL_TEST)
          gpio_set_func(GPIO_PC7 | GPIO_PC6,AS_GPIO);
      #if(UART_FW_UPDATE_ROLE == MASTER)
          gpio_set_output_en(GPIO_PC7 | GPIO_PC6, 1);        //enable output
          gpio_set_input_en(GPIO_PC7 | GPIO_PC6 ,0);         //disable input
          gpio_write(GPIO_PC7,1);
      #else
          gpio_set_output_en(GPIO_PC7 | GPIO_PC6, 0);        //enable output
          gpio_set_input_en(GPIO_PC7 | GPIO_PC6 ,1);         //disable input
          gpio_write(GPIO_PC7,0);
      #endif
          gpio_write(GPIO_PC6,0);
    #endif
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

    #if(INTERNAL_TEST)
    gpio_set_func(GPIO_PC7 | GPIO_PC6,AS_GPIO);
#if(UART_FW_UPDATE_ROLE == MASTER)
    gpio_set_output_en(GPIO_PC7 | GPIO_PC6, 1);        //enable output
    gpio_set_input_en(GPIO_PC7 | GPIO_PC6 ,0);         //disable input
    gpio_write(GPIO_PC7,1);
#else
    gpio_set_output_en(GPIO_PC7 | GPIO_PC6, 0);        //enable output
    gpio_set_input_en(GPIO_PC7 | GPIO_PC6 ,1);         //disable input
    gpio_write(GPIO_PC7,0);
#endif
    gpio_write(GPIO_PC6,0);
#if(UART_FW_UPDATE_ROLE == MASTER)
    if(fw_update_test_cnt >= FW_UPDATE_TEST_TIMES){
      tlkapi_printf(APP_FW_UPDATE_LOG_EN, "[OTA][FLW] master fw update %d times,success %d times,fail %d times\n",fw_update_test_cnt,master_test_success_cnt,master_test_fail_cnt);
      tlkapi_printf(APP_FW_UPDATE_LOG_EN, "[OTA][FLW] master fw update test end!\n");
      gpio_write(GPIO_PC6,1);
    }
#endif
    #endif

	irq_enable();


#endif
}

/////////////////////////////////////////////////////////////////////
// main loop flow
_attribute_ram_code_ void sdk_2p4g_main_loop(void)
{
#if(INTERNAL_TEST)
  #if(UART_FW_UPDATE_ROLE == MASTER)
    if ((fw_update_test_cnt < FW_UPDATE_TEST_TIMES))
    {
        tlkapi_printf(APP_FW_UPDATE_LOG_EN, "[OTA][FLW] master fw update %d times,success %d times,fail %d times\n",fw_update_test_cnt,master_test_success_cnt,master_test_fail_cnt);
        fw_update_test_cnt++;
        gpio_write(GPIO_PC7,1);
        WaitMs(1000);
  #else
    if((gpio_read(GPIO_PC6) == 1)){
        tlkapi_printf(APP_FW_UPDATE_LOG_EN, "[OTA][FLW] slave fw update test end!\n");
        while(1);
    }else if(gpio_read(GPIO_PC7) == 1){
  #endif

#else
  if (FW_UPDATE_Trig)
  {
#endif

#if (APP_BATT_CHECK_ENABLE)
    user_battery_power_check(VBAT_DEEP_THRES_MV);
#endif

#if (APP_FLASH_PROTECTION_ENABLE)
    flash_prot_op_cb(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_BEGIN, 0, 0);
#endif
    FW_UPDATE_Trig = 0;
    FW_UPDATE_PHY_Init(FW_UPDATE_RxIrq);
#if (UART_FW_UPDATE_ROLE == MASTER)
    tlkapi_printf(APP_FW_UPDATE_LOG_EN, "[OTA][FLW] master FW UPDATE START!\n");
    FW_UPDATE_MasterInit(FW_UPDATE_MASTER_BIN_ADDR, FW_UPDATE_FW_VERSION);
#elif(UART_FW_UPDATE_ROLE == SLAVE)
    tlkapi_printf(APP_FW_UPDATE_LOG_EN, "[OTA][FLW] slave FW UPDATE START!\n");
      flash_read_page(Flash_Addr,Flash_Buff_Len, (unsigned char *)Flash_Read_Buff);
      if (Flash_Read_Buff[0] == 0x4b)
      {
          FW_UPDATE_SlaveInit(FW_UPDATE_SLAVE_BIN_ADDR, FW_UPDATE_FW_VERSION);
      }
      else
      {
          FW_UPDATE_SlaveInit(0, FW_UPDATE_FW_VERSION);
      }
#endif
#if (UI_LED_ENABLE)
      gpio_write(GPIO_LED_BLUE, 1);
      WaitMs(80);
      gpio_write(GPIO_LED_BLUE, 0);
      WaitMs(80);
      gpio_write(GPIO_LED_BLUE, 1);
      WaitMs(80);
      gpio_write(GPIO_LED_BLUE, 0);
#endif
      while (1)
      {
#if (UART_FW_UPDATE_ROLE == MASTER)
          FW_UPDATE_MasterStart();
#elif(UART_FW_UPDATE_ROLE == SLAVE)
          FW_UPDATE_SlaveStart();
#endif
          ev_process_timer();
      }
  }
#if (UI_LED_ENABLE)
#if (UART_FW_UPDATE_ROLE == MASTER)
  tlkapi_printf((APP_LOG_EN), "[OTA][FLW] master mainloop\n");
#else
  tlkapi_printf((APP_LOG_EN), "[OTA][FLW] slave mainloop\n");
#endif
  gpio_toggle(GPIO_LED_GREEN);
#endif
  WaitMs(200);

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

	///////////////////////////////////// Battery Check ////////////////////////////////
	#if (APP_BATT_CHECK_ENABLE)
		/*The frequency of low battery detect is controlled by the variable lowBattDet_tick, which is executed every
		 500ms in the demo. Users can modify this time according to their needs.*/
		if(battery_get_detect_enable() && clock_time_exceed(lowBattDet_tick, 500000) ){
			lowBattDet_tick = clock_time();
			user_battery_power_check(VBAT_DEEP_THRES_MV);
		}
	#endif
    ////////////////////////////////////// UI entry /////////////////////////////////
#if (UI_KEYBOARD_ENABLE)
    proc_keyboard(0, 0, 0);
#elif (UI_BUTTON_ENABLE)
    static u8 button_detect_en = 0;
    /* process button 1 second later after power on, to avoid power unstable */
    if(!button_detect_en && clock_time_exceed(0, 1000000)){
        button_detect_en = 1;
    }
    static u32 button_detect_tick = 0;
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

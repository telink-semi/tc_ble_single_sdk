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
#include "../../stack/ble/debug/debug.h"
#include "battery_check.h"
#include "mac.h"
#include "ota.h"
#if (FEATURE_TEST_MODE == OTA)
#if (OTA_ROLE == MASTER)
#define OTA_FW_VERSION          0x0001
#elif(OTA_ROLE == SLAVE)
#define OTA_FW_VERSION          0x0000
#define Flash_Addr              0x08
#define Flash_Buff_Len          1
volatile unsigned char Flash_Read_Buff[Flash_Buff_Len]={0};
#endif


#define OTA_CHANNEL       70
volatile unsigned char    OTA_Trig = 0;

#if(INTERNAL_TEST)
#define OTA_TEST_TIMES      100
_attribute_data_retention_sec_  volatile unsigned int master_test_success_cnt = 0;
_attribute_data_retention_sec_  volatile unsigned int master_test_fail_cnt = 0;
_attribute_data_retention_sec_  volatile unsigned int ota_test_cnt = 0;
#endif

volatile unsigned char first_timeout_done;
volatile unsigned char rx_done_cnt;
volatile unsigned char rx_timeout_done;
volatile unsigned char tx_done_cnt;

_attribute_ram_code_  void irq_2p4g_sdk_handler(void)
{
  unsigned int irq_src = irq_get_src();
  unsigned short src_rf = rf_irq_src_get();

  if (irq_src & FLD_IRQ_ZB_RT_EN)
  {
      if (src_rf & FLD_RF_IRQ_RX)
      {
          rf_irq_clr_src(FLD_RF_IRQ_RX);
          rx_done_cnt++;
          MAC_RxIrqHandler();

      }
      if (src_rf & FLD_RF_IRQ_RX_TIMEOUT)
      {
          rf_irq_clr_src(FLD_RF_IRQ_RX_TIMEOUT);
          rx_timeout_done++;
          MAC_RxTimeOutHandler();
      }
      if (src_rf & FLD_RF_IRQ_FIRST_TIMEOUT) // if rf first timeout irq occurs
      {
          rf_irq_clr_src(FLD_RF_IRQ_FIRST_TIMEOUT);
          MAC_RxTimeOutHandler();
      }
      if (src_rf & FLD_RF_IRQ_TX)
      {
          rf_irq_clr_src(FLD_RF_IRQ_TX);
          tx_done_cnt++;
      }
  }
  rf_irq_clr_src(FLD_RF_IRQ_ALL);
  irq_clr_src();
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
    #if(APP_OTA_LOG_EN)
        blc_debug_addStackLog(STK_LOG_OTA_FLOW);
    #endif
    #endif

    #if(OTA_ROLE == SLAVE)
        flash_read_page(Flash_Addr,Flash_Buff_Len, (unsigned char *)Flash_Read_Buff);
          if (Flash_Read_Buff[0] == 0x4b)
          {
            tlkapi_printf((APP_OTA_LOG_EN), "[OTA][FLW] slave start at 0 address\n");
          }
          else
          {
            tlkapi_printf((APP_OTA_LOG_EN), "[OTA][FLW] slave start at %4x address\n",OTA_SLAVE_BIN_ADDR);
          }
    #endif
    #if(INTERNAL_TEST)
          gpio_set_func(GPIO_PC7 | GPIO_PC6,AS_GPIO);
      #if(OTA_ROLE == MASTER)
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
    #if(OTA_ROLE == MASTER)
    gpio_set_output_en(GPIO_PC7 | GPIO_PC6, 1);        //enable output
    gpio_set_input_en(GPIO_PC7 | GPIO_PC6 ,0);         //disable input
    gpio_write(GPIO_PC7,1);
    #else
    gpio_set_output_en(GPIO_PC7 | GPIO_PC6, 0);        //enable output
    gpio_set_input_en(GPIO_PC7 | GPIO_PC6 ,1);         //disable input
    gpio_write(GPIO_PC7,0);
    #endif
    gpio_write(GPIO_PC6,0);
    #if(OTA_ROLE == MASTER)
    if(ota_test_cnt >= OTA_TEST_TIMES){
      tlkapi_printf(APP_OTA_LOG_EN, "[OTA][FLW] master OTA %d times,success %d times,fail %d times\n",ota_test_cnt,master_test_success_cnt,master_test_fail_cnt);
      tlkapi_printf(APP_OTA_LOG_EN, "[OTA][FLW] master OTA test end!\n");
      gpio_write(GPIO_PC6,1);
    }
    #endif
    #endif

	irq_enable();


#endif
}

/////////////////////////////////////////////////////////////////////s
// main loop flow
_attribute_ram_code_ void sdk_2p4g_main_loop(void)
{
#if(INTERNAL_TEST)
  #if(OTA_ROLE == MASTER)
    if ((ota_test_cnt < OTA_TEST_TIMES))
    {
        tlkapi_printf(APP_OTA_LOG_EN, "[OTA][FLW] master OTA %d times,success %d times,fail %d times\n",ota_test_cnt,master_test_success_cnt,master_test_fail_cnt);
        ota_test_cnt++;
        WaitMs(2000);
  #else
    if((gpio_read(GPIO_PC6) == 1)){
        tlkapi_printf(APP_OTA_LOG_EN, "[OTA][FLW] slave OTA test end!\n");
        while(1);
    }else if(gpio_read(GPIO_PC7) == 1){
  #endif

#else
  if (OTA_Trig)
  {
#endif

#if (APP_BATT_CHECK_ENABLE)
    user_battery_power_check(VBAT_DEEP_THRES_MV);
#endif

#if (APP_FLASH_PROTECTION_ENABLE)
    flash_prot_op_cb(FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_BEGIN, 0, 0);
#endif

      OTA_Trig = 0;
      MAC_Init(OTA_CHANNEL,
               OTA_RxIrq,
               OTA_RxTimeoutIrq,
               OTA_RxTimeoutIrq);
#if (OTA_ROLE == MASTER)
      tlkapi_printf(APP_OTA_LOG_EN, "[OTA][FLW] master OTA START!\n");
      OTA_MasterInit(OTA_MASTER_BIN_ADDR, OTA_FW_VERSION);
#elif(OTA_ROLE == SLAVE)
      tlkapi_printf(APP_OTA_LOG_EN, "[OTA][FLW] slave OTA START!\n");
      flash_read_page(Flash_Addr,Flash_Buff_Len, (unsigned char *)Flash_Read_Buff);
        if (Flash_Read_Buff[0] == 0x4b)
        {
            OTA_SlaveInit(OTA_SLAVE_BIN_ADDR, OTA_FW_VERSION);
        }
        else
        {
            OTA_SlaveInit(0, OTA_FW_VERSION);
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
#if (OTA_ROLE == MASTER)
          OTA_MasterStart();
#elif(OTA_ROLE == SLAVE)
          OTA_SlaveStart();
#endif
      }
  }

#if ((OTA_ROLE == MASTER) && APP_LOG_EN)
  tlkapi_printf((APP_LOG_EN), "[OTA][FLW] master mainloop\n");
#elif ((OTA_ROLE == SLAVE) && APP_LOG_EN)
  tlkapi_printf((APP_LOG_EN), "[OTA][FLW] slave mainloop\n");
#endif
#if (UI_LED_ENABLE)
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

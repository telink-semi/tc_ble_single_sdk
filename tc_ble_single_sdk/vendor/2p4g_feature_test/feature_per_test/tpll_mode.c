/********************************************************************************************************
 * @file    tpll_mode.c
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
#include "stack/2p4g/tpll/tpll.h"
#include "app_config.h"
#include "app.h"
#include "battery_check.h"

#if(FEATURE_TEST_MODE == PER && RF_MODE == TPLL)

#define TX	1
#define RX	2
#define RF_TX_RX_MODE	RX


volatile static unsigned char rf_run_step          		= 0;
#define PTX_PIPE    									  0//warning:B80 only support pipe0,B80B support pipe0~5
volatile static unsigned char chn          			    = 8;

volatile unsigned int  tx_cnt       	= 0;
volatile unsigned char tx_flag          = 0;
volatile unsigned char invalid_pid_flag = 0;
volatile unsigned char maxretry_flag  	= 0;
volatile unsigned char rx_dr_flag     	= 0;
volatile unsigned char rx_flag        	= 0;
unsigned char          tx_payload_len 	= 8;
unsigned char          preamble_len   	= 0;
volatile unsigned char ds_flag        	= 0;
int                    ret            	= 0;
static unsigned char tx_payload[8] = { 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
volatile unsigned char       rx_payload[128]              = {0};

volatile static unsigned int irq_cnt_rx_sync              = 0;
volatile static unsigned int irq_cnt_rx_head_done         = 0;
volatile unsigned int        irq_cnt_invalid_pid          = 0;
volatile unsigned int        irq_cnt_max_retry            = 0;
volatile unsigned int        irq_cnt_tx_ds                = 0;
volatile unsigned int        irq_cnt_tx                   = 0;
volatile unsigned int        irq_cnt_rx                   = 0;
volatile unsigned int        irq_cnt_rx_dr                = 0;
static volatile unsigned int print_time       		      = 0;

#define ACK_PAYLOAD_LEN         6
static unsigned char   ack_payload[ACK_PAYLOAD_LEN] = { 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};

_attribute_ram_code_  void irq_2p4g_sdk_handler(void)
{
  unsigned short src_rf = rf_irq_src_get();
  unsigned char pipe = TPLL_GetTXPipe();
  if (src_rf & FLD_RF_IRQ_TX)
  {
      rf_irq_clr_src(FLD_RF_IRQ_TX);
      irq_cnt_tx++;
      tx_flag = 1;
  }
  if (src_rf & FLD_RF_IRQ_INVALID_PID)
  {
      rf_irq_clr_src(FLD_RF_IRQ_INVALID_PID);
      irq_cnt_invalid_pid++;
      invalid_pid_flag = 1;
  }
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
  if (src_rf & FLD_RF_IRQ_TX_RETRYCNT)
  {
      rf_irq_clr_src(FLD_RF_IRQ_TX_RETRYCNT);
#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
      if (src_rf & FLD_RF_IRQ_RETRY_HIT)
      {
          rf_irq_clr_src(FLD_RF_IRQ_RETRY_HIT);
#endif
      irq_cnt_max_retry++;
      maxretry_flag = 1;
      //adjust rptr
      TPLL_UpdateTXFifoRptr(pipe);
  }
  if (src_rf & FLD_RF_IRQ_TX_DS)
  {
      rf_irq_clr_src(FLD_RF_IRQ_TX_DS);
      irq_cnt_tx_ds++;
      ds_flag = 1;
  }
  if (src_rf & FLD_RF_IRQ_RX_DR)
  {
      rf_irq_clr_src(FLD_RF_IRQ_RX_DR);
      irq_cnt_rx_dr++;
      rx_dr_flag = 1;
  }
  if (src_rf & FLD_RF_IRQ_RX)
  {
      rf_irq_clr_src(FLD_RF_IRQ_RX);
      irq_cnt_rx++;
      rx_flag = 1;
  }
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
  if (src_rf & FLD_RF_IRQ_HIT_SYNC) //if rf rx irq occurs
  {
	  rf_irq_clr_src(FLD_RF_IRQ_HIT_SYNC);
      irq_cnt_rx_sync++;
  }
  if (src_rf & FLD_RF_IRQ_HEADER_DONE) //if rf rx irq occurs
  {
	  rf_irq_clr_src(FLD_RF_IRQ_HEADER_DONE);
      irq_cnt_rx_head_done++;
  }
#endif
  irq_clr_src();
  rf_irq_clr_src(FLD_RF_IRQ_ALL);
}

void baseband_reset(void)
{
    analog_write(0x34,0x81);
    analog_write(0x34,0x80);
}

void rf_param_init(void)
{
    //rf configuration
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    TPLL_SetOutputPower(RF_POWER_P0p00dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_825x)
    TPLL_SetOutputPower(RF_POWER_P0p04dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_827x)
    TPLL_SetOutputPower(RF_POWER_P0p52dBm);
#endif
    TPLL_SetAddressWidth(ADDRESS_WIDTH_5BYTES);
    TPLL_ClosePipe(TPLL_PIPE_ALL);

    #if PTX_PIPE == 0
    unsigned char address[5] = {0x12,0x23,0x34,0x45,0x56}; //{0xaa,0xbb,0xcc,0xdd,0xee};
    TPLL_SetAddress(TPLL_PIPE0, address);
    TPLL_OpenPipe(TPLL_PIPE0);
#if(RF_TX_RX_MODE == TX)
    TPLL_SetTXPipe(TPLL_PIPE0);
#endif
    #endif
    TPLL_SetAutoRetry(0, 150);  //5,150
    TPLL_RxTimeoutSet(500);//if the mode is 250k ,the rx_time_out need more time, as so 1000us is ok!
    TPLL_Preamble_Set(8);
    TPLL_RxSettleSet(80);
    TPLL_TxSettleSet(149);

#if(RF_TX_RX_MODE == TX)
    TPLL_ModeSet(TPLL_MODE_PTX);
#elif(RF_TX_RX_MODE == RX)
    TPLL_ModeSet(TPLL_MODE_PRX);
#endif

    //configure irq
    irq_clr_src();
    rf_irq_clr_src(FLD_RF_IRQ_ALL);

    irq_enable_type(FLD_IRQ_ZB_RT_EN); //enable RF irq
    rf_irq_disable(FLD_RF_IRQ_ALL);
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    rf_irq_enable(FLD_RF_IRQ_TX | FLD_RF_IRQ_TX_DS | FLD_RF_IRQ_TX_RETRYCNT | FLD_RF_IRQ_RX_DR | FLD_RF_IRQ_HIT_SYNC | FLD_RF_IRQ_HEADER_DONE | FLD_RF_IRQ_INVALID_PID);
#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
    rf_irq_enable(FLD_RF_IRQ_TX | FLD_RF_IRQ_TX_DS | FLD_RF_IRQ_RETRY_HIT | FLD_RF_IRQ_RX_DR | FLD_RF_IRQ_INVALID_PID);
#endif
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

		TPLL_Init(TPLL_BITRATE_1MBPS);
		TPLL_SetRFChannel(chn);
		rf_param_init();

//    //rf configuration
//    TPLL_Init(TPLL_BITRATE_2MBPS);
//    TPLL_SetOutputPower(RF_POWER_P0p04dBm);
//    TPLL_SetAddressWidth(ADDRESS_WIDTH_5BYTES);
//    TPLL_ClosePipe(TPLL_PIPE_ALL);
//
//    #if PTX_PIPE == 0
//    unsigned char address[5] = {0x12,0x23,0x34,0x45,0x56}; //{0xaa,0xbb,0xcc,0xdd,0xee};
//    TPLL_SetAddress(TPLL_PIPE0, address);
//    TPLL_OpenPipe(TPLL_PIPE0);
//#if(RF_TX_RX_MODE == TX)
//    TPLL_SetTXPipe(TPLL_PIPE0);
//#endif
//    #endif
//
//    TPLL_SetRFChannel(chn);
//    TPLL_SetAutoRetry(0, 150);  //5,150
//    TPLL_RxTimeoutSet(500);//if the mode is 250k ,the rx_time_out need more time, as so 1000us is ok!
//    TPLL_Preamble_Set(8);
//    TPLL_RxSettleSet(80);
//    TPLL_TxSettleSet(149);
//
//#if(RF_TX_RX_MODE == TX)
//    TPLL_ModeSet(TPLL_MODE_PTX);
//#elif(RF_TX_RX_MODE == RX)
//    TPLL_ModeSet(TPLL_MODE_PRX);
//#endif
//
//    //configure irq
//    irq_clr_src();
//    rf_irq_clr_src(FLD_RF_IRQ_ALL);
//
//    irq_enable_type(FLD_IRQ_ZB_RT_EN); //enable RF irq
//    rf_irq_disable(FLD_RF_IRQ_ALL);
//    rf_irq_enable(FLD_RF_IRQ_TX | FLD_RF_IRQ_TX_DS | FLD_RF_IRQ_TX_RETRYCNT | FLD_RF_IRQ_RX_DR | FLD_RF_IRQ_HIT_SYNC | FLD_RF_IRQ_HEADER_DONE | FLD_RF_IRQ_INVALID_PID);
//    irq_enable(); //enable general irq
//////////////////////////// 2P4G stack Initialization  End //////////////////////////////////


//////////////////////////// User Configuration for 2P4G application ////////////////////////////
	///////////////////// Power Management initialization ///////////////////
	#if(APP_PM_ENABLE)

	#else

	#endif

////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////// User 2P4G application /////////////////////////////////////
#if(RF_TX_RX_MODE == TX)
    preamble_len = TPLL_Preamble_Read();
    ds_flag = 1; // for first start
    WaitMs(2000);
#elif(RF_TX_RX_MODE == RX)
//    WaitMs(5000);
    tlkapi_printf(APP_LOG_EN, "\r\n");
    tlkapi_printf(APP_LOG_EN, "tpll 2404/1M start RX!\r\n");
    print_time = clock_time();
    TPLL_PRXTrig();
#endif
////////////////////////////////////////////////////////////////////////////////////////////////
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

	#if (APP_BATT_CHECK_ENABLE)
		/* ADC settings will lost during deepsleep retention mode, so here need clear flag */
		battery_clear_adc_setting_flag();
	#endif

	DBG_CHN0_HIGH;    //debug

	irq_enable();

#if (UI_KEYBOARD_ENABLE)
    /////////// keyboard GPIO wake_up initialization ////////
    u32 pin[] = KB_DRIVE_PINS;
    for (int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
    {
        cpu_set_gpio_wakeup (pin[i], Level_High,1);  //drive pin high level wake_up deepsleep
    }
#elif (UI_BUTTON_ENABLE)

    cpu_set_gpio_wakeup (SW1_GPIO, Level_Low,1);  //button pin pad low wakeUp suspend/deepSleep
    cpu_set_gpio_wakeup (SW2_GPIO, Level_Low,1);  //button pin pad low wakeUp suspend/deepSleep
#endif

#endif
}

///////////////////////////////main loop flow//////////////////////////////////////s
_attribute_ram_code_ void sdk_2p4g_main_loop(void)
{
#if(RF_TX_RX_MODE == TX)
    if (1 == ds_flag || 1 == maxretry_flag)
    {
        if (1 == ds_flag)
        {
            #if (UI_LED_ENABLE)
            gpio_toggle(GPIO_LED_GREEN);
            #endif
        }
        ds_flag = 0;
        maxretry_flag = 0;

        tx_cnt++;
        if(tx_cnt > 10000){
        	rf_run_step++;
    		dma_reset();
    		baseband_reset();
        	if(rf_run_step == 1){ //2404-2M
        		TPLL_Init(TPLL_BITRATE_2MBPS);
        		chn = 4*2;
        		TPLL_SetRFChannel(chn);
        		rf_param_init();
        		tx_cnt = 1;
        		WaitMs(5000);
        		tlkapi_printf(APP_LOG_EN, "tpll 2404/2M start TX!\r\n");
        	}
        	else if(rf_run_step == 2){//2434-1M
        		TPLL_Init(TPLL_BITRATE_1MBPS);
        		chn = 34*2;
        		TPLL_SetRFChannel(chn);
        		rf_param_init();
        		tx_cnt = 1;
        		WaitMs(5000);
        		tlkapi_printf(APP_LOG_EN, "tpll 2434/1M start TX!\r\n");
        	}
        	else if(rf_run_step == 3){//2434-2M
        		TPLL_Init(TPLL_BITRATE_2MBPS);
        		chn = 34*2;
        		TPLL_SetRFChannel(chn);
        		rf_param_init();
        		tx_cnt = 1;
        		WaitMs(5000);
        		tlkapi_printf(APP_LOG_EN, "tpll 2434/2M start TX!\r\n");
        	}
        	else if(rf_run_step == 4){//2474-1M
        		TPLL_Init(TPLL_BITRATE_1MBPS);
        		chn = 74*2;
        		TPLL_SetRFChannel(chn);
        		rf_param_init();
        		tx_cnt = 1;
        		WaitMs(5000);
        		tlkapi_printf(APP_LOG_EN, "tpll 2474/1M start TX!\r\n");
        	}
        	else if(rf_run_step == 5){//2474-2M
        		TPLL_Init(TPLL_BITRATE_2MBPS);
        		chn = 74*2;
        		TPLL_SetRFChannel(chn);
        		rf_param_init();
        		tx_cnt = 1;
        		WaitMs(5000);
        		tlkapi_printf(APP_LOG_EN, "tpll 2474/2M start TX!\r\n");
        	}
        	else{
        		tlkapi_printf(APP_LOG_EN, "tpll_per_test_done!\r\n");
    			gpio_write(GPIO_LED_RED, 1);
    			while(1);
        	}
        }

        WaitMs(1);
        ret = TPLL_WriteTxPayload(PTX_PIPE, tx_payload, tx_payload_len);
        if (ret)
        {
            TPLL_PTXTrig();
        }
    }
#elif(RF_TX_RX_MODE == RX)
    if (1 == rx_dr_flag)
    {
    	print_time = clock_time();
        rx_dr_flag = 0;
        #if (UI_LED_ENABLE)
        gpio_toggle(GPIO_LED_GREEN);
        #endif
        while(!TPLL_TxFifoEmpty(0));
        TPLL_WriteAckPayload(TPLL_PIPE0, ack_payload, ACK_PAYLOAD_LEN);
    }
    if(clock_time_exceed(print_time, 3000000)){ //per_test_done, print result
    	rf_set_tx_rx_off();
    	tlkapi_printf(APP_LOG_EN, "tpll_irq_cnt_rx: %d, tpll_irq_cnt_rx_crc_ok: %d\r\n", irq_cnt_rx, irq_cnt_rx_dr);
    	irq_cnt_rx = 0;
    	irq_cnt_rx_dr = 0;
    	rf_run_step++;
    	print_time = clock_time();
		dma_reset();
		baseband_reset();
    	if(rf_run_step == 1){ //2404-2M
    		TPLL_Init(TPLL_BITRATE_2MBPS);
    		chn = 4*2;
    		TPLL_SetRFChannel(chn);
    		rf_param_init();
    		tlkapi_printf(APP_LOG_EN, "tpll 2404/2M start RX!\r\n");
    		rx_dr_flag = 0;
    		TPLL_PRXTrig();
    	}
    	else if(rf_run_step == 2){ //2434-1M
    		TPLL_Init(TPLL_BITRATE_1MBPS);
    		chn = 34*2;
    		TPLL_SetRFChannel(chn);
    		rf_param_init();
    		tlkapi_printf(APP_LOG_EN, "tpll 2434/1M start RX!\r\n");
    		rx_dr_flag = 0;
    		TPLL_PRXTrig();
    	}
    	else if(rf_run_step == 3){ //2434-2M
    		TPLL_Init(TPLL_BITRATE_2MBPS);
    		chn = 34*2;
    		TPLL_SetRFChannel(chn);
    		rf_param_init();
    		tlkapi_printf(APP_LOG_EN, "tpll 2434/2M start RX!\r\n");
    		rx_dr_flag = 0;
    		TPLL_PRXTrig();
    	}
    	else if(rf_run_step == 4){ //2474-1M
    		TPLL_Init(TPLL_BITRATE_1MBPS);
    		chn = 74*2;
    		TPLL_SetRFChannel(chn);
    		rf_param_init();
    		tlkapi_printf(APP_LOG_EN, "tpll 2474/1M start RX!\r\n");
    		rx_dr_flag = 0;
    		TPLL_PRXTrig();
    	}
    	else if(rf_run_step == 5){ //2474-2M
    		TPLL_Init(TPLL_BITRATE_2MBPS);
    		chn = 74*2;
    		TPLL_SetRFChannel(chn);
    		rf_param_init();
    		tlkapi_printf(APP_LOG_EN, "tpll 2474/2M start RX!\r\n");
    		rx_dr_flag = 0;
    		TPLL_PRXTrig();
    	}
    	else{
    		tlkapi_printf(APP_LOG_EN, "tpll_per_test_done!\r\n");
			gpio_write(GPIO_LED_GREEN, 1);
			while(1);
    	}
    }
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
}

#endif

/********************************************************************************************************
 * @file    genfsk_mode.c
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
#include "app.h"
#include "battery_check.h"

#if(FEATURE_TEST_MODE == PER && RF_MODE == GENFSK)

#define TX	1
#define RX	2
#define RF_TX_RX_MODE	TX

volatile static unsigned char rf_run_step          		= 0;
volatile static unsigned char chn          				= 8;
//TX
static unsigned char __attribute__ ((aligned (4))) tx_buffer[64] = {0};
unsigned char                                      tx_payload[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
volatile static unsigned char                      tx_done_flag  = 0;
volatile unsigned int                              irq_cnt_tx    = 0;
volatile unsigned int                              tx_cnt    	 = 0;

//RX
#define RX_BUF_LEN              64
#define RX_BUF_NUM              4
volatile static unsigned char rx_buf[RX_BUF_LEN * RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
volatile static unsigned char rx_flag                                                       = 0;
volatile static unsigned char rx_first_timeout_flag                                         = 0;
volatile static unsigned char rx_ptr                                                        = 0;
volatile static unsigned int  irq_cnt_rx                                                    = 0;
volatile static unsigned int  irq_cnt_rx_crc_ok                                             = 0;
volatile static unsigned int  irq_cnt_rx_first_timeout                                      = 0;
volatile static unsigned char *rx_packet                                                    = 0;
static volatile unsigned int  print_time       		   										= 0;

void baseband_reset(void)
{
    analog_write(0x34,0x81);
    analog_write(0x34,0x80);
}

_attribute_ram_code_  void irq_2p4g_sdk_handler(void)
{
    unsigned int irq_src = irq_get_src();
    unsigned short rf_irq_src = rf_irq_src_get();

    if (irq_src & FLD_IRQ_ZB_RT_EN) // rf irq occurs
    {
        if (rf_irq_src & FLD_RF_IRQ_TX) // rf tx irq occurs
        {
            irq_cnt_tx++;
            tx_done_flag = 1;
        }
        if (rf_irq_src & FLD_RF_IRQ_RX) // if rf rx irq occurs
        {
            rf_irq_clr_src(FLD_RF_IRQ_RX);
            irq_cnt_rx++;
            rx_packet = rx_buf + rx_ptr * RX_BUF_LEN;
            rx_ptr = (rx_ptr + 1) % RX_BUF_NUM;
            gen_fsk_rx_buffer_set((unsigned char *)(rx_buf + rx_ptr * RX_BUF_LEN), RX_BUF_LEN);

            if (gen_fsk_is_rx_crc_ok((unsigned char *)rx_packet))
            {
              irq_cnt_rx_crc_ok++;
            }
            rx_flag = 1;

        }
        if (rf_irq_src & FLD_RF_IRQ_FIRST_TIMEOUT) // if rf first timeout irq occurs
        {
            rf_irq_clr_src(FLD_RF_IRQ_FIRST_TIMEOUT);
            rx_first_timeout_flag = 1;
            irq_cnt_rx_first_timeout++;
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

void rf_param_init(void)
{
    unsigned char sync_word[4] = {0x53, 0x78, 0x56, 0x52};
    // it needs to notice that this api is different from vulture / kite
    gen_fsk_preamble_len_set(4);
    gen_fsk_sync_word_len_set(SYNC_WORD_LEN_4BYTE);

    if (sync_word_is_invalid(sync_word, 4)) {
        tlk_printf("sync_word invalid!\n");
        while(1);
    }

    gen_fsk_sync_word_set(GEN_FSK_PIPE0, sync_word); //set pipe0's sync word
    gen_fsk_pipe_open(GEN_FSK_PIPE0); // enable pipe0's reception
    gen_fsk_packet_format_set(GEN_FSK_PACKET_FORMAT_FIXED_PAYLOAD, sizeof(tx_payload));
	#if(RF_TX_RX_MODE == RX)
    gen_fsk_rx_buffer_set((unsigned char *)(rx_buf + rx_ptr * RX_BUF_LEN), RX_BUF_LEN);
	#endif
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    gen_fsk_radio_power_set(RF_POWER_P0p00dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_825x)
    gen_fsk_radio_power_set(RF_POWER_P0p04dBm);
#elif(MCU_CORE_TYPE == MCU_CORE_827x)
    gen_fsk_radio_power_set(RF_POWER_P0p52dBm);
#endif
    gen_fsk_radio_state_set(GEN_FSK_STATE_AUTO); // set transceiver to basic TX state
    gen_fsk_tx_settle_set(149);
    gen_fsk_rx_settle_set(89);

    rf_irq_enable(FLD_RF_IRQ_TX | FLD_RF_IRQ_RX | FLD_RF_IRQ_FIRST_TIMEOUT); // enable rf tx irq
    irq_enable_type(FLD_IRQ_ZB_RT_EN); // enable RF irq
    irq_enable(); // enable general irq
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

		gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS);
		rf_param_init();
		gen_fsk_channel_set(chn);

//    unsigned char sync_word[4] = {0x53, 0x78, 0x56, 0x52};
//    // it needs to notice that this api is different from vulture / kite
//    gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS); //Note that this API must be invoked first before all other APIs
//    gen_fsk_preamble_len_set(4);
//    gen_fsk_sync_word_len_set(SYNC_WORD_LEN_4BYTE);
//
//    if (sync_word_is_invalid(sync_word, 4)) {
//        tlk_printf("sync_word invalid!\n");
//        while(1);
//    }
//
//    gen_fsk_sync_word_set(GEN_FSK_PIPE0, sync_word); //set pipe0's sync word
//    gen_fsk_pipe_open(GEN_FSK_PIPE0); // enable pipe0's reception
//    gen_fsk_packet_format_set(GEN_FSK_PACKET_FORMAT_FIXED_PAYLOAD, sizeof(tx_payload));
//	#if(RF_TX_RX_MODE == RX)
//    gen_fsk_rx_buffer_set((unsigned char *)(rx_buf + rx_ptr * RX_BUF_LEN), RX_BUF_LEN);
//	#endif
//    gen_fsk_radio_power_set(RF_POWER_P0p04dBm);
//    gen_fsk_channel_set(chn);
//    gen_fsk_radio_state_set(GEN_FSK_STATE_AUTO); // set transceiver to basic TX state
//    gen_fsk_tx_settle_set(149);
//    gen_fsk_rx_settle_set(89);
//
//    rf_irq_enable(FLD_RF_IRQ_TX | FLD_RF_IRQ_RX | FLD_RF_IRQ_FIRST_TIMEOUT); // enable rf tx irq
//    irq_enable_type(FLD_IRQ_ZB_RT_EN); // enable RF irq
//    irq_enable(); // enable general irq
//////////////////////////// 2P4G stack Initialization  End //////////////////////////////////


//////////////////////////// User Configuration for 2P4G application ////////////////////////////
#if(RF_TX_RX_MODE == TX)
    //fill the dma info in tx buffer
	tx_buffer[0] = sizeof(tx_payload);
	tx_buffer[1] = 0x00;
	tx_buffer[2] = 0x00;
	tx_buffer[3] = 0x00;
	memcpy(&tx_buffer[4], (const void *)tx_payload, sizeof(tx_payload));
	WaitMs(2000);
	tlkapi_printf(APP_LOG_EN, "\r\n");
	tlkapi_printf(APP_LOG_EN, "genfsk 2404/1M start TX!\r\n");
#elif(RF_TX_RX_MODE == RX)
	//start the SRX
	tlkapi_printf(APP_LOG_EN, "\r\n");
	tlkapi_printf(APP_LOG_EN, "genfsk 2404/1M start RX!\r\n");
	print_time = clock_time();
	gen_fsk_srx_start(clock_time() + 50 * 16, 0);
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
#if (PM_DEEPSLEEP_RETENTION_ENABLE)

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
#if(RF_TX_RX_MODE == TX)
    tx_done_flag = 0;
    gen_fsk_stx_start(tx_buffer, clock_time() + 100 * 16);
    while (tx_done_flag == 0);

    WaitMs(1);
    #if (UI_LED_ENABLE)
    gpio_toggle(GPIO_LED_GREEN);
    #endif
    tx_buffer[4]++;

    tx_cnt++;
    if(tx_cnt >= 10000){
    	rf_run_step++;
		dma_reset();
		baseband_reset();
    	if(rf_run_step == 1){ //2404-2M
    		gen_fsk_datarate_set(GEN_FSK_DATARATE_2MBPS);
    		rf_param_init();
    		chn = 4*2;
    		gen_fsk_channel_set(chn);
    		tx_cnt = 0;
    		WaitMs(5000);
    		tlkapi_printf(APP_LOG_EN, "genfsk 2404/2M start TX!\r\n");
    	}
    	else if(rf_run_step == 2){//2434-1M
    		gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS);
    		rf_param_init();
    		chn = 34*2;
    		gen_fsk_channel_set(chn);
    		tx_cnt = 0;
    		WaitMs(5000);
    		tlkapi_printf(APP_LOG_EN, "genfsk 2434/1M start TX!\r\n");
    	}
    	else if(rf_run_step == 3){//2434-2M
    		gen_fsk_datarate_set(GEN_FSK_DATARATE_2MBPS);
    		rf_param_init();
    		chn = 34*2;
    		gen_fsk_channel_set(chn);
    		tx_cnt = 0;
    		WaitMs(5000);
    		tlkapi_printf(APP_LOG_EN, "genfsk 2434/2M start TX!\r\n");
    	}
    	else if(rf_run_step == 4){//2474-1M
    		gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS);
    		rf_param_init();
    		chn = 74*2;
    		gen_fsk_channel_set(chn);
    		tx_cnt = 0;
    		WaitMs(5000);
    		tlkapi_printf(APP_LOG_EN, "genfsk 2474/1M start TX!\r\n");
    	}
    	else if(rf_run_step == 5){//2474-2M
    		gen_fsk_datarate_set(GEN_FSK_DATARATE_2MBPS);
    		rf_param_init();
    		chn = 74*2;
    		gen_fsk_channel_set(chn);
    		tx_cnt = 0;
    		WaitMs(5000);
    		tlkapi_printf(APP_LOG_EN, "genfsk 2474/2M start TX!\r\n");
    	}
    	else{
    		tlkapi_printf(APP_LOG_EN, "genfsk_per_test_done!\r\n");
			gpio_write(GPIO_LED_RED, 1);
			while(1);
    	}
    }

#elif(RF_TX_RX_MODE == RX)
    if (1 == rx_flag)
    {
        rx_flag = 0;
        #if (UI_LED_ENABLE)
        gpio_toggle(GPIO_LED_GREEN);
        #endif
        print_time = clock_time();
        gen_fsk_srx_start(clock_time() + 50 * 16, 0);
    }

    if (1 == rx_first_timeout_flag)
    {
    	rx_first_timeout_flag = 0;
        gen_fsk_srx_start(clock_time() + 50 * 16, 0);
    }

    if(clock_time_exceed(print_time, 3000000)){ //per_test_done, print result
    	rf_set_tx_rx_off();
    	tlkapi_printf(APP_LOG_EN, "genfsk_irq_cnt_rx: %d, genfsk_irq_cnt_rx_crc_ok: %d\r\n", irq_cnt_rx, irq_cnt_rx_crc_ok);
    	irq_cnt_rx = 0;
    	irq_cnt_rx_crc_ok = 0;
    	rf_run_step++;
    	print_time = clock_time();
		dma_reset();
		baseband_reset();
    	if(rf_run_step == 1){ //2404-2M
    		gen_fsk_datarate_set(GEN_FSK_DATARATE_2MBPS);
    		rf_param_init();
    		rx_ptr = 0;
    		gen_fsk_rx_buffer_set((unsigned char *)(rx_buf + rx_ptr * RX_BUF_LEN), RX_BUF_LEN);
    		chn = 4*2;
    		gen_fsk_channel_set(chn);
    		tlkapi_printf(APP_LOG_EN, "genfsk 2404/2M start RX!\r\n");
    		gen_fsk_srx_start(clock_time() + 50 * 16, 0);
    	}
    	else if(rf_run_step == 2){ //2434-1M
    		gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS);
    		rf_param_init();
    		rx_ptr = 0;
    		gen_fsk_rx_buffer_set((unsigned char *)(rx_buf + rx_ptr * RX_BUF_LEN), RX_BUF_LEN);
    		chn = 34*2;
    		gen_fsk_channel_set(chn);
    		tlkapi_printf(APP_LOG_EN, "genfsk 2434/1M start RX!\r\n");
    		gen_fsk_srx_start(clock_time() + 50 * 16, 0);
    	}
    	else if(rf_run_step == 3){ //2434-2M
    		gen_fsk_datarate_set(GEN_FSK_DATARATE_2MBPS);
    		rf_param_init();
    		rx_ptr = 0;
    		gen_fsk_rx_buffer_set((unsigned char *)(rx_buf + rx_ptr * RX_BUF_LEN), RX_BUF_LEN);
    		chn = 34*2;
    		gen_fsk_channel_set(chn);
    		tlkapi_printf(APP_LOG_EN, "genfsk 2434/2M start RX!\r\n");
    		gen_fsk_srx_start(clock_time() + 50 * 16, 0);
    	}
    	else if(rf_run_step == 4){ //2474-1M
    		gen_fsk_datarate_set(GEN_FSK_DATARATE_1MBPS);
    		rf_param_init();
    		rx_ptr = 0;
    		gen_fsk_rx_buffer_set((unsigned char *)(rx_buf + rx_ptr * RX_BUF_LEN), RX_BUF_LEN);
    		chn = 74*2;
    		gen_fsk_channel_set(chn);
    		tlkapi_printf(APP_LOG_EN, "genfsk 2474/1M start RX!\r\n");
    		gen_fsk_srx_start(clock_time() + 50 * 16, 0);
    	}
    	else if(rf_run_step == 5){ //2474-2M
    		gen_fsk_datarate_set(GEN_FSK_DATARATE_2MBPS);
    		rf_param_init();
    		rx_ptr = 0;
    		gen_fsk_rx_buffer_set((unsigned char *)(rx_buf + rx_ptr * RX_BUF_LEN), RX_BUF_LEN);
    		chn = 74*2;
    		gen_fsk_channel_set(chn);
    		tlkapi_printf(APP_LOG_EN, "genfsk 2474/2M start RX!\r\n");
    		gen_fsk_srx_start(clock_time() + 50 * 16, 0);
    	}
    	else{
    		tlkapi_printf(APP_LOG_EN, "genfsk_per_test_done!\r\n");
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




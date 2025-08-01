/********************************************************************************************************
 * @file    app_ptx.c
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
#if (TPLL_MODE == TPLL_PTX)
#define PTX_PIPE                     0//warning:B80 only support pipe0,B80B support pipe0~5
#define PTX_CHANNEL                  7
#define FAST_CALIBRATION_ENABLE      0

#if(FAST_CALIBRATION_ENABLE && MCU_CORE_TYPE != MCU_CORE_825x)
#define FAST_CALIB_VALUE_ADDRESS     0x44000//0x40000+16kbyte
static volatile unsigned short hpmc[160] = {0};
void TPLL_Fast_SettleCalib(void);
#endif

volatile unsigned char maxretry_flag  = 0;
volatile unsigned char rx_dr_flag     = 0;
volatile unsigned char rx_flag        = 0;
unsigned char          tx_payload_len = 32;
unsigned char          preamble_len   = 0;
volatile unsigned char ds_flag        = 0;
int                    ret            = 0;
static unsigned char tx_payload[32] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                                 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                                                 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                                                 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20};
#if (MCU_CORE_TYPE == MCU_CORE_TC321X)
volatile static unsigned int irq_cnt_rx_sync              = 0;
volatile static unsigned int irq_cnt_rx_head_done         = 0;
#endif
volatile unsigned int        irq_cnt_invalid_pid          = 0;
volatile unsigned int        irq_cnt_tx_max_retry         = 0;
volatile unsigned int        irq_cnt_tx_ds                = 0;
volatile unsigned int        irq_cnt_tx                   = 0;
volatile unsigned int        irq_cnt_rx                   = 0;
volatile unsigned int        irq_cnt_rx_dr                = 0;

_attribute_ram_code_  void irq_2p4g_sdk_handler(void)
{
    unsigned short src_rf = rf_irq_src_get();
    unsigned char pipe = TPLL_GetTXPipe();
    if (src_rf & FLD_RF_IRQ_TX)
    {
      rf_irq_clr_src(FLD_RF_IRQ_TX);
      irq_cnt_tx++;
    }
    if (src_rf & FLD_RF_IRQ_INVALID_PID)
    {
      rf_irq_clr_src(FLD_RF_IRQ_INVALID_PID);
      irq_cnt_invalid_pid++;
    }
    #if (MCU_CORE_TYPE == MCU_CORE_TC321X)
    if (src_rf & FLD_RF_IRQ_TX_RETRYCNT)
    {
      rf_irq_clr_src(FLD_RF_IRQ_TX_RETRYCNT);
      irq_cnt_tx_max_retry++;
      maxretry_flag = 1;
      //adjust rptr
      TPLL_UpdateTXFifoRptr(pipe);
    }
    #elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
    if (src_rf & FLD_RF_IRQ_RETRY_HIT)
    {
      rf_irq_clr_src(FLD_RF_IRQ_RETRY_HIT);
      irq_cnt_tx_max_retry++;
      maxretry_flag = 1;
      //adjust rptr
      TPLL_UpdateTXFifoRptr(pipe);
    }
    #endif
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
    #if (MCU_CORE_TYPE == MCU_CORE_TC321X)
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
    //rf configuration
    TPLL_Init(TPLL_BITRATE_2MBPS);
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
    unsigned char tx_address[5] = {0xe7,0xe7,0xe7,0xe7,0xe7}; //{0xaa,0xbb,0xcc,0xdd,0xee};
    TPLL_SetAddress(TPLL_PIPE0, tx_address);
    TPLL_OpenPipe(TPLL_PIPE0);
    TPLL_SetTXPipe(TPLL_PIPE0);
    #endif

    #if PTX_PIPE == 1
    unsigned char tx_address1[5] = {0x55, 0x44, 0x33, 0x22, 0x11};
    //    unsigned char tx_address1[5] = {0xe7, 0xe7, 0xe7, 0xe7, 0xe7};
    TPLL_SetAddress(TPLL_PIPE1, tx_address1);
    TPLL_OpenPipe(TPLL_PIPE1);
    TPLL_SetTXPipe(TPLL_PIPE1);
    #endif

    #if PTX_PIPE == 2
    unsigned char tx_address2[5] = {0x55, 0x44, 0x33, 0x22, 0x11};
    TPLL_SetAddress(TPLL_PIPE1, tx_address2);
    tx_address2[0] = 0x22;
    TPLL_SetAddress(TPLL_PIPE2, &tx_address2[0]);
    TPLL_OpenPipe(TPLL_PIPE2);
    TPLL_SetTXPipe(TPLL_PIPE2);
    #endif

    #if PTX_PIPE == 3
    unsigned char tx_address3[5] = {0x55, 0x44, 0x33, 0x22, 0x11};
    TPLL_SetAddress(TPLL_PIPE1, tx_address3);
    tx_address3[0] = 0x33;
    TPLL_SetAddress(TPLL_PIPE3, &tx_address3[0]);
    TPLL_OpenPipe(TPLL_PIPE3);
    TPLL_SetTXPipe(TPLL_PIPE3);
    #endif

    TPLL_ModeSet(TPLL_MODE_PTX);
    TPLL_SetRFChannel(7);
    TPLL_SetAutoRetry(0, 150);  //5,150
    TPLL_RxTimeoutSet(500);//if the mode is 250k ,the rx_time_out need more time, as so 1000us is ok!
    TPLL_Preamble_Set(8);
    TPLL_RxSettleSet(80);
    TPLL_TxSettleSet(149);

#if(FAST_CALIBRATION_ENABLE && MCU_CORE_TYPE != MCU_CORE_825x)
    TPLL_Fast_SettleCalib();
    TPLL_SetHpmcCalVal(hpmc[PTX_CHANNEL]);
    TPLL_Fast_RxSettleInit(TPLL_RX_SETTLE_TIME_44US);
    TPLL_Fast_TxSettleInit(TPLL_TX_SETTLE_TIME_53US);
    TPLL_Fast_RxSettleEnable();
    TPLL_Fast_TxSettleEnable();
    TPLL_TxWaitSet(0);
    TPLL_RxWaitSet(0);
    TPLL_Fast_TxSettleSet(50);
    TPLL_Fast_RxSettleSet(50);
    TPLL_ResetBuf();
    TPLL_SetRFChannel(7);
#endif

    //configure irq
    irq_clr_src();
    rf_irq_clr_src(FLD_RF_IRQ_ALL);

    irq_enable_type(FLD_IRQ_ZB_RT_EN); //enable RF irq
    rf_irq_disable(FLD_RF_IRQ_ALL);
#if (MCU_CORE_TYPE == MCU_CORE_TC321X)
    rf_irq_enable(FLD_RF_IRQ_TX | FLD_RF_IRQ_TX_DS | FLD_RF_IRQ_TX_RETRYCNT | FLD_RF_IRQ_RX_DR | FLD_RF_IRQ_HIT_SYNC | FLD_RF_IRQ_HEADER_DONE | FLD_RF_IRQ_INVALID_PID);
#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
    rf_irq_enable(FLD_RF_IRQ_TX | FLD_RF_IRQ_TX_DS | FLD_RF_IRQ_RETRY_HIT | FLD_RF_IRQ_RX_DR | FLD_RF_IRQ_INVALID_PID);
#endif
    irq_enable(); //enable general irq
//////////////////////////// 2P4G stack Initialization  End //////////////////////////////////


//////////////////////////// User Configuration for 2P4G application ////////////////////////////
	///////////////////// Power Management initialization ///////////////////
	#if(APP_PM_ENABLE)

	#else

	#endif

////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////// User 2P4G application /////////////////////////////////////
    preamble_len = TPLL_Preamble_Read();
    ds_flag = 1; // for first start
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
        WaitMs(500);
        tx_payload[1]++;
        ret = TPLL_WriteTxPayload(PTX_PIPE, tx_payload, tx_payload_len);
        if (ret)
        {
            TPLL_PTXTrig();
        }
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

#if(FAST_CALIBRATION_ENABLE && MCU_CORE_TYPE != MCU_CORE_825x)
void TPLL_Fast_SettleCalib(void) {
    unsigned char tmp = 0;
    unsigned char tx_data[2] = {0x11,0x22};
    unsigned short hp[160] = {0};
    flash_read_page(FAST_CALIB_VALUE_ADDRESS,160*2,(unsigned char *)hp);
    if(hp[0] != 0xffff)
    {
        flash_read_page(FAST_CALIB_VALUE_ADDRESS,160*2,(unsigned char *)hpmc);
        return;
    }
    tmp = TPLL_WriteTxPayload(0, (const unsigned char *)tx_data, 2);
    for (int i = 0; i <= 160; i++) {
        TPLL_SetRFChannel(i);
        TPLL_PTXTrig();
        WaitUs(200);
        hpmc[i] = TPLL_GetHpmcCalVal();
        WaitUs(200);
    }
    flash_write_page(FAST_CALIB_VALUE_ADDRESS,160*2,(unsigned char *)hpmc);
    WaitUs(600);
}
#endif
#endif

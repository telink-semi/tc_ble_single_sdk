/********************************************************************************************************
 * @file    main.c
 *
 * @brief   This is the source file for 2.4G SDK
 *
 * @author  2.4G GROUP
 * @date    01,2025
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
#include "app.h"

void irq_2p4g_sdk_handler(void);

/**
 * @brief   IRQ handler
 * @param   none.
 * @return  none.
 */
_attribute_ram_code_ void irq_handler(void)
{

	irq_2p4g_sdk_handler();

}


/**
 * @brief		This is main function
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ int main (void)    //must run in ramcode
{

	DBG_CHN0_LOW;   //debug

	blc_pm_select_internal_32k_crystal();

	#if(MCU_CORE_TYPE == MCU_CORE_825x)
		cpu_wakeup_init();
	#else
		cpu_wakeup_init(LDO_MODE,INTERNAL_CAP_XTAL24M);
	#endif

	int deepRetWakeUp = pm_is_MCU_deepRetentionWakeup();  //MCU deep retention wakeUp

	gpio_init(!deepRetWakeUp);  //analog resistance will keep available in deepSleep mode, so no need initialize again

	clock_init(SYS_CLK_TYPE);

    if(!deepRetWakeUp){//read flash size
        #if FIRMWARE_CHECK_ENABLE
            //Execution time is in ms.such as:48k fw,16M crystal clock,need about 290ms.
            if(flash_fw_check(0xffffffff)){ //return 0, flash fw crc check ok. return 1, flash fw crc check fail
                while(1);                   //Users can process according to the actual application.
            }
        #endif
    }

	if( deepRetWakeUp ){
		user_init_deepRetn();
	}
	else{
		user_init_normal();
	}

#if (RF_DEBUG_IO_ENABLE)
	debug_config();
#endif

    irq_enable();
	while (1) {
#if (MODULE_WATCHDOG_ENABLE && (MCU_CORE_TYPE != MCU_CORE_TC321X))
        wd_clear(); //clear watch dog
#endif
		main_loop();
	}
}

#if (APP_BATT_CHECK_ENABLE)  //battery check must do before OTA relative operation

_attribute_data_retention_  u32 lowBattDet_tick   = 0;

/**
 * @brief       this function is used to process battery power.
 *              The low voltage protection threshold 2.0V is an example and reference value. Customers should
 *              evaluate and modify these thresholds according to the actual situation. If users have unreasonable designs
 *              in the hardware circuit, which leads to a decrease in the stability of the power supply network, the
 *              safety thresholds must be increased as appropriate.
 * @param[in]   none
 * @return      none
 */
_attribute_ram_code_ void user_battery_power_check(u16 alarm_vol_mv)
{
    /*For battery-powered products, as the battery power will gradually drop, when the voltage is low to a certain
      value, it will cause many problems.
        a) When the voltage is lower than operating voltage range of chip, chip can no longer guarantee stable operation.
        b) When the battery voltage is low, due to the unstable power supply, the write and erase operations
            of Flash may have the risk of error, causing the program firmware and user data to be modified abnormally,
            and eventually causing the product to fail. */
    u8 battery_check_returnValue=0;
    if(analog_read(USED_DEEP_ANA_REG) & LOW_BATT_FLG){
        battery_check_returnValue=app_battery_power_check(alarm_vol_mv+200);
    }
    else{
        battery_check_returnValue=app_battery_power_check(alarm_vol_mv);
    }
    if(battery_check_returnValue)
    {
        analog_write(USED_DEEP_ANA_REG,  analog_read(USED_DEEP_ANA_REG) & (~LOW_BATT_FLG));  //clr
    }
    else
    {
        #if (UI_LED_ENABLE)  //led indicate
            for(int k=0;k<3;k++){
                gpio_write(GPIO_LED_BLUE, LED_ON_LEVEL);
                sleep_us(200000);
                gpio_write(GPIO_LED_BLUE, !LED_ON_LEVEL);
                sleep_us(200000);
            }
        #endif

        if(analog_read(USED_DEEP_ANA_REG) & LOW_BATT_FLG){
            tlkapi_printf(APP_BATT_CHECK_LOG_EN, "[APP][BAT] The battery voltage is lower than %dmV, shut down!!!\n", (alarm_vol_mv + 200));
        } else {
            tlkapi_printf(APP_BATT_CHECK_LOG_EN, "[APP][BAT] The battery voltage is lower than %dmV, shut down!!!\n", alarm_vol_mv);
        }


        analog_write(USED_DEEP_ANA_REG,  analog_read(USED_DEEP_ANA_REG) | LOW_BATT_FLG);  //mark

        #if (UI_KEYBOARD_ENABLE)
        u32 pin[] = KB_DRIVE_PINS;
        for (int i=0; i<(sizeof (pin)/sizeof(*pin)); i++)
        {
            cpu_set_gpio_wakeup (pin[i], Level_High, 1);  //drive pin pad high wakeup deepsleep
        }

        cpu_sleep_wakeup(DEEPSLEEP_MODE, PM_WAKEUP_PAD, 0);  //deepsleep
        #endif
    }
}

#endif

#if (APP_FLASH_PROTECTION_ENABLE)
/**
 * @brief      flash protection operation, including all locking & unlocking for application
 *             handle all flash write & erase action for this demo code. use should add more more if they have more flash operation.
 * @param[in]  flash_op_evt - flash operation event, including application layer action and stack layer action event(OTA write & erase)
 *             attention 1: if you have more flash write or erase action, you should should add more type and process them
 *             attention 2: for "end" event, no need to pay attention on op_addr_begin & op_addr_end, we set them to 0 for
 *                          stack event, such as stack OTA write new firmware end event
 * @param[in]  op_addr_begin - operating flash address range begin value
 * @param[in]  op_addr_end - operating flash address range end value
 *             attention that, we use: [op_addr_begin, op_addr_end)
 *             e.g. if we write flash sector from 0x10000 to 0x20000, actual operating flash address is 0x10000 ~ 0x1FFFF
 *                  but we use [0x10000, 0x20000):  op_addr_begin = 0x10000, op_addr_end = 0x20000
 * @return     none
 */
_attribute_data_retention_ u16  flash_lockBlock_cmd = 0;
void app_flash_protection_operation(u8 flash_op_evt, u32 op_addr_begin, u32 op_addr_end)
{
    if(flash_op_evt == FLASH_OP_EVT_APP_INITIALIZATION)
    {
        /* ignore "op addr_begin" and "op addr_end" for initialization event
         * must call "flash protection_init" first, will choose correct flash protection relative API according to current internal flash type in MCU */
        flash_protection_init();
        /* just sample code here, protect all flash area for old firmware and OTA new firmware.
         * user can change this design if have other consideration */
        u32  app_lockBlock = 0;
        app_lockBlock = FLASH_LOCK_FW_LOW_256K; //just demo value, user can change this value according to application

        flash_lockBlock_cmd = flash_change_app_lock_block_to_flash_lock_block(app_lockBlock);

        if(blc_flashProt.init_err){
            tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash protection initialization error!!!\n");
        }
        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] initialization, lock flash\n");
        flash_lock(flash_lockBlock_cmd);
    }
    /* add more flash protection operation for your application if needed */
}
#endif

#if (RF_DEBUG_IO_ENABLE)
/**
 * @brief       this function is only for internal rf test.
 *              TX EN: tx enable.
 *              TX ON: on tx data.
 *              RX EN: rx enable.
 *              TX DATA: high level indicate 1,low level indicate 0,the interval is daterate.
 *              RX_DATA: high level indicate 1,low level indicate 0,the interval is RX DATA CLK.
 *              RX DATA CLK: interval for rx date.
 *              CLK DIG: indicate system clock.
 *              RX ACCESS DET: accesscode begin to detect(rx timstamp).
 * @param[in]   none
 * @return      none
 */
void debug_config(void)
{
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    gpio_set_func(GPIO_PA0, DBG0_IO);  //TX EN
    gpio_set_func(GPIO_PA4, DBG0_IO);  //TX ON
    gpio_set_func(GPIO_PA5, DBG0_IO);  //RX EB
    gpio_set_func(GPIO_PA6, DBG0_IO);  //RX DATE
    gpio_set_func(GPIO_PA7, DBG0_IO);  //RX ACCESS DET
    gpio_set_func(GPIO_PB0, DBG0_IO);  //RX DATA CLK
    gpio_set_func(GPIO_PB2, DBG0_IO);  //TX DATA
    gpio_set_func(GPIO_PB4, DBG0_IO);  //CLK DIG

    write_reg8(0x574, read_reg8(0x574)|BIT(3)|BIT(4)|BIT(5));

#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
    WRITE_REG8(0x5a8, 0xff);//tx_en: PA0 TX_ON:PA1 RX_EN:PA2
    WRITE_REG8(0x586, 0x00);
    WRITE_REG8(0x5b6, READ_REG8(0x5b6) | (1 << 1));
#endif
}
#endif

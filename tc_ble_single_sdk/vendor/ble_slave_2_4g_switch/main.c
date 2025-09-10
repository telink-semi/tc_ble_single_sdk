/********************************************************************************************************
 * @file    main.c
 *
 * @brief   This is the source file for BLE SDK
 *
 * @author  BLE GROUP
 * @date    06,2020
 *
 * @par     Copyright (c) 2020, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "stack/ble/ble.h"
#include "app.h"
#include "./2_4g_test/2p4g_common.h"


/**
 * @brief   IRQ handler
 * @param   none.
 * @return  none.
 */
_attribute_ram_code_ void irq_handler(void)
{
#if(TEST_2P4G_MODE)
    if(!rf_working_mode)
    {
        irq_blt_sdk_handler();
    }
    else{
        irq_2p4_handler();
    }
#else
    irq_blt_sdk_handler();
#endif
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
    #if(TEST_2P4G_MODE)
    rf_working_mode=(analog_read(USED_DEEP_ANA_REG) & RF_WORKING_MODE);
    #endif
	int deepRetWakeUp = pm_is_MCU_deepRetentionWakeup();  //MCU deep retention wakeUp

	gpio_init(!deepRetWakeUp);  //analog resistance will keep available in deepSleep mode, so no need initialize again

	clock_init(SYS_CLK_TYPE);

	#if (MODULE_WATCHDOG_ENABLE)
		wd_set_interval_ms(WATCHDOG_INIT_TIMEOUT,CLOCK_SYS_CLOCK_1MS);
		wd_start();
	#endif

    #if(TEST_2P4G_MODE)
        if(!rf_working_mode){
            rf_drv_ble_init();

            if( deepRetWakeUp ){
                user_init_deepRetn();
            }
            else{
                user_init_normal();
            }
        }
    #else
        rf_drv_ble_init();

        if( deepRetWakeUp ){
            user_init_deepRetn();
        }
        else{
            user_init_normal();
        }
    #endif
    irq_enable();
	while (1) {
#if (MODULE_WATCHDOG_ENABLE)
	#if (MCU_CORE_TYPE == MCU_CORE_TC321X)
		if (g_chip_version != CHIP_VERSION_A0)
	#endif
		{
			wd_clear(); //clear watch dog
		}
#endif
		main_loop();
	}
}


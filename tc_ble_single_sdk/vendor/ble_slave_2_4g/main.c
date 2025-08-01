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

volatile unsigned char irq_bleModeFlag = 1;

/**
 * @brief   IRQ handler
 * @param   none.
 * @return  none.
 */
_attribute_ram_code_ void irq_handler(void)
{
#if (TEST_2P4G_MODE)
    if(!irq_bleModeFlag)
    {
        concurrent_irq_handler();
    }
    else
#endif
    {
        irq_blt_sdk_handler();
    }
}

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

/**
 * @brief		This is main function
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ int main (void)    //must run in ramcode
{

	DBG_CHN0_LOW;   //debug
	#if(MCU_CORE_TYPE == MCU_CORE_825x)
		cpu_wakeup_init();
	#else
		cpu_wakeup_init(LDO_MODE,INTERNAL_CAP_XTAL24M);
	#endif

	int deepRetWakeUp = pm_is_MCU_deepRetentionWakeup();  //MCU deep retention wakeUp

	rf_drv_ble_init();

	gpio_init(!deepRetWakeUp);  //analog resistance will keep available in deepSleep mode, so no need initialize again

	clock_init(SYS_CLK_TYPE);

    user_init_normal();

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


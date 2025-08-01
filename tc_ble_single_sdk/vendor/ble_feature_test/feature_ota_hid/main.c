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

#if (FEATURE_TEST_MODE == TEST_OTA_HID)
/**
 * @brief   IRQ handler
 * @param   none.
 * @return  none.
 */
_attribute_ram_code_ void irq_handler(void)
{
	irq_blt_sdk_handler();
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

	rf_drv_ble_init();

	gpio_init(!deepRetWakeUp);  //analog resistance will keep available in deepSleep mode, so no need initialize again

	clock_init(SYS_CLK_TYPE);

	if( deepRetWakeUp ){
		user_init_deepRetn();
	}
	else{
		user_init_normal();
	}

    irq_enable();

	while (1) {
		main_loop();
	}
}

#endif  //end of (FEATURE_TEST_MODE == ...)

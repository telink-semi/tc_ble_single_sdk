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
#include "driver.h"
#include "app.h"

#if (FEATURE_TEST_MODE == EPD_DRIVER)

/**
 * @brief   IRQ handler
 * @param   none.
 * @return  none.
 */
void irq_2p4g_sdk_handler(void);
_attribute_ram_code_ void irq_handler(void)
{
	irq_2p4g_sdk_handler();
}

_attribute_ram_code_ int main(void)
{
	DBG_CHN0_LOW;   //debug

	blc_pm_select_internal_32k_crystal();

#if (MCU_CORE_TYPE == MCU_CORE_825x)
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
	irq_enable();
	while (1) {
		main_loop();
	}
    return 0;
}

#endif

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
#include "rc_ir_learn.h"
#include "stack/ble/ble.h"
#include "app.h"
#include "battery_check.h"


extern void deep_wakeup_proc(void);

extern void rc_ir_irq_prc(void);


/**
 * @brief   IRQ handler
 * @param   none.
 * @return  none.
 */
_attribute_ram_code_ void irq_handler(void)
{
#if (REMOTE_IR_ENABLE)
	rc_ir_irq_prc();
#endif

#if (REMOTE_IR_LEARN_ENABLE)
	ir_learn_irq_handler();
#endif

	irq_blt_sdk_handler();

#if (BLE_PHYTEST_MODE != PHYTEST_MODE_DISABLE)
	extern void irq_phyTest_handler(void);
	irq_phyTest_handler();
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

	int deepRetWakeUp = pm_is_MCU_deepRetentionWakeup();  //MCU deep retention wakeUp

	rf_drv_ble_init();

	gpio_init(!deepRetWakeUp);  //analog resistance will keep available in deepSleep mode, so no need initialize again

	clock_init(SYS_CLK_TYPE);

	if(!deepRetWakeUp){//read flash size
		#if FIRMWARE_CHECK_ENABLE
			//Execution time is in ms.such as:48k fw,16M crystal clock,need about 290ms.
			if(flash_fw_check(0xffffffff)){ //return 0, flash fw crc check ok. return 1, flash fw crc check fail
				while(1);				    //Users can process according to the actual application.
			}
		#endif
	}

	if( deepRetWakeUp ){
		user_init_deepRetn();
	}
	else{
		#if DEEPBACK_FAST_KEYSCAN_ENABLE
			deep_wakeup_proc();
		#endif
		#if FIRMWARES_SIGNATURE_ENABLE
			blt_firmware_signature_check();
		#endif
		user_init_normal();
	}


    irq_enable();



	while (1) {
#if (MODULE_WATCHDOG_ENABLE && (MCU_CORE_TYPE != MCU_CORE_TC321X))
		wd_clear(); //clear watch dog
#endif
		main_loop();
	}

}


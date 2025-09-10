/********************************************************************************************************
 * @file    main.c
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
#include "app.h"
#include "ota.h"
#include "stack/ble/ble.h"
#if (FEATURE_TEST_MODE == OTA)
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
#if(OTA_MASTER_BIN_ADDR == OTA_BIN_ADDR_0x20000 || OTA_SLAVE_BIN_ADDR == OTA_BIN_ADDR_0x20000 )
	blc_ota_setFirmwareSizeAndBootAddress(124,OTA_BIN_ADDR_0x20000);
#else
	blc_ota_setFirmwareSizeAndBootAddress(252,OTA_BIN_ADDR_0x40000);
#endif

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
		main_loop();
	}
}


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
#if (OTA_SERVER_ENABLE)
    u32 multiBootAddress = blc_ota_getCurrentUsedMultipleBootAddress();
    if(multiBootAddress == MULTI_BOOT_ADDR_0x20000){
        app_lockBlock = FLASH_LOCK_FW_LOW_256K;
    }
    else if(multiBootAddress == MULTI_BOOT_ADDR_0x40000){
        /* attention that 512K capacity flash can not lock all 512K area, should leave some upper sector
         * for system data(SMP storage data & calibration data & MAC address) and user data
         * will use a approximate value */
        app_lockBlock = FLASH_LOCK_FW_LOW_512K;
    }
#else
    app_lockBlock = FLASH_LOCK_FW_LOW_256K; //just demo value, user can change this value according to application
#endif

        flash_lockBlock_cmd = flash_change_app_lock_block_to_flash_lock_block(app_lockBlock);

        if(blc_flashProt.init_err){
            tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash protection initialization error!!!\n");
        }
        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] initialization, lock flash\n");
        flash_lock(flash_lockBlock_cmd);
    }

#if (OTA_SERVER_ENABLE)
    else if(flash_op_evt == FLASH_OP_EVT_STACK_OTA_CLEAR_OLD_FW_BEGIN)
    {
        /* OTA clear old firmware begin event is triggered by stack, in "blc ota_initOtaServer_module", rebooting from a successful OTA.
         * Software will erase whole old firmware for potential next new OTA, need unlock flash if any part of flash address from
         * "op addr_begin" to "op addr_end" is in locking block area.
         * In this sample code, we protect whole flash area for old and new firmware, so here we do not need judge "op addr_begin" and "op addr_end",
         * must unlock flash */
        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA clear old FW begin, unlock flash\n");
        flash_unlock();
    }
    else if(flash_op_evt == FLASH_OP_EVT_STACK_OTA_CLEAR_OLD_FW_END)
    {
        /* ignore "op addr_begin" and "op addr_end" for END event
         * OTA clear old firmware end event is triggered by stack, in "blc ota_initOtaServer_module", erasing old firmware data finished.
         * In this sample code, we need lock flash again, because we have unlocked it at the begin event of clear old firmware */
        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA clear old FW end, restore flash locking\n");
        flash_lock(flash_lockBlock_cmd);
    }
    else if(flash_op_evt == FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_BEGIN)
    {
        /* OTA write new firmware begin event is triggered by stack, when receive first OTA data PDU.
         * Software will write data to flash on new firmware area,  need unlock flash if any part of flash address from
         * "op addr_begin" to "op addr_end" is in locking block area.
         * In this sample code, we protect whole flash area for old and new firmware, so here we do not need judge "op addr_begin" and "op addr_end",
         * must unlock flash */
        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA write new FW begin, unlock flash\n");
        flash_unlock();
    }
    else if(flash_op_evt == FLASH_OP_EVT_STACK_OTA_WRITE_NEW_FW_END)
    {
        /* ignore "op addr_begin" and "op addr_end" for END event
         * OTA write new firmware end event is triggered by stack, after OTA end or an OTA error happens, writing new firmware data finished.
         * In this sample code, we need lock flash again, because we have unlocked it at the begin event of write new firmware */
        tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] OTA write new FW end, restore flash locking\n");
        flash_lock(flash_lockBlock_cmd);
    }
#endif
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
#endif

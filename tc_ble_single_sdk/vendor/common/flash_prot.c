/********************************************************************************************************
 * @file    flash_prot.c
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

#include "flash_prot.h"


_attribute_ble_data_retention_	_attribute_aligned_(4)	flash_prot_t	blc_flashProt;

/* this pointer is used only by stack, when OTA write or erase flash, will give a callback to application layer */
_attribute_ble_data_retention_	_attribute_aligned_(4)	flash_prot_op_callback_t 		flash_prot_op_cb = NULL;


#if (APP_FLASH_PROTECTION_ENABLE)

_attribute_ble_data_retention_	_attribute_aligned_(4)	flash_lock_t 				flash_lock_mid = NULL;
_attribute_ble_data_retention_	_attribute_aligned_(4)	flash_unlock_t 				flash_unlock_mid = NULL;
_attribute_ble_data_retention_	_attribute_aligned_(4)	flash_get_lock_status_t 	flash_get_lock_status_mid = NULL;


_attribute_ble_data_retention_	u16	flash_unlock_status; //record flash unlock status for different flash type


/**
 * @brief      this function is used to register callback for stack, when OTA write or erase flash, send event to application layer
 * @param[in]  cb - flash protection operation callback function
 * @return     none
 */
void blc_appRegisterStackFlashOperationCallback(flash_prot_op_callback_t cb)
{
	flash_prot_op_cb = cb;
}


#if (!MCU_SUPPORT_MULTI_PRIORITY_IRQ)
/**
 * @brief 		this function is used to replace Flash driver API "flash_write_status", solving BLE connection issue on MCU do not support multiple priority IRQ
 * @param[in]  	type	- the type of status.8 bit or 16 bit.
 * @param[in]  	data	- the value of status.
 * @return 		none.
 */
void flash_write_status(flash_status_typedef_e type , unsigned short data)
{
	/* If MCU do not support multiple priority IRQ, Flash write status duration influencing BLE RF IRQ, then lead to BLE data error
	 * use  "blc ll_write_flash_status" to solve the issue, SDK internal will find idle timing to write safely. */
	blc_ll_write_flash_status(type, data);
}
#endif


/**
 * @brief      this function is used to initialize flash protection.Block size of lock is a sample, user can change it according to bin size.
 * 				 This function only shows the flash supported by TELINK testing. If user use other flash, please follow the code below.
 * @param[in]  none
 * @return     none
 */
void flash_protection_init(void)
{
	if(!blc_flash_mid){
		blc_flash_mid = flash_read_mid();
	}
	/* According to the flash mid, execute the corresponding lock flash API. */
	switch(blc_flash_mid)
	{
		#if (FLASH_ZB25WD40B_SUPPORT_EN)
			case MID13325E:
				flash_lock_mid = (flash_lock_t)flash_lock_mid13325e;
				flash_unlock_mid = (flash_unlock_t)flash_unlock_mid13325e;
				flash_get_lock_status_mid = (flash_get_lock_status_t)flash_get_lock_block_mid13325e;
				flash_unlock_status = FLASH_LOCK_NONE_MID13325E;
				break;
		#endif

		#if (FLASH_ZB25WD80B_SUPPORT_EN)
			case MID14325E:
				flash_lock_mid = (flash_lock_t)flash_lock_mid14325e;
				flash_unlock_mid = (flash_unlock_t)flash_unlock_mid14325e;
				flash_get_lock_status_mid = (flash_get_lock_status_t)flash_get_lock_block_mid14325e;
				flash_unlock_status = FLASH_LOCK_NONE_MID14325E;
				break;
		#endif

		#if (FLASH_P25D40SU_SUPPORT_EN)
			case MID136085:
				flash_lock_mid = (flash_lock_t)flash_lock_mid136085;
				flash_unlock_mid = (flash_unlock_t)flash_unlock_mid136085;
				flash_get_lock_status_mid = (flash_get_lock_status_t)flash_get_lock_block_mid136085;
				flash_unlock_status = FLASH_LOCK_NONE_MID136085;
				break;
		#endif

		#if (FLASH_GD25LD40C_SUPPORT_EN || FLASH_GD25LD40E_SUPPORT_EN)
			case MID1360C8:
				flash_lock_mid = (flash_lock_t)flash_lock_mid1360c8;
				flash_unlock_mid = (flash_unlock_t)flash_unlock_mid1360c8;
				flash_get_lock_status_mid = (flash_get_lock_status_t)flash_get_lock_block_mid1360c8;
				flash_unlock_status = FLASH_LOCK_NONE_MID1360C8;
				break;
		#endif

		#if(FLASH_P25Q80U_SUPPORT_EN || FLASH_P25Q80SU_SUPPORT_EN)
			case MID146085:
				flash_lock_mid = (flash_lock_t)flash_lock_mid146085;
				flash_unlock_mid = (flash_unlock_t)flash_unlock_mid146085;
				flash_get_lock_status_mid = (flash_get_lock_status_t)flash_get_lock_block_mid146085;
				flash_unlock_status = FLASH_LOCK_NONE_MID146085;
				break;
		#endif

		#if (FLASH_GD25LD80C_SUPPORT_EN || FLASH_GD25LD80E_SUPPORT_EN)
			case MID1460C8:
				flash_lock_mid = (flash_lock_t)flash_lock_mid1460c8;
				flash_unlock_mid = (flash_unlock_t)flash_unlock_mid1460c8;
				flash_get_lock_status_mid = (flash_get_lock_status_t)flash_get_lock_block_mid1460c8;
				flash_unlock_status = FLASH_LOCK_NONE_MID1460C8;
				break;
		#endif

		#if (FLASH_P25Q16SU_SUPPORT_EN)	//2M capacity
			case MID156085:
				flash_lock_mid = (flash_lock_t)flash_lock_mid156085;
				flash_unlock_mid = (flash_unlock_t)flash_unlock_mid156085;
				flash_get_lock_status_mid = (flash_get_lock_status_t)flash_get_lock_block_mid156085;
				flash_unlock_status = FLASH_LOCK_NONE_MID156085;
				break;
		#endif

		default:
			blc_flashProt.init_err = 1;
			break;
	}
}

/**
 * @brief      this function is used to change application lock block definition to flash lock block definition according to flash type used
 * 			   attention: only processed a few lock block size for SDK demo, if you have more lock block size, please
 * 			              add more code by yourself
 * @param[in]  app_lock_block - application lock block definition
 * @return     flash_lock_block_size - The size of the lock block size of flash.
 */
u16 flash_change_app_lock_block_to_flash_lock_block(flash_app_lock_e app_lock_block)
{
	u32 flash_lock_block_size = 0;

	switch(blc_flash_mid)
	{
		#if (FLASH_ZB25WD40B_SUPPORT_EN) //512K capacity
			case MID13325E:
				if(app_lock_block == FLASH_LOCK_FW_LOW_256K){
					flash_lock_block_size = FLASH_LOCK_LOW_256K_MID13325E;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 256K block!\n");
				}
				else if(app_lock_block == FLASH_LOCK_FW_LOW_512K){
					/* attention 1: use can change this value according to application
					 * attention 2: can not lock stack SMP data storage area
					 * attention 3: firmware size under protection is not 512K, user should calculate
					 * demo code: choose 448K, leave at 64K for system data(SMP storage data & calibration data & MAC address) and user data,
					 *            now firmware size under protection is 448K - 256K = 192K
					 * if this demo can not meet your requirement, you should change !!!*/
					flash_lock_block_size = FLASH_LOCK_LOW_448K_MID13325E;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 448K block!\n");
				}
				else{
					blc_flashProt.init_err = 1; //can not use LOCK LOW 1M for 512K capacity flash
				}
				break;
		#endif

		#if (FLASH_P25D40SU_SUPPORT_EN) //512K capacity
			case MID136085:
				if(app_lock_block == FLASH_LOCK_FW_LOW_256K){
					flash_lock_block_size = FLASH_LOCK_LOW_256K_MID136085;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 256K block!\n");
				}
				else if(app_lock_block == FLASH_LOCK_FW_LOW_512K){
					/* attention 1: use can change this value according to application
					 * attention 2: can not lock stack SMP data storage area
					 * attention 3: firmware size under protection is not 512K, user should calculate
					 * demo code: choose 448K, leave at 64K for system data(SMP storage data & calibration data & MAC address) and user data,
					 *            now firmware size under protection is 448K - 256K = 192K
					 * if this demo can not meet your requirement, you should change !!!*/
					flash_lock_block_size = FLASH_LOCK_LOW_448K_MID136085;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 448K block!\n");
				}
				else{
					blc_flashProt.init_err = 1; //can not use LOCK LOW 1M for 512K capacity flash
				}
				break;
		#endif

		#if (FLASH_ZB25WD80B_SUPPORT_EN) //1M capacity
			case MID14325E:
				if(app_lock_block == FLASH_LOCK_FW_LOW_256K || app_lock_block == FLASH_LOCK_FW_LOW_512K){
					/* attention that :This flash type, minimum lock size is 768K, do not support 256K or other value
					 * demo code will lock 768K when user set OTA 128K or 256K as multiple boot address,
					 * system data(SMP storage data & calibration data & MAC address) is OK;
					 * user data must be stored in flash address bigger than 768K !!!
					 * if this demo code lock area do not meet your requirement, you can change it !!!*/
					flash_lock_block_size = FLASH_LOCK_LOW_768K_MID14325E;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 768K block!\n");
				}
				else if(app_lock_block == FLASH_LOCK_FW_LOW_1M){
					/* attention 1: use can change this value according to application
					 * attention 2: can not lock stack SMP data storage area
					 * attention 3: firmware size under protection is not 1M, user should calculate
					 * demo code: choose 960K, leave 64K for system data(SMP storage data & calibration data & MAC address) and user data,
					 * 			  now firmware size under protection is 960K - 512K = 448K
					 * if this demo can not meet your requirement, you should change !!! */
					flash_lock_block_size = FLASH_LOCK_LOW_960K_MID14325E;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 960K block!\n");
				}
				else{
					blc_flashProt.init_err = 1;
				}
				break;
		#endif


		#if (FLASH_GD25LD40C_SUPPORT_EN || FLASH_GD25LD40E_SUPPORT_EN) //512K capacity
			case MID1360C8:
				if(app_lock_block == FLASH_LOCK_FW_LOW_256K){
					flash_lock_block_size = FLASH_LOCK_LOW_256K_MID1360C8;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 256K block!\n");
				}
				else if(app_lock_block == FLASH_LOCK_FW_LOW_512K){
					/* attention 1: use can change this value according to application
					 * attention 2: can not lock stack SMP data storage area
					 * attention 3: firmware size under protection is not 512K, user should calculate
					 * demo code: choose 448K, leave at 64K for system data(SMP storage data & calibration data & MAC address) and user data,
					 *            now firmware size under protection is 448K - 256K = 192K
					 * if this demo can not meet your requirement, you should change !!!*/
					flash_lock_block_size = FLASH_LOCK_LOW_448K_MID1360C8;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 448K block!\n");
				}
				else{
					blc_flashProt.init_err = 1; //can not use LOCK LOW 1M for 512K capacity flash
				}
				break;
		#endif

		#if(FLASH_P25Q80U_SUPPORT_EN || FLASH_P25Q80SU_SUPPORT_EN)
			case MID146085:
				if(app_lock_block == FLASH_LOCK_FW_LOW_256K){
					flash_lock_block_size = FLASH_LOCK_LOW_256K_MID146085;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 256K block!\n");
				}
				else if(app_lock_block == FLASH_LOCK_FW_LOW_512K){
					flash_lock_block_size = FLASH_LOCK_LOW_512K_MID146085;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 512K block!\n");
				}
				else if(app_lock_block == FLASH_LOCK_FW_LOW_1M){
					/* attention 1: use can change this value according to application
					 * attention 2: can not lock stack SMP data storage area
					 * attention 3: firmware size under protection is not 1M, user should calculate
					 * demo code: choose 960K, leave 64K for system data(SMP storage data & calibration data & MAC address) and user data,
					 * 			  now firmware size under protection is 960K - 512K = 448K
					 * if this demo can not meet your requirement, you should change !!! */
					flash_lock_block_size = FLASH_LOCK_LOW_960K_MID146085;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 960K block!\n");
				}
				else{
					blc_flashProt.init_err = 1;
				}
				break;
		#endif

		#if (FLASH_GD25LD80C_SUPPORT_EN || FLASH_GD25LD80E_SUPPORT_EN) //1M capacity
			case MID1460C8:
				if(app_lock_block == FLASH_LOCK_FW_LOW_256K || app_lock_block == FLASH_LOCK_FW_LOW_512K){
					/* attention that :This flash type, minimum lock size is 768K, do not support 256K or other value
					 * demo code will lock 768K when user set OTA 128K or 256K as multiple boot address,
					 * system data(SMP storage data & calibration data & MAC address) is OK;
					 * user data must be stored in flash address bigger than 768K !!!
					 * if this demo code lock area do not meet your requirement, you can change it !!!*/
					flash_lock_block_size = FLASH_LOCK_LOW_768K_MID1460C8;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 768K block!\n");
				}
				else if(app_lock_block == FLASH_LOCK_FW_LOW_1M){
					/* attention 1: use can change this value according to application
					 * attention 2: can not lock stack SMP data storage area
					 * attention 3: firmware size under protection is not 1M, user should calculate
					 * demo code: choose 960K, leave 64K for system data(SMP storage data & calibration data & MAC address) and user data,
					 * 			  now firmware size under protection is 960K - 512K = 448K
					 * if this demo can not meet your requirement, you should change !!! */
					flash_lock_block_size = FLASH_LOCK_LOW_960K_MID1460C8;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 960K block!\n");
				}
				else{
					blc_flashProt.init_err = 1;
				}
				break;
		#endif

		#if (FLASH_P25Q16SU_SUPPORT_EN)	//2M capacity
			case MID156085:
				if(app_lock_block == FLASH_LOCK_FW_LOW_256K){
					flash_lock_block_size = FLASH_LOCK_LOW_256K_MID156085;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 256K block!\n");
				}
				else if(app_lock_block == FLASH_LOCK_FW_LOW_512K){
					flash_lock_block_size = FLASH_LOCK_LOW_512K_MID156085;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 512K block!\n");
				}
				else if(app_lock_block == FLASH_LOCK_FW_LOW_1M){
					flash_lock_block_size = FLASH_LOCK_LOW_1M_MID156085;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 1M block!\n");
				}
				else if(app_lock_block == FLASH_LOCK_FW_LOW_2M){
					/* attention 1: use can change this value according to application
					 * attention 2: can not lock stack SMP data storage area
					 * attention 3: firmware size under protection is not 2M, user should calculate
					 * demo code: choose 1984K, leave 64K for system data(SMP storage data & calibration data & MAC address) and user data,
					 * 			  now firmware size under protection is 1984K - 1M = 960K
					 * if this demo can not meet your requirement, you should change !!! */
					flash_lock_block_size = FLASH_LOCK_LOW_1984K_MID156085;
					tlkapi_printf(APP_FLASH_PROT_LOG_EN, "[FLASH][PROT] flash lock low 1984K block!\n");
				}
				else{
					blc_flashProt.init_err = 1;
				}
				break;
		#endif

		default:
			break;
	}

	return flash_lock_block_size;
}

/**
 * @brief      this function is used to lock flash.
 * @param[in]  flash_lock_block - flash lock block, different value for different flash type
 * @return     none
 */
void flash_lock(unsigned int flash_lock_cmd)
{
	if(blc_flashProt.init_err){
		return;
	}

	u16 cur_lock_status = flash_get_lock_status_mid();

	if(cur_lock_status == flash_lock_cmd){ //lock status is what we want, no need lock again

	}
	else{ //unlocked or locked block size is not what we want

		if(cur_lock_status != flash_unlock_status){ //locked block size is not what we want, need unlock first
			for(int i = 0; i < 3; i++){ //Unlock flash up to 3 times to prevent failure.
				flash_unlock_mid();
				cur_lock_status = flash_get_lock_status_mid();

				if(cur_lock_status == flash_unlock_status){ //unlock success
					break;
				}
			}
		}

		for(int i = 0; i < 3; i++) //Lock flash up to 3 times to prevent failure.
		{
			flash_lock_mid(flash_lock_cmd);
			cur_lock_status = flash_get_lock_status_mid();
			if(cur_lock_status == flash_lock_cmd){  //lock OK
				break;
			}
		}
	}
}

/**
 * @brief      this function is used to unlock flash.
 * @param[in]  none
 * @return     none
 */
void flash_unlock(void)
{
	if(blc_flashProt.init_err){
		return;
	}

	u16 cur_lock_status = flash_get_lock_status_mid();

	if(cur_lock_status != flash_unlock_status){ //not in lock status
		for(int i = 0; i < 3; i++){ //Unlock flash up to 3 times to prevent failure.
			flash_unlock_mid();
			cur_lock_status = flash_get_lock_status_mid();

			if(cur_lock_status == flash_unlock_status){ //unlock success
				break;
			}
		}
	}
}



#endif

/********************************************************************************************************
 * @file    rf_private_pa.c
 *
 * @brief   This is the source file for B85
 *
 * @author  Driver Group
 * @date    May 8,2018
 *
 * @par     Copyright (c) 2018, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
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
#include "rf_private_pa.h"
#include "gpio.h"
#include "compiler.h"
#include "../../stack/2p4g/genfsk_ll/genfsk_ll.h"

_attribute_data_retention_	rf_private_pa_callback_t  rf_pa_cb = 0;


_attribute_ram_code_sec_ void app_rf_pa_handler(pa_type type)
{
#if(PRIVATE_PA_ENABLE)
	if(type == PRIVATE_PA_TYPE_TX_ON){
	    gpio_set_output_en(PRIVATE_PA_RXEN_PIN, 0);
	    gpio_write(PRIVATE_PA_RXEN_PIN, 0);
	    gpio_set_output_en(PRIVATE_PA_TXEN_PIN, 1);
	    gpio_write(PRIVATE_PA_TXEN_PIN, 1);
}
	else if(type == PRIVATE_PA_TYPE_RX_ON){
	    gpio_set_output_en(PRIVATE_PA_RXEN_PIN, 1);
	    gpio_write(PRIVATE_PA_RXEN_PIN, 1);
	    gpio_set_output_en(PRIVATE_PA_TXEN_PIN, 0);
	    gpio_write(PRIVATE_PA_TXEN_PIN, 0);
	}
	else{
	    gpio_set_output_en(PRIVATE_PA_RXEN_PIN, 0);
	    gpio_write(PRIVATE_PA_RXEN_PIN, 0);
	    gpio_set_output_en(PRIVATE_PA_TXEN_PIN, 0);
	    gpio_write(PRIVATE_PA_TXEN_PIN, 0);
	}
#endif
}


void rf_pa_init(void)
{
#if(PRIVATE_PA_ENABLE)
    //rf_set_power_level_index (RF_POWER_0dBm);
    gpio_set_func(PRIVATE_PA_TXEN_PIN, AS_GPIO);
    gpio_set_output_en(PRIVATE_PA_TXEN_PIN, 0);
    gpio_write(PRIVATE_PA_TXEN_PIN, 0);

    gpio_set_func(PRIVATE_PA_RXEN_PIN, AS_GPIO);
    gpio_set_output_en(PRIVATE_PA_RXEN_PIN, 0);
    gpio_write(PRIVATE_PA_RXEN_PIN, 0);

    rf_pa_cb = app_rf_pa_handler;
#endif
}

_attribute_ram_code_sec_ void rf_pa_handler(short int flag)
{

	if (!rf_pa_cb)
		return;

	switch(flag){
	case FLD_RF_IRQ_TX:
				if (gen_fsk_current_mode == GEN_FSK_MD_STX || gen_fsk_current_mode == GEN_FSK_MD_SRX2TX)
				{
					rf_pa_cb(PRIVATE_PA_TYPE_OFF);
				}
				else if (gen_fsk_current_mode == GEN_FSK_MD_STX2RX)
				{
					rf_pa_cb(PRIVATE_PA_TYPE_RX_ON);
				}
                break;
	case FLD_RF_IRQ_RX:
				if(gen_fsk_current_mode == GEN_FSK_MD_SRX || gen_fsk_current_mode == GEN_FSK_MD_STX2RX)
				{
					rf_pa_cb(PRIVATE_PA_TYPE_OFF);
				}
				if (gen_fsk_current_mode == GEN_FSK_MD_SRX2TX )
				{
					rf_pa_cb(PRIVATE_PA_TYPE_TX_ON);
				}
                break;
	case FLD_RF_IRQ_FIRST_TIMEOUT:
				if (gen_fsk_current_mode == GEN_FSK_MD_SRX)
				{
					  rf_pa_cb(PRIVATE_PA_TYPE_OFF);
				}
                break;
	case FLD_RF_IRQ_RX_TIMEOUT:
				if (gen_fsk_current_mode == GEN_FSK_MD_STX2RX)
				{
					  rf_pa_cb(PRIVATE_PA_TYPE_OFF);
				}
                break;
	default:
		break;
}


}



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

extern my_fifo_t hci_rx_fifo;
extern void user_init_normal(void);
extern void main_loop(void);


/**
 * @brief   IRQ handler
 * @param   none.
 * @return  none.
 */
_attribute_ram_code_ void irq_handler(void)
{
	irq_blt_sdk_handler();

#if (HCI_ACCESS==HCI_USE_UART)
	#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
		if (dma_chn_irq_status_get(FLD_DMA_CHN_UART_RX) & FLD_DMA_CHN_UART_RX)
	#else
		if (dma_chn_irq_status_get(UART_NUM) & FLD_DMA_CHN_UART_RX)
	#endif
		{
			dma_chn_irq_status_clr(FLD_DMA_CHN_UART_RX);
			u8* w = hci_rx_fifo.p + (hci_rx_fifo.wptr & (hci_rx_fifo.num - 1))
					* hci_rx_fifo.size;
			if (w[0] != 0) {
				hci_rx_fifo.wptr++;
				u8* p = hci_rx_fifo.p + (hci_rx_fifo.wptr & (hci_rx_fifo.num - 1))
						* hci_rx_fifo.size;
				reg_dma_uart_rx_addr = (u16) ((u32) p); //switch uart RX dma address
			}
		}
	#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
		if (reg_uart_status1(UART_NUM) & FLD_UART_TX_DONE)
	#else
		if (reg_uart_status1 & FLD_UART_TX_DONE)
	#endif
		{
			extern volatile u8 isUartTxDone;
			isUartTxDone = 1;
			uart_clr_tx_done(UART_NUM);
		}
	#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
		if (reg_uart_status0(UART_NUM) & FLD_UART_RX_ERR_FLAG)//when stop bit error or parity error.
		{
			reg_uart_status0(UART_NUM) = FLD_UART_CLEAR_RX_FLAG;
		}
	#else
		if (reg_uart_status0 & FLD_UART_RX_ERR_FLAG)//when stop bit error or parity error.
		{
			reg_uart_status0 = FLD_UART_CLEAR_RX_FLAG;
		}
	#endif
#endif
}


/**
 * @brief		This is main function
 * @param[in]	none
 * @return      none
 */
_attribute_ram_code_ int main (void)    //must run in ramcode
{
#if (BLE_APP_PM_ENABLE)
	blc_pm_select_internal_32k_crystal();
#endif

#if(MCU_CORE_TYPE == MCU_CORE_825x)
	cpu_wakeup_init();
#else
	cpu_wakeup_init(LDO_MODE,INTERNAL_CAP_XTAL24M);
#endif

	rf_drv_ble_init();

	gpio_init(1);

	clock_init(SYS_CLK_TYPE);

#if	(PM_DEEPSLEEP_RETENTION_ENABLE)
	if( pm_is_MCU_deepRetentionWakeup() ){
		user_init_deepRetn();
	}
	else
#endif
	{
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

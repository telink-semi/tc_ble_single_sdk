/********************************************************************************************************
 * @file    app_phytest.c
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
#include "app_att.h"
#include "../common/blt_led.h"

#if (BLE_PHYTEST_MODE != PHYTEST_MODE_DISABLE )
/*----------------------------------------------*
 *	HCI TX FIFO  = 2 Bytes LEN + n Bytes Data.	*
 *												*
 *	T_txdata_buf = 4 Bytes LEN + n Bytes Data.	*
 *												*
 *	HCI_TXFIFO_SIZE = 2 + n.					*
 *												*
 *	UART_DATA_LEN = n.							*
 *												*
 *	UART_DATA_LEN = HCI_TXFIFO_SIZE - 2.		*
 * ---------------------------------------------*/
#define HCI_RXFIFO_SIZE		20
#define HCI_RXFIFO_NUM		2

#define HCI_TXFIFO_SIZE		20
#define HCI_TXFIFO_NUM		2

#define UART_DATA_LEN    	(HCI_TXFIFO_SIZE - 2)      // data max 252
typedef struct{
	unsigned int len;        // data max 252
	unsigned char data[UART_DATA_LEN];
}uart_data_t;

MYFIFO_INIT(hci_rx_fifo, HCI_RXFIFO_SIZE, HCI_RXFIFO_NUM);
MYFIFO_INIT(hci_tx_fifo, HCI_TXFIFO_SIZE, HCI_TXFIFO_NUM);






#if (BLE_PHYTEST_MODE == PHYTEST_MODE_OVER_HCI_WITH_UART)
volatile u8 isUartTxDone = 1;
#define   Tr_clrUartTxDone()    (isUartTxDone = 0)
#define   Tr_SetUartTxDone()    (isUartTxDone = 1)
#define   Tr_isUartTxDone()     (!isUartTxDone)

#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
#define UART_NUM							UART0
#define UART_CONVERT(...)					UART_NUM,__VA_ARGS__
#define UART_TX_PIN							GPIO_PD0
#define UART_RX_PIN							GPIO_PD1
#else
#define UART_NUM
#define UART_CONVERT(...)					__VA_ARGS__
#define UART_TX_PIN							UART_TX_PB1
#define UART_RX_PIN							UART_RX_PB0
#endif

#define   UART_BAUDRATE        				(115200)
/**
 * @brief		this function is used to process rx uart data.
 * @param[in]	none
 * @return      0 is ok
 */
	int rx_from_uart_cb (void)
	{
		if(my_fifo_get(&hci_rx_fifo) == 0)
		{
			return 0;
		}

		u8* p = my_fifo_get(&hci_rx_fifo);
		u32 rx_len = p[0]; //usually <= 255 so 1 byte should be sufficient

		if (rx_len)
		{
			blc_hci_handler(&p[4], rx_len - 4);
			my_fifo_pop(&hci_rx_fifo);
		}

		return 0;




	}

	/**
	 * @brief		this function is used to process tx uart data.
	 * @param[in]	none
	 * @return      0 is ok
	 */
	int tx_to_uart_cb (void)
	{
		uart_data_t T_txdata_buf;
		if(hci_tx_fifo.wptr == hci_tx_fifo.rptr){
			return 0;//have no data
		}
		if (Tr_isUartTxDone()) {
			return 0;
		}
		u8 *p =  my_fifo_get(&hci_tx_fifo);
		memcpy(&T_txdata_buf.data, p + 2, p[0] | (p[1] << 8));

		T_txdata_buf.len = p[0] | (p[1] << 8);
		Tr_clrUartTxDone();
		uart_send_dma(UART_CONVERT((unsigned char*) (&T_txdata_buf)));
		my_fifo_pop(&hci_tx_fifo);
		return 1;
	}
#endif




_attribute_ram_code_ void irq_phyTest_handler(void)
{
#if(FEATURE_TEST_MODE == TEST_BLE_PHY)
#if(MCU_CORE_TYPE != MCU_CORE_TC321X)
	if(dma_chn_irq_status_get() & FLD_DMA_CHN_UART_RX)
#elif(MCU_CORE_TYPE == MCU_CORE_TC321X)
	if(dma_chn_irq_status_get(FLD_DMA_CHN_UART_RX) & FLD_DMA_CHN_UART_RX)
#endif
	{
		dma_chn_irq_status_clr(FLD_DMA_CHN_UART_RX);
		u8* w = hci_rx_fifo.p + (hci_rx_fifo.wptr & (hci_rx_fifo.num-1)) * hci_rx_fifo.size;
		if(w[0]!=0)
		{
			my_fifo_next(&hci_rx_fifo);
			u8* p = hci_rx_fifo.p + (hci_rx_fifo.wptr & (hci_rx_fifo.num-1)) * hci_rx_fifo.size;
			reg_dma0_addr = (u16)((u32)p);
		}
	}
#if(MCU_CORE_TYPE != MCU_CORE_TC321X)
	if(dma_chn_irq_status_get() & FLD_DMA_CHN_UART_TX)
#elif(MCU_CORE_TYPE == MCU_CORE_TC321X)
	if(dma_chn_irq_status_get(FLD_DMA_CHN_UART_TX) & FLD_DMA_CHN_UART_TX)
#endif
	{
		dma_chn_irq_status_clr(FLD_DMA_CHN_UART_TX);
	}
#if(MCU_CORE_TYPE != MCU_CORE_TC321X)
	if (reg_uart_status1 & FLD_UART_TX_DONE)
#elif(MCU_CORE_TYPE == MCU_CORE_TC321X)
	if (reg_uart_status1(UART_NUM) & FLD_UART_TX_DONE)
#endif
	{
		Tr_SetUartTxDone();
		uart_clr_tx_done(UART_NUM);
	}
#endif
}



extern const led_cfg_t led_cfg[];

void app_trigger_phytest_mode(void)
{
	static u8 phyTestFlag = 0;
	if(!phyTestFlag && blc_ll_getCurrentState() != BLS_LINK_STATE_CONN){  //can not enter phytest in connection state
		phyTestFlag = 1;
		device_led_setup(led_cfg[4]);  //LED_SHINE_FAST: 4


		//adjust some rf parameters here if needed
		write_reg8(0x402, 0x2b);   //must: adjust rf packet preamble for BQB
		blc_phy_setPhyTestEnable( BLC_PHYTEST_ENABLE );
	}

}






void app_phytest_init(void)
{
	blc_phy_initPhyTest_module();
	blc_phy_preamble_length_set(11);

	#if(BLE_PHYTEST_MODE == PHYTEST_MODE_THROUGH_2_WIRE_UART || BLE_PHYTEST_MODE == PHYTEST_MODE_OVER_HCI_WITH_UART)  //uart
		uart_gpio_set(UART_CONVERT(UART_TX_PIN, UART_RX_PIN));
		uart_reset(UART_NUM);
	#endif

	uart_recbuff_init(UART_CONVERT((unsigned char*)hci_rx_fifo_b, hci_rx_fifo.size));


	uart_init_baudrate(UART_CONVERT(UART_BAUDRATE, CLOCK_SYS_CLOCK_HZ, PARITY_NONE, STOP_BIT_ONE));


	uart_dma_enable(UART_CONVERT(1,1));

	irq_set_mask(FLD_IRQ_DMA_EN);
	dma_chn_irq_enable(FLD_DMA_CHN_UART_RX, 1); //uart Rx dma irq enable
	uart_mask_tx_done_irq_enable(UART_NUM);

	uart_mask_error_irq_enable(UART_NUM);// open uart_error_mask,when stop bit error or parity error,it will enter error_interrupt.
	irq_enable_type(FLD_IRQ_UART_EN);// uart_tx use uart_txdone irq
	irq_enable();

	#if (MCU_CORE_TYPE != MCU_CORE_TC321X)
		uart_clr_tx_done();
	#endif

	isUartTxDone = 1;

	#if	(BLE_PHYTEST_MODE == PHYTEST_MODE_THROUGH_2_WIRE_UART)
		blc_register_hci_handler (phy_test_2_wire_rx_from_uart, phy_test_2_wire_tx_to_uart);
	#elif(BLE_PHYTEST_MODE == PHYTEST_MODE_OVER_HCI_WITH_UART)
		blc_register_hci_handler (rx_from_uart_cb, tx_to_uart_cb);		//default handler
	#endif

}








#endif  //end of  BLE_PHYTEST_MODE != PHYTEST_MODE_DISABLE

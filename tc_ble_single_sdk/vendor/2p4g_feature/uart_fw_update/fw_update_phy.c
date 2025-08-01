/********************************************************************************************************
 * @file    fw_update_phy.c
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
#include "driver.h"
#include "tl_common.h"
#include "fw_update_phy.h"
#include "fw_update.h"


#if (FEATURE_TEST_MODE == UART_FW_UPDATE)
#define PHY_TX_BUF_LEN          96
#define PHY_RX_BUF_LEN          96
#define PHY_RX_BUF_NUM          3


#define UART_DATA_LEN    		(96-4)      //data max (UART_DATA_LEN+4) must 16 byte aligned
typedef struct{
    unsigned int dma_len;        // dma len must be 4 byte
    unsigned char data[UART_DATA_LEN];
}uart_data_t;




PHY_Cb_t PHYRxCb = NULL;
static volatile unsigned char PHY_TxFinished = 0;

static uart_data_t PHY_TxBuf __attribute__ ((aligned (4))) = {};


static uart_data_t PHY_RxBuf[PHY_RX_BUF_NUM] __attribute__ ((aligned (4))) = {};
static unsigned char PHY_RxPtr = 0;



void FW_UPDATE_PHY_Init(const PHY_Cb_t RxCb)
{
    //Set UART Rx irq callback
    PHYRxCb = RxCb;
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    uart_recbuff_init(UART_MODULE_SEL,(unsigned char *)&PHY_RxBuf[PHY_RxPtr], PHY_RX_BUF_LEN);
    uart_gpio_set(UART_MODULE_SEL,UART_TX_PIN, UART_RX_PIN);// uart tx/rx pin set
    uart_reset(UART_MODULE_SEL);  //uart module power-on again.
    //baud rate: 115200
    uart_init_baudrate(UART_MODULE_SEL,115200,CLOCK_SYS_CLOCK_HZ,PARITY_NONE, STOP_BIT_ONE);
    uart_dma_enable(UART_MODULE_SEL,1, 1);  //uart data in hardware buffer moved by dma, so we need enable them first
    irq_enable_type(FLD_IRQ_DMA_EN);// uart_rx use dma_rx irq
    dma_chn_irq_enable(FLD_DMA_CHN_UART_RX, 1);     //uart Rx dma irq enable
    uart_mask_tx_done_irq_enable(UART_MODULE_SEL);
    uart_mask_error_irq_enable(UART_MODULE_SEL);// open uart_error_mask,when stop bit error or parity error,it will enter error_interrupt.
    irq_enable_type(FLD_IRQ_UART_EN);// uart_tx use uart_txdone irq
    irq_enable();
#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
    //config UART module
    uart_recbuff_init(  (unsigned char*)&PHY_RxBuf[PHY_RxPtr], PHY_RX_BUF_LEN);
    uart_gpio_set(UART_TX_PA2, UART_RX_PD6);// uart tx/rx pin set
    uart_reset();  //will reset uart digital registers from 0x90 ~ 0x9f, so uart setting must set after this reset
    uart_init(12, 15, PARITY_NONE, STOP_BIT_ONE);   //115200
    uart_dma_enable(1, 1);     //uart data in hardware buffer moved by dma, so we need enable them first
    irq_set_mask(FLD_IRQ_DMA_EN);
    dma_chn_irq_enable(FLD_DMA_CHN_UART_RX | FLD_DMA_CHN_UART_TX, 1);       //uart Rx/Tx dma irq enable
    uart_irq_enable(0, 0);      //uart Rx/Tx irq no need, disable them

#endif

}

int FW_UPDATE_PHY_SendData(const unsigned char *Payload, const int PayloadLen)
{
    //set UART DMA length
	PHY_TxBuf.dma_len = PayloadLen;

    //fill the contents of UART transmission
    memcpy(&PHY_TxBuf.data, Payload, PayloadLen);

#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
    uart_send_dma(UART_MODULE_SEL,(unsigned char*)&PHY_TxBuf);
#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
    uart_dma_send((unsigned char*)&PHY_TxBuf);
#endif
    while(!PHY_TxFinished);
    PHY_TxFinished = 0;

    return PayloadLen;
}

void FW_UPDATE_PHY_RxIrqHandler(void)
{
    //set next rx_buf
	unsigned char *RxPacket = PHY_RxBuf[PHY_RxPtr].data;
	PHY_RxPtr = (PHY_RxPtr + 1) % PHY_RX_BUF_NUM;
#if(MCU_CORE_TYPE == MCU_CORE_TC321X)
	uart_recbuff_init(UART_MODULE_SEL,(unsigned char *)&PHY_RxBuf[PHY_RxPtr], PHY_RX_BUF_LEN);
#elif(MCU_CORE_TYPE == MCU_CORE_825x || MCU_CORE_TYPE == MCU_CORE_827x)
	uart_recbuff_init((unsigned char *)&PHY_RxBuf[PHY_RxPtr], PHY_RX_BUF_LEN);
#endif
    if (PHYRxCb) {
        PHYRxCb(&RxPacket[0]);
    }
}

void FW_UPDATE_PHY_TxIrqHandler(void)
{
    PHY_TxFinished = 1;
}
#endif

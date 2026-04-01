/********************************************************************************************************
 * @file    putchar.c
 *
 * @brief   This is the source file for B85
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
#ifndef WIN32

#include "tl_common.h"
#include "application/print/putchar.h"
#include "drivers.h"


#ifndef USB_PRINT_DEBUG_ENABLE
#define USB_PRINT_DEBUG_ENABLE                    			0
#endif

#if (MCU_CORE_TYPE == CHIP_TYPE_TC321X && USB_PRINT_DEBUG_ENABLE)
	#error "CHIP TC321X doesn't support USB!"
#endif

#if (USB_PRINT_DEBUG_ENABLE)
#define USB_PRINT_TIMEOUT	 10		//  about 10us at 30MHz
#define USB_SWIRE_BUFF_SIZE  248	// 256 - 8

#define USB_EP_IN  		(USB_EDP_PRINTER_IN  & 0X07)	//  from the point of HOST 's view,  IN is the printf out
#define USB_EP_OUT  	(USB_EDP_PRINTER_OUT & 0X07)

int usb_putc(int c) {
	int i = 0;
	while(i ++ < USB_PRINT_TIMEOUT){
		if(!(reg_usb_ep8_fifo_mode & FLD_USB_ENP8_FULL_FLAG)){
			reg_usb_ep_dat(USB_EP_IN) = (unsigned char)c;
			return c;
		}
	}
	return -1;
}

static inline void swire_set_clock(unsigned char div){
	reg_swire_ctl2 = div;
}

static int swire_is_init = 0;
void swire_init(){
#if(USB_SOMATIC_ENABLE)
    //  when USB_SOMATIC_ENABLE, USB_EDP_PRINTER_OUT disable
#else
//	r_usb.ep_adr[USB_EP_IN] = r_usb.ep_adr[USB_EP_OUT] = 0;
	reg_usb_ep_ptr(USB_EP_IN) = reg_usb_ep_ptr(USB_EP_OUT) = 0;
	reg_usb_ep8_send_max = 64;				// 32 * 8 == 256

	//swire_set_clock(2);

#endif
}

int swire_putc(int c) {
#if(USB_SOMATIC_ENABLE)
    //  when USB_SOMATIC_ENABLE, USB_EDP_PRINTER_OUT disable
#else
	if(!swire_is_init){
		swire_init();
		swire_is_init = 1;
	}
	int i = 0;
	while(i ++ < USB_PRINT_TIMEOUT){
		if(reg_usb_ep_ptr(USB_EP_IN) - reg_usb_ep_ptr(USB_EP_OUT) <= USB_SWIRE_BUFF_SIZE){	//  not full
			reg_usb_ep_dat(USB_EP_IN) = (unsigned char)c;
			return c;
		}
	}
#endif
	return -1;
}
#endif

#if (UART_PRINT_DEBUG_ENABLE)

#ifndef DEBUG_INFO_TX_PIN
#error  "Please define DEBUG_INFO_TX_PIN in your board header file and configure the pin as GPIO output mode"
#endif

#if (MCU_CORE_TYPE == CHIP_TYPE_TC321X)
#define UART_DEBUG_TX_PIN_REG	reg_gpio_out_set_clear(DEBUG_INFO_TX_PIN)
#else
#define UART_DEBUG_TX_PIN_REG	reg_gpio_out(DEBUG_INFO_TX_PIN)
#endif

/* system Timer for B85m: 16Mhz, Constant */
#ifndef		BIT_INTERVAL
#define		BIT_INTERVAL		(16000000/PRINT_BAUD_RATE)
#endif
extern volatile unsigned char sys_clock_print;

_attribute_ram_code_
void uart_putb(unsigned short *p)
{
	unsigned int  j = 0;
#if(CLOCK_SYS_CLOCK_HZ == 16000000)
	unsigned int i =0;
	unsigned int  bit_nop = sys_clock_print * 125000/PRINT_BAUD_RATE-2;
#elif(CLOCK_SYS_CLOCK_HZ == 24000000)
	unsigned int i =0;
	unsigned int  bit_nop = sys_clock_print * 125000/PRINT_BAUD_RATE-1;
#elif(CLOCK_SYS_CLOCK_HZ == 32000000 || CLOCK_SYS_CLOCK_HZ == 48000000)
	unsigned long t1 = read_reg32(0x740);
	unsigned long t2 = 0;
#endif
	for(j=0;j<10;j++)
	{
		/*
		 * according to nop to control UART period when mcu clock is 16M and 32M, the baud rate is 1M and the period is 1us, so we need 16 nop for a loop
		 * bit_nop is determined by system clock, and control how many times the 'for loop' run, depending on the clock, decide how much nop to compensate
		 */
#if(CLOCK_SYS_CLOCK_HZ == 16000000 || CLOCK_SYS_CLOCK_HZ == 24000000)
		/* system clock 16M and 24M*/
		for(i=0; i<bit_nop;i++)
		{
			asm("tnop");
		}
		asm("tnop");
		asm("tnop");
		asm("tnop");
		#if(CLOCK_SYS_CLOCK_HZ == 24000000 && MCU_CORE_TYPE != CHIP_TYPE_TC321X)
			asm("tnop");
		#endif
#elif(CLOCK_SYS_CLOCK_HZ == 32000000 || CLOCK_SYS_CLOCK_HZ == 48000000)
		/*  system clock 32M and 48M  */
		t2 = t1;
		while(t1 - t2 < BIT_INTERVAL){
			t1  = read_reg32(0x740);
		}
#endif

		UART_DEBUG_TX_PIN_REG = p[j];
	}
}

int uart_putc(char byte) //GPIO simulate uart print func
{
#if(MCU_CORE_TYPE == CHIP_TYPE_TC321X)
    unsigned short tmp_bit0 = (DEBUG_INFO_TX_PIN & 0xff) << 8;
    unsigned short tmp_bit1 = DEBUG_INFO_TX_PIN & 0xff;
#else
	unsigned short tmp_bit0 = UART_DEBUG_TX_PIN_REG & (~(DEBUG_INFO_TX_PIN & 0xff));
	unsigned short tmp_bit1 = UART_DEBUG_TX_PIN_REG | (DEBUG_INFO_TX_PIN & 0xff);
#endif
	unsigned short bit[10] = {0};

	bit[0] = tmp_bit0;
	bit[1] = (byte & 0x01)? tmp_bit1 : tmp_bit0;
	bit[2] = ((byte>>1) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[3] = ((byte>>2) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[4] = ((byte>>3) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[5] = ((byte>>4) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[6] = ((byte>>5) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[7] = ((byte>>6) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[8] = ((byte>>7) & 0x01)? tmp_bit1 : tmp_bit0;
	bit[9] = tmp_bit1;

	unsigned char r = irq_disable();
	uart_putb(bit);
	irq_restore(r);
	return byte;
}
#endif

int putchar(int c){
#if (UART_PRINT_DEBUG_ENABLE)
	return uart_putc((char)c);
#elif (USB_PRINT_DEBUG_ENABLE)
	if(reg_usb_host_conn){
		swire_is_init = 0;		// should re-init swire if connect swire again
		return usb_putc((char)c);
	}else{
		return swire_putc((char)c);
	}
#else
	return c;
#endif
}

#endif


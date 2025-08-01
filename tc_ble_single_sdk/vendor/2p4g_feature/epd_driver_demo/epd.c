/********************************************************************************************************
 * @file    epd.c
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
#include "driver.h"
#include "tl_common.h"
#include "epd.h"
#include "fonts.h"
#include "gui.h"
#define TAG_USE          1

#define EPD_COST         1

#define Simulation          1

#ifdef TAG_USE
#if (__PROJECT_8258_2P4G_FEATURE__)
#define EPD_SPI_CS_PIN        GPIO_PD2
#define EPD_SPI_CK_PIN        GPIO_PD7
#define EPD_SPI_DO_PIN        GPIO_PB7
#define EPD_RESET_PIN         GPIO_PC0
#define EPD_BS1_PIN           GPIO_PB5
#define EPD_PWR_CTRL_PIN      GPIO_PB1
#define EPD_BUSY_PIN          GPIO_PB4
#define EPD_DC_PIN            GPIO_PD3
#elif (__PROJECT_8278_2P4G_FEATURE__)
#define EPD_SPI_CS_PIN        GPIO_PD2
#define EPD_SPI_CK_PIN        GPIO_PD7
#define EPD_SPI_DO_PIN        GPIO_PB7
#define EPD_RESET_PIN         GPIO_PB3
#define EPD_BS1_PIN           GPIO_PB5
#define EPD_PWR_CTRL_PIN      GPIO_PB0
#define EPD_BUSY_PIN          GPIO_PB4
#define EPD_DC_PIN            GPIO_PB2
#elif (__PROJECT_TC321X_2P4G_FEATURE__)
#define EPD_SPI_CS_PIN        GPIO_PD5
#define EPD_SPI_CK_PIN        GPIO_PD6
#define EPD_SPI_DO_PIN        GPIO_PD7
#define EPD_RESET_PIN         GPIO_PD2
#define EPD_BS1_PIN           GPIO_PD0
#define EPD_PWR_CTRL_PIN      GPIO_PE0
#define EPD_BUSY_PIN          GPIO_PD1
#define EPD_DC_PIN            GPIO_PD3
#endif
#endif

#define     EPD_POWER_ON()     gpio_write(EPD_PWR_CTRL_PIN,0)

#define     EPD_POWER_OFF()    do{\
    gpio_write(EPD_DC_PIN,0); \
    } while(0)

#define EPD_ENABLE_WRITE_CMD()   gpio_write(EPD_DC_PIN,0)
#define EPD_ENABLE_WRITE_DATA()  gpio_write(EPD_DC_PIN,1)

#define EPD_IS_BUSY()            (!gpio_read(EPD_BUSY_PIN))

const unsigned int epd_size_pixels[EPD_SIZE_NUM] = {2756, 4736, 15000};

void EPD_DLY_LP(unsigned int ms)
{

#ifdef EPD_COST
	cpu_sleep_wakeup(SUSPEND_MODE, PM_WAKEUP_TIMER, ClockTime()+(ms-40)*1000*sys_tick_per_us);
	WaitMs(40);
#else
	cpu_sleep_wakeup(SUSPEND_MODE, PM_WAKEUP_TIMER, ClockTime()+ ms*1000*sys_tick_per_us);
#endif

}

void EPD_Init(void)
{
    //reset pin configuration
	gpio_set_func(EPD_RESET_PIN,AS_GPIO);
    gpio_set_output_en(EPD_RESET_PIN, 1);
    gpio_set_input_en(EPD_RESET_PIN, 0);
    gpio_write(EPD_RESET_PIN, 0);
    //BS1 pin configuration
    //select 4-lines SPI interface
    gpio_set_func(EPD_BS1_PIN,AS_GPIO);
    gpio_set_output_en(EPD_BS1_PIN, 1);
    gpio_set_input_en(EPD_BS1_PIN, 0);
    gpio_write(EPD_BS1_PIN, 0);
    //EPD power supply control pin configuration
    gpio_set_func(EPD_PWR_CTRL_PIN,AS_GPIO);
    gpio_set_output_en(EPD_PWR_CTRL_PIN, 1);
    gpio_set_input_en(EPD_PWR_CTRL_PIN, 0);
    gpio_write(EPD_PWR_CTRL_PIN, 1);
    //EPD Data/Command control pin configuration
    gpio_set_func(EPD_DC_PIN,AS_GPIO);
    gpio_set_output_en(EPD_DC_PIN, 1);
    gpio_set_input_en(EPD_DC_PIN, 0);
    gpio_write(EPD_DC_PIN, 0);
    //EPD Busy Status pin configuratioin
    gpio_set_func(EPD_BUSY_PIN,AS_GPIO);
    gpio_set_output_en(EPD_BUSY_PIN, 0);
    gpio_set_input_en(EPD_BUSY_PIN, 1);
    gpio_setup_up_down_resistor(EPD_BUSY_PIN,PM_PIN_PULLUP_1M);

#ifdef Simulation

        //cs pin configuration
    	gpio_set_func(EPD_SPI_CS_PIN,AS_GPIO);
    	gpio_set_output_en(EPD_SPI_CS_PIN, 1);
    	gpio_set_input_en(EPD_SPI_CS_PIN, 0);
    	gpio_write(EPD_SPI_CS_PIN, 1);
    	//clk pin configuration
    	gpio_set_func(EPD_SPI_CK_PIN,AS_GPIO);
    	gpio_set_output_en(EPD_SPI_CK_PIN, 1);
    	gpio_set_input_en(EPD_SPI_CK_PIN, 0);
    	gpio_write(EPD_SPI_CK_PIN, 0);
    	//DO pin configuration
    	gpio_set_func(EPD_SPI_DO_PIN,AS_GPIO);
    	gpio_set_output_en(EPD_SPI_DO_PIN, 1);
    	gpio_set_input_en(EPD_SPI_DO_PIN, 0);
    	gpio_write(EPD_SPI_DO_PIN, 1);
#else
        spi_master_init(SPI_CLK_1M, SPI_MODE0);
        spi_master_gpio_set(SPI_GPIO_GROUP_B6B7D2D7);
        spi_masterCSpin_select(EPD_SPI_CS_PIN);

#endif

}



static void EPD_SPI_Write(unsigned char value)
{
#ifdef Simulation
	unsigned char i;

	WaitUs(10);
	for(i=0; i<8; i++)
	{
	gpio_write(EPD_SPI_CK_PIN,0);
	 WaitUs(1);
	if(value & 0x80){
	gpio_write(EPD_SPI_DO_PIN,1);
	}
	else{
	gpio_write(EPD_SPI_DO_PIN,0);
	}
	value = (value << 1);
	WaitUs(1);
	WaitUs(1);
	gpio_write(EPD_SPI_CK_PIN,1);
	WaitUs(1);
	}
#else
	spi_write(&value, 1, 0, 0, EPD_SPI_CS_PIN);
#endif
}

static void EPD_WriteCmd(unsigned char cmd)
{
    //WaitUs(10);
    gpio_write(EPD_SPI_CS_PIN,0);
    EPD_ENABLE_WRITE_CMD();
    EPD_SPI_Write(cmd);
    gpio_write(EPD_SPI_CS_PIN,1);
}

static void EPD_WriteData(unsigned char data)
{
    //WaitUs(10);
    gpio_write(EPD_SPI_CS_PIN,0);
    EPD_ENABLE_WRITE_DATA();
    EPD_SPI_Write(data);
    gpio_write(EPD_SPI_CS_PIN,1);
}

static void EPD_CheckStatus(void)
{

    while (EPD_IS_BUSY()) {
    	EPD_DLY_LP(200);
    }
}

static void EPD_LoadImage(unsigned char *image, int size)
{
    int i;
    EPD_WriteCmd(0x10);
    for (i = 0; i < size; i++) {
        EPD_WriteData(0xff);
    }

    EPD_WriteCmd(0x13);
    for (i = 0; i < size; i++) {
        EPD_WriteData(image[i]);
    }
    WaitMs(2);
}

//refer to the datasheet(WF0290T5PCZ10230HU51) 8.2 (page 36)
void EPD_Display(unsigned char *image, int size)
{


	//system power
    EPD_POWER_ON();
    //Reset the EPD driver IC
    gpio_write(EPD_BS1_PIN,0);
    gpio_write(EPD_RESET_PIN,0);
    EPD_DLY_LP(1000);
    gpio_write(EPD_RESET_PIN,1);
    EPD_DLY_LP(1000);
    //Booster soft start
    EPD_WriteCmd(0x06);
    EPD_WriteData(0x17);
    EPD_WriteData(0x17);
    EPD_WriteData(0x17);
    //power on
    EPD_WriteCmd(0x04);

    //check BUSY pin
    EPD_CheckStatus();

    //panel setting
    EPD_WriteCmd(0x00);
    EPD_WriteData(0x1f);
    EPD_WriteData((size == epd_size_pixels[EPD_SIZE_4_2]) ? 0xaf : 0x0f);

    //resolution setting
    EPD_WriteCmd(0x61);

    EPD_WriteData(0x80);
    EPD_WriteData(0x01);
    EPD_WriteData(0x28);


    //Vcom and data interval setting
    EPD_WriteCmd(0X50);
    EPD_WriteData(0x97);

    //load image data to EPD
    EPD_LoadImage(image, size);

    //trigger display refresh
    EPD_WriteCmd(0x12);
    EPD_DLY_LP(100);
    //check BUSY pin
    EPD_CheckStatus();

    //Vcom and data interval setting
    EPD_WriteCmd(0X50);
    EPD_WriteData(0xf7);

    //power off
    EPD_WriteCmd(0X02);

    //deep sleep
    EPD_WriteCmd(0X07);
    EPD_WriteData(0xa5);

    EPD_POWER_OFF();
}



void EPD_Close(void)
{

   // GPIO_AllTurnOff();
	gpio_set_func(EPD_RESET_PIN,AS_GPIO);
	gpio_setup_up_down_resistor(EPD_RESET_PIN,PM_PIN_PULLUP_10K);
    gpio_set_output_en(EPD_RESET_PIN, 0);
    gpio_set_input_en(EPD_RESET_PIN, 1);
    gpio_setup_up_down_resistor(EPD_RESET_PIN,PM_PIN_PULLUP_10K);

    //BS1 pin configuration
    gpio_set_func(EPD_BS1_PIN,AS_GPIO);
    gpio_set_output_en(EPD_BS1_PIN, 0);
    gpio_set_input_en(EPD_BS1_PIN, 1);
    gpio_setup_up_down_resistor(EPD_BS1_PIN,PM_PIN_PULLDOWN_100K);

    //EPD power supply control pin configuration
    gpio_set_func(EPD_PWR_CTRL_PIN,AS_GPIO);
    gpio_setup_up_down_resistor(EPD_PWR_CTRL_PIN,PM_PIN_PULLDOWN_100K);
    gpio_set_output_en(EPD_PWR_CTRL_PIN, 0);
    gpio_set_input_en(EPD_PWR_CTRL_PIN, 1);
    gpio_setup_up_down_resistor(EPD_PWR_CTRL_PIN,PM_PIN_PULLDOWN_100K);


    //EPD Data/Command control pin configuration
    gpio_set_func(EPD_DC_PIN,AS_GPIO);
    gpio_set_output_en(EPD_DC_PIN, 0);
    gpio_set_input_en(EPD_DC_PIN, 1);
    gpio_setup_up_down_resistor(EPD_DC_PIN,PM_PIN_PULLUP_1M);


    //EPD Busy Status pin configuratioin
    gpio_set_func(EPD_BUSY_PIN,AS_GPIO);
    gpio_set_output_en(EPD_BUSY_PIN, 0);
    gpio_set_input_en(EPD_BUSY_PIN, 1);
    gpio_setup_up_down_resistor(EPD_BUSY_PIN,PM_PIN_PULLDOWN_100K); //turn off pull-up resistor


    //Turn off all SPI IOs
    gpio_set_func(EPD_SPI_CS_PIN,AS_GPIO);
    gpio_set_output_en(EPD_SPI_CS_PIN, 0);
    gpio_set_input_en(EPD_SPI_CS_PIN, 1);
    gpio_setup_up_down_resistor(EPD_SPI_CS_PIN,PM_PIN_PULLUP_1M);


    gpio_set_func(EPD_SPI_CK_PIN,AS_GPIO);
    gpio_set_output_en(EPD_SPI_CK_PIN, 0);
    gpio_set_input_en(EPD_SPI_CK_PIN, 1);
    gpio_setup_up_down_resistor(EPD_SPI_CK_PIN,PM_PIN_PULLDOWN_100K);


    gpio_set_func(EPD_SPI_DO_PIN,AS_GPIO);
    gpio_set_output_en(EPD_SPI_DO_PIN, 0);
    gpio_set_input_en(EPD_SPI_DO_PIN, 1);
    gpio_setup_up_down_resistor(EPD_SPI_DO_PIN,PM_PIN_PULLDOWN_100K);




}

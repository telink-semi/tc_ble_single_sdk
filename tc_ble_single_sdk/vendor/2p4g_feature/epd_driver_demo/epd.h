/********************************************************************************************************
 * @file    epd.h
 *
 * @brief   This is the header file for 2.4G SDK
 *
 * @author  2.4G Group
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
#ifndef _EPD_H_
#define _EPD_H_
#define FLASH_LOCATION_NODE_IMAGE    0x01e000
#define FLASH_SECTOR_SIZE            0x1000
#define FLASH_PAGE_SIZE              256

enum {
    EPD_SIZE_2_1 = 0,
    EPD_SIZE_2_9,
    EPD_SIZE_4_2,
    EPD_SIZE_NUM,
};

extern const unsigned int epd_size_pixels[EPD_SIZE_NUM];

extern void EPD_Init(void);
extern void EPD_Display(unsigned char *image, int size);
extern void EPD_Close(void);

#endif /*_EPD_H_*/

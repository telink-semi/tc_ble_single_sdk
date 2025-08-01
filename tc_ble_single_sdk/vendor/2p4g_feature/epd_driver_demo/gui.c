/********************************************************************************************************
 * @file    gui.c
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

#define X_channel 16
#define Y_channel 15
#define X_MAX 296
#define Y_MAX 128

#define E_UNIT 8
#define T_UNIT 2

#define GUI_FONT_WIDTH     8
#define GUI_FONT_HEIGHT    16
#define EPD_DATA_SIZE      4736

enum {
    GUI_ERROR_CODE_NO_ERROR = 0,
    GUI_ERROR_CODE_OVERFLOW,
    GUI_ERROR_CODE_NOT_SUPPORT_CHAR,
};

typedef struct {
    const unsigned char *font;
    unsigned char data;
}Font_TypeDef;

Font_TypeDef font_tbl[] = {
    {NUM_0, '0'},
    {NUM_1, '1'},
    {NUM_2, '2'},
    {NUM_3, '3'},
    {NUM_4, '4'},
    {NUM_5, '5'},
    {NUM_6, '6'},
    {NUM_7, '7'},
    {NUM_8, '8'},
    {NUM_9, '9'},
    {CHAR_A, 'A'},
    {CHAR_B, 'B'},
    {CHAR_C, 'C'},
    {CHAR_D, 'D'},
    {CHAR_E, 'E'},
    {CHAR_F, 'F'},
    {CHAR_G, 'G'},
    {CHAR_H, 'H'},
    {CHAR_I, 'I'},
    {CHAR_J, 'J'},
    {CHAR_K, 'K'},
    {CHAR_L, 'L'},
    {CHAR_M, 'M'},
    {CHAR_N, 'N'},
    {CHAR_O, 'O'},
    {CHAR_P, 'P'},
    {CHAR_Q, 'Q'},
    {CHAR_R, 'R'},
    {CHAR_S, 'S'},
    {CHAR_T, 'T'},
    {CHAR_U, 'U'},
    {CHAR_V, 'V'},
    {CHAR_W, 'W'},
    {CHAR_X, 'X'},
    {CHAR_Y, 'Y'},
    {CHAR_Z, 'Z'},
    {CHAR_Ove, '.'},
    {CHAR_Space, ' '},
};

__attribute__((section(".ram_code"))) static unsigned char *GUI_GetFont(unsigned char data)
{
    int i = 0;
    
    for (i = 0; i < sizeof(font_tbl)/sizeof(Font_TypeDef); i++) {
        if (data == font_tbl[i].data) {
            return (unsigned char *)(font_tbl[i].font);
        }
    }

    return NULL;
}

__attribute__((section(".ram_code"))) void GUI_BytesToHexStr(const unsigned char *bytes, int len, unsigned char *str)
{
    unsigned char tmp;
    int i;
    for (i = 0; i < len; i++) {
        tmp = bytes[len-1-i] / 16;
        if (tmp < 10) {
            str[i*2] = '0' + tmp;
        }
        else {
            str[i*2] = 'A' + (tmp - 10);
        }

        tmp = bytes[len-1-i] % 16;
        if (tmp < 10) {
            str[i*2+1] = '0' + tmp;
        }
        else {
            str[i*2+1] = 'A' + (tmp - 10);
        }
    }
    str[2*len] = '\0';
}

__attribute__((section(".ram_code")))void GUI_IntToDecStr(unsigned int data, unsigned char *str)
{
    unsigned char tmp;
    int i = 0;
    while (data) {
        tmp = data % 10;
        data /= 10;
        str[i++] = tmp + '0';
    }
    str[i] = '\0';

    int j = strlen((const char *)str) - 1;
    for (i = 0; i < j; i++, j--) {
        tmp = str[i];
        str[i] = str[j];
        str[j] = tmp;
    }
}

__attribute__((section(".ram_code"))) unsigned char GUI_DispChar(unsigned char *image, int x, int y, unsigned char data)
{
    assert(image);

    int i, j;

    if ((x+GUI_FONT_WIDTH) > X_MAX || (y+GUI_FONT_HEIGHT) > Y_MAX) {
        return GUI_ERROR_CODE_OVERFLOW;
    }

    unsigned char *font = GUI_GetFont(data);
    if (NULL == font) {
        return GUI_ERROR_CODE_NOT_SUPPORT_CHAR;
    }

    for (i = 0; i < E_UNIT; i++) {
        for (j = 0; j < T_UNIT; j++) {
            image[((x+i)*X_channel+Y_channel-y-j)] = font[T_UNIT*i+!j];
        }  
    }

    return GUI_ERROR_CODE_NO_ERROR;
}

__attribute__((section(".ram_code")))unsigned char GUI_DispStr(unsigned char *image, int x, int y, char *str, unsigned char dir)
{
    assert(image);

    int len = strlen((const char *)str);

    int i;
    if (dir) { //horizontal 
        if ((len*GUI_FONT_WIDTH+x) > X_MAX) {
            return GUI_ERROR_CODE_OVERFLOW;
        }
        for (i = 0; i < len; i++) {
            GUI_DispChar(image, x+i*8, y, str[i]);
        }
    }
    else {
        if ((len*GUI_FONT_HEIGHT+y) > Y_MAX) {
            return GUI_ERROR_CODE_OVERFLOW;
        }
        for (i = 0; i < len; i++) {
            GUI_DispChar(image, x, y+i*2, str[i]);
        }
    }
    return GUI_ERROR_CODE_NO_ERROR;
}

__attribute__((section(".ram_code")))unsigned char GUI_DispPic(unsigned char *image, int x, int y, const unsigned char *pic, unsigned char width, unsigned char height)
{
    int i, j, bit, n_bit;

    n_bit = (height) / E_UNIT;

    if (y) {
        bit = (height + y) / E_UNIT;
    }
    else {
        bit = height / E_UNIT;
    }

    if (x > X_MAX || y > Y_MAX) {
        return GUI_ERROR_CODE_OVERFLOW;
    }

   
    for (i = 0; i < width; i++) {
        for (j = 0; j < n_bit; j++) {  
            image[((x+i)*X_channel+Y_channel-y-j)] = pic[n_bit*(1+i)-j-1];
        }
    }

    return GUI_ERROR_CODE_NO_ERROR;
}

__attribute__((section(".ram_code")))void GUI_Clear(unsigned char *image, unsigned char colour)
{
    unsigned char data = colour ? 0xff : 0;
    memset(image, data, EPD_DATA_SIZE);
}

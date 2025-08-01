/********************************************************************************************************
 * @file    gui.h
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
#ifndef  _TELINK_ESL_GUI_H
#define  _TELINK_ESL_GUI_H

#define GUI_FONT_WIDTH     8
#define GUI_FONT_HEIGHT    16

typedef struct {
int X_channel;
int Y_channel;
int X_MAX;
int Y_MAX;
} NODESIZE_Typedef;

extern NODESIZE_Typedef Node_Size;

extern void GUI_BytesToHexStr(const unsigned char *bytes, int len, unsigned char *str);
extern void GUI_IntToDecStr(unsigned int data, unsigned char *str);
extern unsigned char GUI_DispChar(unsigned char *image, int x, int y, unsigned char data);
extern unsigned char GUI_DispStr(unsigned char *image, int x, int y, char *str, unsigned char dir); //dir: 1-horizontal 0-vertical
extern unsigned char GUI_DispPic(unsigned char *image, int x, int y, const unsigned char *pic, unsigned char width, unsigned char height);
extern void GUI_Clear(unsigned char *image, unsigned char colour); //colour: 0-black, 1-white



#endif

/********************************************************************************************************
 * @file    sync_word_check.h
 *
 * @brief   This is the header file for 2.4G SDK
 *
 * @author  2.4G GROUP
 * @date    05,2025
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
#include "tl_common.h"

#ifndef SYNC_WORD_CHECK_H_
#define SYNC_WORD_CHECK_H_

//typedef char   int8_t;
//typedef short  int16_t;
//typedef int    int32_t;
//typedef unsigned char   uint8_t;
//typedef unsigned short  uint16_t;
//typedef unsigned int    uint32_t;
//typedef unsigned long long int    uint64_t;


/**
 * @brief This function is to check whether sync word is valid, the sync_word here is consistent with
 *          the parameter sync_word passed into function gen_fsk_sync_word_set.
 *          Test results using 100,000 randomly generated 4-byte access codes showed approximately 62% pass rate.
 *          Detailed test results are as follows:
 *          ret count:
 *          ( 0)62561
 *          (-1)0
 *          (-2)0
 *          (-3)10882
 *          (-4)0
 *          (-5)12258
 *          (-6)1548
 *          (-7)12742
 *          (-8)9
 * @param[in] sync_word sync word
 * @param[in] len       length of access code.
 * @return              0:valid;
 *                      -1:length error;
 *                      -2:all bytes same;
 *                      -3:transitions of uppper 7 bits less than 2;.
 *                      -4:the current AC is the same as or only differs by 1bit from the specific AC;
 *                      -5:the 8 LSb have at most three 1;
 *                      -6:the 16 LSb have more than 11 transitions;
 *                      -7:consecutive more than 6;
 *                      -8:have more than 24 transitions;
 */
int sync_word_is_invalid(const unsigned char* sync_word, unsigned char len);

#endif

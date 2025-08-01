/********************************************************************************************************
 * @file    sync_word_check.c
 *
 * @brief   This is the source file for 2.4G SDK
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
#include "sync_word_check.h"
#include "string.h"

#define SYNC_WORD_CHECK_LOG_EN          0

#if SYNC_WORD_CHECK_LOG_EN
#define log_printf  tlk_printf
#else
#define log_printf
#endif

#define COMPARE_FOR_BYTES                1
#define CHECK_UPPER_7_BITS_TRANSITIONS   1

#if 0
#include <stdarg.h>
__attribute__((used)) void tlk_printf(const char *format, ...) {
    char str_buff[100] = {0};
    va_list args;

    va_start(args, format);

    vsnprintf(str_buff, sizeof(str_buff)-1, format, args);

    va_end(args);

    tlkapi_sendData(DUMP_APP_MSG, (unsigned char *)str_buff, 0, 0);
}
#endif

static int tl_hacker_popcnt(unsigned int n)
{
//    tlk_printf("popcnt:0x%08X\n", n);
    n -= (n >> 1) & 0x55555555;
    n = (n & 0x33333333) + ((n >> 2) & 0x33333333);
    n = ((n >> 4) + n) & 0x0F0F0F0F;
    n += n >> 8;
    n += n >> 16;
    return n & 0x0000003F;
}

int all_bytes_same(const unsigned char* bytes, unsigned char len)
{
    unsigned char i = 0;

    for (i = 1; i < len; i++) {
        if (bytes[0] != bytes[i]) {
            return 0;
        }
    }

    return 1;
}

#if 0
static unsigned char bit_swap8(unsigned char original)
{
    unsigned char ret = 0;
    unsigned char i = 0;

    for (i = 0; i < 8; i++) {
        if (original & 0x01) {
            ret |= 0x01;
        }
        else {
            ret &= 0xfe;
        }
        if (i == 7) {
            break;
        }
        ret <<= 1;
        original >>= 1;
    }

    return ret;
}
#else
static unsigned char bit_swap8(unsigned char original)
{
    unsigned char ret = 0;

    for (unsigned char i = 0; i < 8; i++) {
        ret <<= 1;
        ret |= (original & 0x01);
        original >>= 1;
    }
    return ret;
}
#endif

/**
 * @brief This function is to check whether access code is valid.
 * @param[in] ac        access code.
 * @param[in] len       length of access code.
 * @param[in] swap_bit  whether the high and low bits of each byte in ac need to be swapped.
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
int access_code_is_invalid(const unsigned char* ac, unsigned char len, unsigned char swap_bit)
{
    unsigned char access_code[5] = {0, 0, 0, 0, 0};
    unsigned short aa_high = 0;
    int tmp_trans = 0;
    unsigned char bits_diff0 = 0;
    unsigned char bits_diff1 = 0;
    unsigned char bits_diff2 = 0;
    unsigned char transitions = 0;
    unsigned char consecutive = 1;
    unsigned char one_cnt = 0;
    unsigned char prev_bit = 0;
    unsigned int temp_ac = 0;
    unsigned int mask = 0;
    unsigned int mask_max = ((len > 3) ? 0x80000000 : 0x800000);
    unsigned int temp0 = 0;
    unsigned int temp1 = 0;
    unsigned int temp2 = 0;
    unsigned char i = 0;
#if SYNC_WORD_CHECK_LOG_EN
    unsigned char j = 0;
    char str[50] = {'0'};
#endif
    unsigned int trans_cnt = 0;

    if ((len < 3) || (len > 5)) {
#if SYNC_WORD_CHECK_LOG_EN
        log_printf("<Invalid: length error>(%d)\n", len);
#endif
        return -1;
    }

    memcpy(access_code, ac, len);

    if (swap_bit) {
        for (i = 0; i < len; i++)
            access_code[i] = bit_swap8(access_code[i]);
    }

    /* Ensure correct assignment of temp_ac based on length */
    switch(len) {
        case 3:
            temp_ac = (access_code[0]) | (access_code[1] << 8) | (access_code[2] << 16);
            aa_high = access_code[1] | (access_code[2] << 8);
            break;
        case 4:
            temp_ac = (access_code[0]) | (access_code[1] << 8) | (access_code[2] << 16) | (access_code[3] << 24);
            aa_high = access_code[2] | (access_code[3] << 8);
            break;
        case 5:
            temp_ac = (access_code[1]) | (access_code[2] << 8) | (access_code[3] << 16) | (access_code[4] << 24);
            aa_high = access_code[3] | (access_code[4] << 8);
            break;
        default:
            return -1;
    }

#if SYNC_WORD_CHECK_LOG_EN
    log_printf("ac:0x%04X\n", temp_ac);
#endif

    if (all_bytes_same((unsigned char *)&temp_ac, (len > 3) ? 4 : 3)) {
#if SYNC_WORD_CHECK_LOG_EN
        log_printf("<Invalid: all bytes same>\n");
#endif
        return -2;
    }

#if CHECK_UPPER_7_BITS_TRANSITIONS
    /* Upper 7 bits must have 2 transitions */
    tmp_trans = (short)aa_high >> 9;
#else
    /* Upper 6 bits must have 2 transitions */
    tmp_trans = (short)aa_high >> 10;
#endif

//    tlk_printf("aa_high: 0x%08X\n", aa_high);
//    tlk_printf("(short)aa_high: 0x%08X\n", (short)aa_high);
//    tlk_printf("(short)aa_high >> 9: 0x%08X\n", (short)aa_high >> 9);
//    tlk_printf("tmp_trans: 0x%08X\n", tmp_trans);
//    tlk_printf("tmp_trans >> 1: 0x%08X\n", tmp_trans >> 1);
//    tlk_printf("^:0x%08X\n", tmp_trans ^ (tmp_trans >> 1));
//    tlk_printf("u^:0x%08X\n", (unsigned int)(tmp_trans ^ (tmp_trans >> 1)));
    trans_cnt = tl_hacker_popcnt(tmp_trans ^ (tmp_trans >> 1));
    if (trans_cnt < 2) {
#if SYNC_WORD_CHECK_LOG_EN
        log_printf("<Invalid: %d transitions of upper 7 bits less than 2>\n", trans_cnt);
#endif
        return -3;
    }
//    log_printf("%d transitions\n", trans_cnt);

    /* Cannot be access address or be 1 bit different */
    /* Air data flow MSByte first LSBit first */
    if (len == 3) {
        temp0 = temp_ac ^ 0x8E89BE;/* BE 89 8E */
        temp1 = temp_ac ^ 0xE23A1A;/* 1A 3A E2 */
        temp2 = temp_ac ^ 0x389673;/* 73 96 38 */
    }
    else {
        temp0 = temp_ac ^ 0x8E89BED6;/* D6 BE 89 8E */
        temp1 = temp_ac ^ 0xE23A1A33;/*0xE23A1A33CE -- CE 33 1A 3A E2 */
        temp2 = temp_ac ^ 0x389673DB;/*0x389673DB7B -- 7B DB 73 96 38 */
    }

    for (mask = 0x00000001; mask != 0; mask <<= 1) {
        if ((mask & temp0)) {
            ++bits_diff0;
        }
        if ((mask & temp1)) {
            ++bits_diff1;
        }
        if ((mask & temp2)) {
            ++bits_diff2;
        }
        if ((bits_diff0 > 1) && (bits_diff1 > 1) && (bits_diff2 > 1)) {
            break;
        }
    }

    if ((bits_diff0 <= 1) || (bits_diff1 <= 1) || (bits_diff2 <= 1)) {
#if SYNC_WORD_CHECK_LOG_EN
        log_printf("<Invalid: Cannot be access address or be 1 bit different>\n");
#endif
        return -4;
    }

    /* Cannot have more than 24 transitions */
    one_cnt = 0;
    mask = 0x00000001;


    if (temp_ac & mask) {
        one_cnt++;
    }

#if SYNC_WORD_CHECK_LOG_EN
    i = 4;
    str[j++] = (mask & temp_ac) ? '1' : '0';
#endif


    while (mask < mask_max) {
        prev_bit = (temp_ac & mask) != 0;
        mask <<= 1;

#if SYNC_WORD_CHECK_LOG_EN
        if (((j)%i) == 0) {
            str[j++] = ' ';
            i = j + 4;
        }
        str[j++] = (mask & temp_ac) ? '1' : '0';
#endif

        if (mask & temp_ac) {
            if (prev_bit == 0) {
                ++transitions;
                consecutive = 1;
            }
            else {
                ++consecutive;
            }
            one_cnt++;
        }
        else {
            if (prev_bit == 0) {
                ++consecutive;
            }
            else {
                ++transitions;
                consecutive = 1;
            }
        }

        /* 8 lsb should have at least three 1 */
        if (mask == 0x00000080 && one_cnt < 3) {
#if SYNC_WORD_CHECK_LOG_EN
            log_printf("<Invalid: 8 lsb should have at least three 1:one%d-0bL %s>\n", one_cnt, str);
#endif
            return -5;
        }

        /* 16 lsb should have no more than 11 transitions */
        if (mask == 0x00008000 && transitions > 11) {
#if SYNC_WORD_CHECK_LOG_EN
            log_printf("<Invalid: 16 lsb should have no more than 11 transitions:one%d-t%d-0bL %s>\n", one_cnt, transitions, str);
#endif
            return -6;
        }

        /* This is invalid */
        if (consecutive > 6) {
#if SYNC_WORD_CHECK_LOG_EN
            log_printf("<Invalid: Consecutive more than 6:one%d-c%d-0bL %s>\n", one_cnt, consecutive, str);
#endif
            return -7;
        }
    }

    /* Cannot be more than 24 transitions */
    if (transitions > 24) {
#if SYNC_WORD_CHECK_LOG_EN
        log_printf("<Invalid: Cannot be more than 24 transitions-one%d-t%d-0bL %s>\n", one_cnt, transitions, str);
#endif
        return -8;
    }

    /* We have a valid access address */
    return 0;
}

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
 * @param[in] sync_word	sync word
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
int sync_word_is_invalid(const unsigned char* sync_word, unsigned char len)
{
	return access_code_is_invalid(sync_word, len, 1);
}

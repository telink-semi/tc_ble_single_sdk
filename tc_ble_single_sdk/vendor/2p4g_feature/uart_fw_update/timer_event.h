/********************************************************************************************************
 * @file    timer_event.h
 *
 * @brief   This is the header file for 2.4G SDK
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
#ifndef _TIMER_EVENT_H_
#define _TIMER_EVENT_H_

#include "driver.h"

#define LengthOfArray(arr_name) (sizeof(arr_name)/sizeof(arr_name[0]))
#define TIMER_SAFE_MARGIN_US (4000000) //please fix me

typedef int (*ev_timer_callback_t)(void *data);


/**
 *  @brief Definition for timer event
 */
typedef struct ev_time_event_t {
    ev_timer_callback_t     cb;            //!< Callback function when expire, this must be specified
    u32                     t;             //!< Used internal
    u32                     interval;      //!< Used internal
    void                    *data;         //!< Callback function arguments.
    u32                     valid;
    u32                     busy;
} ev_time_event_t;

extern ev_time_event_t *ev_on_timer(ev_timer_callback_t cb,void *data, u32 t_us);
extern void ev_unon_timer(ev_time_event_t **e);//ok
extern void ev_process_timer();
extern int is_timer_expired(ev_time_event_t *e);//ok

#endif /* _TIMER_EVENT_H_ */

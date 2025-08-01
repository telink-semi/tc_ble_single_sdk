/********************************************************************************************************
 * @file    timer_event.c
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
#include "timer_event.h"
#if (FEATURE_TEST_MODE == UART_FW_UPDATE)
ev_time_event_t timer_list[10];

static int inline ev_is_timer_expired(ev_time_event_t *e, u32 now)
{
    assert(e);
    return ((u32)(now - e->t) < (TIMER_SAFE_MARGIN_US*sys_tick_per_us));
}


__attribute__((section(".ram_code")))void ev_start_timer(ev_time_event_t * e)
{  
    assert(e);

    u8 r = irq_disable();

    u32 now = ClockTime();
    u32 t = now + e->interval;    // becare of overflow

    e->t = t;
    e->valid = 1;
   
    irq_restore(r);
}

__attribute__((section(".ram_code")))void ev_cancel_timer(ev_time_event_t * e)
{
    assert(e);

    u8 r = irq_disable();
    e->valid = 0;
    e->busy = 0;
    irq_restore(r);
}

__attribute__((section(".ram_code")))ev_time_event_t *ev_on_timer(ev_timer_callback_t cb, void *data, u32 t_us)
{
    int i;
    ev_time_event_t *e = NULL;

    assert(cb);

    for (i = 0; i < LengthOfArray(timer_list); i++)
    {
        if (timer_list[i].busy == 0)
        {
            timer_list[i].busy = 1;
            e = timer_list + i;
            break;
        }
    }
    if (e == NULL) {
        while(1);
    }
    e->interval = t_us * sys_tick_per_us;
    e->cb = cb;
    e->data = data;
    ev_start_timer(e);
    return e;
}

__attribute__((section(".ram_code")))void ev_unon_timer(ev_time_event_t ** e)
{
    assert(e && (*e));
    
    ev_cancel_timer(*e);
    *e = NULL;
}


/* Process time events */
__attribute__((section(".ram_code")))void ev_process_timer()
{
    u32 now = ClockTime();
    ev_time_event_t *te;
    for (te = timer_list; te < timer_list + LengthOfArray(timer_list); te++) {
        if ((!is_timer_expired(te)) && ev_is_timer_expired(te, now))
        {
            int t;
            if ((unsigned int)(te->cb) < 0x100 || (unsigned int)(te->cb) > 0x20000) {
                while(1);
            }
            t = te->cb(te->data);
            if(t < 0) {
                ev_cancel_timer(te);        // delete timer
            }
            else if(0 == t) {
                te->t = now + te->interval;    // becare of overflow
            }
            else {
                te->interval = t * sys_tick_per_us;
                te->t = now + te->interval;    // becare of overflow
            }
        }
    }
}

__attribute__((section(".ram_code")))int is_timer_expired(ev_time_event_t *e)
{
    if ( e == NULL ) {
        return TRUE;
    }
    if (e->valid == 1) {
        return FALSE;
    }
    else {
        return TRUE;
    }
}
#endif

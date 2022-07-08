/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-08     hehung       the first version
 */


#include "app_stopwatch.h"
#include <stdint.h>

uint8_t watchstop_start_flag = 0u;

void watchstop_start(void)
{
    watchstop_start_flag = 1;
}

void watchstop_counter(t_ws_time* current_ws_time, uint16_t add_cycle)
{
    current_ws_time->ws_millisec += add_cycle;
    if(current_ws_time->ws_millisec >= 1000)
    {
        current_ws_time->ws_millisec = 0;
        current_ws_time->ws_sec ++;
        if(current_ws_time->ws_sec >= 60)
        {
            current_ws_time->ws_sec = 0;
            current_ws_time->ws_min ++;
            if(current_ws_time->ws_min >= 60)
            {
                current_ws_time->ws_min = 0;
                current_ws_time->ws_hour ++;
            }
        }
    }
}


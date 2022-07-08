/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-08     hehung       the first version
 */
#ifndef APPLICATIONS_APP_STOPWATCH_H_
#define APPLICATIONS_APP_STOPWATCH_H_


typedef struct{
    unsigned char ws_hour;
    unsigned char ws_min;
    unsigned char ws_sec;
    unsigned short ws_millisec;
}t_ws_time;

void watchstop_counter(t_ws_time* current_ws_time, unsigned short add_cycle);

#endif /* APPLICATIONS_APP_STOPWATCH_H_ */

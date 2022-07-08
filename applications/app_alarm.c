/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-19     hehung       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "app_alarm.h"
#include "app_rtc.h"


#define ALARM_PIN               GET_PIN(A, 9)


struct tm *cur_tm;    /*current time*/
static uint8_t alarm_hour = 0u;
static uint8_t alarm_min = 0u;
static uint8_t alarm_open_flg = 0u;
static uint8_t alarm_opened_flg = 0u;
static uint8_t alarm_enable_flg = 0u;

static void Alarm_Management(void);

void Alarm_ThreadEntry(void *parameter)
{
    rt_pin_mode(ALARM_PIN, PIN_MODE_OUTPUT);

    while(1)
    {
        if(alarm_enable_flg == 1)
        {
            Alarm_Management();
        }
        else
        {
            /*Do Nothing*/
        }

        rt_thread_mdelay(1000);
    }
}

static void Alarm_Management(void)
{
    static uint8_t glast_hour = 0u;
    static uint8_t glast_min = 0u;
    static uint8_t first_enter = 0;

    cur_tm = RTC_GetLocalTime();
    if(first_enter == 0)
    {
        first_enter = 1;
        glast_hour = cur_tm->tm_hour;
        glast_min = cur_tm->tm_min;
    }
    else {

    }

    if((cur_tm->tm_hour == alarm_hour) &&
            ((cur_tm->tm_min == alarm_min)&&(cur_tm->tm_min != glast_min)))
    {
 //       if(alarm_opened_flg == 0)
 //       {
            alarm_opened_flg = 1;
            alarm_open_flg = 1;
//        }
//        else {
//
//        }
    }
    else
    {
        /*Do Nothing*/
    }

    if(alarm_open_flg == 1)
    {
        rt_pin_write(ALARM_PIN, 1);    /*´ò¿ªÄÖÖÓ*/
    }
    else
    {
        rt_pin_write(ALARM_PIN, 0);
    }

//    if((glast_hour == 23) && (cur_tm->tm_hour==0))
//    {
//        alarm_opened_flg = 0;
//    }
//    else
//    {
//        /*Do Nohting*/
//    }

    glast_hour = cur_tm->tm_hour;
    glast_min = cur_tm->tm_min;
}

void Alarm_SetAlarmTime(uint8_t adjust_unit)
{
    switch(adjust_unit)
    {
        case 0:
            alarm_hour ++;
            if(alarm_hour >= 24)
                alarm_hour = 0;
            break;
        case 1:
            alarm_min ++;
            if(alarm_min >= 60)
                alarm_min = 0;
            break;
        default:
            break;
    }
}

uint8_t Alarm_GetAlarmHour(void)
{
    return alarm_hour;
}

uint8_t Alarm_GetAlarmMin(void)
{
    return alarm_min;
}

void Alarm_TurnOffAlarm(void)
{
    alarm_open_flg = 0;
}

void Alarm_TurnOnAlarm(void)
{
    alarm_open_flg = 1;
}

void Alarm_SetAlarmEnable(uint8_t en_flg)
{
    if(en_flg <= 1)
        alarm_enable_flg = en_flg;
    else {
        alarm_enable_flg = 0;
    }
}

uint8_t Alarm_GetAlarmEnableFlg(void)
{
    return alarm_enable_flg;
}


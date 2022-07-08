/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-15     hehung       the first version
 */

#include "app_rtc.h"
#include <finsh.h>
#include <rtdevice.h>
#include <time.h>
#include <string.h>
#include <rtthread.h>
#include "..\\rt-thread\\components\\drivers\\include\\drivers\rtc.h"
#include "ntp.h"

static rt_thread_t NTP_thread = NULL;
static struct tm *Current_Date = NULL;
static uint8_t ntp_en_flg = 1u;
static uint32_t ntp_get_cycle = 0;  /*每24小时获取一次网咯时间*/

#define NTP_GET_CYCLE    (60*60*24)

//static char* RTC_GetDate(void);
static struct tm* RTC_GetDate(void);
static void NTP_sync_ThreadEnrty(void* parameter);

void RTC_ThreadEntry(void* parameter)
{
    /*Set initial time*/
    set_date(2022, 06, 21);
    set_time(14, 38 ,30);

    NTP_thread = rt_thread_create("ntp_sync", NTP_sync_ThreadEnrty, RT_NULL, 1536, 26, 2);
    if (NTP_thread)
    {
        rt_thread_startup(NTP_thread);
    }
    else
    {
        //return -RT_ENOMEM;
    }

    while(1)
    {
        Current_Date = RTC_GetDate();  /*Get Current Time*/

        if(ntp_get_cycle > NTP_GET_CYCLE)
        {
            ntp_get_cycle = 0;
            NTP_thread = rt_thread_create("ntp_sync", NTP_sync_ThreadEnrty, RT_NULL, 1536, 26, 2);
            if (NTP_thread)
            {
                rt_thread_startup(NTP_thread);
            }
            else
            {
                //return -RT_ENOMEM;
            }
        }
        else
        {
            ntp_get_cycle ++;
        }

        rt_thread_mdelay(1000);
    }
}

/*
 * Get network time thread
 */
static void NTP_sync_ThreadEnrty(void* parameter)
{
    time_t tmp_ntp_get_time = 0;

    extern time_t ntp_sync_to_rtc(const char *host_name);
    /* first sync delay for network connect */
    rt_thread_delay(RTC_NTP_FIRST_SYNC_DELAY * RT_TICK_PER_SECOND);

    while (1)
    {
        if(ntp_en_flg == 1)
        {
            tmp_ntp_get_time = ntp_sync_to_rtc(NULL);
            if(tmp_ntp_get_time)
            {
                if(RT_EOK == rt_thread_delete(NTP_thread))
                    rt_kprintf("NTP thread deleted successfully\n");
            }
            else
            {

            }
        }
        else
        {
            /*Do Nothing*/
        }
//        rt_kprintf("%ld", tmp_ntp_get_time);
        rt_thread_delay(1000/*RTC_NTP_SYNC_PERIOD * RT_TICK_PER_SECOND*/);
    }
}

/*
 * Get Current Time
 * */
struct tm* RTC_GetLocalTime(void)
{
    return Current_Date;
}


/*
 * Get Current Time from RTC local time
 * */
static struct tm* RTC_GetDate(void)
{
    time_t now;
    /* output current time */
    now = time(RT_NULL);

    return (localtime(&now));
}

uint8_t RTC_GetNTPEnableFlg(void)
{
    return ntp_en_flg;
}

void RTC_SetNTPEnableFlg(uint8_t en_flg)
{
    ntp_en_flg = en_flg;
}

static void RTC_GetDateShell(void)
{
    time_t now;
    /* output current time */
    now = time(RT_NULL);

    rt_kprintf("%s", ctime(&now));
    rt_kprintf("%d", gmtime(&now)->tm_sec);
    rt_kprintf("%s", localtime(&now)->tm_sec);
//    rt_kprintf("%d", now);
}
MSH_CMD_EXPORT(RTC_GetDateShell, get date and time or set [year month day hour min sec]);

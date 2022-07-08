/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-15     hehung       the first version
 */

#ifndef __APP_RTC_H__
#define __APP_RTC_H__

void RTC_ThreadEntry(void* parameter);
struct tm* RTC_GetLocalTime(void);
unsigned char RTC_GetNTPEnableFlg(void);
void RTC_SetNTPEnableFlg(unsigned char en_flg);

#endif

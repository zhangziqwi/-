/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-19     hehung       the first version
 */
#ifndef APPLICATIONS_APP_ALARM_H_
#define APPLICATIONS_APP_ALARM_H_

void Alarm_ThreadEntry(void *parameter);
void Alarm_SetAlarmTime(unsigned char adjust_unit);
uint8_t Alarm_GetAlarmHour(void);
uint8_t Alarm_GetAlarmMin(void);
void Alarm_TurnOffAlarm(void);
void Alarm_TurnOnAlarm(void);
void Alarm_SetAlarmEnable(unsigned char en_flg);
uint8_t Alarm_GetAlarmEnableFlg(void);

#endif /* APPLICATIONS_APP_ALARM_H_ */

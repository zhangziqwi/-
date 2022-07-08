/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-15     hehung       the first version
 */

#ifndef __APP_KEY_H__
#define __APP_KEY_H__


typedef enum
{
    KEY_PRESSED  = 0,
    KEY_RELEASED = 1
}t_KeySta;

#define KEY_NUM                 1u
#define PRESS_TIME_1            (uint16_t)50u    //short press, < 500ms
#define PRESS_TIME_2            (uint16_t)150u   //more short press, 500ms < PRESS_TIME_2 < 1500ms
#define PRESS_TIME_3            (uint16_t)250u   //less than long press, 1500ms < PRESS_TIME_3 < 2500ms
#define PRESS_STATUS_NUM        4u
#define PRESS_STATUS_1          0u
#define PRESS_STATUS_2          1u
#define PRESS_STATUS_3          2u
#define PRESS_STATUS_4          3u


void KEY_ThreadEntry(void* parameter);
t_KeySta KEY_GetKeyState(unsigned int paraKeyNum, unsigned char paraKeyStatus);
void KEY_ClrKeyState(unsigned int paraKeyNum, unsigned char paraKeyStatus);
void Key_PressMethod(void);
unsigned short KEY_GetKeyPressCnt(unsigned char paraKeyNum);


#endif

/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-15     hehung       the first version
 */

#include "app_key.h"
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <u8g2_port.h>


#define KEY1_PIN                GET_PIN(H, 4)
//#define KEY2_PIN                GET_PIN(A, 8)



static t_KeySta Key_Cmd[KEY_NUM][PRESS_STATUS_NUM];
static t_KeySta Current_key_state[KEY_NUM] = {KEY_RELEASED};
static t_KeySta key_pin_state[KEY_NUM] = {KEY_RELEASED};
static t_KeySta gLast_key_pin_state[KEY_NUM] = {KEY_RELEASED};
static uint16_t press_cnt[KEY_NUM] = {0u};

static void Key_Filter(void);


/*Key control thread*/
void KEY_ThreadEntry(void* parameter)
{

    uint8_t tmp_i = 0u, tmp_j = 0u;

    /*KEY Initialization*/
    rt_pin_mode(KEY1_PIN, PIN_MODE_INPUT);

    for(tmp_i=0; tmp_i<KEY_NUM; tmp_i++)
    {
        for(tmp_j=0; tmp_j<PRESS_STATUS_NUM ; tmp_j++)
        {
            Key_Cmd[tmp_i][tmp_j] = KEY_RELEASED;
        }
    }

    while(1)
    {
        Current_key_state[0u] = rt_pin_read(KEY1_PIN);
        Key_PressMethod();

//        for(tmp_i=0; tmp_i<KEY_NUM; tmp_i++)
//        {
//            KEY_Filter();
//            /*Key pressed*/
//            if((0u == key_pin_state[tmp_i]) && (1u == gLast_key_pin_state[tmp_i]))
//            {
//                Key_Cmd[tmp_i] = KEY_PRESSED;
//            }
//            else if((1u == key_pin_state[tmp_i]) && (0u == gLast_key_pin_state[tmp_i]))
//            {
//                Key_Cmd[tmp_i] = KEY_RELEASED;
//            }
//            else
//            {
//                /*Do Nothing*/
//            }
//
//            gLast_key_pin_state[tmp_i] = key_pin_state[tmp_i];
//        }

        rt_thread_mdelay(10);
    }
}

void Key_PressMethod(void)
{
    uint8_t tmp_i = 0u;

    for(tmp_i=0; tmp_i<KEY_NUM; tmp_i++)
    {
        Key_Filter();

        if(KEY_PRESSED == key_pin_state[tmp_i])
        {
            press_cnt[tmp_i]++;
            Key_Cmd[tmp_i][PRESS_STATUS_1] = KEY_RELEASED;
            Key_Cmd[tmp_i][PRESS_STATUS_2] = KEY_RELEASED;
            Key_Cmd[tmp_i][PRESS_STATUS_3] = KEY_RELEASED;
            Key_Cmd[tmp_i][PRESS_STATUS_4] = KEY_RELEASED;
        }
        else if((KEY_RELEASED == key_pin_state[tmp_i]) && (KEY_PRESSED == gLast_key_pin_state[tmp_i]))
        {
            if(press_cnt[tmp_i] < PRESS_TIME_1)
            {
                Key_Cmd[tmp_i][PRESS_STATUS_1] = KEY_PRESSED;
            }
            else if(press_cnt[tmp_i] < PRESS_TIME_2)
            {
                Key_Cmd[tmp_i][PRESS_STATUS_2] = KEY_PRESSED;
            }
            else if(press_cnt[tmp_i] < PRESS_TIME_3)
            {
                Key_Cmd[tmp_i][PRESS_STATUS_3] = KEY_PRESSED;
            }
            else
            {
                Key_Cmd[tmp_i][PRESS_STATUS_4] = KEY_PRESSED;
            }
            press_cnt[tmp_i]=0u;
        }
        else
        {
            /*Do Nothing*/
        }

        gLast_key_pin_state[tmp_i] = key_pin_state[tmp_i];
    }
}

/*switch filter*/
static void Key_Filter(void)
{
    static int filter_cnt[KEY_NUM] = {0};
    uint8_t tmp_i = 0u;

    for(tmp_i=0u; tmp_i<KEY_NUM; tmp_i++)
    {
        if(KEY_PRESSED == Current_key_state[tmp_i])
        {
            if(filter_cnt[tmp_i] < 5)
            {
                filter_cnt[tmp_i] ++;
            }
            else
            {
                key_pin_state[tmp_i] = KEY_PRESSED;
            }
        }
        else
        {
            if(filter_cnt[tmp_i] > 0)
            {
                filter_cnt[tmp_i] --;
            }
            else
            {
                key_pin_state[tmp_i] = KEY_RELEASED;
            }
        }
    }
}

/*Get switch status*/
t_KeySta KEY_GetKeyState(unsigned int paraKeyNum, unsigned char paraKeyStatus)
{
    return Key_Cmd[paraKeyNum][paraKeyStatus];
}

/*Clear switch status*/
void KEY_ClrKeyState(unsigned int paraKeyNum, unsigned char paraKeyStatus)
{
    Key_Cmd[paraKeyNum][paraKeyStatus] = KEY_RELEASED;
}

unsigned short KEY_GetKeyPressCnt(unsigned char paraKeyNum)
{
    return (press_cnt[paraKeyNum]);
}


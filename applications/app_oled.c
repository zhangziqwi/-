/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-15     hehung       the first version  u8g2.h
 */

#include "codetab.h"
#include "app_oled.h"
#include <rthw.h>
#include <rtthread.h>
#include <rtdevice.h>
#include <u8g2_port.h>
#include <stdio.h>
#include <drv_soft_i2c.h>
#include "app_rtc.h"
#include "bmp.h"
#include "app_key.h"
//#include "app_weather.h"
#include <string.h>
#include "app_stopwatch.h"
//#include "app_sensor.h"
#include "math.h"
#include "app_compass.h"
//#include "app_step.h"
#include "app_alarm.h"

#define OLED_I2C_PIN_SCL                     GET_PIN(H, 14)
#define OLED_I2C_PIN_SDA                     GET_PIN(C, 7)

#define PI 3.1415

/*********************************定义stopwatch任务的相关参数**************************************/
#define StopWatch_Priority        4
#define StopWatch_TimeSlices      5
rt_thread_t      stopwatch_thread;            // 线程控制块
/*********************************定义指南针任务的相关参数**************************************/
#define Compass_Priority        4
#define Compass_TimeSlices      5
rt_thread_t      compass_thread;            // 线程控制块
/*********************************定义闹钟任务的相关参数**************************************/
#define Alarm_Priority        6
#define Alarm_TimeSlices      5
rt_thread_t      alarm_thread;            // 线程控制块

const static char *date_eng[12] = {"Jan.", "Feb.", "Mar.", "Apr.",
                                   "may.", "Jun.", "Jul.", "Aug.",
                                   "Sept.","Oct.", "Nov.", "Dec."};
const static char *Date_week[7] = { "Sun.",  "Mon.", "Tues.", "Wed.", "Thur.", "Fri.", "Sat."};

t_ws_time current_ws_t = {0, 0, 0, 0};
t_ws_time *current_ws_t_p = &current_ws_t;

uint8_t ws_start_flg = 0u;


static void OLED_DrawHz(u8g2_t *u8g2, uint8_t x, uint8_t y, uint8_t chr, uint8_t mode);
static void OLED_Hz_String(u8g2_t *u8g2, uint8_t x, uint8_t y, uint8_t chr_S, uint8_t chr_E, uint8_t mode);
static void OLED_ShowBMP(u8g2_t *u8g2, uint8_t x, uint8_t y, uint8_t mode, uint8_t bmp[][16], uint32_t len);
static void OLED_ShowBMP2(u8g2_t *u8g2, uint8_t x, uint8_t y, uint8_t mode, uint8_t bmp[][8], uint32_t len, uint8_t size);
void StopWatch_ThreadEntry(void* parameter);

/*
 * OLED thread entry
 */
char weather_city_info[20];
char weather_info[20] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                          ' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};
uint8_t list_menu_ptr = 0u;
uint8_t tmp_disp_flg = 0u;
extern double my_battery_temperature;
extern int16_t my_battery_voltage;
int int_my_battery_temperature;
double buf_my_battery_temperature;
void OLED_ThreadEntry(void* parameter)
{
    u8g2_t u8g2;
//    uint8_t tmp_disp_flg = 0u;
    struct tm *cur_tm;    /*current time*/
    char *str_date = NULL;
    char *str_time = NULL;
    char *str_my_battery_temperature = NULL;
    char *str_my_battery_voltage = NULL;
    uint16_t dis_cnt_cycle = 0u;
    uint8_t dis_wifi_blink = 0u;
    char *str_key_press_cnt = NULL;
    uint8_t weather_blink_cnt = 0u;
    uint8_t weather_blink_flg = 0u;
    uint8_t Last_weather_blink_flg = 0u;
    uint32_t weather_get_cnt = 0u;    /*the counter for get the weather*/

    uint8_t list_string[3][2];
    uint8_t list_cursor[3] = {1u};
    uint8_t list_cursor_pointer = 0u;
//    uint8_t list_menu_ptr = 0u;

    uint8_t ws_list_cursor[3] = {1u};
    uint8_t ws_list_cursor_pointer = 0u;

    //温度
    char temp_str_dis[6] = {0};
    uint8_t temp_dis_cnt = 0u;
    uint8_t temp_dis_menu_pointer= 0u;

    short compass_ptr_angle = 0;

    uint8_t compass_pnt_x = 0;
    uint8_t compass_pnt_y = 0;
    uint8_t compass_pnt_l = 0;
    uint8_t compass_angle = 0;
    char *str_dir[4] = {NULL};

    char str_step[6] = {0};
    uint8_t step_mode_ptr = 1;

    uint8_t setting_mode_ptr = 0;
    uint8_t setting_net_rtc_en_flg = 1;
    uint8_t setting_alarm_en_flg = 0;
    uint8_t setting_alarm_time_ptr = 0x01;
    uint8_t setting_from_dis_flg = 0;
    char str_alarm_setting[5] = {0};

    // Initialization
    u8g2_Setup_ssd1306_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8x8_rt_gpio_and_delay);
    u8x8_SetPin(u8g2_GetU8x8(&u8g2), U8X8_PIN_I2C_CLOCK, OLED_I2C_PIN_SCL);
    u8x8_SetPin(u8g2_GetU8x8(&u8g2), U8X8_PIN_I2C_DATA, OLED_I2C_PIN_SDA);

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    while(1)
    {
        switch(tmp_disp_flg)
        {
            case 0u:    /*Startup display*/
                if(dis_cnt_cycle == 10u)
                {
                    u8g2_ClearBuffer(&u8g2);
                    OLED_ShowBMP(&u8g2, 0, 0, 0, Battery_logo1, Get_Rtt_logo_len());
                    u8g2_SendBuffer(&u8g2);
                }
                else if(dis_cnt_cycle == 20u)
                {
                    u8g2_ClearBuffer(&u8g2);
                    OLED_ShowBMP(&u8g2, 0, 0, 1, Battery_logo2, Get_Battery_logo_len());
                    u8g2_SendBuffer(&u8g2);

//                    u8g2_ClearBuffer(&u8g2);
//                    OLED_Hz_String(&u8g2, 10, 10, 53, 58, 1);
//                    OLED_Hz_String(&u8g2, 10, 36, 59, 60, 1);
//                    u8g2_SendBuffer(&u8g2);

                }

                if(dis_cnt_cycle == 30u)
                {
                    u8g2_ClearBuffer(&u8g2);
                    OLED_ShowBMP(&u8g2, 0, 0, 0, Battery_logo3, Get_Rtt_logo_len());
                    u8g2_SendBuffer(&u8g2);
                }
                if(dis_cnt_cycle == 40u)
                {
                    u8g2_ClearBuffer(&u8g2);
                    OLED_ShowBMP(&u8g2, 0, 0, 0, Battery_logo4, Get_Rtt_logo_len());
                    u8g2_SendBuffer(&u8g2);
                }






                else if(dis_cnt_cycle >= 60u)
                {
                    tmp_disp_flg = 1u;
                    dis_cnt_cycle = 0u;
                    cur_tm = RTC_GetLocalTime();
                }
                else
                {
                    /*Do Nothing*/
                }

                dis_cnt_cycle ++;

                break;
            case 1u:    /*Time Display*/
                u8g2_ClearBuffer(&u8g2);    /*Clear OLED*/

                if((dis_cnt_cycle%5) == 0u)
                {
                    dis_cnt_cycle = 1u;

                    // cur_tm->tm_year + 1900   //year
                    cur_tm = RTC_GetLocalTime();

                    if(RT_TRUE != rt_wlan_is_connected())
                    {
                        dis_wifi_blink = (dis_wifi_blink==0)?1:0;
                        weather_blink_flg = (weather_blink_flg==0)?1:0;
                    }
                    else
                    {
                        /*Do Nothing*/
                    }
                }
                else
                {
                    dis_cnt_cycle ++;
                }



                u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
              //  sprintf(str_date, "%s - %s  %02d", Date_week[cur_tm->tm_wday], date_eng[cur_tm->tm_mon], cur_tm->tm_mday);
                sprintf(str_date, "%02d-%02d", 1+cur_tm->tm_mon, cur_tm->tm_mday);
                u8g2_DrawStr(&u8g2, 24, 60, str_date);



                u8g2_DrawFrame(&u8g2, 0, 0, 128, 64);
             //   u8g2_SetFont(&u8g2, u8g2_font_ncenB18_tr);

                u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
                sprintf(str_time, "%02d:%02d:%02d", cur_tm->tm_hour, cur_tm->tm_min, cur_tm->tm_sec);
                u8g2_DrawStr(&u8g2, 65, 60, str_time);


            //   OLED_Hz_String(&u8g2, 10, 10, 61, 64, 1);

           //      u8g2_SendBuffer(&u8g2) ;




                buf_my_battery_temperature=my_battery_temperature;
                rt_kprintf(" my_battery_temperature %f !\n", my_battery_temperature);
                buf_my_battery_temperature=buf_my_battery_temperature*10;

                int_my_battery_temperature=(int)buf_my_battery_temperature;
                OLED_Hz_String(&u8g2, 10, 6, 66, 68, 1);      //温度
                OLED_Hz_String(&u8g2, 74, 6, 61, 63, 1);      //电压

                u8g2_SetFont(&u8g2, u8g2_font_helvR14_tn);

          ////////////////////////////////////////////////

                if (my_battery_voltage<250)
               {
                   my_battery_voltage=0;
               }

                sprintf(str_my_battery_temperature, "%d.%d", int_my_battery_temperature/10, int_my_battery_temperature%10 );
                u8g2_DrawStr(&u8g2, 14, 41, str_my_battery_temperature);   //温度值





       /*///////////////////////////////////////////////////////////////////////////
                if (my_battery_voltage>250)

                {

                          sprintf(str_my_battery_temperature, "%d.%d", 52, 3 );
                           u8g2_DrawStr(&u8g2, 14, 41, str_my_battery_temperature);   //温度值
                }

                if (my_battery_voltage<250)

                {

                          sprintf(str_my_battery_temperature, "%d.%d", 0, 0 );
                           u8g2_DrawStr(&u8g2, 14, 41, str_my_battery_temperature);   //温度值
                           my_battery_voltage=0;
                }

         //////////////////////////////////////////////////////////////////////////*/







                sprintf(str_my_battery_voltage, "%d.%02d",my_battery_voltage/100, my_battery_voltage%100);

                u8g2_DrawStr(&u8g2, 78, 41, str_my_battery_voltage);       //电压值




//                /*WIFI is connected*/
                if(RT_TRUE == rt_wlan_is_connected())
                {
                    OLED_ShowBMP2(&u8g2, 3,48, 1, Wifi_16X16, Get_Wifi_16X16(), 16);

                }
//                    weather_get_cnt ++;
//                    /*If the weather is not get, get the weather every 5s*/
//                    if(weather_info[0] == ' ')
//                    {
//                        if(weather_get_cnt > 50)
//                        {
//                            weather_get_cnt = 0u;
//                            Weather_GetWeather(AREA_ID);
////                            weather_info = Weather_GetWeatherInfo();
//                            strcpy(weather_city_info, Weather_GetWeatherCityInfo());
//                            strcpy(weather_info, Weather_GetWeatherInfo());
//                            rt_kprintf("1111weather_city_info - %s\n", weather_city_info);
//                            rt_kprintf("1111weather_info - %s\n", weather_info);
//                        }
//                        else
//                        {
//
//                        }
//                        u8g2_SetFont(&u8g2, u8g2_font_open_iconic_weather_2x_t);
//                        switch(weather_blink_cnt)
//                        {
//                            case 0: u8g2_DrawGlyph(&u8g2, 100, 62, 69);break;
//                            case 1: u8g2_DrawGlyph(&u8g2, 100, 62, 67);break;
//                            case 2: u8g2_DrawGlyph(&u8g2, 100, 62, 66);break;
//                            case 3: u8g2_DrawGlyph(&u8g2, 100, 62, 65);break;
//                            case 4: u8g2_DrawGlyph(&u8g2, 100, 62, 64);break;
//                        }
//
//                        if(weather_blink_cnt>=4)
//                        {
//                            weather_blink_cnt = 0;
//                        }
//                        else {
//                            ++weather_blink_cnt;
//                        }
//
//                    }
//                    else    /*get the weather every 1h*/
//                    {
//                        if(weather_get_cnt > (10*60*60))
//                        {
//                            weather_get_cnt = 0u;
//                            Weather_GetWeather(AREA_ID);
////                            weather_info = Weather_GetWeatherInfo();
//                            strcpy(weather_city_info, Weather_GetWeatherCityInfo());
//                            strcpy(weather_info, Weather_GetWeatherInfo());
//                        }
//                        else
//                        {
//
//                        }
//
//                        if(strstr(weather_info, "雨") == 0)
//                        {
//                            weather_blink_cnt = 1;
//                        }
//                        else if(strstr(weather_info, "晴") == 0 && (strstr(weather_info, "云") || strstr(weather_info, "阴")) == 0)
//                        {
//                            weather_blink_cnt = 3;
//                        }
//                        else if(strstr(weather_info, "云") == 0 || strstr(weather_info, "阴") == 0)
//                        {
//                            weather_blink_cnt = 4;
//                        }
//                        else if(strstr(weather_info, "晴") == 0)
//                        {
//                            weather_blink_cnt = 0;
//                        }
//
//                        u8g2_SetFont(&u8g2, u8g2_font_open_iconic_weather_2x_t);
//                        switch(weather_blink_cnt)
//                        {
//                            case 0: u8g2_DrawGlyph(&u8g2, 100, 62, 69);break;    /*SUN*/
//                            case 1: u8g2_DrawGlyph(&u8g2, 100, 62, 67);break;    /*RAIN*/
//                            case 2: u8g2_DrawGlyph(&u8g2, 100, 62, 66);break;    /*NIGHT*/
//                            case 3: u8g2_DrawGlyph(&u8g2, 100, 62, 65);break;    /*SUN-CLOUD*/
//                            case 4: u8g2_DrawGlyph(&u8g2, 100, 62, 64);break;    /*CLOUD*/
//                        }
//                    }
//
//                }
                else  /*WIFI disconnect*/
                {
                    if(0u == dis_wifi_blink)
                    {
                        OLED_ShowBMP2(&u8g2, 3, 46, 1, Wifi_16X16, Get_Wifi_16X16(), 16);
                    }
                    else
                    {
                        /*Do Nothing*/
                    }

                }
//                    if(Last_weather_blink_flg != weather_blink_flg)
//                    {
//                        if(weather_blink_cnt>=4)
//                        {
//                            weather_blink_cnt = 0;
//                        }
//                        else {
//                            ++weather_blink_cnt;
//                        }
//                    }
//                    else
//                    {
//
//                    }
//                    if(0 == weather_blink_flg)
//                    {
//                        u8g2_SetFont(&u8g2, u8g2_font_open_iconic_weather_2x_t);
//                        switch(weather_blink_cnt)
//                        {
//                            case 0: u8g2_DrawGlyph(&u8g2, 100, 62, 69);break;
//                            case 1: u8g2_DrawGlyph(&u8g2, 100, 62, 67);break;
//                            case 2: u8g2_DrawGlyph(&u8g2, 100, 62, 66);break;
//                            case 3: u8g2_DrawGlyph(&u8g2, 100, 62, 65);break;
//                            case 4: u8g2_DrawGlyph(&u8g2, 100, 62, 64);break;
//                        }
//                    }
//                    else
//                    {
//
//                    }
//                }

                /*Display switch press time*/
                if(0 != KEY_GetKeyPressCnt(0))
                {
                    u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
                    if(KEY_GetKeyPressCnt(0) < PRESS_TIME_1)
                    {
                        sprintf(str_key_press_cnt, "KEY%d", 1);
                    }
                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_2)
                    {
                        sprintf(str_key_press_cnt, "KEY%d", 2);
                    }
                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_3)
                    {
                        sprintf(str_key_press_cnt, "KEY%d", 3);
                    }
                    else
                    {
                        sprintf(str_key_press_cnt, "KEY%d", 4);
                    }
                    u8g2_DrawStr(&u8g2, 64-16, 64-8, str_key_press_cnt);
                }
                else
                {
                  /*Do Nothing*/
                }

                u8g2_SendBuffer(&u8g2);    /*Display*/

//                /*开关命令*/
//                if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_1))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_1);
//                    //关闭闹钟
//                    if(Alarm_GetAlarmEnableFlg() == 1)
//                    {
//                        Alarm_TurnOffAlarm();
//                    }
//                    else
//                    {
//                        /*Do Nothing*/
//                    }
//                }
//                if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_2))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_2);
//                    //进入设置界面
//                    tmp_disp_flg = 9;
//                    setting_from_dis_flg = 0;
//                }
//                if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_3))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_3);
//                }
//                if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_4))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_4);
//
//                    tmp_disp_flg = 2u;    /*switch to other display */
//                    list_string[0][0] = 17;
//                    list_string[0][1] = 18;
//                    list_string[1][0] = 19;
//                    list_string[1][1] = 20;
//                    list_string[2][0] = 21;
//                    list_string[2][1] = 22;
//                    list_cursor[0] = 0;
//                    list_cursor[1] = 1;
//                    list_cursor[2] = 1;
//                    list_cursor_pointer = 0;
//                }
//                else
//                {
//                    /*Do Nothing*/
//                }

             //   Last_weather_blink_flg = weather_blink_flg;

                break;

//            case 2u:
//                //设置 - 时间设置，网络时间设置
//                //秒表 -
//                //温度 - 网络温度，传感器温度
//                //湿度
//                //气压
//                //步数
//
//                /*开关命令*/
//                if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_2))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_2);
////                    tmp_disp_flg = 1u;    /*switch to other display */
//                    switch(list_menu_ptr)
//                    {
//                        case 0: tmp_disp_flg = 1u; break;
//                        case 1: tmp_disp_flg = 9u; setting_from_dis_flg = 1; break;    /*设置*/
//                        case 2: tmp_disp_flg = 3u;           /*秒表*/
//                                /* 创建线程 秒表线程 */
//                                stopwatch_thread = rt_thread_create("stopwatch_thread",
//                                                                    StopWatch_ThreadEntry,
//                                                                    RT_NULL,
//                                                                    512,
//                                                                    StopWatch_Priority,
//                                                                    StopWatch_TimeSlices);
//                                if (stopwatch_thread != RT_NULL)
//                                    rt_thread_startup(stopwatch_thread);
//                                ws_list_cursor_pointer = 0;
//                                ws_list_cursor[0] = 0;
//                                ws_list_cursor[1] = 1;
//                                ws_list_cursor[2] = 1;
//                                ws_start_flg = 1;
//                                current_ws_t.ws_millisec = 0;
//                            break;
//                        case 3: tmp_disp_flg = 4u;
//                                break;   /*步数*/
//                        case 4: tmp_disp_flg = 5u;
//                                temp_dis_cnt = 0;
//                                rt_enter_critical();
//                                Init_LPS22HH();
//                                rt_exit_critical();
//                            break;   /*温度*/
//                        case 5: tmp_disp_flg = 6u;
//                                rt_enter_critical();
//                                Init_HTS221();
//                                rt_exit_critical();
//                                temp_dis_cnt = 0;
//                            break;   /*湿度*/
//                        case 6: tmp_disp_flg = 7u;
//                                rt_enter_critical();
//                                Init_LPS22HH();
//                                rt_exit_critical();
//                                temp_dis_cnt = 0;
//                            break;   /*气压*/
//                        case 7: tmp_disp_flg = 8u;
//                                /* 创建线程 指南针线程 */
//                                compass_thread = rt_thread_create("compass_control",
//                                                            Compass_ThreadEntry,
//                                                            RT_NULL,                      //线程入口函数参数
//                                                            512,
//                                                            Compass_Priority,
//                                                            Compass_TimeSlices);
//                                if (compass_thread != RT_NULL)
//                                    rt_thread_startup(compass_thread);
//                            break;   /*指南针*/
//                    }
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_4))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_4);
//                    tmp_disp_flg = 1u;
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_1))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_1);
//                    list_cursor_pointer ++;
//                    if(list_cursor_pointer<=2)
//                    {
//
//                        list_menu_ptr = list_cursor_pointer;
//                    }
//                    else
//                    {
//                        list_menu_ptr++;
//                        list_string[0][0] = list_string[1][0];
//                        list_string[0][1] = list_string[1][1];
//                        list_string[1][0] = list_string[2][0];
//                        list_string[1][1] = list_string[2][1];
//                        if(list_string[2][0]+1 >= 32)
//                        {
////                            list_cursor_pointer = 0;
//                            list_menu_ptr = 0;
//                            list_string[2][0] = 17;
//                            list_string[2][1] = 18;
//                        }
//                        else
//                        {
//                            list_string[2][0] = list_string[2][1]+1;
//                            list_string[2][1] = list_string[2][1]+2;
//                        }
//
//                    }
//
//                    switch(list_cursor_pointer)
//                    {
//                        case 0u:list_cursor[0]=0;list_cursor[1]=1;list_cursor[2]=1;
//                            break;
//                        case 1u:list_cursor[0]=1;list_cursor[1]=0;list_cursor[2]=1;
//                            break;
//                        case 2u:list_cursor[0]=1;list_cursor[1]=1;list_cursor[2]=0;
//                            break;
//                        default: break;
//                    }
//                }
//                u8g2_ClearBuffer(&u8g2);
//                //OLED_ShowBMP(&u8g2, 0, 0, 1, eeworld_logo, Get_eeworld_logo_len());
//                OLED_Hz_String(&u8g2, 36, 16, list_string[0][0], list_string[0][1], list_cursor[0]);
//                OLED_Hz_String(&u8g2, 36, 32, list_string[1][0], list_string[1][1], list_cursor[1]);
//                OLED_Hz_String(&u8g2, 36, 48, list_string[2][0], list_string[2][1], list_cursor[2]);
//                /*Display switch press time*/
//               if(0 != KEY_GetKeyPressCnt(0))
//               {
//                   u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
//                   if(KEY_GetKeyPressCnt(0) < PRESS_TIME_1)
//                   {
//                       sprintf(str_key_press_cnt, "KEY%d", 1);
//                   }
//                   else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_2)
//                   {
//                       sprintf(str_key_press_cnt, "KEY%d", 2);
//                   }
//                   else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_3)
//                   {
//                       sprintf(str_key_press_cnt, "KEY%d", 3);
//                   }
//                   else
//                   {
//                       sprintf(str_key_press_cnt, "KEY%d", 4);
//                   }
//                   u8g2_DrawStr(&u8g2, 64-16, 12, str_key_press_cnt);
//               }
//               else
//               {
//                   /*Do Nothing*/
//               }
//
//               u8g2_SetFont(&u8g2, u8g2_font_open_iconic_gui_2x_t);
//               u8g2_DrawGlyph(&u8g2, 100, 16, 65);
//               u8g2_SetFont(&u8g2, u8g2_font_open_iconic_mime_2x_t);
//               u8g2_DrawGlyph(&u8g2, 10, 16, 65);
//               u8g2_SetFont(&u8g2, u8g2_font_open_iconic_mime_4x_t);
//               u8g2_DrawGlyph(&u8g2, 80, 58, 65);
//
//               u8g2_SendBuffer(&u8g2);    /*Display*/
//
//               break;
//
//            case 3u:    /*Stopwatch*/
//                u8g2_ClearBuffer(&u8g2);
//
//                u8g2_SetFont(&u8g2, u8g2_font_open_iconic_gui_2x_t);
//                u8g2_DrawGlyph(&u8g2, 100, 16, 65);
//                u8g2_SetFont(&u8g2, u8g2_font_open_iconic_app_2x_t);
//                u8g2_DrawGlyph(&u8g2, 10, 16, 72);
//
//                if(ws_start_flg == 1)
//                    OLED_Hz_String(&u8g2, 8, 48, 33, 34, ws_list_cursor[0]);
//                else
//                    OLED_Hz_String(&u8g2, 8, 48, 35, 36, ws_list_cursor[0]);
//                OLED_Hz_String(&u8g2, 48, 48, 37, 38, ws_list_cursor[1]);
//                OLED_Hz_String(&u8g2, 88, 48, 17, 18, ws_list_cursor[2]);
//
//                u8g2_DrawFrame(&u8g2, 12, 22, 100, 18);
//                u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
//                sprintf(str_time, "%02d:%02d:%02d:%03d", current_ws_t.ws_hour,
//                                                        current_ws_t.ws_min, current_ws_t.ws_sec, current_ws_t.ws_millisec);
//                u8g2_DrawStr(&u8g2, 16, 36, str_time);
//
//                /*Display switch press time*/
//                if(0 != KEY_GetKeyPressCnt(0))
//                {
//                    u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
//                    if(KEY_GetKeyPressCnt(0) < PRESS_TIME_1)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 1);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_2)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 2);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_3)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 3);
//                    }
//                    else
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 4);
//                    }
//                    u8g2_DrawStr(&u8g2, 64-16, 12, str_key_press_cnt);
//                }
//                else
//                {
//                /*Do Nothing*/
//                }
//
//                u8g2_SendBuffer(&u8g2);
//
//                if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_1))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_1);
//                    switch(ws_list_cursor_pointer)
//                    {
//                        case 0: ws_start_flg = (ws_start_flg^1)&0x0f; break;
//                        case 1: current_ws_t.ws_hour = 0;
//                                current_ws_t.ws_min = 0;
//                                current_ws_t.ws_sec = 0;
//                                current_ws_t.ws_millisec = 0;
//                           break;
//                        case 2: tmp_disp_flg = 2;rt_thread_delete(stopwatch_thread); break;
//                    }
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_2))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_2);
//
//                    ws_list_cursor_pointer ++;
//                    if(ws_list_cursor_pointer > 2)
//                        ws_list_cursor_pointer = 0;
//                    switch(ws_list_cursor_pointer)
//                    {
//                        case 0: ws_list_cursor[0]=0;ws_list_cursor[1]=1;ws_list_cursor[2]=1; break;
//                        case 1: ws_list_cursor[0]=1;ws_list_cursor[1]=0;ws_list_cursor[2]=1; break;
//                        case 2: ws_list_cursor[0]=1;ws_list_cursor[1]=1;ws_list_cursor[2]=0; break;
//                        default: ws_list_cursor_pointer = 0; break;
//                    }
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_3))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_3);
//
//                    tmp_disp_flg = 2u;
//                    rt_thread_delete(stopwatch_thread);
//                }
//                else if (KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_4))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_4);
//                    tmp_disp_flg = 1u;
//                    rt_thread_delete(stopwatch_thread);
//                }
//                break;
//
//            case 4u: /*计算步数*/
//                u8g2_ClearBuffer(&u8g2);
//
//                u8g2_SetFont(&u8g2, u8g2_font_open_iconic_gui_2x_t);
//                u8g2_DrawGlyph(&u8g2, 100, 16, 65);
//                OLED_Hz_String(&u8g2, 16, 0, 23, 24, 1);
//                OLED_Hz_String(&u8g2, 16, 48, 37, 38, step_mode_ptr);
//                OLED_Hz_String(&u8g2, 80, 48, 17, 18, (~step_mode_ptr)&0x01);
//
//                sprintf(str_step, "%06d", Step_GetStep());
//
//                u8g2_SetFont(&u8g2, u8g2_font_helvB18_te);
//                u8g2_DrawStr(&u8g2, 64-32, 38, str_step);
//
//                /*Display switch press time*/
//                if(0 != KEY_GetKeyPressCnt(0))
//                {
//                    u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
//                    if(KEY_GetKeyPressCnt(0) < PRESS_TIME_1)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 1);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_2)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 2);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_3)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 3);
//                    }
//                    else
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 4);
//                    }
//                    u8g2_DrawStr(&u8g2, 64-16, 12, str_key_press_cnt);
//                }
//                else
//                {
//                /*Do Nothing*/
//                }
//                u8g2_SendBuffer(&u8g2);
//
//                if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_1))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_1);
//                    step_mode_ptr = (~step_mode_ptr)&0x01;
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_2))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_2);
//                    if(step_mode_ptr == 0)
//                    {
//                        Step_ClearStep();
//                    }
//                    else
//                    {
//                        tmp_disp_flg = 2;
//                    }
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_3))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_3);
//                    tmp_disp_flg = 2;
//                }
//                else if (KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_4))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_4);
//                    tmp_disp_flg = 1;
//                }
//                break;
//
//            case 5u:    /*温度*/
//                u8g2_ClearBuffer(&u8g2);
//
//                OLED_Hz_String(&u8g2, 0, 0, 25, 26, 1);
//                if(0u == temp_dis_menu_pointer)
//                {
//                    OLED_Hz_String(&u8g2, 16, 48, 39, 40, 0);
//                    OLED_Hz_String(&u8g2, 80, 48, 17, 18, 1);
//                }
//                else
//                {
//                    OLED_Hz_String(&u8g2, 16, 48, 39, 40, 1);
//                    OLED_Hz_String(&u8g2, 80, 48, 17, 18, 0);
//                }
//
//                if(temp_dis_cnt%10 == 0)    /*Get the temperature every 1s*/
//                {
//                    temp_dis_cnt = 1;
//                    rt_enter_critical();
//                    sprintf(temp_str_dis, "%02d.%dC", (int)(Get_Temp()/10), (int)(Get_Temp()%10));
//                    rt_exit_critical();
//                }
//                else
//                {
//                }
//                temp_dis_cnt ++;
//                u8g2_SetFont(&u8g2, u8g2_font_helvB18_te);
//                u8g2_DrawStr(&u8g2, 64-30, 38, temp_str_dis);
//                u8g2_SetFont(&u8g2, u8g2_font_open_iconic_gui_2x_t);
//                u8g2_DrawGlyph(&u8g2, 100, 16, 65);
//
//                /*Display switch press time*/
//                if(0 != KEY_GetKeyPressCnt(0))
//                {
//                    u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
//                    if(KEY_GetKeyPressCnt(0) < PRESS_TIME_1)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 1);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_2)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 2);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_3)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 3);
//                    }
//                    else
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 4);
//                    }
//                    u8g2_DrawStr(&u8g2, 64-16, 12, str_key_press_cnt);
//                }
//                else
//                {
//                    /*Do Nothing*/
//                }
//                u8g2_SendBuffer(&u8g2);
//
//                if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_1))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_1);
//
//                    temp_dis_menu_pointer = (~temp_dis_menu_pointer)&0x01;
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_2))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_2);
//
//                    if(0u == temp_dis_menu_pointer)
//                    {
//                        rt_enter_critical();
//                        sprintf(temp_str_dis, "%02d.%dC", (int)(Get_Temp()/10), (int)(Get_Temp()%10));
//                        rt_exit_critical();
//                    }
//                    else
//                    {
//                        tmp_disp_flg = 2;
//                    }
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_3))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_2);
//                    tmp_disp_flg = 2;
//                }
//                else if (KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_4))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_4);
//                    tmp_disp_flg = 1;
//                }
//                break;
//
//            case 6u:    /*湿度*/
//                u8g2_ClearBuffer(&u8g2);
//
//                OLED_Hz_String(&u8g2, 0, 0, 27, 28, 1);
//                if(0u == temp_dis_menu_pointer)
//                {
//                    OLED_Hz_String(&u8g2, 16, 48, 39, 40, 0);
//                    OLED_Hz_String(&u8g2, 80, 48, 17, 18, 1);
//                }
//                else
//                {
//                    OLED_Hz_String(&u8g2, 16, 48, 39, 40, 1);
//                    OLED_Hz_String(&u8g2, 80, 48, 17, 18, 0);
//                }
//
//                if(temp_dis_cnt%10 == 0)    /*Get the temperature every 1s*/
//                {
//                    temp_dis_cnt = 1;
//                    rt_enter_critical();
//                    sprintf(temp_str_dis, "%02d.%d%%", (int)(HTS221_GetCalHumi()/10), (int)(HTS221_GetCalHumi()%10));
//                    rt_exit_critical();
//                }
//                else
//                {
//                }
//                temp_dis_cnt ++;
//                u8g2_SetFont(&u8g2, u8g2_font_helvB18_te);
//                u8g2_DrawStr(&u8g2, 64-28, 38, temp_str_dis);
//                u8g2_SetFont(&u8g2, u8g2_font_open_iconic_gui_2x_t);
//                u8g2_DrawGlyph(&u8g2, 100, 16, 65);
//
//                /*Display switch press time*/
//                if(0 != KEY_GetKeyPressCnt(0))
//                {
//                    u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
//                    if(KEY_GetKeyPressCnt(0) < PRESS_TIME_1)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 1);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_2)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 2);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_3)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 3);
//                    }
//                    else
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 4);
//                    }
//                    u8g2_DrawStr(&u8g2, 64-16, 12, str_key_press_cnt);
//                }
//                else
//                {
//                    /*Do Nothing*/
//                }
//                u8g2_SendBuffer(&u8g2);
//
//                if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_1))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_1);
//
//                    temp_dis_menu_pointer = (~temp_dis_menu_pointer)&0x01;
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_2))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_2);
//
//                    if(0u == temp_dis_menu_pointer)
//                    {
//                        rt_enter_critical();
//                        sprintf(temp_str_dis, "%02d.%d%%", (int)(HTS221_GetCalHumi()/10), (int)(HTS221_GetCalHumi()%10));
//                        rt_exit_critical();
//                    }
//                    else
//                    {
//                        tmp_disp_flg = 2;
//                    }
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_3))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_2);
//                    tmp_disp_flg = 2;
//                }
//                else if (KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_4))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_4);
//                    tmp_disp_flg = 1;
//                }
//                break;
//
//            case 7u:    /*气压*/
//                u8g2_ClearBuffer(&u8g2);
//
//                OLED_Hz_String(&u8g2, 0, 0, 29, 30, 1);
//                if(0u == temp_dis_menu_pointer)
//                {
//                    OLED_Hz_String(&u8g2, 16, 48, 39, 40, 0);
//                    OLED_Hz_String(&u8g2, 80, 48, 17, 18, 1);
//                }
//                else
//                {
//                    OLED_Hz_String(&u8g2, 16, 48, 39, 40, 1);
//                    OLED_Hz_String(&u8g2, 80, 48, 17, 18, 0);
//                }
//
//                if(temp_dis_cnt%10 == 0)    /*Get the temperature every 1s*/
//                {
//                    temp_dis_cnt = 1;
//                    rt_enter_critical();
//                    sprintf(temp_str_dis, "%04d.%dhPA", (int)(Get_Pressure()/10), (int)(Get_Pressure()%10));
//                    rt_exit_critical();
//                }
//                else
//                {
//                }
//                temp_dis_cnt ++;
//                u8g2_SetFont(&u8g2, u8g2_font_helvB18_te);
//                u8g2_DrawStr(&u8g2, 64-64, 38, temp_str_dis);
//                u8g2_SetFont(&u8g2, u8g2_font_open_iconic_gui_2x_t);
//                u8g2_DrawGlyph(&u8g2, 100, 16, 65);
//
//                /*Display switch press time*/
//                if(0 != KEY_GetKeyPressCnt(0))
//                {
//                    u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
//                    if(KEY_GetKeyPressCnt(0) < PRESS_TIME_1)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 1);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_2)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 2);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_3)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 3);
//                    }
//                    else
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 4);
//                    }
//                    u8g2_DrawStr(&u8g2, 64-16, 12, str_key_press_cnt);
//                }
//                else
//                {
//                    /*Do Nothing*/
//                }
//                u8g2_SendBuffer(&u8g2);
//
//                if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_1))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_1);
//
//                    temp_dis_menu_pointer = (~temp_dis_menu_pointer)&0x01;
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_2))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_2);
//
//                    if(0u == temp_dis_menu_pointer)
//                    {
//                        rt_enter_critical();
//                        sprintf(temp_str_dis, "%04d.%dhPA", (int)(Get_Pressure()/10), (int)(Get_Pressure()%10));
//                        rt_exit_critical();
//                    }
//                    else
//                    {
//                        tmp_disp_flg = 2;
//                    }
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_3))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_2);
//                    tmp_disp_flg = 2;
//                }
//                else if (KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_4))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_4);
//                    tmp_disp_flg = 1;
//                }
//                break;
//
//            case 8u:    /*磁力计*/
//                u8g2_ClearBuffer(&u8g2);
//
//                compass_pnt_l = 30;
//
//                u8g2_SetFont(&u8g2, u8g2_font_open_iconic_app_2x_t);
//                u8g2_DrawGlyph(&u8g2, 10, 16, 70);
//                u8g2_SetFont(&u8g2, u8g2_font_open_iconic_gui_2x_t);
//                u8g2_DrawGlyph(&u8g2, 100, 16, 65);
//                u8g2_SetFont(&u8g2, u8g2_font_helvB08_te);
//                sprintf(str_dir[0], "%c", 'W');
//                u8g2_DrawStr(&u8g2, 64-compass_pnt_l+4, 32+4, str_dir[0]);
//                sprintf(str_dir[1], "%c", 'E');
//                u8g2_DrawStr(&u8g2, 64+compass_pnt_l-8, 32+4, str_dir[1]);
//                sprintf(str_dir[2], "%c", 'S');
//                u8g2_DrawStr(&u8g2, 64-2, 32+compass_pnt_l-4, str_dir[2]);
//                sprintf(str_dir[3], "%c", 'N');
//                u8g2_DrawStr(&u8g2, 64-2, 32-compass_pnt_l+10, str_dir[3]);
//
//
//                u8g2_DrawCircle(&u8g2, 64, 32, compass_pnt_l, U8G2_DRAW_ALL);
//
////                compass_ptr_angle += 10;    /*For Test*/
//                compass_ptr_angle = Compass_GetAzimut();
////                compass_ptr_angle = compass_ptr_angle<0?((int)(abs(compass_ptr_angle))+180):compass_ptr_angle;
//                compass_ptr_angle = compass_ptr_angle>0?(360-(int)(abs(compass_ptr_angle))):(-compass_ptr_angle);
//                compass_ptr_angle = compass_ptr_angle>=360?0:compass_ptr_angle;
//
//
//                if(compass_ptr_angle < 90)    /*右上半圆*/
//                {
//                    compass_angle = compass_ptr_angle;
//                    compass_pnt_x = 64+compass_pnt_l * sin(PI/180*compass_angle);
//                    compass_pnt_y = 32-compass_pnt_l * cos(PI/180*compass_angle);
//
//                    if(compass_angle < 23)
//                    {
//                        OLED_Hz_String(&u8g2, 10, 48, 44, 44, 1);
//                    }
//                    else if(compass_angle < 68)
//                    {
//                        OLED_Hz_String(&u8g2, 2, 48, 41, 41, 1);
//                        OLED_Hz_String(&u8g2, 18, 48, 44, 44, 1);
//                    }
//                    else
//                    {
//                        OLED_Hz_String(&u8g2, 10, 48, 41, 41, 1);
//                    }
//                }
//                else if(compass_ptr_angle < 180)    /*右下半圆*/
//                {
//                    compass_angle = compass_ptr_angle-90;
//                    compass_pnt_x = 64+compass_pnt_l * cos(PI/180*compass_angle);
//                    compass_pnt_y = 32+compass_pnt_l * sin(PI/180*compass_angle);
//
//                    if(compass_angle < 23)
//                    {
//                        OLED_Hz_String(&u8g2, 10, 48, 41, 41, 1);
//                    }
//                    else if(compass_angle < 68)
//                    {
//                        OLED_Hz_String(&u8g2, 2, 48, 41, 41, 1);
//                        OLED_Hz_String(&u8g2, 18, 48, 42, 42, 1);
//                    }
//                    else
//                    {
//                        OLED_Hz_String(&u8g2, 10, 48, 44, 42, 1);
//                    }
//                }
//                else if(compass_ptr_angle < 270)    /*左下半圆*/
//                {
//                    compass_angle = compass_ptr_angle-180;
//                    compass_pnt_x = 64-compass_pnt_l * sin(PI/180*compass_angle);
//                    compass_pnt_y = 32+compass_pnt_l * cos(PI/180*compass_angle);
//
//                    if(compass_angle < 23)
//                    {
//                        OLED_Hz_String(&u8g2, 10, 48, 42, 42, 1);
//                    }
//                    else if(compass_angle < 68)
//                    {
//                        OLED_Hz_String(&u8g2, 2, 48, 43, 43, 1);
//                        OLED_Hz_String(&u8g2, 18, 48, 42, 42, 1);
//                    }
//                    else
//                    {
//                        OLED_Hz_String(&u8g2, 10, 48, 43, 43, 1);
//                    }
//                }
//                else    /*左上半圆*/
//                {
//                    compass_angle = compass_ptr_angle-270;
//                    compass_pnt_x = 64-compass_pnt_l * cos(PI/180*compass_angle);
//                    compass_pnt_y = 32-compass_pnt_l * sin(PI/180*compass_angle);
//
//                    if(compass_angle < 23)
//                    {
//                        OLED_Hz_String(&u8g2, 10, 48, 43, 43, 1);
//                    }
//                    else if(compass_angle < 68)
//                    {
//                        OLED_Hz_String(&u8g2, 2, 48, 43, 43, 1);
//                        OLED_Hz_String(&u8g2, 18, 48, 44, 44, 1);
//                    }
//                    else
//                    {
//                        OLED_Hz_String(&u8g2, 10, 48, 44, 44, 1);
//                    }
//                }
//
//                u8g2_DrawLine(&u8g2, 64, 32, compass_pnt_x, compass_pnt_y);
//
//                /*Display switch press time*/
//                if(0 != KEY_GetKeyPressCnt(0))
//                {
//                    u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
//                    if(KEY_GetKeyPressCnt(0) < PRESS_TIME_1)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 1);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_2)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 2);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_3)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 3);
//                    }
//                    else
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 4);
//                    }
//                    u8g2_DrawStr(&u8g2, 64-16, 12, str_key_press_cnt);
//                }
//                else
//                {
//                /*Do Nothing*/
//                }
//                u8g2_SendBuffer(&u8g2);
//
//                if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_1))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_1);
////                    tmp_disp_flg = 1;
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_2))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_2);
//                    tmp_disp_flg = 2;
//                    rt_thread_delete(compass_thread);
//                }
//                else if (KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_4))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_4);
//                    tmp_disp_flg = 1;
//                    rt_thread_delete(compass_thread);
//                }
//                break;
//            case 9u:    /*设置*/
//                u8g2_ClearBuffer(&u8g2);
//
//                u8g2_SetFont(&u8g2, u8g2_font_open_iconic_all_2x_t);
//                u8g2_DrawGlyph(&u8g2, 10, 16, 129);
//                u8g2_SetFont(&u8g2, u8g2_font_open_iconic_gui_2x_t);
//                u8g2_DrawGlyph(&u8g2, 100, 16, 65);
//
//                /*显示菜单*/
//                OLED_Hz_String(&u8g2, 8, 24, 45, 48, setting_mode_ptr);
//                OLED_Hz_String(&u8g2, 8, 42, 49, 50, (~setting_mode_ptr)&0x01);
//
//                if(setting_net_rtc_en_flg  == 1)
//                    OLED_Hz_String(&u8g2, 100, 24, 5, 5, 1);
//                else {
//                    OLED_Hz_String(&u8g2, 100, 24, 4, 4, 1);
//                }
//
//                /*Display switch press time*/
//                if(0 != KEY_GetKeyPressCnt(0))
//                {
//                    u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
//                    if(KEY_GetKeyPressCnt(0) < PRESS_TIME_1)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 1);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_2)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 2);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_3)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 3);
//                    }
//                    else
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 4);
//                    }
//                    u8g2_DrawStr(&u8g2, 64-16, 12, str_key_press_cnt);
//                }
//                else
//                {
//                /*Do Nothing*/
//                }
//                u8g2_SendBuffer(&u8g2);
//
//                if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_1))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_1);
//
//                    setting_mode_ptr = (~setting_mode_ptr)&0x01;
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_2))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_2);
//
//                    if(setting_mode_ptr == 0)    /*使能网络时间*/
//                    {
//                        if(setting_net_rtc_en_flg == 0)
//                        {
//                            setting_net_rtc_en_flg = 1;
//                            RTC_SetNTPEnableFlg(1);
//                        }
//                        else
//                        {
//                            setting_net_rtc_en_flg = 0;
//                            RTC_SetNTPEnableFlg(0);
//                        }
//                    }
//                    else    /*设置闹钟*/
//                    {
//                        tmp_disp_flg = 10;
//                    }
//
//                }
//                else if (KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_3))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_3);
//                    if(setting_from_dis_flg == 0)
//                        tmp_disp_flg = 1;
//                    else {
//                        tmp_disp_flg = 2;
//                    }
//                }
//                else if (KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_4))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_4);
//                    tmp_disp_flg = 1;
//                }
//                break;
//            case 10u:    /*设置*/
//                u8g2_ClearBuffer(&u8g2);
//
//                u8g2_SetFont(&u8g2, u8g2_font_open_iconic_gui_2x_t);
//                u8g2_DrawGlyph(&u8g2, 100, 16, 65);
//                OLED_Hz_String(&u8g2, 0, 0, 49, 50, 1);
//
//                /*显示菜单*/
//                OLED_Hz_String(&u8g2, 8, 24, 51, 52, (~setting_alarm_time_ptr)&0x01);
//                OLED_Hz_String(&u8g2, 40, 24, 49, 50, (~setting_alarm_time_ptr)&0x01);
//                if(setting_alarm_en_flg == 0)
//                    OLED_Hz_String(&u8g2, 100, 24, 4, 4, 1);
//                else if(setting_alarm_en_flg == 1)
//                    OLED_Hz_String(&u8g2, 100, 24, 5, 5, 1);
//
//                OLED_Hz_String(&u8g2, 8, 42, 19, 20, 1);
//                sprintf(str_alarm_setting, "%02d:%02d", Alarm_GetAlarmHour(), Alarm_GetAlarmMin());
//                u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
//                u8g2_DrawStr(&u8g2, 60, 42+12, str_alarm_setting);
//
//                if(setting_alarm_time_ptr == 0x02)
//                {
//                    u8g2_DrawLine(&u8g2, 58, 55, 58+16, 55);
//                    u8g2_DrawLine(&u8g2, 58, 56, 58+16, 56);
//                    u8g2_DrawLine(&u8g2, 58, 57, 58+16, 57);
//                }
//                else if(setting_alarm_time_ptr == 0x04)
//                {
//                    u8g2_DrawLine(&u8g2, 80, 55, 80+16, 55);
//                    u8g2_DrawLine(&u8g2, 80, 56, 80+16, 56);
//                    u8g2_DrawLine(&u8g2, 80, 57, 80+16, 57);
//                }
//                else
//                {
//                    /*Do Nothing*/
//                }
//
//                /*Display switch press time*/
//                if(0 != KEY_GetKeyPressCnt(0))
//                {
//                    u8g2_SetFont(&u8g2, u8g2_font_helvB10_te);
//                    if(KEY_GetKeyPressCnt(0) < PRESS_TIME_1)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 1);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_2)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 2);
//                    }
//                    else if(KEY_GetKeyPressCnt(0) < PRESS_TIME_3)
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 3);
//                    }
//                    else
//                    {
//                       sprintf(str_key_press_cnt, "KEY%d", 4);
//                    }
//                    u8g2_DrawStr(&u8g2, 64-16, 12, str_key_press_cnt);
//                }
//                else
//                {
//                /*Do Nothing*/
//                }
//                u8g2_SendBuffer(&u8g2);
//
//                if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_1))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_1);
//
//                    if(setting_alarm_time_ptr == 0x01)    /*开启闹钟*/
//                    {
//                        if(setting_alarm_en_flg == 1)
//                        {
//                            setting_alarm_en_flg = 0;
//                            Alarm_SetAlarmEnable(0);
//                            if(alarm_thread != NULL)
//                                rt_thread_delete(alarm_thread);
//                        }
//                        else {
//                            setting_alarm_en_flg = 1;
//                            Alarm_SetAlarmEnable(1);
//                            /* 创建线程 闹钟线程 */
//                            alarm_thread = rt_thread_create("Alarm",
//                                                        Alarm_ThreadEntry,
//                                                        RT_NULL,                      //线程入口函数参数
//                                                        512,
//                                                        Alarm_Priority,
//                                                        Alarm_TimeSlices);
//                            if (alarm_thread != RT_NULL)
//                                rt_thread_startup(alarm_thread);
//                        }
//                    }
//                    else if(setting_alarm_time_ptr == 0x02)    /*设置小时*/
//                    {
//                        Alarm_SetAlarmTime(0);
//                    }
//                    else if(setting_alarm_time_ptr == 0x04)    /*设置分钟*/
//                    {
//                        Alarm_SetAlarmTime(1);
//                    }
//                    else {
//
//                    }
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_2))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_2);
//
//                    if(setting_alarm_time_ptr == 0x01)
//                    {
//                        setting_alarm_time_ptr = 0x02;
//                    }
//                    else if(setting_alarm_time_ptr == 0x02)
//                    {
//                        setting_alarm_time_ptr = 0x04;
//                    }
//                    else
//                    {
//                        setting_alarm_time_ptr = 0x01;
//                    }
//
//                }
//                else if(KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_3))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_3);
//
//                    tmp_disp_flg = 9;
//                }
//                else if (KEY_PRESSED == KEY_GetKeyState(0, PRESS_STATUS_4))
//                {
//                    KEY_ClrKeyState(0, PRESS_STATUS_4);
//                    tmp_disp_flg = 1;
//                }
//                break;
            default:break;
        }

        rt_thread_mdelay(100);
    }
}

void StopWatch_ThreadEntry(void* parameter)
{
    while(1)
    {
        if(0 == ws_start_flg)
        {
            watchstop_counter(current_ws_t_p, 100);
        }
        rt_thread_mdelay(100);
    }
}


/*Display more than one hanzi*/
static void OLED_Hz_String(u8g2_t *u8g2, uint8_t x, uint8_t y, uint8_t chr_S, uint8_t chr_E, uint8_t mode)
{
    uint8_t i;
    for(i=chr_S; i<=chr_E; i++)
    {
        OLED_DrawHz(u8g2, x+16*(i-chr_S), y, i, mode);
    }
}

/*Display a HanZi*/
static void OLED_DrawHz(u8g2_t *u8g2, uint8_t x, uint8_t y, uint8_t chr, uint8_t mode)
{
    uint8_t temp,t,t1;
    uint8_t y0=y;
    uint8_t csize=32;
    chr=chr*2;
    for(t=0;t<csize;t++)
    {
        temp=Hz[chr][t];
        for(t1=0;t1<8;t1++)
        {
            if(temp&0x80)
            {
                if(1 == mode)
                    u8g2_DrawPixel(u8g2, x, y);
            }
            else
            {
                if(1 != mode)
                    u8g2_DrawPixel(u8g2, x, y);
            }

            temp<<=1;
            y++;
            if((y-y0) == 16)
            {
                y=y0;
                x++;
                break;
            }
        }
    }
}

/*DIsplay a bmp picture*/
static void OLED_ShowBMP(u8g2_t *u8g2, uint8_t x, uint8_t y, uint8_t mode, uint8_t bmp[][16], uint32_t len)
{
    uint16_t t,t1;
    uint8_t  y0=y;
    uint8_t  temp;
    uint16_t len2=len/16;

    for(t=0;t<len;t++)
    {
        temp=bmp[0][t];
        for(t1=0;t1<8;t1++)
        {
            if(temp&0x80)
            {
                if(1 == mode)
                    u8g2_DrawPixel(u8g2, x, y);
            }
            else
            {
                if(1 != mode)
                    u8g2_DrawPixel(u8g2, x, y);
            }

            temp<<=1;
            y++;
            if((y-y0) == len2)
            {
                y = y0;
                x ++;
                break;
            }
        }
    }
}

static void OLED_ShowBMP2(u8g2_t *u8g2, uint8_t x, uint8_t y, uint8_t mode, uint8_t bmp[][8], uint32_t len, uint8_t size)
{
    uint16_t t,t1;
    uint8_t  y0 = y;
    uint8_t  temp;
    uint16_t len2 = len/3.2;

    if(size == 24)
        len2 = (uint16_t)(len/3.2);
    else if(size == 16)
        len2 = (uint16_t)(len/2);

    for(t=0;t<len;t++)
    {
        temp=bmp[0][t];
        for(t1=0;t1<8;t1++)
        {
            if(temp&0x80)
            {
                if(1 == mode)
                    u8g2_DrawPixel(u8g2, x, y);
            }
            else
            {
                if(1 != mode)
                    u8g2_DrawPixel(u8g2, x, y);
            }

            temp<<=1;
            y++;
            if((y-y0) == len2)
            {
                y = y0;
                x ++;
                break;
            }
        }
    }
}

/*For a Test*/
static void OLED_Test(int argc,char *argv[])
{
    u8g2_t u8g2;

    // Initialization
    u8g2_Setup_ssd1306_i2c_128x64_noname_f( &u8g2, U8G2_R0, u8x8_byte_sw_i2c, u8x8_rt_gpio_and_delay);
    u8x8_SetPin(u8g2_GetU8x8(&u8g2), U8X8_PIN_I2C_CLOCK, OLED_I2C_PIN_SCL);
    u8x8_SetPin(u8g2_GetU8x8(&u8g2), U8X8_PIN_I2C_DATA, OLED_I2C_PIN_SDA);

    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);

    // Draw Graphics
    /* full buffer example, setup procedure ends in _f */
    u8g2_ClearBuffer(&u8g2);
    u8g2_DrawFrame(&u8g2, 0, 0, 128, 64);
    u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
    u8g2_DrawStr(&u8g2, 40, 30, "OLED Test");
    u8g2_DrawStr(&u8g2, 68, 46, "- hehung");
    u8g2_SendBuffer(&u8g2);
}
MSH_CMD_EXPORT(OLED_Test, oled test -hehung);

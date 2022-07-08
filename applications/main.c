/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-09-02     RT-Thread    first version netdev netdev_is_internet_up printonenet_mqtt_initonenet_upload_cycle
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#include "app_oled.h"
#include "app_rtc.h"
#include "app_key.h"
//#include "app_sensor.h"
//#include "app_step.h"   A0
#include "app_myiic.h"
#include "app_compass.h"
#include "app_my_ad.h"
#include "app_battery_voltage.h"
#include "app_sine_wave.h"
#include "app_onenet_send.h"


#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>


#include <netdev.h>
#include <rtdbg.h>


#define LED1_PIN GET_PIN(I, 8)
#define jidianqi0 GET_PIN(A, 8)
#define jidianqi1 GET_PIN(A, 15)
#define jidianqi2 GET_PIN(H, 15)
#define jidianqi3 GET_PIN(H, 13)




extern void wlan_autoconnect_init(void);
static int ThreadInit(void);


/* 邮箱控制块 */
 struct rt_mailbox mb;
/* 用于放邮件的内存池 */
static char mb_pool[128];



/*********************************定义LED任务的相关参数**************************************/
#define LED1_Priority       10
#define LED1_TimeSlices     5
static struct rt_thread     led1_thread;            // 线程控制块
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t           rt_led1_thread_stack[256];    //线程栈
static void Led1_ThreadEntry(void* parameter);     //线程LED1
/*********************************定义OLED任务的相关参数**************************************/
#define OLED_Priority       7
#define OLED_TimeSlices     5
static struct rt_thread     oled_thread;            // 线程控制块
ALIGN(RT_ALIGN_SIZE)    //栈对齐方式
static rt_uint8_t           rt_oled_thread_stack[2048];    //线程栈
/*********************************定义OLED任务的相关参数**************************************/
#define RTC_Priority        8
#define RTC_TimeSlices      5
static struct rt_thread     rtc_thread;            // 线程控制块
ALIGN(RT_ALIGN_SIZE)    //栈对齐方式
static rt_uint8_t           rt_rtc_thread_stack[512];    //线程栈
/*********************************定义KEY任务的相关参数**************************************/
#define KEY_Priority        5
#define KEY_TimeSlices      5
static struct rt_thread     key_thread;            // 线程控制块
ALIGN(RT_ALIGN_SIZE)    //栈对齐方式
static rt_uint8_t           rt_key_thread_stack[512];    //线程栈
///*********************************定义pedometer任务的相关参数**************************************/
//#define Pedometer_Priority        4
//#define Pedometer_TimeSlices      5
//static struct rt_thread     pedometer_thread;            // 线程控制块
//ALIGN(RT_ALIGN_SIZE)    //栈对齐方式
//static rt_uint8_t           rt_pedometer_thread_stack[512];    //线程栈
//



/*********************************定义my_AD任务的相关参数**************************************/
#define My_AD_Priority        3
#define My_AD_TimeSlices      5
static struct rt_thread     My_AD_thread;            // 线程控制块
ALIGN(RT_ALIGN_SIZE)    //栈对齐方式
static rt_uint8_t           My_AD_thread_stack[512];    //线程栈


/*********************************定义battery_voltage任务的相关参数**************************************/
#define Battery_Voltage_Priority        10
#define Battery_Voltage_TimeSlices      5
static struct rt_thread     Battery_Voltage_thread;            // 线程控制块
ALIGN(RT_ALIGN_SIZE)    //栈对齐方式
static rt_uint8_t           Battery_Voltage_thread_stack[512];    //线程栈


/*********************************定义正弦波sine_wave任务的相关参数**************************************/
#define sine_wave_Priority        2
#define sine_wave_TimeSlices      5
static struct rt_thread     sine_wave_thread;            // 线程控制块
ALIGN(RT_ALIGN_SIZE)    //栈对齐方式
static rt_uint8_t           rt_sine_wave_thread_stack[5120];    //线程栈


/*********************************定义onenet_send任务的相关参数**************************************/
#define Onenet_Send_Priority        10
#define Onenet_Send_TimeSlices      5
static struct rt_thread     Onenet_Send_thread;            // 线程控制块
ALIGN(RT_ALIGN_SIZE)    //栈对齐方式
static rt_uint8_t           Onenet_Send_thread_stack[5048];    //线程栈






int main(void)
{
    rt_uint32_t count = 1;

    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(jidianqi0, PIN_MODE_OUTPUT);
    rt_pin_mode(jidianqi1, PIN_MODE_OUTPUT);
    rt_pin_mode(jidianqi2, PIN_MODE_OUTPUT);
    rt_pin_mode(jidianqi3, PIN_MODE_OUTPUT);



    rt_err_t result;

   //   初始化一个 mailbox
     result = rt_mb_init(&mb,
                         "mbt",                     //  名称是 mbt
                         &mb_pool[0],              //   邮箱用到的内存池是 mb_pool
                         sizeof(mb_pool) / 4,       //  邮箱中的邮件数目，因为一封邮件占 4 字节
                         RT_IPC_FLAG_FIFO);         //  采用 FIFO 方式进行线程等待
     if (result != RT_EOK)
     {
         rt_kprintf("init mailbox failed.\n");
         return -1;
     }

    IIC_Init();
    my_DA_conventer_init();
    my_AD_conventer_init();
    /* init Wi-Fi auto connect feature */
    wlan_autoconnect_init();
    /* enable auto reconnect on WLAN device */
    rt_wlan_config_autoreconnect(RT_TRUE);

    (void)ThreadInit();

    while(1)
    {

//        if(!netdev_is_internet_up(netdev_get_by_name("w0"))){
//
//               rt_kprintf("network error\r\n");
//           }
//        else {
//
//            rt_kprintf("network haha\r\n");
//
//        }
//
        rt_thread_mdelay(2000)


        ;
    }
    return RT_EOK;
}

/*
 * Thread Initial
 * */
static int ThreadInit(void)
{
    rt_err_t tmp_result1 = 0;
    rt_err_t tmp_result2 = 0;
    rt_err_t tmp_result3 = 0;
    rt_err_t tmp_result4 = 0;
    rt_err_t tmp_result5 = 0;
    rt_err_t tmp_result6 = 0;
    rt_err_t tmp_result7 = 0;
    rt_err_t tmp_result8 = 0;

    // 创建静态线程
    tmp_result1 = rt_thread_init(&led1_thread,                 //线程控制块
                               "led1_blink",                 //线程名字，在shell里面可以看到
                               Led1_ThreadEntry,            //线程入口函数
                               RT_NULL,                      //线程入口函数参数
                               &rt_led1_thread_stack[0],     //线程栈起始地址
                               sizeof(rt_led1_thread_stack), //线程栈大小
                               LED1_Priority,                //线程的优先级
                               LED1_TimeSlices);             //线程时间片
    // 创建静态线程
    tmp_result2 = rt_thread_init(&oled_thread,                //线程控制块
                               "oled_display",               //线程名字，在shell里面可以看到
                               OLED_ThreadEntry,             //线程入口函数
                               RT_NULL,                      //线程入口函数参数
                               &rt_oled_thread_stack[0],     //线程栈起始地址
                               sizeof(rt_oled_thread_stack), //线程栈大小
                               OLED_Priority,                //线程的优先级
                               OLED_TimeSlices);             //线程时间片

    // 创建静态线程
    tmp_result3 = rt_thread_init(&rtc_thread,                //线程控制块
                               "rtc_time",               //线程名字，在shell里面可以看到
                               RTC_ThreadEntry,             //线程入口函数
                               RT_NULL,                      //线程入口函数参数
                               &rt_rtc_thread_stack[0],     //线程栈起始地址
                               sizeof(rt_rtc_thread_stack), //线程栈大小
                               RTC_Priority,                //线程的优先级
                               RTC_TimeSlices);             //线程时间片

    // 创建静态线程
    tmp_result4 = rt_thread_init(&key_thread,                //线程控制块
                               "key_control",               //线程名字，在shell里面可以看到
                               KEY_ThreadEntry,             //线程入口函数
                               RT_NULL,                      //线程入口函数参数
                               &rt_key_thread_stack[0],     //线程栈起始地址
                               sizeof(rt_key_thread_stack), //线程栈大小
                               KEY_Priority,                //线程的优先级
                               KEY_TimeSlices);             //线程时间片

    // 创建静态线程
//    tmp_result5 = rt_thread_init(&pedometer_thread,                //线程控制块
//                               "pedometer_control",               //线程名字，在shell里面可以看到
//                               Pedometer_ThreadEntry,             //线程入口函数
//                               RT_NULL,                      //线程入口函数参数
//                               &rt_pedometer_thread_stack[0],     //线程栈起始地址
//                               sizeof(rt_pedometer_thread_stack), //线程栈大小
//                               Pedometer_Priority,                //线程的优先级
//                               Pedometer_TimeSlices);             //线程时间片


    tmp_result5 = rt_thread_init(&sine_wave_thread,              //线程控制块
                               "sine_wave",               //线程名字，在shell里面可以看到
                               sine_wave_ThreadEntry,             //线程入口函数
                               RT_NULL,                      //线程入口函数参数
                               &rt_sine_wave_thread_stack[0],     //线程栈起始地址
                               sizeof(rt_sine_wave_thread_stack), //线程栈大小
                               sine_wave_Priority,                //线程的优先级
                               sine_wave_TimeSlices);             //线程时间片


    tmp_result6 = rt_thread_init(&My_AD_thread,                //线程控制块
                               "My_AD",               //线程名字，在shell里面可以看到
                               My_AD_ThreadEntry,             //线程入口函数
                               RT_NULL,                      //线程入口函数参数
                               &My_AD_thread_stack[0],     //线程栈起始地址
                               sizeof(My_AD_thread_stack), //线程栈大小
                               My_AD_Priority,                //线程的优先级
                               My_AD_TimeSlices);             //线程时间片


    tmp_result7 = rt_thread_init(&Battery_Voltage_thread,                //线程控制块
                               "Battery_Voltage",               //线程名字，在shell里面可以看到
                               Battery_Voltage_ThreadEntry,             //线程入口函数
                               RT_NULL,                      //线程入口函数参数
                               &Battery_Voltage_thread_stack[0],     //线程栈起始地址
                               sizeof(Battery_Voltage_thread_stack), //线程栈大小
                               Battery_Voltage_Priority,                //线程的优先级
                               Battery_Voltage_TimeSlices);             //线程时间片

    tmp_result8 = rt_thread_init(&Onenet_Send_thread,                //线程控制块
                               "onenet_send",               //线程名字，在shell里面可以看到
                               Onenet_Send_ThreadEntry,             //线程入口函数
                               RT_NULL,                      //线程入口函数参数
                               &Onenet_Send_thread_stack[0],     //线程栈起始地址
                               sizeof(Onenet_Send_thread_stack), //线程栈大小
                               Onenet_Send_Priority,                //线程的优先级
                               Onenet_Send_TimeSlices);             //线程时间片





    if(RT_EOK == tmp_result1)
    {
        rt_thread_startup(&led1_thread);             //启动线程led0_thread，开启调度
    }
    else
    {
        return -RT_ENOMEM;
    }

    if(RT_EOK == tmp_result2)
    {
        rt_thread_startup(&oled_thread);             //启动线程led0_thread，开启调度
    }
    else
    {
        return -RT_ENOMEM;
    }

    if(RT_EOK == tmp_result3)
    {
        rt_thread_startup(&rtc_thread);             //启动线程led0_thread，开启调度
    }
    else
    {
        return -RT_ENOMEM;
    }

    if(RT_EOK == tmp_result4)
    {
        rt_thread_startup(&key_thread);             //启动线程led0_thread，开启调度
    }
    else
    {
        return -RT_ENOMEM;
    }

    if(RT_EOK == tmp_result5)
     {
         rt_thread_startup(&sine_wave_thread);             //启动线程led0_thread，开启调度
     }
     else
     {
         return -RT_ENOMEM;
     }


    if(RT_EOK == tmp_result6)
    {
        rt_thread_startup(&My_AD_thread);             //启动线程led0_thread，开启调度
    }

    else
    {
        return -RT_ENOMEM;
    }

    if(RT_EOK == tmp_result7)
    {
        rt_thread_startup(&Battery_Voltage_thread);             //启动线程led0_thread，开启调度
    }

    else
    {
        return -RT_ENOMEM;
    }

    if(RT_EOK == tmp_result8)
    {
        rt_thread_startup(&Onenet_Send_thread);             //启动线程led0_thread，开启调度
    }

    else
    {
        return -RT_ENOMEM;
    }

    return RT_EOK;
}

/*
 * LED 0 thread
 */
static void Led1_ThreadEntry(void* parameter)
{
//    uint64_t tmp_led1_counter = 0u;

    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);    //初始化LED1引脚

    while (1)
    {
        rt_pin_write(LED1_PIN, rt_pin_read(LED1_PIN)^1);


        rt_pin_write(jidianqi0, rt_pin_read(jidianqi0)^1);
        rt_thread_mdelay(500);

        rt_pin_write(jidianqi1, rt_pin_read(jidianqi0)^1);

        rt_thread_mdelay(500);
        rt_pin_write(jidianqi2, rt_pin_read(jidianqi0)^1);

        rt_thread_mdelay(500);
        rt_pin_write(jidianqi3, rt_pin_read(jidianqi0)^1);

        rt_thread_mdelay(500);




    }
}



#include "stm32h7xx.h"
static int vtor_config(void)
{
    /* Vector Table Relocation in Internal QSPI_FLASH */
    SCB->VTOR = QSPI_BASE;
    return 0;
}
INIT_BOARD_EXPORT(vtor_config);



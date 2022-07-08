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


/* ������ƿ� */
 struct rt_mailbox mb;
/* ���ڷ��ʼ����ڴ�� */
static char mb_pool[128];



/*********************************����LED�������ز���**************************************/
#define LED1_Priority       10
#define LED1_TimeSlices     5
static struct rt_thread     led1_thread;            // �߳̿��ƿ�
ALIGN(RT_ALIGN_SIZE)
static rt_uint8_t           rt_led1_thread_stack[256];    //�߳�ջ
static void Led1_ThreadEntry(void* parameter);     //�߳�LED1
/*********************************����OLED�������ز���**************************************/
#define OLED_Priority       7
#define OLED_TimeSlices     5
static struct rt_thread     oled_thread;            // �߳̿��ƿ�
ALIGN(RT_ALIGN_SIZE)    //ջ���뷽ʽ
static rt_uint8_t           rt_oled_thread_stack[2048];    //�߳�ջ
/*********************************����OLED�������ز���**************************************/
#define RTC_Priority        8
#define RTC_TimeSlices      5
static struct rt_thread     rtc_thread;            // �߳̿��ƿ�
ALIGN(RT_ALIGN_SIZE)    //ջ���뷽ʽ
static rt_uint8_t           rt_rtc_thread_stack[512];    //�߳�ջ
/*********************************����KEY�������ز���**************************************/
#define KEY_Priority        5
#define KEY_TimeSlices      5
static struct rt_thread     key_thread;            // �߳̿��ƿ�
ALIGN(RT_ALIGN_SIZE)    //ջ���뷽ʽ
static rt_uint8_t           rt_key_thread_stack[512];    //�߳�ջ
///*********************************����pedometer�������ز���**************************************/
//#define Pedometer_Priority        4
//#define Pedometer_TimeSlices      5
//static struct rt_thread     pedometer_thread;            // �߳̿��ƿ�
//ALIGN(RT_ALIGN_SIZE)    //ջ���뷽ʽ
//static rt_uint8_t           rt_pedometer_thread_stack[512];    //�߳�ջ
//



/*********************************����my_AD�������ز���**************************************/
#define My_AD_Priority        3
#define My_AD_TimeSlices      5
static struct rt_thread     My_AD_thread;            // �߳̿��ƿ�
ALIGN(RT_ALIGN_SIZE)    //ջ���뷽ʽ
static rt_uint8_t           My_AD_thread_stack[512];    //�߳�ջ


/*********************************����battery_voltage�������ز���**************************************/
#define Battery_Voltage_Priority        10
#define Battery_Voltage_TimeSlices      5
static struct rt_thread     Battery_Voltage_thread;            // �߳̿��ƿ�
ALIGN(RT_ALIGN_SIZE)    //ջ���뷽ʽ
static rt_uint8_t           Battery_Voltage_thread_stack[512];    //�߳�ջ


/*********************************�������Ҳ�sine_wave�������ز���**************************************/
#define sine_wave_Priority        2
#define sine_wave_TimeSlices      5
static struct rt_thread     sine_wave_thread;            // �߳̿��ƿ�
ALIGN(RT_ALIGN_SIZE)    //ջ���뷽ʽ
static rt_uint8_t           rt_sine_wave_thread_stack[5120];    //�߳�ջ


/*********************************����onenet_send�������ز���**************************************/
#define Onenet_Send_Priority        10
#define Onenet_Send_TimeSlices      5
static struct rt_thread     Onenet_Send_thread;            // �߳̿��ƿ�
ALIGN(RT_ALIGN_SIZE)    //ջ���뷽ʽ
static rt_uint8_t           Onenet_Send_thread_stack[5048];    //�߳�ջ






int main(void)
{
    rt_uint32_t count = 1;

    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(jidianqi0, PIN_MODE_OUTPUT);
    rt_pin_mode(jidianqi1, PIN_MODE_OUTPUT);
    rt_pin_mode(jidianqi2, PIN_MODE_OUTPUT);
    rt_pin_mode(jidianqi3, PIN_MODE_OUTPUT);



    rt_err_t result;

   //   ��ʼ��һ�� mailbox
     result = rt_mb_init(&mb,
                         "mbt",                     //  ������ mbt
                         &mb_pool[0],              //   �����õ����ڴ���� mb_pool
                         sizeof(mb_pool) / 4,       //  �����е��ʼ���Ŀ����Ϊһ���ʼ�ռ 4 �ֽ�
                         RT_IPC_FLAG_FIFO);         //  ���� FIFO ��ʽ�����̵߳ȴ�
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

    // ������̬�߳�
    tmp_result1 = rt_thread_init(&led1_thread,                 //�߳̿��ƿ�
                               "led1_blink",                 //�߳����֣���shell������Կ���
                               Led1_ThreadEntry,            //�߳���ں���
                               RT_NULL,                      //�߳���ں�������
                               &rt_led1_thread_stack[0],     //�߳�ջ��ʼ��ַ
                               sizeof(rt_led1_thread_stack), //�߳�ջ��С
                               LED1_Priority,                //�̵߳����ȼ�
                               LED1_TimeSlices);             //�߳�ʱ��Ƭ
    // ������̬�߳�
    tmp_result2 = rt_thread_init(&oled_thread,                //�߳̿��ƿ�
                               "oled_display",               //�߳����֣���shell������Կ���
                               OLED_ThreadEntry,             //�߳���ں���
                               RT_NULL,                      //�߳���ں�������
                               &rt_oled_thread_stack[0],     //�߳�ջ��ʼ��ַ
                               sizeof(rt_oled_thread_stack), //�߳�ջ��С
                               OLED_Priority,                //�̵߳����ȼ�
                               OLED_TimeSlices);             //�߳�ʱ��Ƭ

    // ������̬�߳�
    tmp_result3 = rt_thread_init(&rtc_thread,                //�߳̿��ƿ�
                               "rtc_time",               //�߳����֣���shell������Կ���
                               RTC_ThreadEntry,             //�߳���ں���
                               RT_NULL,                      //�߳���ں�������
                               &rt_rtc_thread_stack[0],     //�߳�ջ��ʼ��ַ
                               sizeof(rt_rtc_thread_stack), //�߳�ջ��С
                               RTC_Priority,                //�̵߳����ȼ�
                               RTC_TimeSlices);             //�߳�ʱ��Ƭ

    // ������̬�߳�
    tmp_result4 = rt_thread_init(&key_thread,                //�߳̿��ƿ�
                               "key_control",               //�߳����֣���shell������Կ���
                               KEY_ThreadEntry,             //�߳���ں���
                               RT_NULL,                      //�߳���ں�������
                               &rt_key_thread_stack[0],     //�߳�ջ��ʼ��ַ
                               sizeof(rt_key_thread_stack), //�߳�ջ��С
                               KEY_Priority,                //�̵߳����ȼ�
                               KEY_TimeSlices);             //�߳�ʱ��Ƭ

    // ������̬�߳�
//    tmp_result5 = rt_thread_init(&pedometer_thread,                //�߳̿��ƿ�
//                               "pedometer_control",               //�߳����֣���shell������Կ���
//                               Pedometer_ThreadEntry,             //�߳���ں���
//                               RT_NULL,                      //�߳���ں�������
//                               &rt_pedometer_thread_stack[0],     //�߳�ջ��ʼ��ַ
//                               sizeof(rt_pedometer_thread_stack), //�߳�ջ��С
//                               Pedometer_Priority,                //�̵߳����ȼ�
//                               Pedometer_TimeSlices);             //�߳�ʱ��Ƭ


    tmp_result5 = rt_thread_init(&sine_wave_thread,              //�߳̿��ƿ�
                               "sine_wave",               //�߳����֣���shell������Կ���
                               sine_wave_ThreadEntry,             //�߳���ں���
                               RT_NULL,                      //�߳���ں�������
                               &rt_sine_wave_thread_stack[0],     //�߳�ջ��ʼ��ַ
                               sizeof(rt_sine_wave_thread_stack), //�߳�ջ��С
                               sine_wave_Priority,                //�̵߳����ȼ�
                               sine_wave_TimeSlices);             //�߳�ʱ��Ƭ


    tmp_result6 = rt_thread_init(&My_AD_thread,                //�߳̿��ƿ�
                               "My_AD",               //�߳����֣���shell������Կ���
                               My_AD_ThreadEntry,             //�߳���ں���
                               RT_NULL,                      //�߳���ں�������
                               &My_AD_thread_stack[0],     //�߳�ջ��ʼ��ַ
                               sizeof(My_AD_thread_stack), //�߳�ջ��С
                               My_AD_Priority,                //�̵߳����ȼ�
                               My_AD_TimeSlices);             //�߳�ʱ��Ƭ


    tmp_result7 = rt_thread_init(&Battery_Voltage_thread,                //�߳̿��ƿ�
                               "Battery_Voltage",               //�߳����֣���shell������Կ���
                               Battery_Voltage_ThreadEntry,             //�߳���ں���
                               RT_NULL,                      //�߳���ں�������
                               &Battery_Voltage_thread_stack[0],     //�߳�ջ��ʼ��ַ
                               sizeof(Battery_Voltage_thread_stack), //�߳�ջ��С
                               Battery_Voltage_Priority,                //�̵߳����ȼ�
                               Battery_Voltage_TimeSlices);             //�߳�ʱ��Ƭ

    tmp_result8 = rt_thread_init(&Onenet_Send_thread,                //�߳̿��ƿ�
                               "onenet_send",               //�߳����֣���shell������Կ���
                               Onenet_Send_ThreadEntry,             //�߳���ں���
                               RT_NULL,                      //�߳���ں�������
                               &Onenet_Send_thread_stack[0],     //�߳�ջ��ʼ��ַ
                               sizeof(Onenet_Send_thread_stack), //�߳�ջ��С
                               Onenet_Send_Priority,                //�̵߳����ȼ�
                               Onenet_Send_TimeSlices);             //�߳�ʱ��Ƭ





    if(RT_EOK == tmp_result1)
    {
        rt_thread_startup(&led1_thread);             //�����߳�led0_thread����������
    }
    else
    {
        return -RT_ENOMEM;
    }

    if(RT_EOK == tmp_result2)
    {
        rt_thread_startup(&oled_thread);             //�����߳�led0_thread����������
    }
    else
    {
        return -RT_ENOMEM;
    }

    if(RT_EOK == tmp_result3)
    {
        rt_thread_startup(&rtc_thread);             //�����߳�led0_thread����������
    }
    else
    {
        return -RT_ENOMEM;
    }

    if(RT_EOK == tmp_result4)
    {
        rt_thread_startup(&key_thread);             //�����߳�led0_thread����������
    }
    else
    {
        return -RT_ENOMEM;
    }

    if(RT_EOK == tmp_result5)
     {
         rt_thread_startup(&sine_wave_thread);             //�����߳�led0_thread����������
     }
     else
     {
         return -RT_ENOMEM;
     }


    if(RT_EOK == tmp_result6)
    {
        rt_thread_startup(&My_AD_thread);             //�����߳�led0_thread����������
    }

    else
    {
        return -RT_ENOMEM;
    }

    if(RT_EOK == tmp_result7)
    {
        rt_thread_startup(&Battery_Voltage_thread);             //�����߳�led0_thread����������
    }

    else
    {
        return -RT_ENOMEM;
    }

    if(RT_EOK == tmp_result8)
    {
        rt_thread_startup(&Onenet_Send_thread);             //�����߳�led0_thread����������
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

    rt_pin_mode(LED1_PIN, PIN_MODE_OUTPUT);    //��ʼ��LED1����

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



/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-04-20     Zhang_Ziqi       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "drv_common.h"
#define ADC_DEV_NAME        "adc1"      /* ADC 设备名称 */
#define ADC_DEV_CHANNEL     0         /* ADC 通道 */
#define REFER_VOLTAGE       330         /* 参考电压 3.3V,数据精度乘以100保留2位小数*/
#define CONVERT_BITS        (1 << 16)   /* 转换位数为16位 */

#include "app_battery_voltage.h"

#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>


#include <netdev.h>
#include <rtdbg.h>
rt_ubase_t my_battery_voltage;
void Battery_Voltage_ThreadEntry(void* parameter)


{
    rt_uint32_t value, vol[20];
    rt_adc_device_t adc_dev;
    rt_err_t ret = RT_EOK;
    int k=0;
    int data_full_flag=0;
 //   vol1=vol2=vol3=vol4=vol5=vol6=vol7=vol8=0;
    extern int16_t mb;
    while(1)
  {
     if(netdev_is_internet_up(netdev_get_by_name("w0")))
     {

     /* 查找设备 */
    adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
    if (adc_dev == RT_NULL)
    {
        rt_kprintf("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
        return RT_ERROR;
    }

    /* 使能设备 */
    ret = rt_adc_enable(adc_dev, ADC_DEV_CHANNEL);


//   for(int i=0;i<20;i++)
//   {
    /* 读取采样值 */
    value = rt_adc_read(adc_dev, ADC_DEV_CHANNEL);
//    rt_kprintf("the ad3 value is :%d \n", value);



    /* 转换为对应电压值 */
    vol[k] = value * REFER_VOLTAGE / CONVERT_BITS;
     k++;
     if (k==10)
     {
         k=0;
     data_full_flag=1;
     }

     if (data_full_flag==1)
     {

         data_full_flag=0;

          for (int i = 0; i < 10; i++)//冒泡排序
                 {
                     for (int j = 0; j < 10-i; j++)//i<9-i
                     {
                         if (vol[j] > vol[j + 1])//满足条件进行交换
                         {
                             int temp = vol[j];
                             vol[j] = vol[j + 1];
                             vol[j + 1] = temp;
                         }
                     }
                 }

             my_battery_voltage=vol[5]*2.64;

     }


//   }




//    rt_kprintf("the ad3 voltage is :%d.%02d \n", my_battery_voltage / 100, my_battery_voltage % 100);

    /* 关闭通道 */
 //   ret = rt_adc_disable(adc_dev, ADC_DEV_CHANNEL);

 //   return ret;

      rt_mb_send(&mb, my_battery_voltage);

}




     rt_thread_mdelay(1000);  //这个很重要，否则连不上网！！！！！！


  }

}

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
#define ADC_DEV_NAME        "adc1"      /* ADC �豸���� */
#define ADC_DEV_CHANNEL     1         /* ADC ͨ�� */
#define REFER_VOLTAGE       330         /* �ο���ѹ 3.3V,���ݾ��ȳ���100����2λС��*/
#define CONVERT_BITS        (1 << 16)   /* ת��λ��Ϊ16λ */

#include "app_my_ad.h"


#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>


#include <netdev.h>
#include <rtdbg.h>
#include <onenet.h>


 void My_AD_ThreadEntry(void* parameter)


{
    rt_uint32_t value, vol;
    rt_adc_device_t adc_dev;
    rt_err_t ret = RT_EOK;

 while(1)
  {

  //       extern mb;
         if(netdev_is_internet_up(netdev_get_by_name("w0")))
         {


             /* �����豸 */
             adc_dev = (rt_adc_device_t)rt_device_find(ADC_DEV_NAME);
             if (adc_dev == RT_NULL)
             {
                 rt_kprintf("adc sample run failed! can't find %s device!\n", ADC_DEV_NAME);
                 return RT_ERROR;
             }

             /* ʹ���豸 */
             ret = rt_adc_enable(adc_dev, ADC_DEV_CHANNEL);
             /* ��ȡ����ֵ */
             value = rt_adc_read(adc_dev, ADC_DEV_CHANNEL);
    //    rt_kprintf("the value is :%d \n", value);

             /* ת��Ϊ��Ӧ��ѹֵ */
             vol = value * REFER_VOLTAGE / CONVERT_BITS;
      //       my_battery_temperature=vol;
   //          rt_kprintf("the ad1 voltage is :%d.%02d \n", vol / 100, vol % 100);
         //    rt_mb_send(&mb, vol);


             /* �ر�ͨ�� */
     //   ret = rt_adc_disable(adc_dev, ADC_DEV_CHANNEL);

     //   return ret;
         }

         rt_thread_mdelay(1000);

   }


}


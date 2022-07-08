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
#include "app_sine_wave.h"
#include "app_myiic.h"
#include <drivers/pin.h>
#include "drivers/hwtimer.h"
/*********************************定义定时器的相关参数**************************************/

#define HWTIMER_DEV_NAME   "timer13"     /* 定时器名称 */
rt_err_t ret = RT_EOK;
rt_hwtimerval_t timeout_s;      /* 定时器超时值 */
rt_device_t hw_dev = RT_NULL;   /* 定时器设备句柄 */
rt_hwtimer_mode_t mode;         /* 定时器模式 */
rt_uint32_t freq = 1000000;               /* 计数频率 */




#define My_DA_D7 GET_PIN(H, 2)
#define My_DA_D6 GET_PIN(B, 1)
#define My_DA_D5 GET_PIN(E, 5)
#define My_DA_D4 GET_PIN(E, 6)
#define My_DA_D3 GET_PIN(A, 12)
#define My_DA_D2 GET_PIN(E, 4)
#define My_DA_D1 GET_PIN(A, 6)
#define My_DA_D0 GET_PIN(A, 7)
#define DA_Clock GET_PIN(H, 3)


#define My_AD_D7 GET_PIN(H, 9)
#define My_AD_D6 GET_PIN(H, 10)
#define My_AD_D5 GET_PIN(D, 13)
#define My_AD_D4 GET_PIN(H, 7)
#define My_AD_D3 GET_PIN(G, 7)
#define My_AD_D2 GET_PIN(H, 8)
#define My_AD_D1 GET_PIN(C, 6)
#define My_AD_D0 GET_PIN(I, 0)









#define Pi 3.14159265
#define sine_max_current 0.0002


unsigned char auc_SinParam[128] = {
0x7F,0x85,0x8B,0x91,0x97,0x9D,0xA3,0xA9,0xAF,0xB5,0xBA,0xC0,0xC5,0xCA,0xCF,0xD4,
0xD8,0xDD,0xE1,0xE5,0xE8,0xEB,0xEF,0xF1,0xF4,0xF6,0xF8,0xFA,0xFB,0xFC,0xFD,0xFD,
0xFE,0xFD,0xFD,0xFC,0xFB,0xFA,0xF8,0xF6,0xF4,0xF1,0xEF,0xEB,0xE8,0xE5,0xE1,0xDD,
0xD8,0xD4,0xCF,0xCA,0xC5,0xC0,0xBA,0xB5,0xAF,0xA9,0xA3,0x9D,0x97,0x91,0x8B,0x85,
0x7F,0x78,0x72,0x6C,0x66,0x60,0x5A,0x54,0x4E,0x48,0x43,0x3D,0x38,0x33,0x2E,0x29,
0x25,0x20,0x1C,0x18,0x15,0x12,0x0E,0x0C,0x09,0x07,0x05,0x03,0x02,0x01,0x00,0x00,
0x00,0x00,0x00,0x01,0x02,0x03,0x05,0x07,0x09,0x0C,0x0E,0x12,0x15,0x18,0x1C,0x20,
0x25,0x29,0x2E,0x33,0x38,0x3D,0x43,0x48,0x4E,0x54,0x5A,0x60,0x66,0x6C,0x72,0x78};


uint8_t ack;
unsigned char i=0;
unsigned char j=0;
unsigned char k=0;
unsigned char sine_phase_delay_time_buf=0;
unsigned char sine_phase_delay_time=0;
unsigned char sine_phase_delay_time_count=0;
unsigned char sine_phase_delay_time_count_fullflag=0;
unsigned char my_sine_phase_delay_time[20];
double sine_angle;
int my_sine_angle;
int my_sinwave_max;
double battery_temprature;
double my_battery_temperature;

double long sinwave_max=0;

unsigned char data[128];
unsigned char data_buf[10];


unsigned char sine_data[10][128];
unsigned char sine_data_x=0;
unsigned char sine_data_y=0;
unsigned char ave_sine_data_x=0;
unsigned char ave_sine_data_y=0;

long sine_data_sum=0;

long ave_sine_data[128];


unsigned char full_flag;


void my_DA_conventer_init (void)
{
    rt_pin_mode(My_DA_D7, PIN_MODE_OUTPUT);
    rt_pin_mode(My_DA_D6, PIN_MODE_OUTPUT);
    rt_pin_mode(My_DA_D5, PIN_MODE_OUTPUT);
    rt_pin_mode(My_DA_D4, PIN_MODE_OUTPUT);
    rt_pin_mode(My_DA_D3, PIN_MODE_OUTPUT);
    rt_pin_mode(My_DA_D2, PIN_MODE_OUTPUT);
    rt_pin_mode(My_DA_D1, PIN_MODE_OUTPUT);
    rt_pin_mode(My_DA_D0, PIN_MODE_OUTPUT);
  //  rt_pin_mode(My_DA_D0, PIN_MODE_OUTPUT);
    rt_pin_mode(DA_Clock, PIN_MODE_OUTPUT);




}

void my_AD_conventer_init (void)

{
    rt_pin_mode(My_AD_D7, PIN_MODE_INPUT);
    rt_pin_mode(My_AD_D6, PIN_MODE_INPUT);
    rt_pin_mode(My_AD_D5, PIN_MODE_INPUT);
    rt_pin_mode(My_AD_D4, PIN_MODE_INPUT);
    rt_pin_mode(My_AD_D3, PIN_MODE_INPUT);
    rt_pin_mode(My_AD_D2, PIN_MODE_INPUT);
    rt_pin_mode(My_AD_D1, PIN_MODE_INPUT);
    rt_pin_mode(My_AD_D0, PIN_MODE_INPUT);

}



void my_DA_conventer (unsigned char data)
{

    rt_pin_write(My_DA_D7, (data & 0x80) >> 7);
    rt_pin_write(My_DA_D6, (data & 0x40) >> 6);
    rt_pin_write(My_DA_D5, (data & 0x20) >> 5);
    rt_pin_write(My_DA_D4, (data & 0x10) >> 4);
    rt_pin_write(My_DA_D3, (data & 0x08) >> 3);
    rt_pin_write(My_DA_D2, (data & 0x04) >> 2);
    rt_pin_write(My_DA_D1, (data & 0x02) >> 1);
    rt_pin_write(My_DA_D0, (data & 0x01)     );

}


unsigned char my_AD_conventer (void)
{

    unsigned char data=0;
   data=(rt_pin_read(My_AD_D7) | data)<<1;
   data=(rt_pin_read(My_AD_D6) | data)<<1;
   data=(rt_pin_read(My_AD_D5) | data)<<1;
   data=(rt_pin_read(My_AD_D4) | data)<<1;
   data=(rt_pin_read(My_AD_D3) | data)<<1;
   data=(rt_pin_read(My_AD_D2) | data)<<1;
   data=(rt_pin_read(My_AD_D1) | data)   ;

return data;
}





/* 定时器超时回调函数 */
static rt_err_t timeout_cb(rt_device_t dev, rt_size_t size)
{
    my_DA_conventer(auc_SinParam[i++]);

    rt_pin_write(DA_Clock, rt_pin_read(DA_Clock)^1);  //ADDA时钟
    if(i==128)
           i=0;

        if(i==32)
        {
            sine_data_y=0;
            sine_data_x++;
         }

        if (sine_data_x==10)
         {
             sine_data_x=0;
             full_flag=1;
          }

     if(full_flag==0)
    {
        sine_data[sine_data_x][sine_data_y]=my_AD_conventer ();
    }

     sine_data_y=sine_data_y+1;

   return 0;
}



void sine_wave_ThreadEntry(void* parameter)
{


    /* 查找定时器设备 */
       hw_dev = rt_device_find(HWTIMER_DEV_NAME);
       if (hw_dev == RT_NULL)
       {
           rt_kprintf("hwtimer sample run failed! can't find %s device!\n", HWTIMER_DEV_NAME);
           return RT_ERROR;
       }

       /* 以读写方式打开设备 */
       ret = rt_device_open(hw_dev, RT_DEVICE_OFLAG_RDWR);
       if (ret != RT_EOK)
       {
           rt_kprintf("open %s device failed!\n", HWTIMER_DEV_NAME);
           return ret;
       }

       /* 设置超时回调函数 */
       rt_device_set_rx_indicate(hw_dev, timeout_cb);

       /* 设置计数频率(若未设置该项，默认为1Mhz 或 支持的最小计数频率) */
       rt_device_control(hw_dev, HWTIMER_CTRL_FREQ_SET, &freq);
       /* 设置模式为周期性定时器（若未设置，默认是HWTIMER_MODE_ONESHOT）*/
       mode = HWTIMER_MODE_PERIOD;
       ret = rt_device_control(hw_dev, HWTIMER_CTRL_MODE_SET, &mode);
       if (ret != RT_EOK)
       {
           rt_kprintf("set mode failed! ret is :%d\n", ret);
           return ret;
       }


       /* 设置定时器超时值并启动定时器 */
          timeout_s.sec = 0;      /* 秒 */
          timeout_s.usec = 195;     /* 微秒 */
          if (rt_device_write(hw_dev, 0, &timeout_s, sizeof(timeout_s)) != sizeof(timeout_s))
          {
              rt_kprintf("set timeout value failed\n");
              return RT_ERROR;
          }



    while (1)
   {


      if(full_flag==1)
      {
          for (ave_sine_data_y=0;ave_sine_data_y<128;ave_sine_data_y++)   /*   中位值滤波   */
          {
              for (ave_sine_data_x=1;ave_sine_data_x<10;ave_sine_data_x++)
              {
                  sine_data_sum =sine_data[ave_sine_data_x][ave_sine_data_y]+sine_data_sum;
      //            rt_kprintf("sine_data[%d][%d] is :%d !\n",ave_sine_data_x,ave_sine_data_y ,sine_data[ave_sine_data_x][ave_sine_data_y]);
      //            rt_kprintf("sine_data_sum is :%d !\n",sine_data_sum);
              }
              ave_sine_data[ave_sine_data_y] =sine_data_sum/9;
              sine_data_sum=0;
            //  rt_kprintf("sine_data[%d][%d] is :%d !\n",ave_sine_data_x,ave_sine_data_y ,sine_data[ave_sine_data_x][ave_sine_data_y]);

        //      rt_kprintf("ave_sine_data[%d] is :%d !\n", ave_sine_data_y,  ave_sine_data[ave_sine_data_y]);

     //         rt_kprintf("di %d hang!\n", ave_sine_data_y);
          }
            sinwave_max=ave_sine_data[0];
          for(j=1; j<128 ; j++)//因为已经假设第一个为max,所以i从1开始       计算第一个最大值位置
           {
              if(ave_sine_data[j]> sinwave_max)
              {
                sinwave_max = ave_sine_data[j];
                sine_phase_delay_time = j+1;
               }
            //  rt_kprintf("ave_sine_data[%d] is %d !\n", j,ave_sine_data[j]);
            }
          sinwave_max=ave_sine_data[0];
          for(j=1; j<128 ; j++)
               {

                  if(ave_sine_data[j]>= sinwave_max)//计算最后一个最大值
                  {
                    sinwave_max = ave_sine_data[j];
                    sine_phase_delay_time =(sine_phase_delay_time+j+1)/2;    //取中间值
                   }
                }


          my_sinwave_max=(int)sinwave_max;

          my_sine_phase_delay_time [sine_phase_delay_time_count]=sine_phase_delay_time;

          sine_phase_delay_time_count++;

 //         rt_kprintf("my_sine_phase_delay_time[%d] is:%d !\n",sine_phase_delay_time_count,my_sine_phase_delay_time [sine_phase_delay_time_count] );

          if (sine_phase_delay_time_count==20)
          {
          sine_phase_delay_time_count=0;
          sine_phase_delay_time_count_fullflag=1;

 //         rt_kprintf("my_sine_phase_delay_time[%d] is:%d !\n",sine_phase_delay_time_count,my_sine_phase_delay_time [sine_phase_delay_time_count] );
          }

  //        rt_kprintf("my_sine_phase_delay_time[%d] is:%d !\n",sine_phase_delay_time_count,my_sine_phase_delay_time [sine_phase_delay_time_count] );

  //        rt_kprintf("sine_phase_delay_time22222 is %d !\n", sine_phase_delay_time);


               sine_phase_delay_time=sine_phase_delay_time-50;            //相位校准

           //     rt_kprintf("sine_phase_delay_time is %d !\n", sine_phase_delay_time);

          sine_angle=sine_phase_delay_time;
          sine_angle= sine_angle/128*1000;

          my_sine_angle=(int)sine_angle;//整形参数送到onenet  my_sine_angle/1000 *pi.
         // sine_angle=  (sine_angle/128)*2*Pi;

    //     rt_kprintf(" sine_angle is %f !\n", sine_angle);

        //  rt_kprintf("Pi is %d !\n", sine_phase_delay_time);
         //       sine_angle=   sin( sine_angle);  //阻抗角度

      //         rt_kprintf(" sin sine_angle is %f !\n", sine_angle);

        //   battery_temprature= 600/ (sinwave_max/256*5/sine_max_current*sine_angle-5428);
        //   battery_temprature=33.15/(1.8*sine_angle-0.2);


        // rt_kprintf(" battery_temprature is %f !\n", my_battery_temperature);
          full_flag=0;     /*数据转换结束*/



//     if (full_flag==1)
//     {
//         for (j=0;j<128;j++)
//         { rt_kprintf("sine_data[1][%d] is :%d !\n",j ,sine_data[1][j]);}
//         full_flag=0;
//
//     }




      if (sine_phase_delay_time_count_fullflag==1)
      {
          for (int i = 0; i < 20; i++)//冒泡排序
              {
                  for (int j = 0; j < 20-i; j++)//i<9-i
                  {
                      if (my_sine_phase_delay_time[j] > my_sine_phase_delay_time[j + 1])//满足条件进行交换
                      {
                          int temp = my_sine_phase_delay_time[j];
                          my_sine_phase_delay_time[j] = my_sine_phase_delay_time[j + 1];
                          my_sine_phase_delay_time[j + 1] = temp;
                      }
                  }
              }


             sine_phase_delay_time_count_fullflag=0;


       my_battery_temperature= (my_sine_phase_delay_time[10]+6.96)/3.04 ;

      }


       }



rt_thread_mdelay(5);



   }

}








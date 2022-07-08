

#include <rtthread.h>
#include "app_onenet_send.h"

#include <stdlib.h>

#include <onenet.h>
#include <lwip/inet.h>
#include <lwip/sockets.h>
#include <lwip/netdb.h>


#include <netdev.h>
#include <rtdbg.h>
//#include "app_oled.h"
//#include <u8g2_port.h>
//#include <string.h>

//char *str_votage_temprature = NULL;
void Onenet_Send_ThreadEntry(void* parameter)
{
    rt_uint32_t onenet_mqtt_init_flag = 0;
extern struct rt_mailbox mb;
//extern double my_battery_temperature;
int my_battery_voltage;
extern int int_my_battery_temperature;
extern int my_sine_angle;
extern int my_sinwave_max;
//u8g2_t u8g4;
    while (1)
    {
        if(netdev_is_internet_up(netdev_get_by_name("w0")))
        {

          if(onenet_mqtt_init_flag==0)
          {
              onenet_mqtt_init_flag=1;
              onenet_mqtt_init();
          }


          if (rt_mb_recv(&mb, (rt_uint32_t *)&my_battery_voltage, RT_WAITING_NO) == RT_EOK)
          {

              rt_kprintf("thread1: get a mail from mailbox, the content:%d\n", my_battery_voltage);

          }


              onenet_mqtt_upload_digit("temperature", int_my_battery_temperature);
              onenet_mqtt_upload_digit("voltage", my_battery_voltage);
              onenet_mqtt_upload_digit("my_sine_angle", my_sine_angle);
              onenet_mqtt_upload_digit("my_sinwave_max", my_sinwave_max);

         }

        rt_thread_mdelay(1000);


    }


}

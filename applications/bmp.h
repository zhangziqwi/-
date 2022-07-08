/*
GD32E231
作者：hehung
时间：2019/5/25
QQ：1398660197
邮箱：1398660197@qq.com
声明：未经作者允许禁止转载
*/

#ifndef __BMP_H
#define __BMP_H

unsigned int Get_my_logo_len(void);
unsigned int Get_Rtt_logo_len(void);
unsigned int Get_Battery_logo_len(void);

unsigned int Get_Wifi_16X16(void);
unsigned int Get_Wifi_24X24(void);

extern unsigned char my_logo[][16];
extern unsigned char RTT_logo[][16];
extern unsigned char Battery_logo1[][16];
extern unsigned char Battery_logo2[][16];
extern unsigned char Battery_logo3[][16];
extern unsigned char Battery_logo4[][16];


extern unsigned char Wifi_16X16[][8];
extern unsigned char Wifi_24X24[][8];

#endif



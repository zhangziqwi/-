/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-13     hehung       the first version
 */

#include <rtthread.h>
#include <rtdevice.h>
#include "app_compass.h"
#include "app_myiic.h"
#include "app_sensor.h"
#include <math.h>
#include <rthw.h>
#include "app_step.h"

static void LIS2MDL_InitAndSelfTest(void);
static void LIS2DW12_Init(void);

/*磁场传感器*/
short compass_out_x = 0;
short compass_out_y = 0;
short compass_out_z = 0;
double compass_azimuth_r = 0.0f;    /*航向角*/
short compass_azimuth_a = 0;
short compass_angle_cal = 0;

/*重力加速器*/
short acc_x = 0;
short acc_y = 0;
short acc_z = 0;
short acc_x_yz_a = 0;
short acc_y_xz_a = 0;

short Hy = 0;
short Hx = 0;

#define PI    3.1415

short Compass_GetAzimut(void)
{
    return compass_azimuth_a;
}

void Compass_ThreadEntry(void *parameter)
{
    LIS2MDL_InitAndSelfTest();
    LIS2DW12_Init();

    while(1)
    {
//        rt_enter_critical();
        compass_out_x = (short)GET_X_LIS2MDL();
        compass_out_y = (short)GET_Y_LIS2MDL();
        compass_out_z = (short)GET_Z_LIS2MDL();
//        rt_exit_critical();

        compass_azimuth_r = atan2((double)compass_out_y, (double)compass_out_x);
        compass_azimuth_a = (int)((compass_azimuth_r/PI)*180);

        /*补偿算法*/
        acc_x = LSM6DSO_GetAccX();
        acc_y = LSM6DSO_GetAccY();
        acc_z = LSM6DSO_GetAccZ();

        acc_x_yz_a = atan2((double)acc_x, (double)sqrt(acc_y*acc_y + acc_z*acc_z));
        acc_y_xz_a = atan2((double)acc_y, (double)sqrt(acc_x*acc_x + acc_z*acc_z));

        Hy = compass_out_y*cos(acc_y_xz_a) + compass_out_x*sin(acc_y_xz_a)*sin(acc_x_yz_a)
                                           - compass_out_z*cos(acc_x_yz_a)*sin(acc_y_xz_a);
        Hx = compass_out_x*cos(acc_x_yz_a) + compass_out_z*sin(acc_x_yz_a);

        compass_azimuth_r = atan2((double)Hy, (double)Hx);
        compass_azimuth_a = (int)((compass_azimuth_r/PI)*180);

        rt_kprintf("X-%03d = Y-%03d = Z-%03d = Azimuth-%03d\n", compass_out_x, compass_out_y, compass_out_z, compass_azimuth_a);

        rt_thread_mdelay(100);
    }

}

/*LIS2MDL 自测*/
static void LIS2MDL_InitAndSelfTest(void)
{
    int OUTX_F = 0;
    int OUTY_F = 0;
    int OUTZ_F = 0;

    int OUTX_E = 0;
    int OUTY_E = 0;
    int OUTZ_E = 0;

    int i = 0;
    uint8_t ret_status = 0x00;

    if(0 == Set_Reg_LIS2MDL(LIS2MDL_ADDR_W_1, LIS2MDL_CFG_A, 0x8C))
    {
        rt_kprintf("%s","IF_CTRL set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","IF_CTRL set fail\r\n");
    }

    if(0 == Set_Reg_LIS2MDL(LIS2MDL_ADDR_W_1, LIS2MDL_CFG_B, 0x02))
    {
        rt_kprintf("%s","IF_CTRL set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","IF_CTRL set fail\r\n");
    }

    if(0 == Set_Reg_LIS2MDL(LIS2MDL_ADDR_W_1, LIS2MDL_CFG_C, 0x10))
    {
        rt_kprintf("%s","IF_CTRL set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","IF_CTRL set fail\r\n");
    }

    rt_thread_delay(20);  /*wait 20ms*/

    ret_status = Get_Status_LIS2MDL();
    rt_kprintf("%d\n",ret_status);
    if((ret_status&0x08) == 0x08)
    {
        for(i=0; i<50; i++)
        {
            OUTX_F += (short)GET_X_LIS2MDL();
            OUTY_F += (short)GET_Y_LIS2MDL();
            OUTZ_F += (short)GET_Z_LIS2MDL();
            rt_thread_delay(1);
        }
        OUTX_F = (short)(OUTX_F/50);
        OUTY_F = (short)(OUTY_F/50);
        OUTZ_F = (short)(OUTZ_F/50);
        rt_kprintf("%s","Set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","Set fail\r\n");
    }

    if(0 == Set_Reg_LIS2MDL(LIS2MDL_ADDR_W_1, LIS2MDL_CFG_C, 0x12))
    {
        rt_kprintf("%s","IF_CTRL C set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","IF_CTRL C set fail\r\n");
    }

    rt_thread_delay(60);  /*wait 20ms*/

    ret_status = Get_Status_LIS2MDL();
    rt_kprintf("%d\n",ret_status);
    if((ret_status&0x08) == 0x08)
    {
        for(i=0; i<50; i++)
        {
            OUTX_E += (short)GET_X_LIS2MDL();
            OUTY_E += (short)GET_Y_LIS2MDL();
            OUTZ_E += (short)GET_Z_LIS2MDL();
            rt_thread_delay(1);
        }
        OUTX_E = (short)(OUTX_E/50);
        OUTY_E = (short)(OUTY_E/50);
        OUTZ_E = (short)(OUTZ_E/50);
        rt_kprintf("%s","Set successful\r\n");

    }
    else
    {
        rt_kprintf("%s","Set fail\r\n");
    }
}

/*加速度计初始化*/
static void LIS2DW12_Init(void)
{
    if((0 == Set_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_CTRL1, 0x16))
        && (0 == Set_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_CTRL6, 0x01)))
    {
        rt_kprintf("%s","LIS2DW12-IF_CTRL set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","LIS2DW12-IF_CTRL set fail\r\n");
    }
}

/*Test*/
static void LIS2MDL_Test(void)
{
    int OUTX_F = 0;
    int OUTY_F = 0;
    int OUTZ_F = 0;

    int OUTX_E = 0;
    int OUTY_E = 0;
    int OUTZ_E = 0;

    int i = 0;
    uint8_t ret_status = 0x00;

    if(0 == Set_Reg_LIS2MDL(LIS2MDL_ADDR_W_1, LIS2MDL_CFG_A, 0x8C))
    {
        rt_kprintf("%s","IF_CTRL set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","IF_CTRL set fail\r\n");
    }

    if(0 == Set_Reg_LIS2MDL(LIS2MDL_ADDR_W_1, LIS2MDL_CFG_B, 0x02))
    {
        rt_kprintf("%s","IF_CTRL set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","IF_CTRL set fail\r\n");
    }

    if(0 == Set_Reg_LIS2MDL(LIS2MDL_ADDR_W_1, LIS2MDL_CFG_C, 0x10))
    {
        rt_kprintf("%s","IF_CTRL set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","IF_CTRL set fail\r\n");
    }

    rt_thread_delay(20);  /*wait 20ms*/

    ret_status = Get_Status_LIS2MDL();
    rt_kprintf("%d\n",ret_status);
    if((ret_status&0x08) == 0x08)
    {
        for(i=0; i<50; i++)
        {
            OUTX_F += (short)GET_X_LIS2MDL();
            OUTY_F += (short)GET_Y_LIS2MDL();
            OUTZ_F += (short)GET_Z_LIS2MDL();
            rt_thread_delay(1);
        }
        OUTX_F = (short)(OUTX_F/50);
        OUTY_F = (short)(OUTY_F/50);
        OUTZ_F = (short)(OUTZ_F/50);
        rt_kprintf("%s","Set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","Set fail\r\n");
    }


    if(0 == Set_Reg_LIS2MDL(LIS2MDL_ADDR_W_1, LIS2MDL_CFG_C, 0x12))
    {
        rt_kprintf("%s","IF_CTRL C set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","IF_CTRL C set fail\r\n");
    }

    rt_thread_delay(60);  /*wait 20ms*/

    ret_status = Get_Status_LIS2MDL();
    rt_kprintf("%d\n",ret_status);
    if((ret_status&0x08) == 0x08)
    {
        for(i=0; i<50; i++)
        {
            OUTX_E += (short)GET_X_LIS2MDL();
            OUTY_E += (short)GET_Y_LIS2MDL();
            OUTZ_E += (short)GET_Z_LIS2MDL();
            rt_thread_delay(1);
        }
        OUTX_E = (short)(OUTX_E/50);
        OUTY_E = (short)(OUTY_E/50);
        OUTZ_E = (short)(OUTZ_E/50);
        rt_kprintf("%s","Set successful\r\n");

    }
    else
    {
        rt_kprintf("%s","Set fail\r\n");
    }

    rt_kprintf("OUT X F%d\n", OUTX_F);
    rt_kprintf("OUT Y F%d\n", OUTY_F);
    rt_kprintf("OUT Z F%d\n", OUTZ_F);

    rt_kprintf("OUT X E%d\n", OUTX_E);
    rt_kprintf("OUT Y E%d\n", OUTY_E);
    rt_kprintf("OUT Z E%d\n", OUTZ_E);

    rt_kprintf("OUT X O%d\n", OUTX_E-OUTX_F);
    rt_kprintf("OUT Y O%d\n", OUTY_E-OUTY_F);
    rt_kprintf("OUT Z O%d\n", OUTZ_E-OUTZ_F);

    if(0 == Set_Reg_LIS2MDL(LIS2MDL_ADDR_W_1, LIS2MDL_CFG_C, 0x10))
    {
        rt_kprintf("%s","IF_CTRL set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","IF_CTRL set fail\r\n");
    }
    while(1)
    {
        OUTX_F = (short)GET_X_LIS2MDL();
        OUTY_F = (short)GET_Y_LIS2MDL();
        OUTZ_F = (short)GET_Z_LIS2MDL();
        rt_kprintf("OUT X F-%03d => OUT Y F-%03d => OUT Z F-%03d\n", OUTX_F, OUTY_F, OUTZ_F);
        rt_thread_delay(50);
    }

}
MSH_CMD_EXPORT(LIS2MDL_Test, i2c STTS751 temperature sample);

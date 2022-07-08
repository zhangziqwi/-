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
#include <rthw.h>
#include "app_step.h"
#include "app_myiic.h"
#include "app_sensor.h"

static void step_counter(void);
static uint16_t LSM6DSO_ReadPedometerStep(void);
static int LSM6DSO_ReadPedometerTrigger(void);
static void LSM6DSO_InitPedometer(void);
static void LSM6DSO_Init(void);

unsigned char bad_flag[3];
unsigned int array0[3]={1,1,1};
unsigned int array1[3]={1,1,1};
unsigned int array2[3]={0,0,0};
unsigned int adresult[3];
unsigned int max[3]={0,0,0};
unsigned int min[3]={1000,1000,1000};
unsigned int dc[3]={500,500,500};
unsigned int vpp[3]={30,30,30};
unsigned int precision[3]={5,5,5};
unsigned int old_fixed[3];
unsigned int new_fixed[3];
unsigned int STEPS=0;
unsigned int time_r=0;
short BUF[3];   // 接收数据缓存区

short gyro_x = 0;
short gyro_y = 0;
short gyro_z = 0;

short acc_x_o = 0;
short acc_y_o = 0;
short acc_z_o = 0;

short LSM6DSO_GetAccX(void)
{
    return acc_x_o;
}

short LSM6DSO_GetAccY(void)
{
    return acc_y_o;
}

short LSM6DSO_GetAccZ(void)
{
    return acc_y_o;
}

uint16_t Step_GetStep(void)
{
    return STEPS;
}

void Step_ClearStep(void)
{
    STEPS = 0;
}

void Pedometer_ThreadEntry(void *parameter)
{
    LSM6DSO_Init();

    while(1)
    {
//        gyro_x = Get_X_G_LSM6DSO();
//        gyro_y = Get_Y_G_LSM6DSO();
//        gyro_z = Get_Z_G_LSM6DSO();
        acc_x_o = Get_X_A_LSM6DSO();
        acc_y_o = Get_Y_A_LSM6DSO();
        acc_z_o = Get_Z_A_LSM6DSO();
        BUF[0] = acc_x_o;
        BUF[1] = acc_y_o;
        BUF[2] = acc_z_o;

//        rt_kprintf("G_X:%06d | G_Y:%06d | G_Z:%06d\r\n", gyro_x, gyro_y, gyro_z);
//        rt_kprintf("A_X:%06d | A_Y:%06d | A_Z:%06d\r\n", acc_x_o, acc_y_o, acc_z_o);
//        rt_kprintf("Who_am_i:%X\r\n",Get_WHO_AM_I_LSM6DSO());

        step_counter();
//        rt_kprintf("step=%d\r\n", STEPS);

        rt_thread_mdelay(100);
    }
}

static void step_counter(void)
{
    static uint8_t sampling_counter=0;
    uint8_t jtemp;

    //------------------------------------------采样滤波----------------------//
    for(jtemp=0; jtemp<=2; jtemp++)      //jtemp 0,1,2分别代表x，y，z
    {
        array2[jtemp] = array1[jtemp];
        array1[jtemp] = array0[jtemp];
        array0[jtemp] = BUF[jtemp];

        adresult[jtemp] = array0[jtemp]+array1[jtemp]+array2[jtemp];
        adresult[jtemp] = adresult[jtemp]/3;
        if(adresult[jtemp] > max[jtemp])
        {
            max[jtemp]=adresult[jtemp];
        }
        else
        {
            /*Do Nothing*/
        }
        if(adresult[jtemp] < min[jtemp])
        {
            min[jtemp]=adresult[jtemp];
        }
        else
        {
            /*Do Nothing*/
        }
    }
    sampling_counter = sampling_counter+1;
    //----------------------------------计算动态门限和动态精度-----------------------//
    if(sampling_counter >= 50)
    {
        sampling_counter = 0;
        for(jtemp=0; jtemp<=2; jtemp++)
        {
            vpp[jtemp] = max[jtemp]-min[jtemp];
            dc[jtemp] = min[jtemp]+(vpp[jtemp]>>1);    //dc为阈值
            max[jtemp] = 0;
            min[jtemp] = 1023;
            bad_flag[jtemp] = 0;
            if(vpp[jtemp] >= 160)
            {
                precision[jtemp] = vpp[jtemp]/32; //8
            }
            else if((vpp[jtemp]>=50)&& (vpp[jtemp]<160))
            {
                precision[jtemp] = 4;
            }
            else if((vpp[jtemp]>=15) && (vpp[jtemp]<50))
            {
                precision[jtemp] = 3;
            }
            else
            {
                precision[jtemp] = 2;
                bad_flag[jtemp] = 1;
            }
        }
    }
    //--------------------------线性移位寄存器--------------------------------------
    for(jtemp=0; jtemp<=2; jtemp++)
    {
        old_fixed[jtemp] = new_fixed[jtemp];

        if(adresult[jtemp] >= new_fixed[jtemp])
        {
            if((adresult[jtemp]-new_fixed[jtemp]) >= precision[jtemp])
            {
                new_fixed[jtemp] = adresult[jtemp];
            }
            else
            {
                /*Do Nothing*/
            }
        }
        else if(adresult[jtemp]<new_fixed[jtemp])
        {
            if((new_fixed[jtemp]-adresult[jtemp]) >= precision[jtemp])
            {
                new_fixed[jtemp] = adresult[jtemp];
            }
            else
            {
                /*Do Nothing*/
            }
        }
        else
        {
            /*Do Nothing*/
        }
    }
    //------------------------- 动态门限判决 ----------------------------------
    if((vpp[0]>=vpp[1])&&(vpp[0]>=vpp[2]))   //x轴最活跃
    {
        if((old_fixed[0]>=dc[0])&&(new_fixed[0]<dc[0])&&(bad_flag[0]==0))
        {
            STEPS = STEPS+1;
        }
        else
        {
            /*Do Nothing*/
        }
    }
    else if((vpp[1]>=vpp[0])&&(vpp[1]>=vpp[2]))  //y轴最活跃
    {
        if((old_fixed[1]>=dc[1])&&(new_fixed[1]<dc[1])&&(bad_flag[1]==0))
        {
            STEPS = STEPS+1;
        }
        else
        {
            /*Do Nothing*/
        }
    }
    else if((vpp[2]>=vpp[1]) && (vpp[2]>=vpp[0]))    //z轴最活跃
    {
        if((old_fixed[2]>=dc[2]) && (new_fixed[2]<dc[2])&&(bad_flag[2]==0))
        {
            STEPS = STEPS+1;
        }
        else
        {
            /*Do Nothing*/
        }
    }
    else
    {
        /*Do Nothing*/
    }
}

static uint16_t LSM6DSO_ReadPedometerStep(void)
{
    return (Get_Reg(LIS2MDL_ADDR_W_1, LSM6DSO_STEP_COUNTER_H)<<8)|(Get_Reg(LIS2MDL_ADDR_W_1, LSM6DSO_STEP_COUNTER_L));
}

static int LSM6DSO_ReadPedometerTrigger(void)
{
    return (Get_Reg(LIS2MDL_ADDR_W_1, LSM6DSO_EMB_FUNC_SRC));
}

static void LSM6DSO_Init(void)
{
    if((0 == Set_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_CTRL1_XL, 0x20))
        && (0 == Set_Reg_LSM6DSO(LSM6DSO_ADDR_W_1, LSM6DSO_CTRL2_G, 0x12))
        && (0 == Set_Reg_LSM6DSO(LSM6DSO_ADDR_W_1, LSM6DSO_FUNC_CFG_ACCESS, 0x00)))
    {
        rt_kprintf("%s","LSM6DSO set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","LSM6DSO set fail\r\n");
    }
}

static void LSM6DSO_InitPedometer(void)
{
    uint8_t status = 0;
    if((0 == Set_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_FUNC_CFG_ACCESS, 0x80))
        && (0 == Set_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_EMB_FUNC_EN_A, 0x28))
        && (0 == Set_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_EMB_FUNC_EN_B, 0x11)))
    {
        rt_kprintf("%s","LSM6DSO EMB set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","LSM6DSO EMB set fail\r\n");
    }

    status = Get_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_EMB_FUNC_SRC);
    if((0 == Set_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_EMB_FUNC_SRC, 0x80|status)))
    {
        rt_kprintf("%s","LSM6DSO EMB set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","LSM6DSO EMB set fail\r\n");
    }
}

static void LSM6DSO_SendUart(void)
{
//    Set_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_FUNC_CFG_ACCESS, 0x80);
    while(Get_Reg(LIS2MDL_ADDR_W_1, LSM6DSO_FUNC_CFG_ACCESS) == 0)
    {
        Set_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_FUNC_CFG_ACCESS, 0x80);
        rt_kprintf("FUNC_CFG_ACCESS=%d \n", Get_Reg(LIS2MDL_ADDR_W_1, LSM6DSO_FUNC_CFG_ACCESS));
    }
    Set_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_FUNC_CFG_ACCESS, 0x00);

    rt_kprintf("Step-meter=%d \n", LSM6DSO_ReadPedometerStep());

    rt_kprintf("Step-meter-triggered=%d \n", LSM6DSO_ReadPedometerTrigger());

}

static void step_counter_test(void)
{
    LSM6DSO_InitPedometer();

    LSM6DSO_SendUart();
}

MSH_CMD_EXPORT(step_counter_test, LSM6DSO pedometer sample);



/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-02     hehung       the first version
 */
#include <rtthread.h>
#include <rtdevice.h>
#include "app_sensor.h"
#include "app_myiic.h"
#include <rthw.h>


static int IIC_WrDat(uint8_t IIC_Add, uint8_t IIC_Reg, uint8_t IIC_Data);
static int IIC_Read(uint8_t IIC_Add, uint8_t IIC_Reg);

//IIC写
static int IIC_WrDat(uint8_t IIC_Add, uint8_t IIC_Reg, uint8_t IIC_Data)
{
    uint8_t ret_suc = 1;

    IIC_Start();
    IIC_Send_Byte(IIC_Add & 0xfe);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Reg);         //write data
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Data);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Stop();

    return 0;
}

//IIC读
static int IIC_Read(uint8_t IIC_Add, uint8_t IIC_Reg)
{
    int ret;
    uint8_t ret_suc = 1;

    IIC_Start();
    IIC_Send_Byte(IIC_Add & 0xfe);
    ret_suc = IIC_Wait_Ack();
//    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Reg);
    ret_suc = IIC_Wait_Ack();
//    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Start();
    IIC_Send_Byte(IIC_Add | 0x01);
    ret_suc = IIC_Wait_Ack();
//    if(ret_suc == 1)    {IIC_Stop();return -1;}

    ret=IIC_Read_Byte(0);
    IIC_Stop();

    return ret;
}

//设置寄存器
int Set_Reg(uint8_t add, uint8_t reg, uint8_t dat)
{
    int ret_succ = -1;
    uint8_t i = 20u;        //失败之后尝试的次数

    do{
        if(0 == IIC_WrDat(add, reg, dat))
        {
            ret_succ = 0;       //设置成功
            break;
        }
        else
        {
            i--;                //获取失败，再次尝试
        }
    }while(i>0u);

    return ret_succ;
}

//读取寄存器
int Get_Reg(uint8_t add, uint8_t reg)
{
//    uint16_t i = 20u;
    int ret_get = -1;
//
//    do{
        ret_get = IIC_Read(add, reg);
//        if(-1 != ret_get)
//        {
//            break;              //获取数据成功
//        }
//        else
//        {
//            i--;                //获取失败，再次尝试
//        }
//    }while(i>0u);

    return ret_get;
}

/*
 * STTS751
 *
 */
//读取温度高字节
int Read_Temperature_H_STTS751(void)
{
    return Get_Reg(STTS751_ADDR, STTS751_REG_TEMPVH);
}

//读取温度低字节
int Read_Temperature_L_STTS751(void)
{
    return Get_Reg(STTS751_ADDR, STTS751_REG_TEMPVL);
}

//获取Manualfacture ID
signed char Read_STTS751_Manualfacture_ID(void)
{
    return (Get_Reg(STTS751_ADDR, STTS751_REG_MANUALID));
}

//获取产品ID
signed char Read_STTS751_Product_ID(void)
{
    return (Get_Reg(STTS751_ADDR, STTS751_REG_PRODUCT));
}

//发送数据到串口
void Send_Uart_STTS751(void)
{
    rt_kprintf("%s","<-----------STTS751--------->\r\n");

    rt_kprintf("Temp:%d.%d\r\n",Read_Temperature_H_STTS751(),Read_Temperature_L_STTS751());             //发送温度

    rt_kprintf("M_ID:%X\r\n",Read_STTS751_Manualfacture_ID());

    rt_kprintf("P_ID:%d\r\n",Read_STTS751_Product_ID());
    rt_kprintf("%s","<--------STTS751-END------->\r\n");
}

/*
 *LPS22HH
 */
//读取我是谁
uint8_t Get_WHO_AM_I_LPS22HH(void)
{
    return Get_Reg(LPS22HH_ADDR_W_1, LPS22HH_REG_WHO_AM_I);
}


//获取状态寄存器
int Get_Status_LPS22HH(void)
{
    return Get_Reg(LPS22HH_ADDR_W_1, LPS22HH_REG_STATUS);
}

//获取压力值XL
int Get_Pressure_XL_LPS22HH(void)
{
    return Get_Reg(LPS22HH_ADDR_W_1, LPS22HH_REG_PRE_OUT_XL);
}

//获取压力值L
int Get_Pressure_L_LPS22HH(void)
{
    return Get_Reg(LPS22HH_ADDR_W_1, LPS22HH_REG_PRE_OUT_L);
}

//获取压力值H
int Get_Pressure_H_LPS22HH(void)
{
    return Get_Reg(LPS22HH_ADDR_W_1, LPS22HH_REG_PRE_OUT_H);
}

int Get_Pressure(void)
{
    static float Last_return_p = 0.0f;
    int get_p;

    if((Get_Pressure_H_LPS22HH() != -1)
        &&(Get_Pressure_L_LPS22HH() != -1)
        &&(Get_Pressure_XL_LPS22HH() != -1))
    {
        get_p = (Get_Pressure_H_LPS22HH()<<16)|(Get_Pressure_L_LPS22HH()<<8)|(Get_Pressure_XL_LPS22HH());
        Last_return_p = ((float)get_p / 4096.0f);
    }
    else
    {

    }

    return (Last_return_p*10);
}

//获取温度值H
int Get_Temp_L_LPS22HH(void)
{
    return Get_Reg(LPS22HH_ADDR_W_1, LPS22HH_REG_TEMP_OUT_L);
}

//获取温度值H
int Get_Temp_H_LPS22HH(void)
{
    return Get_Reg(LPS22HH_ADDR_W_1, LPS22HH_REG_TEMP_OUT_H);
}

int Get_Temp(void)
{
    static float Last_return_t = 0.0f;
    int get_t;

    if((Get_Temp_H_LPS22HH() != -1)
        &&(Get_Temp_L_LPS22HH() != -1))
    {
        get_t = (Get_Temp_H_LPS22HH()<<8)|(Get_Temp_L_LPS22HH());
        Last_return_t = ((float) get_t / 100.0f);
    }
    else
    {

    }

    return (Last_return_t*10);
}

void Init_LPS22HH(void)
{
    if(0 == Set_Reg(LPS22HH_ADDR_W_1, LPS22HH_REG_IF_CTRL, 0x1A))
    {
        rt_kprintf("%s","PS22HH-IF_CTRL set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","PS22HH-IF_CTRL set fail\r\n");
    }

    if(0 == Set_Reg(LPS22HH_ADDR_W_1, LPS22HH_REG_CTRL_1, 0x18))
    {
        rt_kprintf("%s","PS22HH-CTRL_1 set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","PS22HH-CTRL_1 set fail\r\n");
    }
}

//发送数据到串口
void Send_Uart_LPS22HH(void)
{
    rt_kprintf("%s","<-----------LPS22HH--------->\n");
    rt_kprintf("%s","<-----------START--------->\n");
    rt_kprintf("%d.%dhPA\n", (int)(Get_Pressure()/10), (int)(Get_Pressure()%10));

    rt_kprintf("%d.%d℃\n", (int)(Get_Temp()/10), (int)(Get_Temp()%10));

    rt_kprintf("Who_am_i:%X\r\n",Get_WHO_AM_I_LPS22HH());
    rt_kprintf("%s","<------------END---------->\n");
}

/*
 * HTS221
 */

//读取我是谁
int Get_WHO_AM_I_HTS221(void)
{
    return Get_Reg(HTS221_ADDR_W_1, HTS221_REG_WHO_AM_I);
}


//获取状态寄存器
int Get_Status_HTS221(void)
{
    return Get_Reg(HTS221_ADDR_W_1, HTS221_REG_STATUS);
}

//获取压力值L
int Get_Humidity_L_HTS221(void)
{
    return Get_Reg(HTS221_ADDR_W_1, HTS221_REG_HUMI_OUT_L);
}

//获取压力值H
int Get_Humidity_H_HTS221(void)
{
    return Get_Reg(HTS221_ADDR_W_1, HTS221_REG_HUMI_OUT_H);
}

int Get_Humidity_HTS221(void)
{
    static int Last_return_p = 0;
    int get_p;

    if((Get_Humidity_H_HTS221() != -1)
        &&(Get_Humidity_L_HTS221() != -1))
    {
        get_p = ((Get_Humidity_H_HTS221()<<8)|(Get_Humidity_L_HTS221()));
        //Last_return_p = ((float)get_p / 4096.0f);
        Last_return_p = get_p;
    }
    else
    {

    }
    return Last_return_p;
}

//获取温度值H
int Get_Temp_L_HTS221(void)
{
    return Get_Reg(HTS221_ADDR_W_1, HTS221_REG_TEMP_OUT_L);
}

//获取温度值H
int Get_Temp_H_HTS221(void)
{
    return Get_Reg(HTS221_ADDR_W_1, HTS221_REG_TEMP_OUT_H);
}

int Get_Temp_HTS221(void)
{
    static int Last_return_t = 0;
    int get_t;

    if((Get_Temp_H_HTS221() != -1)
        &&(Get_Temp_L_HTS221() != -1))
    {
        get_t = ((Get_Temp_H_HTS221()<<8)|(Get_Temp_L_HTS221()));
//      Last_return_t = ((float) get_t / 100.0f);
        Last_return_t = get_t;
    }
    else
    {

    }
    return Last_return_t;
}

int Get_H0_rH_x2_HTS221(void)
{
    return Get_Reg(HTS221_ADDR_W_1, HTS221_REG_H0_rH_x2);
}
int Get_H1_rH_x2_HTS221(void)
{
    return Get_Reg(HTS221_ADDR_W_1, HTS221_REG_H1_rH_x2);
}

int Get_T0_degC_x8_HTS221(void)
{
    return ((Get_Reg(HTS221_ADDR_W_1, HTS221_REG_T1_T0)&0x03)<<8)|Get_Reg(HTS221_ADDR_W_1, HTS221_REG_T0_degC_x8);
}
int Get_T1_degC_x8_HTS221(void)
{
    return (((Get_Reg(HTS221_ADDR_W_1, HTS221_REG_T1_T0)>>2)&0x03)<<8)|Get_Reg(HTS221_ADDR_W_1, HTS221_REG_T1_degC_x8);
}

int Get_H0_T0_Output_HTS221(void)
{
    return ((Get_Reg(HTS221_ADDR_W_1, HTS221_REG_H0_T0_OUT2)<<8) | Get_Reg(HTS221_ADDR_W_1, HTS221_REG_H0_T0_OUT1));
}
int Get_H1_T0_Output_HTS221(void)
{
    return ((Get_Reg(HTS221_ADDR_W_1, HTS221_REG_H1_T0_OUT2)<<8) | Get_Reg(HTS221_ADDR_W_1, HTS221_REG_H1_T0_OUT1));
}

int Get_T0_Output_HTS221(void)
{
    return ((Get_Reg(HTS221_ADDR_W_1, HTS221_REG_T0_OUT2)<<8) | Get_Reg(HTS221_ADDR_W_1, HTS221_REG_T0_OUT1));
}
int Get_T1_Output_HTS221(void)
{
    return ((Get_Reg(HTS221_ADDR_W_1, HTS221_REG_T1_OUT2)<<8) | Get_Reg(HTS221_ADDR_W_1, HTS221_REG_T1_OUT1));
}

int HTS221_GetCalHumi(void)
{
    float k = 0.0f;
    float b = 0.0f;
    int y1 = Get_H0_rH_x2_HTS221();
    int y2 = Get_H1_rH_x2_HTS221();
    int x1 = (short)Get_H0_T0_Output_HTS221();
    int x2 = (short)Get_H1_T0_Output_HTS221();
    float Humi = 0.0f;

//    rt_kprintf("H1:%d\r\n", y1);
//    rt_kprintf("H2:%d\r\n", y2);
//    rt_kprintf("H_OUT1:%d - %d - %d\r\n", Get_Reg(HTS221_ADDR_W_1, HTS221_REG_H0_T0_OUT2), Get_Reg(HTS221_ADDR_W_1, HTS221_REG_H0_T0_OUT1), (short)x1);
//    rt_kprintf("H_OUT2:%d - %d - %d\r\n", Get_Reg(HTS221_ADDR_W_1, HTS221_REG_H1_T0_OUT2), Get_Reg(HTS221_ADDR_W_1, HTS221_REG_H1_T0_OUT1), (short)x2);
//
//    rt_kprintf("H_OUT:%d - %d - %d\r\n", Get_Humidity_H_HTS221(), Get_Humidity_L_HTS221(), Get_Humidity_HTS221());

    k = (float)(y2-y1)/(float)(x2-x1);
    b = (float)y1-k*(float)x1;

//    rt_kprintf("H_OUT2:%d - %d - y=%dx+%d\r\n", (int)k , (int)b, (int)k, (int)b);

    Humi = (((short)Get_Humidity_HTS221())*k+b)/2;
    if(Humi > 100)
        Humi = 100;
    else if(Humi < 0)
        Humi = 0;

    return (int)(Humi*10);
}

int HTS221_GetCalTemp(void)
{
    float k = 0.0f;
    float b = 0.0f;
    int y1 = Get_T0_degC_x8_HTS221();
    int y2 = Get_T1_degC_x8_HTS221();
    int x1 = (short)Get_T0_Output_HTS221();
    int x2 = (short)Get_T1_Output_HTS221();
    float tempe = 0u;

//    rt_kprintf("T1:%d\r\n", y1);
//    rt_kprintf("T2:%d\r\n", y2);
//    rt_kprintf("T_OUT1:%d - %d\r\n", Get_Reg(HTS221_ADDR_W_1, HTS221_REG_T0_OUT2), Get_Reg(HTS221_ADDR_W_1, HTS221_REG_T0_OUT1));
//    rt_kprintf("T_OUT2:%d - %d\r\n", Get_Reg(HTS221_ADDR_W_1, HTS221_REG_T1_OUT2), Get_Reg(HTS221_ADDR_W_1, HTS221_REG_T1_OUT1));
//
//    rt_kprintf("T_OUT:%d - %d\r\n", Get_Temp_H_HTS221(), Get_Temp_L_HTS221());

    k = (float)(y2-y1)/(float)(x2-x1);
    b = y1-k*x1;

    tempe = ((((short)Get_Temp_HTS221())*k)+b)/8;

    return (int)(tempe*10);
}

void Init_HTS221(void)
{
    if(0 == Set_Reg(HTS221_ADDR_W_1, HTS221_AV_CONF, 0x3Fu))  //0x09
    {
        rt_kprintf("%s","HTS221-IF_CTRL set successful\n");
    }
    else
    {
        rt_kprintf("%s","HTS221-IF_CTRL set fail\n");
    }

    if(0 == Set_Reg(HTS221_ADDR_W_1, HTS221_REG_CTRL_1, 0x82u))
    {
        rt_kprintf("%s","HTS221-CTRL_1 set successful\n");
    }
    else
    {
        rt_kprintf("%s","HTS221-CTRL_1 set fail\r\n");
    }
}

//发送数据到串口
void Send_Uart_HTS221(void)
{
    rt_kprintf("%s","<-----------HTS221--------->\n");
    rt_kprintf("Humi:%d.%d%\r\n",(int)(HTS221_GetCalHumi()/10), (int)(HTS221_GetCalHumi()%10));

    rt_kprintf("Temp:%d.%d℃\n",(int)(HTS221_GetCalTemp()/10), (int)(HTS221_GetCalTemp()%10));

    rt_kprintf("Who_am_i:%X\r\n",Get_WHO_AM_I_HTS221());
    rt_kprintf("%s","<---------HTS221-END------->\n");
}


/*
 * LIS2DW12 磁场传感器
 */
//LIS2DW12写
static int IIC_WrDat_LIS2DW12(uint8_t IIC_Add, uint8_t IIC_Reg, uint8_t IIC_Data)
{
    uint8_t ret_suc = 1;

    IIC_Start();
    IIC_Send_Byte(IIC_Add & 0xfe);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Reg);         //write data
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Data);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Stop();

    return 0;
}


//LIS2DW12读
static int IIC_Read_LIS2DW12(uint8_t IIC_Add, uint8_t IIC_Reg)
{
    int ret;
    uint8_t ret_suc = 1;

    IIC_Start();
    IIC_Send_Byte(IIC_Add & 0xfe);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Reg);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Start();
    IIC_Send_Byte(IIC_Add | 0x01);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    ret=IIC_Read_Byte(1);
    IIC_Stop();

    return ret;
}

//LIS2DW12读2个寄存器
static int IIC_Read2_LIS2DW12(uint8_t IIC_Add, uint8_t IIC_Reg)
{
    int ret;
    uint8_t ret_suc = 1;

    IIC_Start();
    IIC_Send_Byte(IIC_Add & 0xfe);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Reg);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Start();
    IIC_Send_Byte(IIC_Add | 0x01);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    ret=IIC_Read_Byte(1);
    ret=(IIC_Read_Byte(0)<<8)|ret;

    IIC_Stop();

    return ret;
}

//设置寄存器
int Set_Reg_LIS2DW12(uint8_t add, uint8_t reg, uint8_t dat)
{
    int ret_succ = -1;
    uint8_t i = 20u;        //失败之后尝试的次数

    do{
        if(0 == IIC_WrDat_LIS2DW12(add, reg, dat))
        {
            ret_succ = 0;       //设置成功
            break;
        }
        else
        {
            i--;                //获取失败，再次尝试
        }
    }while(i>0u);

    return ret_succ;
}

//读取寄存器
int Get_Reg_LIS2DW12(uint8_t add, uint8_t reg)
{
    uint8_t i = 20u;
    int ret_get = -1;

    do{
        ret_get = IIC_Read_LIS2DW12(add, reg);
        if(-1 != ret_get)
        {
            break;              //获取数据成功
        }
        else
        {
            i--;                //获取失败，再次尝试
        }
    }while(i>0u);

    return ret_get;
}

//读取我是谁
int Get_WHO_AM_I_LIS2DW12(void)
{
    return Get_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_REG_WHO_AM_I);
}


//获取状态寄存器
int Get_Status_LIS2DW12(void)
{
    return Get_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_CFG_STATUS);
}

float Get_Temp_1_LIS2DW12(void)
{
    short temp_reg = (Get_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_OUT_T_H)<<8)|Get_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_OUT_T_L);
    float temp_cal = 0.0f;

    temp_cal = 25.0+temp_reg/16.0/16.0;

    return temp_cal;
}

signed char Get_Temp_2_LIS2DW12(void)
{
    return (25+(Get_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_OUT_T)));
}

short Get_X_LIS2DW12(void)
{
    return ((Get_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_OUT_X_H)<<8)|Get_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_OUT_X_L));
}

short Get_Y_LIS2DW12(void)
{
    return ((Get_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_OUT_Y_H)<<8)|Get_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_OUT_Y_L));
}

short Get_Z_LIS2DW12(void)
{
    return ((Get_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_OUT_Z_H)<<8)|Get_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_OUT_Z_L));
}

void Init_LIS2DW12(void)
{
    if(0 == Set_Reg_LIS2DW12(LIS2DW12_ADDR_W_1, LIS2DW12_CTRL1, 0x16))
    {
        rt_kprintf("%s","LIS2DW12-IF_CTRL set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","LIS2DW12-IF_CTRL set fail\r\n");
    }
}


//发送数据到串口
void Send_Uart_LIS2DW12(void)
{
//    rt_kprintf("%s","<-----------LIS2DW12--------->\r\n");
    rt_kprintf("X:%05d | Y:%05d | Z:%05d\r\n", Get_X_LIS2DW12(), Get_Y_LIS2DW12(), Get_Z_LIS2DW12());
//
//    rt_kprintf("Temp:%d℃ | %d℃\r\n", Get_Temp_1_LIS2DW12(), Get_Temp_2_LIS2DW12());
//
//    rt_kprintf("Status:0x%X\r\n", Get_Status_LIS2DW12());
//
//    rt_kprintf("Who_am_i:%X\r\n", Get_WHO_AM_I_LIS2DW12());
//    rt_kprintf("%s","<--------LIS2DW12-END------->\r\n\n");
}



/*
 * LSM6DSO 六轴传感器
 */
//LSM6DSO写
static int IIC_WrDat_LSM6DSO(uint8_t IIC_Add, uint8_t IIC_Reg, uint8_t IIC_Data)
{
    uint8_t ret_suc = 1;

    IIC_Start();
    IIC_Send_Byte(IIC_Add & 0xfe);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Reg);         //write data
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Data);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Stop();

    return 0;
}


//LSM6DSO读
static int IIC_Read_LSM6DSO(uint8_t IIC_Add, uint8_t IIC_Reg)
{
    int ret;
    uint8_t ret_suc = 1;

    IIC_Start();
    IIC_Send_Byte(IIC_Add & 0xfe);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Reg);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Start();
    IIC_Send_Byte(IIC_Add | 0x01);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    ret=IIC_Read_Byte(1);
    IIC_Stop();

    return ret;
}

//LSM6DSO读2个寄存器
static int IIC_Read2_LSM6DSO(uint8_t IIC_Add, uint8_t IIC_Reg)
{
    int ret;
    uint8_t ret_suc = 1;

    IIC_Start();
    IIC_Send_Byte(IIC_Add & 0xfe);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Reg);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Start();
    IIC_Send_Byte(IIC_Add | 0x01);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    ret=IIC_Read_Byte(1);
    ret=(IIC_Read_Byte(0)<<8)|ret;

    IIC_Stop();

    return ret;
}

//设置寄存器
int Set_Reg_LSM6DSO(uint8_t add, uint8_t reg, uint8_t dat)
{
    int ret_succ = -1;
 //   uint8_t i = 20u;        //失败之后尝试的次数

//    do{
        if(0 == IIC_WrDat_LSM6DSO(add, reg, dat))
        {
            ret_succ = 0;       //设置成功
//            break;
        }
        else
        {
//            i--;                //获取失败，再次尝试
        }
//    }while(i>0u);

    return ret_succ;
}

//读取寄存器
int Get_Reg_LSM6DSO(uint8_t add, uint8_t reg)
{
//    uint8_t i = 20u;
    int ret_get = -1;

//    do{
        ret_get = IIC_Read(add, reg);
//        if(-1 != ret_get)
//        {
//            break;              //获取数据成功
//        }
//        else
//        {
//            i--;                //获取失败，再次尝试
//        }
//    }while(i>0u);

    return ret_get;
}

//读取我是谁
int Get_WHO_AM_I_LSM6DSO(void)
{
    return Get_Reg_LSM6DSO(LSM6DSO_ADDR_W_1, LSM6DSO_WHO_AM_I);
}

//获取状态寄存器
int Get_Status_LSM6DSO(void)
{
    return Get_Reg_LSM6DSO(LSM6DSO_ADDR_W_1, LSM6DSO_CFG_STATUS);
}

float Get_Temp_LSM6DSO(void)
{
    return (Get_Reg_LSM6DSO(LSM6DSO_ADDR_W_1, LSM6DSO_TEMP_H)<<8)|Get_Reg_LSM6DSO(LSM6DSO_ADDR_W_1, LSM6DSO_TEMP_L);
}

short Get_X_G_LSM6DSO(void)
{
    return ((Get_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_OUT_X_H_G)<<8)|Get_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_OUT_X_L_G));
}

short Get_Y_G_LSM6DSO(void)
{
    return ((Get_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_OUT_Y_H_G)<<8)|Get_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_OUT_Y_L_G));
}

short Get_Z_G_LSM6DSO(void)
{
    return ((Get_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_OUT_Z_H_G)<<8)|Get_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_OUT_Z_L_G));
}

short Get_X_A_LSM6DSO(void)
{
    return ((Get_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_OUT_X_H_A)<<8)|Get_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_OUT_X_L_A));
}

short Get_Y_A_LSM6DSO(void)
{
    return ((Get_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_OUT_Y_H_A)<<8)|Get_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_OUT_Y_L_A));
}

short Get_Z_A_LSM6DSO(void)
{
    return ((Get_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_OUT_Z_H_A)<<8)|Get_Reg(LSM6DSO_ADDR_W_1, LSM6DSO_OUT_Z_L_A));
}

void Init_LSM6DSO(void)
{
    if(0 == Set_Reg_LSM6DSO(LSM6DSO_ADDR_W_1, LSM6DSO_CTRL1_XL, 0x10))
    {
        rt_kprintf("%s","LSM6DSO-IF_CTRL set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","LSM6DSO-IF_CTRL set fail\r\n");
    }
    if(0 == Set_Reg_LSM6DSO(LSM6DSO_ADDR_W_1, LSM6DSO_CTRL2_G, 0x12))
    {
        rt_kprintf("%s","LSM6DSO-IF_CTRL set successful\r\n");
    }
    else
    {
        rt_kprintf("%s","LSM6DSO-IF_CTRL set fail\r\n");
    }
}


//发送数据到串口
void Send_Uart_LSM6DSO(void)
{
    rt_kprintf("%s","<-----------LSM6DSO--------->\r\n");
    rt_kprintf("G_X:%d | G_Y:%d | G_Z:%d\r\n", Get_X_G_LSM6DSO(),Get_Y_G_LSM6DSO(),Get_Z_G_LSM6DSO());

    rt_kprintf("A_X:%d | A_Y:%d | A_Z:%d\r\n", Get_X_A_LSM6DSO(),Get_Y_A_LSM6DSO(),Get_Z_A_LSM6DSO());

//    rt_kprintf("Temp:%.2f\r\n", Get_Temp_LSM6DSO());

    rt_kprintf("Who_am_i:%X\r\n",Get_WHO_AM_I_LSM6DSO());
    rt_kprintf("%s","<--------LSM6DSO-END-------->\r\n\n");
}

//LIS2MDL写
static int IIC_WrDat_LIS2MDL(uint8_t IIC_Add, uint8_t IIC_Reg, uint8_t IIC_Data)
{
    uint8_t ret_suc = 1;

    IIC_Start();
    IIC_Send_Byte(IIC_Add & 0xfe);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Reg);         //write data
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Data);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Stop();

    return 0;
}


//LIS2MDL读
static int IIC_Read_LIS2MDL(uint8_t IIC_Add, uint8_t IIC_Reg)
{
    int ret;
    uint8_t ret_suc = 1;

    IIC_Start();
    IIC_Send_Byte(IIC_Add & 0xfe);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Reg);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Start();
    IIC_Send_Byte(IIC_Add | 0x01);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    ret=IIC_Read_Byte(1);
    IIC_Stop();

    return ret;
}

//LIS2MDL读2个寄存器
static int IIC_Read2_LIS2MDL(uint8_t IIC_Add, uint8_t IIC_Reg)
{
    int ret;
    uint8_t ret_suc = 1;

    IIC_Start();
    IIC_Send_Byte(IIC_Add & 0xfe);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Send_Byte(IIC_Reg);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    IIC_Start();
    IIC_Send_Byte(IIC_Add | 0x01);
    ret_suc = IIC_Wait_Ack();
    if(ret_suc == 1)    {IIC_Stop();return -1;}

    ret=IIC_Read_Byte(1);
    ret=(IIC_Read_Byte(0)<<8)|ret;

    IIC_Stop();

    return ret;
}

/*LIS2MDL 磁场传感器*/
//设置寄存器
int Set_Reg_LIS2MDL(uint8_t add, uint8_t reg, uint8_t dat)
{
    int ret_succ = -1;
    uint8_t i = 20u;        //失败之后尝试的次数

    do{
        if(0 == IIC_WrDat_LIS2MDL(add, reg, dat))
        {
            ret_succ = 0;       //设置成功
            break;
        }
        else
        {
            i--;                //获取失败，再次尝试
        }
    }while(i>0u);

    return ret_succ;
}

//读取寄存器
int Get_Reg_LIS2MDL(uint8_t add, uint8_t reg)
{
    uint8_t i = 20u;
    int ret_get = -1;

    do{
        ret_get = IIC_Read_LIS2MDL(add, reg);
        if(-1 != ret_get)
        {
            break;              //获取数据成功
        }
        else
        {
            i--;                //获取失败，再次尝试
        }
    }while(i>0u);

    return ret_get;
}

//读取我是谁
int Get_WHO_AM_I_LIS2MDL(void)
{
    return Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_REG_WHO_AM_I);
}


//获取状态寄存器
int Get_Status_LIS2MDL(void)
{
    return Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_CFG_STATUS);
}

short GET_X_LIS2MDL(void)
{
    return (Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_OUTX_H)<<8)|(Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_OUTX_L));
}

short GET_Y_LIS2MDL(void)
{
    return (Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_OUTY_H)<<8)|(Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_OUTY_L));
}

short GET_Z_LIS2MDL(void)
{
    return (Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_OUTZ_H)<<8)|(Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_OUTZ_L));
}

short GET_Offset_X_LIS2MDL(void)
{
    return (Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_OFFSET_X_REG_H)<<8)|(Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_OFFSET_X_REG_L));
}

short GET_Offset_Y_LIS2MDL(void)
{
    return (Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_OFFSET_Y_REG_H)<<8)|(Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_OFFSET_Y_REG_L));
}

short GET_Offset_Z_LIS2MDL(void)
{
    return (Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_OFFSET_Z_REG_H)<<8)|(Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_OFFSET_Z_REG_L));
}


int Get_Temp_LIS2MDL(void)
{

    float read_temp = 0.0f;

    read_temp = (float)((short)((Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_TEMP_H)<<8)|(Get_Reg(LIS2MDL_ADDR_W_1, LIS2MDL_TEMP_L))));
    read_temp /= 8.0;
    read_temp += 25;

    return (read_temp*10);
}

void Init_LIS2MDL(void)
{
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
}

//发送数据到串口
void Send_Uart_LIS2MDL(void)
{

    rt_thread_delay(10);
    rt_kprintf("%s","<-----------LIS2MDL--------->\r\n");
    rt_kprintf("Offset_X:%d | Offset_Y:%d | Offset_Z:%d\r\n",GET_Offset_X_LIS2MDL(),GET_Offset_Y_LIS2MDL(),GET_Offset_Z_LIS2MDL());

    rt_kprintf("X:%d | Y:%d | Z:%d\r\n",GET_X_LIS2MDL(),GET_Y_LIS2MDL(),GET_Z_LIS2MDL());

    rt_kprintf("Temp:%d.%d\r\n", (Get_Temp_LIS2MDL()/10), ((Get_Temp_LIS2MDL()<0?-(Get_Temp_LIS2MDL()):Get_Temp_LIS2MDL())%10));

    rt_kprintf("Status:0x%X\r\n", Get_Status_LIS2MDL());

    rt_kprintf("Who_am_i:%X\r\n",Get_WHO_AM_I_LIS2MDL());
    rt_kprintf("%s","<---------LIS2MDL-END-------->\r\n\n");
}


//void app_myiic_opt(void)
//{
//    rt_enter_critical();
//    IIC_Init();
//    rt_thread_delay(10);
//    Init_LPS22HH();
//    Init_HTS221();
// //   rt_thread_delay(10);
//    Init_LIS2DW12();
////    rt_thread_delay(10);
////    Init_LSM6DSO();
////    rt_thread_delay(10);
//    Init_LIS2MDL();
//
////    Send_Uart_LPS22HH();
////    Send_Uart_HTS221();
////    Send_Uart_LIS2DW12();
////    Send_Uart_LSM6DSO();
//    Send_Uart_LIS2MDL();
//    rt_exit_critical();
//}
//
///* 导出到 msh 命令列表中 */
//MSH_CMD_EXPORT(app_myiic_opt, i2c STTS751 temperature sample);

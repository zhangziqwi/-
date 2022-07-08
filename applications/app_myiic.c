/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-15     hehung       the first version
 */

#include "app_myiic.h"
#include <rtthread.h>
#include <rtdevice.h>
#include <stdint.h>
#include <rthw.h>
#include <board.h>
#include "drv_config.h"


#define SET_IIC_SCL() rt_pin_write(BSP_I2C1_SCL_PIN, PIN_HIGH)
#define CLR_IIC_SCL() rt_pin_write(BSP_I2C1_SCL_PIN, PIN_LOW)

#define SET_IIC_SDA() rt_pin_write(BSP_I2C1_SDA_PIN, PIN_HIGH)
#define CLR_IIC_SDA() rt_pin_write(BSP_I2C1_SDA_PIN, PIN_LOW)

#define READ_SDA      rt_pin_read(BSP_I2C1_SDA_PIN)


#define SDA_IN()      rt_pin_mode(BSP_I2C1_SDA_PIN, PIN_MODE_INPUT)
#define SDA_OUT()     rt_pin_mode(BSP_I2C1_SDA_PIN, PIN_MODE_OUTPUT)

static void SysTick_delay_us(uint32_t us);


void IIC_Init(void)
{
    rt_pin_mode(BSP_I2C1_SCL_PIN, PIN_MODE_OUTPUT);
    rt_pin_mode(BSP_I2C1_SDA_PIN, PIN_MODE_OUTPUT);

//    rt_pin_write(BSP_I2C1_SCL_PIN, PIN_HIGH);
//    rt_pin_write(BSP_I2C1_SDA_PIN, PIN_HIGH);
    SET_IIC_SCL();//拉高电平
    SET_IIC_SDA();
}

void IIC_Start(void)
{
	SDA_OUT();
	SET_IIC_SDA();
	SET_IIC_SCL();
	SysTick_delay_us(4);
 	CLR_IIC_SDA();
	SysTick_delay_us(4);
	CLR_IIC_SCL();
}

void IIC_Stop(void)
{
	SDA_OUT();
	CLR_IIC_SCL();
	CLR_IIC_SDA();
 	SysTick_delay_us(4);
	SET_IIC_SCL();
	SET_IIC_SDA();
	SysTick_delay_us(4);
}

uint8_t IIC_Wait_Ack(void)
{
	uint16_t ucErrTime=0;

	SET_IIC_SDA();SysTick_delay_us(1);
	SET_IIC_SCL();SysTick_delay_us(1);
	SDA_IN();
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC_Stop();
			return 1;
		}
	}
	CLR_IIC_SCL();
	SDA_OUT();
	return 0;
}

void IIC_Ack(void)
{
	SDA_OUT();
	CLR_IIC_SCL();
//	SysTick_delay_us(4);
//	SDA_OUT();
	CLR_IIC_SDA();
	SysTick_delay_us(2);
	SET_IIC_SCL();
	SysTick_delay_us(2);
	CLR_IIC_SCL();
}
//涓嶄骇鐢烝CK搴旂瓟
void IIC_NAck(void)
{
	SDA_OUT();
	CLR_IIC_SCL();
//	SysTick_delay_us(4);
	SET_IIC_SDA();
	SysTick_delay_us(2);
	SET_IIC_SCL();
	SysTick_delay_us(2);
	CLR_IIC_SCL();
//	SysTick_delay_us(4);
}

void IIC_Send_Byte(uint8_t txd)
{
    uint8_t t;
	SDA_OUT();
    CLR_IIC_SCL();
//	SysTick_delay_us(4);
    for(t=1;t<=8;t++)
    {
//        IIC_SDA=(txd&0x80)>>7;
		if((txd&0x80)>>7)
			SET_IIC_SDA();
		else
			CLR_IIC_SDA();
        txd<<=1;
		SysTick_delay_us(2);
		SET_IIC_SCL();
		SysTick_delay_us(2);
		CLR_IIC_SCL();
		SysTick_delay_us(2);
    }
}

uint8_t IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0x00;
//	CLR_IIC_SCL();
//	SysTick_delay_us(4);
//	SET_IIC_SDA();
	SDA_IN();
    for(i=1;i<=8;i++ )
	{
        CLR_IIC_SCL();
        SysTick_delay_us(2);
		SET_IIC_SCL();
        receive<<=1;
        if(READ_SDA)
			receive ++;
//		CLR_IIC_SCL();
		SysTick_delay_us(1);
    }
    if (!ack)
        IIC_NAck();
    else
        IIC_Ack();
	SDA_OUT();
    return receive;
}

/**
 * The time delay function.
 *
 * @param microseconds.
 */

static void SysTick_delay_us(uint32_t us)
{
    uint32_t ticks;
    uint32_t told, tnow, tcnt = 0;
    uint32_t reload = SysTick->LOAD;

    ticks = us * reload / (1000000 / RT_TICK_PER_SECOND);
    told = SysTick->VAL;
    while(1)
    {
        tnow = SysTick->VAL;
        if (tnow != told)
        {
            if (tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                tcnt += reload - tnow + told;
            }
            told = tnow;
            if (tcnt >= ticks)
            {
                break;
            }
        }
    }
}

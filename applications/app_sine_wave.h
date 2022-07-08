/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2022-04-20     Zhang_Ziqi       the first version
 */
#ifndef APPLICATIONS_APP_SINE_WAVE_H_
#define APPLICATIONS_SINE_WAVE_H_
void my_DA_conventer_init (void);

void my_AD_conventer_init (void);
unsigned char my_AD_conventer (void);
void my_DA_conventer (unsigned char);
void sine_wave_ThreadEntry(void* parameter);
uint8_t DACconversion(unsigned char sla,unsigned char c,  unsigned char Val);

#endif /* APPLICATIONS_APP_SINE_WAVE_H_ */

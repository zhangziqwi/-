/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-13     hehung       the first version
 */
#ifndef APPLICATIONS_APP_COMPASS_H_
#define APPLICATIONS_APP_COMPASS_H_

extern short compass_out_x;
extern short compass_out_y;
extern short compass_out_z;


void Compass_ThreadEntry(void *parameter);
void Compass_GetAngle(void);
short Compass_GetAzimut(void);


#endif /* APPLICATIONS_APP_COMPASS_H_ */

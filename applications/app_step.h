/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-12-13     hehung       the first version
 */
#ifndef APPLICATIONS_APP_STEP_H_
#define APPLICATIONS_APP_STEP_H_


short LSM6DSO_GetAccX(void);
short LSM6DSO_GetAccY(void);
short LSM6DSO_GetAccZ(void);
uint16_t Step_GetStep(void);
void Step_ClearStep(void);
void Pedometer_ThreadEntry(void *parameter);



#endif /* APPLICATIONS_APP_STEP_H_ */

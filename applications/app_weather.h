/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-15     hehung       the first version
 */
#ifndef __APP_WEATHER_H__
#define __APP_WEATHER_H__

struct Weather_w
{
    char *city;
    char *weather;
};

#define GET_HEADER_BUFSZ        1024        //ͷ����С
#define GET_RESP_BUFSZ          1024        //��Ӧ��������С
#define GET_URL_LEN_MAX         256         //��ַ��󳤶�
#define GET_URI                 "http://www.weather.com.cn/data/cityinfo/%s.html" //��ȡ������ API
#define AREA_ID                 "101040100" //������� ID

void Weather_GetWeather(char *api_city_code);
char* Weather_GetWeatherCityInfo(void);
char* Weather_GetWeatherInfo(void);

#endif

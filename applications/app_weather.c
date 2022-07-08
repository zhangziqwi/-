/*
 * Copyright (c) 2006-2020, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2020-11-15     hehung       the first version
 */

#include "app_weather.h"
#include <webclient.h>  /* ʹ�� HTTP Э���������ͨ����Ҫ������ͷ�ļ� */
#include <sys/socket.h> /* ʹ��BSD socket����Ҫ����socket.hͷ�ļ� */
#include <netdb.h>
#include <cJSON.h>
#include <finsh.h>

struct Weather_w *app_w = NULL;
char Weather_City[20];
char Weather_weather[20] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',
                            ' ',' ',' ',' ',' ',' ',' ',' ',' ',' '};

static void Weather_DataParse(rt_uint8_t *data);

char* Weather_GetWeatherCityInfo(void)
{
    return Weather_City;
}
char* Weather_GetWeatherInfo(void)
{
    return Weather_weather;
}

/* �������ݽ��� */
static void Weather_DataParse(rt_uint8_t *data)
{
    cJSON *root = RT_NULL, *object = RT_NULL, *item = RT_NULL;

    root = cJSON_Parse((const char *)data);
    if (!root)
    {
        rt_kprintf("No memory for cJSON root!\n");
        return;
    }
    object = cJSON_GetObjectItem(root, "weatherinfo");

    item = cJSON_GetObjectItem(object, "city");
    strcpy(Weather_City, item->valuestring);
//    app_w->city = item->valuestring;
//    rt_kprintf("\ncityName:%s ", app_w->city);

//    item = cJSON_GetObjectItem(object, "temp1");
//    rt_kprintf("\ntemp    :%s ", item->valuestring);

    item = cJSON_GetObjectItem(object, "weather");
    strcpy(Weather_weather, item->valuestring);
//    app_w->weather = item->valuestring;
//    rt_kprintf("\nwd      :%s ", app_w->weather);

    if (root != RT_NULL)
        cJSON_Delete(root);
}

void Weather_GetWeather(char *api_city_code)
{
    rt_uint8_t *buffer = RT_NULL;
    int resp_status;
    struct webclient_session *session = RT_NULL;
    char *weather_url = RT_NULL;
    int content_length = -1, bytes_read = 0;
    int content_pos = 0;

    /* Ϊ weather_url ����ռ� */
    weather_url = rt_calloc(1, GET_URL_LEN_MAX);
    if (weather_url == RT_NULL)
    {
        rt_kprintf("No memory for weather_url!\n");
        goto __exit;
    }
    /* ƴ�� GET ��ַ */
    rt_snprintf(weather_url, GET_URL_LEN_MAX, GET_URI, api_city_code);
//    sprintf(weather_url, "%s%s", GET_URI, AREA_ID);
//    rt_kprintf("weather_url=%s\n", weather_url);
    /* �����Ự����������Ӧ�Ĵ�С */
    session = webclient_session_create(GET_HEADER_BUFSZ);
//    rt_kprintf("session = %s\n", session);
    if (session == RT_NULL)
    {
        rt_kprintf("No memory for get header!\n");
        goto __exit;
    }

    /* ���� GET ����ʹ��Ĭ�ϵ�ͷ�� */
    if ((resp_status = webclient_get(session, weather_url)) != 200)
    {
        rt_kprintf("webclient GET request failed, response(%d) error.\n", resp_status);
        goto __exit;
    }

    /* �������ڴ�Ž������ݵĻ��� */
    buffer = rt_calloc(1, GET_RESP_BUFSZ);
    if (buffer == RT_NULL)
    {
        rt_kprintf("No memory for data receive buffer!\n");
        goto __exit;
    }

    content_length = webclient_content_length_get(session);
//    rt_kprintf("content_length = %d\n", content_length);
    if (content_length < 0)
    {
        /* ���ص������Ƿֿ鴫���. */
        do
        {
            bytes_read = webclient_read(session, buffer, GET_RESP_BUFSZ);
            if (bytes_read <= 0)
            {
                break;
            }
        }while (1);
 //       rt_kprintf("bytes_read1 = %s\n", bytes_read);
    }
    else
    {
        do
        {
            bytes_read = webclient_read(session, buffer,
                                        content_length - content_pos > GET_RESP_BUFSZ ?
                                        GET_RESP_BUFSZ : content_length - content_pos);
            if (bytes_read <= 0)
            {
                break;
            }
            content_pos += bytes_read;
        }while (content_pos < content_length);
 //       rt_kprintf("bytes_read2 = %s\n", bytes_read);
    }
//    rt_kprintf("buffer = %s\n", buffer);
    /* �������ݽ��� */
    Weather_DataParse(buffer);

__exit:
    /* �ͷ���ַ�ռ� */
    if (weather_url != RT_NULL)
        rt_free(weather_url);
    /* �رջỰ */
    if (session != RT_NULL)
        webclient_close(session);
    /* �ͷŻ������ռ� */
    if (buffer != RT_NULL)
        rt_free(buffer);
}

static void Weather_DataParse2(rt_uint8_t *data)
{
    cJSON *root = RT_NULL, *object = RT_NULL, *item = RT_NULL;

    root = cJSON_Parse((const char *)data);
    if (!root)
    {
        rt_kprintf("No memory for cJSON root!\n");
        return;
    }
    object = cJSON_GetObjectItem(root, "weatherinfo");

    item = cJSON_GetObjectItem(object, "city");
    app_w->city = item->valuestring;
    rt_kprintf("\ncityName:%s ", app_w->city);
    strcpy(Weather_City, item->valuestring);
    rt_kprintf("\ncityName:%s ", Weather_City);
//    item = cJSON_GetObjectItem(object, "temp1");
//    rt_kprintf("\ntemp    :%s ", item->valuestring);

    item = cJSON_GetObjectItem(object, "weather");
    app_w->weather = item->valuestring;
    rt_kprintf("\nwd      :%s ", app_w->weather);
    strcpy(Weather_weather, item->valuestring);
    rt_kprintf("\ncityName:%s ", Weather_weather);

    if (root != RT_NULL)
        cJSON_Delete(root);
}

void Weather_Weather(int argc, char **argv)
{
    rt_uint8_t *buffer = RT_NULL;
    int resp_status;
    struct webclient_session *session = RT_NULL;
    char *weather_url = RT_NULL;
    int content_length = -1, bytes_read = 0;
    int content_pos = 0;

    /* Ϊ weather_url ����ռ� */
    weather_url = rt_calloc(1, GET_URL_LEN_MAX);
    if (weather_url == RT_NULL)
    {
        rt_kprintf("No memory for weather_url!\n");
        goto __exit;
    }
    /* ƴ�� GET ��ַ */
    rt_snprintf(weather_url, GET_URL_LEN_MAX, GET_URI, AREA_ID);
//    sprintf(weather_url, "%s%s", GET_URI, AREA_ID);
//    rt_kprintf("weather_url=%s\n", weather_url);
    /* �����Ự����������Ӧ�Ĵ�С */
    session = webclient_session_create(GET_HEADER_BUFSZ);
//    rt_kprintf("session = %s\n", session);
    if (session == RT_NULL)
    {
        rt_kprintf("No memory for get header!\n");
        goto __exit;
    }

    /* ���� GET ����ʹ��Ĭ�ϵ�ͷ�� */
    if ((resp_status = webclient_get(session, weather_url)) != 200)
    {
        rt_kprintf("webclient GET request failed, response(%d) error.\n", resp_status);
        goto __exit;
    }

    /* �������ڴ�Ž������ݵĻ��� */
    buffer = rt_calloc(1, GET_RESP_BUFSZ);
    if (buffer == RT_NULL)
    {
        rt_kprintf("No memory for data receive buffer!\n");
        goto __exit;
    }

    content_length = webclient_content_length_get(session);
//    rt_kprintf("content_length = %d\n", content_length);
    if (content_length < 0)
    {
        /* ���ص������Ƿֿ鴫���. */
        do
        {
            bytes_read = webclient_read(session, buffer, GET_RESP_BUFSZ);
            if (bytes_read <= 0)
            {
                break;
            }
        }while (1);
 //       rt_kprintf("bytes_read1 = %s\n", bytes_read);
    }
    else
    {
        do
        {
            bytes_read = webclient_read(session, buffer,
                                        content_length - content_pos > GET_RESP_BUFSZ ?
                                        GET_RESP_BUFSZ : content_length - content_pos);
            if (bytes_read <= 0)
            {
                break;
            }
            content_pos += bytes_read;
        }while (content_pos < content_length);
 //       rt_kprintf("bytes_read2 = %s\n", bytes_read);
    }
//    rt_kprintf("buffer = %s\n", buffer);
    /* �������ݽ��� */
    Weather_DataParse2(buffer);

__exit:
    /* �ͷ���ַ�ռ� */
    if (weather_url != RT_NULL)
        rt_free(weather_url);
    /* �رջỰ */
    if (session != RT_NULL)
        webclient_close(session);
    /* �ͷŻ������ռ� */
    if (buffer != RT_NULL)
        rt_free(buffer);
}

MSH_CMD_EXPORT(Weather_Weather, Get weather by webclient);


/*
 * ����:101010100����:101010300˳��:101010400����:101010500ͨ��:101010600��ƽ:101010700����:101010800��̨:101010900
 * ʯ��ɽ:101011000����:101011100��ɽ:101011200����:101011300��ͷ��:101011400ƽ��:101011500�˴���:101011600��ү��:101011700
 * ���ӿ�:101011800�����ϵ���:101011900ի��:101012000ϼ����:101012100��������:101012200����:101010200���:101030100����:101030300
 * ����:101030400����:101030500����:101030600����:101031400����:101030800����:101030900����:101031000����:101031100���:101031200
 * ����:101030200����:101030700�Ϻ�:101020100��ɽ:101020300�ζ�:101020500�ϻ�:101020600�ֶ�:101021300����:101020800�ɽ�:101020900
 * ����:101021000����:101021100��һ�:101021200����:101020200��ɽ:101020700ʯ��ׯ:101090101�żҿ�:101090301�е�:101090402��ɽ:101090501
 * �ػʵ�:101091101����:101090701��ˮ:101090801��̨:101090901����:101091001����:101090201�ȷ�:101090601֣��:101180101����:101180301
 * ���:101180401ƽ��ɽ:101180501����:101180601����:101180701����:101180801����:101180901����:101181001����:101181101�ױ�:101181201
 * ���:101181301�ܿ�:101181401���:101181501פ���:101181601����Ͽ:101181701��Դ:101181801����:101180201�Ϸ�:101220101�ߺ�:101220301
 * ����:101220401��ɽ:101220501����:101220601����:101220701����:101220801����:101220901��ɽ:101221001����:101221101����:101221201
 * ͭ��:101221301����:101221401����:101221501����:101221601����:101221701����:101220201����:101210101��ɽ:101211101����:101210201
 * ����:101210301��:101210901����:101210501̨��:101210601����:101210701��ˮ:101210801����:101211001����:101210401����:101040100
 * �ϴ�:101040300�ϴ�:101040400����:101040500��ʢ:101040600�山:101040700����:101040800����:101040900
 * ����:101041000ǭ��:101041100�������:101041200��������:101041300����:101041400����:101041500�ǿ�:101041600
 * ����:101041700��Ϫ:101041800���:101041900��ɽ:101042000����:101042100�潭:101042200��ƽ:101042300
 * ����:101042400ʯ��:101042500����:101042600�ٲ�:101042700ͭ��:101042800�ɽ:101042900�ᶼ:101043000
 * ��¡:101043100��ˮ:101043200�뽭:101043300����:101043400��ɽ:101043600ɳƺ��:101043700����:101040200
 * ����:101230101Ȫ��:101230501����:101230601����:101230701����:101230509��ƽ:101230901����:101230201
 * ����:101230301����:101230401����:101230801����:101160101ƽ��:101160301����:101160401����:101160501
 * ���:101160601������:101161401��Ȫ:101160801��ˮ:101160901�䶼:101161001����:101161101����:101161201
 * ����:101161301����:101160201��Ҵ:101160701����:101280101����:101280301÷��:101280401��ͷ:101280501
 * ����:101280601�麣:101280701��ɽ:101280800����:101280901տ��:101281001����:101281101��Դ:101281201
 * ��Զ:101281301�Ƹ�:101281401����:101281501��ݸ:101281601��ɽ:101281701����:101281801����:101281901
 * ï��:101282001��β:101282101�ع�:101280201����:101300101����:101300301����:101300401����:101300501
 * ����:101300601���Ǹ�:101301401���:101300801����:101300901��ɫ:101301001����:101301101�ӳ�:101301201
 * ����:101301301����:101300201����:101300701����:101260101��˳:101260301����:101260401����:101260906
 * ͭ��:101260601�Ͻ�:101260701����ˮ:101260801����:101260201����:101260501����:101290101���:101290301
 * ��ɽ:101290601��Ϫ:101290701����:101290801�ն�:101290901��ͨ:101291001�ٲ�:101291101ŭ��:101291201
 * �������:101291301����:101291401�º�:101291501����:101291601����:101290201����:101290401��ɽ:101290501
 * ���ͺ���:101080101�ں�:101080301����:101080401ͨ��:101080501����������:101081201������˹:101080701
 * �ٺ�:101080801���ֺ���:101080901���ױ���:101081000��������:101081101��ͷ:101080201���:101080601�ϲ�:101240101
 * ����:101240301����:101240401�˴�:101240501ӥ̶:101241101����:101240701������:101240801Ƽ��:101240901
 * ����:101241001�Ž�:101240201����:101240601�人:101200101�Ƹ�:101200501����:101200801�˲�:101200901
 * ��ʩ:101201001ʮ��:101201101��ũ��:101201201����:101201301����:101201401����:101201501����:101201601
 * Ǳ��:101201701�差:101200201����:101200301Т��:101200401��ʯ:101200601����:101200701�ɶ�:101270101
 * �Թ�:101270301����:101270401�ϳ�:101270501����:101270601����:101270701�㰲:101270801����:101270901
 * ����:101271001�˱�:101271101�ڽ�:101271201����:101271301��ɽ:101271401üɽ:101271501��ɽ:101271601
 * �Ű�:101271701����:101271801����:101271901����:101272001��Ԫ:101272101��֦��:101270201����:101170101
 * ����:101170501��ԭ:101170401ʯ��ɽ:101170201����:101170301����:101150101����:101150301����:101150801
 * ����:101150501����:101150601����:101150701����:101150201����:101150401����:101120101Ϋ��:101120601
 * ����:101120901����:101121001����:101121101��Ӫ:101121201����:101121301��ׯ:101121401����:101121501
 * ����:101121601�ĳ�:101121701�ൺ:101120201�Ͳ�:101120301����:101120401��̨:101120501����:101120701
 * ̩��:101120801����:101110101�Ӱ�:101110300����:101110401ͭ��:101111001����:101110601����:101110701
 * ����:101110801����:101110901����:101110200μ��:101110501̫ԭ:101100101�ٷ�:101100701�˳�:101100801
 * ˷��:101100901����:101101001����:101100501��ͬ:101100201��Ȫ:101100301����:101100401����:101100601
 * ����:101101100��³ľ��:101130101ʯ����:101130301����:101130401��³��:101130501�����:101130601������:101130701
 * ������:101130801��ʲ:101130901����:101131001����:101131101����:101131201����:101131301����̩:101131401
 * ��ͼʲ:101131501����:101131601��������:101130201����:101140101ɽ��:101140301����:101140701����:101140501
 * ����:101140601�տ���:101140201��֥:101140401̨����:101340101����:101340201̨��:101340401����:101310101
 * ����:101310201����:101310202�ٸ�:101310203����:101310204����:101310205����:101310206��ɳ:101310207
 * ����:101310208����:101310209�Ͳ�:101310210��:101310211�Ĳ�:101310212��ͤ:101310214����:101310215
 * ��ˮ:101310216��ɳ:101310217��ɳ��:101310220�ֶ�:101310221��ָɽ:101310222��ɽ:101310102��ɳ:101250101
 * ����:101250301����:101250401����:101250501����:101250601����:101250700¦��:101250801����:101250901
 * ����:101251001�żҽ�:101251101����:101251201ǭ��:101251301����:101251401����:101251501��̶:101250201
 * �Ͼ�:101190101��:101190301����:101190401��ͨ:101190501����:101190601��Ǩ:101191301����:101190801
 * ����:101190901���Ƹ�:101191001����:101191101̩��:101191201����:101190201�γ�:101190701������:101050101
 * ĵ����:101050301��ľ˹:101050401�绯:101050501�ں�:101050601˫Ѽɽ:101051301����:101050801����:101050901
 * ��̨��:101051002����:101051101�׸�:101051201�������:101050201���˰���:101050701����:101060101�Ӽ�:101060301
 * ��ƽ:101060401��ɽ:101060901�׳�:101060601��Դ:101060701��ԭ:101060801����:101060201ͨ��:101060501
 * ����:101070101��ɽ:101070301��˳:101070401��Ϫ:101070501����:101070601��«��:101071401Ӫ��:101070801
 * ����:101070901����:101071001����:101071101����:101071201�̽�:101071301����:101070201����:101070701
 * */


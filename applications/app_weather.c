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
#include <webclient.h>  /* 使用 HTTP 协议与服务器通信需要包含此头文件 */
#include <sys/socket.h> /* 使用BSD socket，需要包含socket.h头文件 */
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

/* 天气数据解析 */
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

    /* 为 weather_url 分配空间 */
    weather_url = rt_calloc(1, GET_URL_LEN_MAX);
    if (weather_url == RT_NULL)
    {
        rt_kprintf("No memory for weather_url!\n");
        goto __exit;
    }
    /* 拼接 GET 网址 */
    rt_snprintf(weather_url, GET_URL_LEN_MAX, GET_URI, api_city_code);
//    sprintf(weather_url, "%s%s", GET_URI, AREA_ID);
//    rt_kprintf("weather_url=%s\n", weather_url);
    /* 创建会话并且设置响应的大小 */
    session = webclient_session_create(GET_HEADER_BUFSZ);
//    rt_kprintf("session = %s\n", session);
    if (session == RT_NULL)
    {
        rt_kprintf("No memory for get header!\n");
        goto __exit;
    }

    /* 发送 GET 请求使用默认的头部 */
    if ((resp_status = webclient_get(session, weather_url)) != 200)
    {
        rt_kprintf("webclient GET request failed, response(%d) error.\n", resp_status);
        goto __exit;
    }

    /* 分配用于存放接收数据的缓冲 */
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
        /* 返回的数据是分块传输的. */
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
    /* 天气数据解析 */
    Weather_DataParse(buffer);

__exit:
    /* 释放网址空间 */
    if (weather_url != RT_NULL)
        rt_free(weather_url);
    /* 关闭会话 */
    if (session != RT_NULL)
        webclient_close(session);
    /* 释放缓冲区空间 */
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

    /* 为 weather_url 分配空间 */
    weather_url = rt_calloc(1, GET_URL_LEN_MAX);
    if (weather_url == RT_NULL)
    {
        rt_kprintf("No memory for weather_url!\n");
        goto __exit;
    }
    /* 拼接 GET 网址 */
    rt_snprintf(weather_url, GET_URL_LEN_MAX, GET_URI, AREA_ID);
//    sprintf(weather_url, "%s%s", GET_URI, AREA_ID);
//    rt_kprintf("weather_url=%s\n", weather_url);
    /* 创建会话并且设置响应的大小 */
    session = webclient_session_create(GET_HEADER_BUFSZ);
//    rt_kprintf("session = %s\n", session);
    if (session == RT_NULL)
    {
        rt_kprintf("No memory for get header!\n");
        goto __exit;
    }

    /* 发送 GET 请求使用默认的头部 */
    if ((resp_status = webclient_get(session, weather_url)) != 200)
    {
        rt_kprintf("webclient GET request failed, response(%d) error.\n", resp_status);
        goto __exit;
    }

    /* 分配用于存放接收数据的缓冲 */
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
        /* 返回的数据是分块传输的. */
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
    /* 天气数据解析 */
    Weather_DataParse2(buffer);

__exit:
    /* 释放网址空间 */
    if (weather_url != RT_NULL)
        rt_free(weather_url);
    /* 关闭会话 */
    if (session != RT_NULL)
        webclient_close(session);
    /* 释放缓冲区空间 */
    if (buffer != RT_NULL)
        rt_free(buffer);
}

MSH_CMD_EXPORT(Weather_Weather, Get weather by webclient);


/*
 * 北京:101010100朝阳:101010300顺义:101010400怀柔:101010500通州:101010600昌平:101010700延庆:101010800丰台:101010900
 * 石景山:101011000大兴:101011100房山:101011200密云:101011300门头沟:101011400平谷:101011500八达岭:101011600佛爷顶:101011700
 * 汤河口:101011800密云上甸子:101011900斋堂:101012000霞云岭:101012100北京城区:101012200海淀:101010200天津:101030100宝坻:101030300
 * 东丽:101030400西青:101030500北辰:101030600蓟县:101031400汉沽:101030800静海:101030900津南:101031000塘沽:101031100大港:101031200
 * 武清:101030200宁河:101030700上海:101020100宝山:101020300嘉定:101020500南汇:101020600浦东:101021300青浦:101020800松江:101020900
 * 奉贤:101021000崇明:101021100徐家汇:101021200闵行:101020200金山:101020700石家庄:101090101张家口:101090301承德:101090402唐山:101090501
 * 秦皇岛:101091101沧州:101090701衡水:101090801邢台:101090901邯郸:101091001保定:101090201廊坊:101090601郑州:101180101新乡:101180301
 * 许昌:101180401平顶山:101180501信阳:101180601南阳:101180701开封:101180801洛阳:101180901商丘:101181001焦作:101181101鹤壁:101181201
 * 濮阳:101181301周口:101181401漯河:101181501驻马店:101181601三门峡:101181701济源:101181801安阳:101180201合肥:101220101芜湖:101220301
 * 淮南:101220401马鞍山:101220501安庆:101220601宿州:101220701阜阳:101220801亳州:101220901黄山:101221001滁州:101221101淮北:101221201
 * 铜陵:101221301宣城:101221401六安:101221501巢湖:101221601池州:101221701蚌埠:101220201杭州:101210101舟山:101211101湖州:101210201
 * 嘉兴:101210301金华:101210901绍兴:101210501台州:101210601温州:101210701丽水:101210801衢州:101211001宁波:101210401重庆:101040100
 * 合川:101040300南川:101040400江津:101040500万盛:101040600渝北:101040700北碚:101040800巴南:101040900
 * 长寿:101041000黔江:101041100万州天城:101041200万州龙宝:101041300涪陵:101041400开县:101041500城口:101041600
 * 云阳:101041700巫溪:101041800奉节:101041900巫山:101042000潼南:101042100垫江:101042200梁平:101042300
 * 忠县:101042400石柱:101042500大足:101042600荣昌:101042700铜梁:101042800璧山:101042900丰都:101043000
 * 武隆:101043100彭水:101043200綦江:101043300酉阳:101043400秀山:101043600沙坪坝:101043700永川:101040200
 * 福州:101230101泉州:101230501漳州:101230601龙岩:101230701晋江:101230509南平:101230901厦门:101230201
 * 宁德:101230301莆田:101230401三明:101230801兰州:101160101平凉:101160301庆阳:101160401武威:101160501
 * 金昌:101160601嘉峪关:101161401酒泉:101160801天水:101160901武都:101161001临夏:101161101合作:101161201
 * 白银:101161301定西:101160201张掖:101160701广州:101280101惠州:101280301梅州:101280401汕头:101280501
 * 深圳:101280601珠海:101280701佛山:101280800肇庆:101280901湛江:101281001江门:101281101河源:101281201
 * 清远:101281301云浮:101281401潮州:101281501东莞:101281601中山:101281701阳江:101281801揭阳:101281901
 * 茂名:101282001汕尾:101282101韶关:101280201南宁:101300101柳州:101300301来宾:101300401桂林:101300501
 * 梧州:101300601防城港:101301401贵港:101300801玉林:101300901百色:101301001钦州:101301101河池:101301201
 * 北海:101301301崇左:101300201贺州:101300701贵阳:101260101安顺:101260301都匀:101260401兴义:101260906
 * 铜仁:101260601毕节:101260701六盘水:101260801遵义:101260201凯里:101260501昆明:101290101红河:101290301
 * 文山:101290601玉溪:101290701楚雄:101290801普洱:101290901昭通:101291001临沧:101291101怒江:101291201
 * 香格里拉:101291301丽江:101291401德宏:101291501景洪:101291601大理:101290201曲靖:101290401保山:101290501
 * 呼和浩特:101080101乌海:101080301集宁:101080401通辽:101080501阿拉善左旗:101081201鄂尔多斯:101080701
 * 临河:101080801锡林浩特:101080901呼伦贝尔:101081000乌兰浩特:101081101包头:101080201赤峰:101080601南昌:101240101
 * 上饶:101240301抚州:101240401宜春:101240501鹰潭:101241101赣州:101240701景德镇:101240801萍乡:101240901
 * 新余:101241001九江:101240201吉安:101240601武汉:101200101黄冈:101200501荆州:101200801宜昌:101200901
 * 恩施:101201001十堰:101201101神农架:101201201随州:101201301荆门:101201401天门:101201501仙桃:101201601
 * 潜江:101201701襄樊:101200201鄂州:101200301孝感:101200401黄石:101200601咸宁:101200701成都:101270101
 * 自贡:101270301绵阳:101270401南充:101270501达州:101270601遂宁:101270701广安:101270801巴中:101270901
 * 泸州:101271001宜宾:101271101内江:101271201资阳:101271301乐山:101271401眉山:101271501凉山:101271601
 * 雅安:101271701甘孜:101271801阿坝:101271901德阳:101272001广元:101272101攀枝花:101270201银川:101170101
 * 中卫:101170501固原:101170401石嘴山:101170201吴忠:101170301西宁:101150101黄南:101150301海北:101150801
 * 果洛:101150501玉树:101150601海西:101150701海东:101150201海南:101150401济南:101120101潍坊:101120601
 * 临沂:101120901菏泽:101121001滨州:101121101东营:101121201威海:101121301枣庄:101121401日照:101121501
 * 莱芜:101121601聊城:101121701青岛:101120201淄博:101120301德州:101120401烟台:101120501济宁:101120701
 * 泰安:101120801西安:101110101延安:101110300榆林:101110401铜川:101111001商洛:101110601安康:101110701
 * 汉中:101110801宝鸡:101110901咸阳:101110200渭南:101110501太原:101100101临汾:101100701运城:101100801
 * 朔州:101100901忻州:101101001长治:101100501大同:101100201阳泉:101100301晋中:101100401晋城:101100601
 * 吕梁:101101100乌鲁木齐:101130101石河子:101130301昌吉:101130401吐鲁番:101130501库尔勒:101130601阿拉尔:101130701
 * 阿克苏:101130801喀什:101130901伊宁:101131001塔城:101131101哈密:101131201和田:101131301阿勒泰:101131401
 * 阿图什:101131501博乐:101131601克拉玛依:101130201拉萨:101140101山南:101140301阿里:101140701昌都:101140501
 * 那曲:101140601日喀则:101140201林芝:101140401台北县:101340101高雄:101340201台中:101340401海口:101310101
 * 三亚:101310201东方:101310202临高:101310203澄迈:101310204儋州:101310205昌江:101310206白沙:101310207
 * 琼中:101310208定安:101310209屯昌:101310210琼海:101310211文昌:101310212保亭:101310214万宁:101310215
 * 陵水:101310216西沙:101310217南沙岛:101310220乐东:101310221五指山:101310222琼山:101310102长沙:101250101
 * 株洲:101250301衡阳:101250401郴州:101250501常德:101250601益阳:101250700娄底:101250801邵阳:101250901
 * 岳阳:101251001张家界:101251101怀化:101251201黔阳:101251301永州:101251401吉首:101251501湘潭:101250201
 * 南京:101190101镇江:101190301苏州:101190401南通:101190501扬州:101190601宿迁:101191301徐州:101190801
 * 淮安:101190901连云港:101191001常州:101191101泰州:101191201无锡:101190201盐城:101190701哈尔滨:101050101
 * 牡丹江:101050301佳木斯:101050401绥化:101050501黑河:101050601双鸭山:101051301伊春:101050801大庆:101050901
 * 七台河:101051002鸡西:101051101鹤岗:101051201齐齐哈尔:101050201大兴安岭:101050701长春:101060101延吉:101060301
 * 四平:101060401白山:101060901白城:101060601辽源:101060701松原:101060801吉林:101060201通化:101060501
 * 沈阳:101070101鞍山:101070301抚顺:101070401本溪:101070501丹东:101070601葫芦岛:101071401营口:101070801
 * 阜新:101070901辽阳:101071001铁岭:101071101朝阳:101071201盘锦:101071301大连:101070201锦州:101070701
 * */



#include "osapi.h"
#include "cJSON.h"
#include "c_types.h"
#include "user_json.h"
#include "user_debug.h"
#include "ip_addr.h"
#include "user_muc.h"


static int ICACHE_FLASH_ATTR 
parse_data_time(const char *ibuf,
                    int *year, int *mon, int *day, int *hour, int *min, int *se)
{
    char yyear[8] = {0}, mmonth[4] = {0}, dday[4] = {0}, hhour[4] = {0}, mmin[4] = {0}, sse[4] = {0};

    os_strncpy(yyear, ibuf, 4);
    os_strncpy(mmonth, ibuf + 5, 2);
    os_strncpy(dday, ibuf + 8, 2);
    os_strncpy(hhour, ibuf + 11, 2);
    os_strncpy(mmin, ibuf + 14, 2);
    os_strncpy(sse, ibuf + 17, 2);

    *year = atoi(yyear);
    *mon = atoi(mmonth);
    *day = atoi(dday);
    *hour = atoi(hhour);
    *min = atoi(mmin);
    *se = atoi(sse);

    return 0;
}

static int ICACHE_FLASH_ATTR
add_switch(char *times, char *ttime, char *enable)
{
    if (times != NULL && ttime != NULL && enable != NULL)
    {
        // TODO: 进行解析
        int i = 0, b = 0, offset = 0, y, mm, d, h, m, s, on;
        char c;
        if (os_strlen(enable) == 1) // "1"
        {
            on = atoi(enable);
        }
        else
        {
            // FIXME: 数据错误 需要作出调整
        }
        if (os_strlen(ttime) == 19) //"2017-10-24 10:24:00"
        {
            parse_data_time(ttime, &y, &mm, &d, &h, &m, &s);
        }
        else
        {
            // FIXME: 数据错误 需要作出调整
        }
        if (os_strlen(times) > 1)
        {
            for (i = 0; i < os_strlen(times); i = i + 2)
            {
                c = times[i];
                offset = atoi(&c);
                offset = offset - 1;
                b = b | 1 << offset;
            }
            // 加入时间
            add_user_alarm(b, on, h, m);
        }
        else if (os_strlen(times) == 1)
        {
            c = times[i];
            offset = atoi(&c);
            if (offset == 8)
            {
                b = 0x7F;
                add_user_alarm(b, on, h, m);
            }
            else if (offset == 0)
            {
                int ds, ls;
                // 加入到延时队列
                ls = ec_get_timestamp();
                ds = ec_mktime(y, mm, d, h, m, s) - ls;

                if (ds > 0)
                {
                    add_user_delay(on, ds);
                }
                else
                {
                    // FIXME: 数据错误
                }
            }
            else
            {
                // FIXME: 数据错误
            }
        }
        else
        {
            // FIXME: 数据错误
        }
    }
    return 0;
}

static int ICACHE_FLASH_ATTR 
add_scene(char *ttime, char *enable)
{

    if (ttime != NULL && enable != NULL)
    {
        // TODO: 进行解析
        int on = 0, ds, ls, y, mm, d, h, m, s;
        if (os_strlen(enable) == 1) // "1"
        {
            on = atoi(enable);
        }
        else
        {
            // FIXME: 数据错误 需要作出调整
        }
        if (os_strlen(ttime) == 19) //"2017-10-24 10:24:00"
        {
            parse_data_time(ttime, &y, &mm, &d, &h, &m, &s);
        }
        else
        {
            // FIXME: 数据错误 需要作出调整
        }
        ls = ec_get_timestamp();
        ds = ec_mktime(y, mm, d, h, m, s) - ls;
        if (ds > 0)
        {
            // 加入到延时队列
            add_user_delay(on, ds);
        }
        else
        {
            // FIXME: 数据错误
        }
        return 0;
    }
    return -1;
}

int ICACHE_FLASH_ATTR
json_parse_config(char *json,
                  struct jabber_config *jconfig,
                  struct wifi_config *wconfig, char *register_url)
{

    if (jconfig != NULL && wconfig != NULL)
    {
        cJSON *root, *t;

        root = cJSON_Parse(json);
        os_memset(jconfig, 0x0, sizeof(struct jabber_config));
        os_memset(wconfig, 0x0, sizeof(struct wifi_config));

        t = cJSON_GetObjectItem(root, "ssid");
        if (t->type == cJSON_String)
        {
            char *tmp;
            tmp = t->valuestring;
            os_memcpy(wconfig->ssid, tmp, os_strlen(tmp));
        }

        t = cJSON_GetObjectItem(root, "pssd");
        if (t->type == cJSON_String)
        {
            char *tmp;
            tmp = t->valuestring;
            os_memcpy(wconfig->password, tmp, os_strlen(tmp));
        }

        t = cJSON_GetObjectItem(root, "hurl");
        if (t->type == cJSON_String)
        {
            char *tmp;
            tmp = t->valuestring;
            os_memcpy(register_url, tmp, os_strlen(tmp));
        }

        t = cJSON_GetObjectItem(root, "jport");
        if (t->type == cJSON_String)
        {
            char *tmp;
            tmp = t->valuestring;
            // ec_log("json port %d \r\n");
            jconfig->port = atoi(tmp);
            ec_log("json port %d \r\n", jconfig->port);
        }

        t = cJSON_GetObjectItem(root, "jip");
        if (t->type == cJSON_String)
        {
            char *tmp;
            tmp = t->valuestring;
            if(os_strlen(tmp)>4)
            {
                ec_log("xmpp ip %s\r\n",tmp);
                jconfig->ip.addr = ipaddr_addr(tmp);
                ec_log("xmpp ip %d\r\n", jconfig->ip.addr);
            }
            else
            {
                jconfig->ip.addr = 0;
            }
            
        }

        t = cJSON_GetObjectItem(root, "appid");
        if (t->type == cJSON_String)
        {
            char *tmp;
            tmp = t->valuestring;
            os_memcpy(jconfig->app_username, tmp, os_strlen(tmp));
        }

        t = cJSON_GetObjectItem(root, "jdomain");
        if (t->type == cJSON_String)
        {
            char *tmp;
            tmp = t->valuestring;
            os_memcpy(jconfig->domain, tmp, os_strlen(tmp));
        }

        t = cJSON_GetObjectItem(root, "jhost");
        if (t->type == cJSON_String)
        {
            char *tmp;
            tmp = t->valuestring;
            if(os_strlen(tmp)>1)
            {
                os_memcpy(jconfig->host_name, tmp, os_strlen(tmp));
            }
            else
            {
                // jconfig->host_name = NULL;
            }
            
        }

        t = cJSON_GetObjectItem(root, "jres");
        if (t->type == cJSON_String)
        {
            char *tmp;
            tmp = t->valuestring;
            os_memcpy(jconfig->resources, tmp, os_strlen(tmp));
        }
        ec_log("-----json_parse_config %d \r\n", j_config.port);
        cJSON_Delete(root);
    }
    return 0;
}

//
int ICACHE_FLASH_ATTR
json_parse_switch(char *json, char *linkid, int *type)
{
    int i = 0;
    cJSON *root = NULL, *t = NULL, *list = NULL, *tp = NULL;
    char *times = NULL, *ttime = NULL, *enable = NULL;
    root = cJSON_Parse(json);

    stop_all_user_alarm();
    stop_all_user_delay();

    t =  cJSON_GetObjectItem(root, "type");
    *type = atoi(t->valuestring);
    
    t =  cJSON_GetObjectItem(root, "linkid");
    // linkid = t->valuestring;
    os_memcpy(linkid, t->valuestring, os_strlen(t->valuestring));
    list = cJSON_GetObjectItem(root, "switchList");
    for (i; i < cJSON_GetArraySize(list); i++)
    {

        t = cJSON_GetArrayItem(list, i);

        tp = cJSON_GetObjectItem(t, "times");
        if (tp->type == cJSON_String)
        {
            times = tp->valuestring;
        }

        tp = cJSON_GetObjectItem(t, "time");
        if (tp->type == cJSON_String)
        {
            ttime = tp->valuestring;
        }

        tp = cJSON_GetObjectItem(t, "enable");
        if (tp->type == cJSON_String)
        {
            enable = tp->valuestring;
        }
        add_switch(times, ttime, enable);
    }

    i = 0;

    list = cJSON_GetObjectItem(root, "sceneList");
    for (i; i < cJSON_GetArraySize(list); i++)
    {
        t = cJSON_GetArrayItem(list, i);
        tp = cJSON_GetObjectItem(t, "time");
        if (tp->type == cJSON_String)
        {
            ttime = tp->valuestring;
        }
        tp = cJSON_GetObjectItem(t, "enable");
        if (tp->type == cJSON_String)
        {
            enable = tp->valuestring;
        }
        add_scene(ttime, enable);
    }
    cJSON_Delete(root);
    return 0;
}

int ICACHE_FLASH_ATTR
json_parse_register(char *json)
{
    cJSON *root = NULL, *t = NULL;
    char *error_code = NULL;
    ec_log("json_parse_register %s \r\n",json);
    root = cJSON_Parse(json);
    t = cJSON_GetObjectItem(root, "error_code");
    if (t->type == cJSON_String)
    {
        error_code = t->valuestring;
        if (os_strcmp(error_code, "0") == 0)
        {
            ec_log("====json_parse_register==== \r\n");
            cJSON_Delete(root);
            return 1;
        }
        ec_log("eroor code %s\r\n", error_code);
        cJSON_Delete(root);
        return -1;
    }

    cJSON_Delete(root);
    return -1;
}

int ICACHE_FLASH_ATTR 
json_parse_async(char *json)
{
    // TODO: 去进行数据解析
    char *linkid;
    ec_get_asyncinfo(linkid);
    return 0;
}

/**
 * 返回值 1 表示成功 0 表示设置失败
 */
int ICACHE_FLASH_ATTR
json_parse_commamd(char *json, char *linkid)
{
    int b;
    char cb[4] = { 0 };
    return ec_switch(b);
}
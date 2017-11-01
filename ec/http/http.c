#include "user_debug.h"
#include "http_parser.h"
#include "http.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "http_io.h"

static char recv_buf[1024] = {0};
static char http_body[1024] = {0};
static char soc_send_buf[1024] = {0};
static char http_respons_buf[1024] = {0};

static void dump_url(const char *url, const struct http_parser_url *u, int type);

static http_failure_callback http_failure_handler;
static http_success_callback http_success_handler;

static http_parser parser;
static http_parser_settings settings;

static int ICACHE_FLASH_ATTR
on_message_begin(http_parser *_)
{
    (void)_;
    //kal_prompt_trace(MOD_MMI,"\n***MESSAGE BEGIN***\n\n");
    return 0;
}

static int ICACHE_FLASH_ATTR
on_headers_complete(http_parser *_)
{
    (void)_;
    //kal_prompt_trace(MOD_MMI,"\n***HEADERS COMPLETE***\n\n");
    return 0;
}

static int ICACHE_FLASH_ATTR
on_message_complete(http_parser *_)
{
    (void)_;

    ec_log("\r\n*** http MESSAGE COMPLETE***\r\n\r\n");
    ec_log("\r\n>>> %s <<< \r\n\r\n", http_respons_buf);

    // MARK: 数据接收完成
    if (http_success_handler)
    {
        http_success_handler(http_respons_buf, os_strlen(http_respons_buf));
    }

    e_soc_close();

    return 0;
}

static int ICACHE_FLASH_ATTR
on_url(http_parser *_, const char *at, size_t length)
{
    (void)_;
    // kal_prompt_trace(MOD_MMI,"Url: %s\n", at);
    // kal_prompt_trace(MOD_MMI,"Url: %.*s\n", (int)length, at);
    return 0;
}

static int ICACHE_FLASH_ATTR
on_header_field(http_parser *_, const char *at, size_t length)
{
    (void)_;

    return 0;
}

static int ICACHE_FLASH_ATTR
on_header_value(http_parser *_, const char *at, size_t length)
{
    (void)_;

    return 0;
}

static int ICACHE_FLASH_ATTR
on_body(http_parser *_, const char *at, size_t length)
{
    (void)_;

    strncat(http_respons_buf, at, length);

    return 0;
}

static void ICACHE_FLASH_ATTR
e_http_init()
{
    http_failure_handler = NULL;
    http_success_handler = NULL;
    // socket_id = -1;

    // memset(&g_test_ip, 0x0, sizeof(g_test_ip));
    os_memset(recv_buf, 0x0, sizeof(recv_buf));
    os_memset(http_body, 0x0, sizeof(http_body));
    os_memset(soc_send_buf, 0x0, sizeof(soc_send_buf));
    os_memset(http_respons_buf, 0x0, sizeof(http_respons_buf));

    http_parser_init(&parser, HTTP_RESPONSE);
    os_memset(&settings, 0x0, sizeof(settings));
    settings.on_message_begin = on_message_begin;
    settings.on_url = on_url;
    settings.on_header_field = on_header_field;
    settings.on_header_value = on_header_value;
    settings.on_headers_complete = on_headers_complete;
    settings.on_body = on_body;
    settings.on_message_complete = on_message_complete;
}

static void ICACHE_FLASH_ATTR
e_http_recv(char *data, unsigned short len)
{
    size_t parsed;
    os_memcpy(recv_buf, data, len);
    parsed = http_parser_execute(&parser, &settings, recv_buf, len);
}

static void ICACHE_FLASH_ATTR
dump_url(const char *url, const struct http_parser_url *u, int type)
{
    char CRLF[] = {0x0d, 0x0a, 0x00};
    unsigned int i;
    int port;
    char host[128] = {0};
    int body_len = 0;
    char content_length[32] = {0};
    char line[64] = {0};
    char host_url[32] = {0};

    if (u->port != 0)
    {
        // g_test_ip.port = u->port;
        port = u->port;
    }
    else
    {
        // g_test_ip.port = 80;
        port = 80;
    }

    if (type == 0)
    {
        os_strcat(line, "GET ");
    }
    else
    {
        os_strcat(line, "POST ");
    }

    for (i = 0; i < UF_MAX; i++)
    {
        if ((u->field_set & (1 << i)) == 0)
        {
            ec_log(" == field_data[%u]: unset\r\n", i);
            continue;
        }

        switch (i)
        {
        case 1:
            os_strcat(host, "Host: ");
            strncat(host_url, url + u->field_data[i].off, u->field_data[i].len);
            strncat(host, url + u->field_data[i].off, u->field_data[i].len);
            break;
        case 2:
            os_strcat(host, ":");
            strncat(host, url + u->field_data[i].off, u->field_data[i].len);
            break;
        case 3:
            strncat(line, url + u->field_data[i].off, u->field_data[i].len);
            os_strcat(line, " HTTP/1.1");
            break;
        default:
            break;
        }
    }

    body_len = (int)os_strlen(http_body);
    if (body_len > 0)
    {
        os_sprintf(content_length, "Content-Length: %d", body_len);
    }
    os_strcat(soc_send_buf, line);
    os_strcat(soc_send_buf, CRLF);
    os_strcat(soc_send_buf, host);
    os_strcat(soc_send_buf, CRLF);
    if (body_len > 0)
    {
        os_strcat(soc_send_buf, content_length);
        os_strcat(soc_send_buf, CRLF);
    }
    os_strcat(soc_send_buf, "Connection: Closed");
    os_strcat(soc_send_buf, CRLF);
    os_strcat(soc_send_buf, CRLF);

    if (body_len > 0)
    {
        os_strcat(soc_send_buf, http_body);
    }

    // 发起连接
    e_soc_creat(host_url, port, soc_send_buf, e_http_recv);
}

// 创建连接部分 初始化部分 请求解析部分
int ICACHE_FLASH_ATTR
http_request(const char *url,
             int type, // type 表示请求类型
             const char *body,
             http_success_callback success_handl,
             http_failure_callback failure_handl)
{
    int len, result;
    struct http_parser_url u;

    if (success_handl == NULL || failure_handl == NULL)
    {
        return -1;
    }

    e_http_init();
    http_parser_url_init(&u);

    len = (int)os_strlen(url);

    http_failure_handler = failure_handl;
    http_success_handler = success_handl;

    result = http_parser_parse_url(url, len, 0, &u);
    // kal_prompt_trace(MOD_MMI,"gbc_dm_http_request %d", result);
    if (result != 0)
    {
        // printf("http request error : %d\n", result);
        // MARK: URL错误 进行返回错误
        if (http_failure_handler)
        {
            http_failure_handler(2009);
        }
        return;
    }
    // MARK: 保存body数据 全局数据
    if (body)
    {
        os_strcpy(http_body, body);
    }
    // TODO: 加上定时器对其进行超时判断
    dump_url(url, &u, type);
    return 0;
}

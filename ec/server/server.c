
#include "osapi.h"
#include "server.h"
#include "user_interface.h"
#include "user_debug.h"
#include "espconn.h"
#include "http_parser.h"
#include "mem.h"

#define REQUEST_HEAD "HTTP/1.1 200 OK\r\n\
Server: Apache/2.4.27 (Unix) PHP/7.1.7\r\n\
Accept-Ranges: bytes\r\n\
Content-Length: %d\r\n\
Content-Type: text/html\r\n\r\n%s"

static http_parser parser = {0};
static http_parser_settings settings = {0};
static char http_respons_buf[256] = {0};
static server_recv_callback server_recv_handler;

#define TASK_QUEUE_LEN = 4;
static os_event_t *task_queue;

static struct espconn esp_conn;
static esp_tcp esptcp;

static int ICACHE_FLASH_ATTR
on_message_begin(http_parser *_)
{
    (void)_;
    ec_log("on_message_begin \r\n");
    return 0;
}

static int ICACHE_FLASH_ATTR
on_headers_complete(http_parser *_)
{
    (void)_;
    ec_log("on_headers_complete\r\n");
    return 0;
}

void ICACHE_FLASH_ATTR send_codec_encode(char *buff, int len, char *buff_out);



static int ICACHE_FLASH_ATTR
on_message_complete(http_parser *_)
{
    (void)_;
    char body[64] = "{\"errcode\":0, \"errmsg\":\"ok\" }";
    char base64_body[128] = { 0 };
    char soc_send_buffer[512] = {0};
    // TODO: 关闭连接

    ec_log("\r\n***MESSAGE COMPLETE***\r\n");
    ec_log("\r\n>>> %s <<<\r\n", http_respons_buf);

    if (server_recv_handler)
    {
        server_recv_handler(http_respons_buf, os_strlen(http_respons_buf));
    }

    // MARK: 进行简单的加密发送
    send_codec_encode(body, os_strlen(body), base64_body);

    // MARK: 这里进行数据的回复
    os_sprintf(soc_send_buffer, REQUEST_HEAD, os_strlen(base64_body), base64_body);
    espconn_send(&esp_conn, soc_send_buffer, os_strlen(soc_send_buffer));

    return 0;
}

static int ICACHE_FLASH_ATTR
on_url(http_parser *_, const char *at, size_t length)
{
    (void)_;
    // ec_log("on_url  %s \r\n",at);
    return 0;
}

static int ICACHE_FLASH_ATTR
on_header_field(http_parser *_, const char *at, size_t length)
{
    (void)_;
    // ec_log("on_header_field  %s \r\n",at);
    return 0;
}

static int ICACHE_FLASH_ATTR
on_header_value(http_parser *_, const char *at, size_t length)
{
    (void)_;
    // ec_log("on_header_value  %s \r\n",at);
    return 0;
}

static int ICACHE_FLASH_ATTR
on_body(http_parser *_, const char *at, size_t length)
{
    (void)_;
    // ec_log("on_body  %s \r\n",at);
    strncat(http_respons_buf, at, length);
    return 0;
}

static void ICACHE_FLASH_ATTR
server_parser_init()
{

    os_memset(http_respons_buf, 0x0, sizeof(http_respons_buf));

    os_memset(&parser, 0, sizeof(parser));
    http_parser_init(&parser, HTTP_REQUEST);

    os_memset(&settings, 0, sizeof(settings));
    settings.on_message_begin = on_message_begin;
    settings.on_url = on_url;
    settings.on_header_field = on_header_field;
    settings.on_header_value = on_header_value;
    settings.on_headers_complete = on_headers_complete;
    settings.on_body = on_body;
    settings.on_message_complete = on_message_complete;
}

static void ICACHE_FLASH_ATTR
server_recv(void *arg, char *pusrdata, unsigned short length)
{
    size_t parsed;
    struct espconn *pesp_conn = arg;
    ec_log("server_recv %d [%s]\r\n", length, pusrdata);
    ec_log("server_recv server's %d.%d.%d.%d:%d \r\n", pesp_conn->proto.tcp->remote_ip[0],
           pesp_conn->proto.tcp->remote_ip[1], pesp_conn->proto.tcp->remote_ip[2],
           pesp_conn->proto.tcp->remote_ip[3], pesp_conn->proto.tcp->remote_port);
    // 解析数据
    // TODO: 这部分需要进行对数据进行判断
    server_parser_init();
    // TODO: 这里是否需要进连接关闭
    parsed = http_parser_execute(&parser, &settings, pusrdata, length);
}

static void ICACHE_FLASH_ATTR
server_recon(void *arg, sint8 err)
{
    struct espconn *pesp_conn = arg;

    ec_log("server_recon server's %d.%d.%d.%d:%d err %d reconnect\r\n", pesp_conn->proto.tcp->remote_ip[0],
           pesp_conn->proto.tcp->remote_ip[1], pesp_conn->proto.tcp->remote_ip[2],
           pesp_conn->proto.tcp->remote_ip[3], pesp_conn->proto.tcp->remote_port, err);
}

static void ICACHE_FLASH_ATTR
server_discon(void *arg)
{
    struct espconn *pesp_conn = arg;

    ec_log("server_discon server's %d.%d.%d.%d:%d disconnect\r\n", pesp_conn->proto.tcp->remote_ip[0],
           pesp_conn->proto.tcp->remote_ip[1], pesp_conn->proto.tcp->remote_ip[2],
           pesp_conn->proto.tcp->remote_ip[3], pesp_conn->proto.tcp->remote_port);
}

static void ICACHE_FLASH_ATTR
server_send_cb(void *arg)
{
    // TODO: 这里可以断开连接 发送任务
    // system_os_post(USER_TASK_PRIO_0, 0, 0);
}

static void ICACHE_FLASH_ATTR
server_listen(void *arg)
{
    struct espconn *pesp_conn = arg;
    ec_log("server_listen server's %d.%d.%d.%d:%d disconnect\r\n", pesp_conn->proto.tcp->remote_ip[0],
           pesp_conn->proto.tcp->remote_ip[1], pesp_conn->proto.tcp->remote_ip[2],
           pesp_conn->proto.tcp->remote_ip[3], pesp_conn->proto.tcp->remote_port);

    espconn_regist_reconcb(pesp_conn, server_recon);
    espconn_regist_recvcb(pesp_conn, server_recv);
    espconn_regist_disconcb(pesp_conn, server_discon);
    espconn_regist_sentcb(pesp_conn, server_send_cb);
}

static void ICACHE_FLASH_ATTR
on_task(os_event_t *e)
{
    // TODO: 断开连接
    // espconn_disconnect(&esp_conn);
    // espconn_delete(&esp_conn);
}

void ICACHE_FLASH_ATTR
server_init(uint32 port, server_recv_callback handler)
{

    esp_conn.type = ESPCONN_TCP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = port;
    server_recv_handler = handler;
    ec_log("server init \r\n");

    espconn_regist_connectcb(&esp_conn, server_listen);

    espconn_accept(&esp_conn);

    // TODO: 创建任务
}

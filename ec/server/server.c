
#include "user_debug.h"
#include "espconn.h"


static http_parser parser;
static http_parser_settings settings;
static char http_respons_buf[256] = {0};

static int ICACHE_FLASH_ATTR
on_message_begin(http_parser *_)
{
    (void)_;
    
    return 0;
}

static int ICACHE_FLASH_ATTR
on_headers_complete(http_parser *_)
{
    (void)_;
    
    return 0;
}

static int ICACHE_FLASH_ATTR
on_message_complete(http_parser *_)
{
    (void)_;

    // TODO: 关闭连接
    
    // soc_close(socket_id);
    // GBC_sys_stop_timer(gbc_dm_timer);

    //   kal_prompt_trace(MOD_MMI, "\n***MESSAGE COMPLETE***\n\n");
    //   kal_prompt_trace(MOD_MMI, "\n>>>%s<<<\n\n", http_respons_buf);

    // MARK: 数据接收完成

    return 0;
}

static int ICACHE_FLASH_ATTR
on_url(http_parser *_, const char *at, size_t length)
{
    (void)_;

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
server_parser_init()
{
    // memset(recv_buf, 0x0, sizeof(recv_buf));
    // memset(http_body, 0x0, sizeof(http_body));
    // memset(soc_send_buf, 0x0, sizeof(soc_send_buf));
    os_memset(http_respons_buf, 0x0, sizeof(http_respons_buf));

    http_parser_init(&parser, HTTP_RESPONSE);

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
    INFO("server_recv %d [%s]\r\n", length, pusrdata);
    // 解析数据
    parsed = http_parser_execute(&parser, &settings, pusrdata, len);
}

static void ICACHE_FLASH_ATTR
server_recon(void *arg, sint8 err)
{
    struct espconn *pesp_conn = arg;

    INFO("server's %d.%d.%d.%d:%d err %d reconnect\n", pesp_conn->proto.tcp->remote_ip[0],
    		pesp_conn->proto.tcp->remote_ip[1],pesp_conn->proto.tcp->remote_ip[2],
    		pesp_conn->proto.tcp->remote_ip[3],pesp_conn->proto.tcp->remote_port, err);
}

static void ICACHE_FLASH_ATTR
server_discon(void *arg)
{
    struct espconn *pesp_conn = arg;

    INFO("server's %d.%d.%d.%d:%d disconnect\n", pesp_conn->proto.tcp->remote_ip[0],
        		pesp_conn->proto.tcp->remote_ip[1],pesp_conn->proto.tcp->remote_ip[2],
        		pesp_conn->proto.tcp->remote_ip[3],pesp_conn->proto.tcp->remote_port);
}

static void ICACHE_FLASH_ATTR
server_send_cb(void *arg)
{

}

static void ICACHE_FLASH_ATTR
server_listen(void *arg)
{
    struct espconn *pesp_conn = arg;

    espconn_regist_recvcb(pesp_conn, server_recv);
    espconn_regist_disconcb(pesp_conn, server_discon);
    espconn_regist_sentcb(pesp_conn, server_send_cb);
}

void ICACHE_FLASH_ATTR
server_init(uint32 port)
{
    LOCAL struct espconn esp_conn;
    LOCAL esp_tcp esptcp;

    esp_conn.type = ESPCONN_TCP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = port;
    server_parser_init();
    espconn_regist_connectcb(&esp_conn, server_listen);
    espconn_regist_reconcb(pesp_conn, server_recon);
    espconn_accept(&esp_conn);
}

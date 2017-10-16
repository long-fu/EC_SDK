
#include "debug.h"
#include "espconn.h"

static void ICACHE_FLASH_ATTR
server_recv(void *arg, char *pusrdata, unsigned short length)
{
    INFO("server_recv_ %d [%s]\r\n", length, pusrdata);
    // TODO: 这里需要发送数据
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
    wifi_set_opmode(SOFTAP_MODE);
    esp_conn.type = ESPCONN_TCP;
    esp_conn.state = ESPCONN_NONE;
    esp_conn.proto.tcp = &esptcp;
    esp_conn.proto.tcp->local_port = port;
    espconn_regist_connectcb(&esp_conn, server_listen);
    espconn_regist_reconcb(pesp_conn, server_recon);
    espconn_accept(&esp_conn);
}

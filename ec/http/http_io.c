#include "user_interface.h"
#include "os_type.h"
#include "user_debug.h"
#include "http_io.h"
#include "ec_io.h"
#include "mem.h"

static soc_recv_callback soc_recv_handler = NULL;
static char *soc_send_data = NULL;
static int soc_send_len = 0;

static void ICACHE_FLASH_ATTR
e_soc_recv(char *pusrdata, unsigned short len)
{
    if (soc_recv_handler)
    {
        soc_recv_handler(pusrdata, len);
    }
}

void ICACHE_FLASH_ATTR
e_soc_send(const char *data, int len)
{
    if ((data == NULL) || (len == 0))
    {
        return;
    }
    ec_io_send((uint8 *)data, (uint16)len);
}

void ICACHE_FLASH_ATTR
io_notify_cb(unsigned short status, void *arg)
{
    if (status == EC_IO_CONNET)
    {
        e_soc_send(soc_send_data, soc_send_len);
    }
}

// 关闭连接
void ICACHE_FLASH_ATTR
e_soc_close()
{
    soc_send_data = NULL;
    soc_send_len = 0;
    soc_recv_handler = NULL;
    ec_io_close();
}

// 创建连接
void ICACHE_FLASH_ATTR
e_soc_creat(char *host,
    int port,
    char *send_data,
    soc_recv_callback recv_handler)
{
    if(host == NULL || os_strlen(host) < 3)
    {
        return;
    }

    soc_send_len = os_strlen(send_data);
    soc_send_data = (char *) os_malloc(soc_send_len + 1);

    os_memset(soc_send_data, 0x0, soc_send_len);
    os_memcpy(soc_send_data, send_data, soc_send_len);

    soc_recv_handler = recv_handler;
    ec_io_connet(host, port);
    ec_log("http connet %s:%d \r\n",host,port);
    ec_io_regist_recvcb(e_soc_recv);

    ec_io_regist_notifycb(io_notify_cb);
}
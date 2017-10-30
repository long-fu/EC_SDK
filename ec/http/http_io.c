
#include "http_parser.h"
#include "user_debug.h"
#include "ip_addr.h"
#include "osapi.h"
#include "mem.h"
#include "espconn.h"
#include "http_io.h"
#include "ip_addr.h"
// soc 连接状态回调
// 0 进行连接中
// 1 连接成功 -- 在这里才能可以进行网络数据的发送
// 2 host 解析成功 获取ip地址
// -2 错误操作 导致连接断开 -- 在内部进行 重连
// -3 主动断开连接
// -4 发送buffer满了
// -5 域名解析出错

static soc_connect_status_callback soc_connect_status_handler = NULL;
static soc_recv_callback soc_recv_handler = NULL;
// static char soc_recv_buffer[512];
#define  ESPCONN_BUFFER_SIZE (2920)
static uint8 espconn_buffer[ESPCONN_BUFFER_SIZE];
static uint32 espconn_data_len = 0;
// FALSE 表示发送的数据追加到发送buffer中
// TRUE 表示数据可以立即进行发送
static bool espconn_flag = FALSE;

static ip_addr_t espconn_ip = { 0 };
static struct espconn *espconn_ptr = NULL;

static void ICACHE_FLASH_ATTR
espconn_on_recon_cb(void *arg, sint8 errType)
{
    struct espconn *espconn_ptr = (struct espconn *)arg;
    ec_log("espconn reconnect\r\n");
    espconn_flag = FALSE;
    soc_connect_status_handler(-2);
    espconn_connect(espconn_ptr);
}

// notify at module that espconn has received data
// 5. 这里会通知数据接收
static void ICACHE_FLASH_ATTR
espconn_on_recv(void *arg, char *pusrdata, unsigned short len)
{
    struct espconn *espconn_ptr = (struct espconn *)arg;
    // at_fake_uart_rx(pusrdata,len);
    // TODO: 这里会接收到网络数据
    ec_log("http on recv %d: [%s]\r\n", len, pusrdata);
    // os_memset(pusrdata, 0x0, 512);
    // os_memcpy(pusrdata, pusrdata, len);
    if (soc_recv_handler) 
    {
        // ec_log("-- -- cb recv ---- \r\n");
        soc_recv_handler(pusrdata, len);
    }
    // TODO: 
    // espconn_disconnect(espconn_ptr);
    // espconn_delete(espconn_ptr);
}

static void ICACHE_FLASH_ATTR
espconn_on_send_cb(void *arg)
{
    struct espconn *espconn_ptr = (struct espconn *)arg;
    ec_log("espconn_on_send_cb\r\n");
    espconn_flag = TRUE;
    if (espconn_data_len)
    {
        espconn_send(espconn_ptr, espconn_buffer, espconn_data_len);
        espconn_data_len = 0;
    }
}

static void ICACHE_FLASH_ATTR
espconn_on_discon_cb(void *arg)
{
    struct espconn *espconn_ptr = (struct espconn *)arg;

    ec_log("espconn disconnected\r\n");
    espconn_flag = FALSE;
    soc_connect_status_handler(-3);
    // TODO: 这里进行重连 这里是否需要进行重连 有待考察
    // espconn_connect(espconn_ptr);
}

/// 3. 连接成功后的回调
static void ICACHE_FLASH_ATTR
espconn_on_connect_cb(void *arg)
{
    int ret;
    ec_log("espconn connected\r\n");
    // espconn_set_opt((struct espconn*)arg,ESPCONN_COPY);
    espconn_flag = TRUE;
    espconn_data_len = 0;
    // MARK: 进行后续的注册回调
    

    // ec_log("espconn_on_connect_cb 00 %d\r\n",ret);
    // 注册 TCP 连接正常断开成功的回调函数
    ret = espconn_regist_disconcb(espconn_ptr, espconn_on_discon_cb);
    // ec_log("espconn_on_connect_cb 11 %d\r\n",ret);
    // 注册成功接收网络数据的回调函数
    ret = espconn_regist_recvcb(espconn_ptr, espconn_on_recv);
    // ec_log("espconn_on_connect_cb 22 %d\r\n",ret);
    // 注册网络数据发送成功的回调函数
    ret = espconn_regist_sentcb(espconn_ptr, espconn_on_send_cb);
    // ec_log("espconn_on_connect_cb 33 %d\r\n",ret);
    // TODO: 这里可以进行数据的发送
    soc_connect_status_handler(1);
}

/// 2. 域名解析后的回调
static void ICACHE_FLASH_ATTR
espconn_on_dns_cb(const char *name, ip_addr_t *ipaddr, void *arg)
{
    if (ipaddr == NULL)
    {
        ec_log("DNS: Found , but got no ip, try to reconnect\r\n");
        // FIXME: DNS: Found , but got no ip, try to reconnect
        soc_connect_status_handler(-5);
        return;
    }

    ec_log("DNS: found ip %d.%d.%d.%d\r\n",
         *((uint8 *)&ipaddr->addr),
         *((uint8 *)&ipaddr->addr + 1),
         *((uint8 *)&ipaddr->addr + 2),
         *((uint8 *)&ipaddr->addr + 3));

    if (espconn_ip.addr == 0 && ipaddr->addr != 0)
    {
        // MARK: 域名解析成功进行连接
        os_memcpy(espconn_ptr->proto.tcp->remote_ip, &ipaddr->addr, sizeof(ipaddr->addr));
        // TODO: 对返回值进行判断
        espconn_connect(espconn_ptr);
    }
}

// 关闭连接
void ICACHE_FLASH_ATTR 
e_soc_close()
{
    espconn_data_len = 0;
    espconn_flag = FALSE;
    soc_connect_status_handler = NULL;
    soc_recv_handler = NULL;
    ec_log("e_soc_close\r\n");
    os_memset(espconn_buffer, 0x0, ESPCONN_BUFFER_SIZE);
    if (espconn_ptr != NULL)
    {
        // TODO: 需要对返回值进行判读
        // 1. 断开
        espconn_disconnect(espconn_ptr);
        // 2. 删除
        espconn_delete(espconn_ptr);
    }
}

/// 这部分需要在连接成功后去调用是最完整的
// 发送数据
void ICACHE_FLASH_ATTR
e_soc_send(const char *data, int len)
{
    if ((data == NULL) || (len == 0))
    {
        return;
    }
    ec_log("e_sco_send %d: [%s]\r\n", len, data);
    if (espconn_flag)
    {
        int ret;
        // TODO: 对返回值进行判断
        ret = espconn_send(espconn_ptr, (uint8 *)data, len);
        // ec_log("send  %d \r\n",ret);
        espconn_flag = FALSE;
    }
    else
    {
        if (len <= (ESPCONN_BUFFER_SIZE - espconn_data_len))
        {
            os_memcpy(espconn_buffer + espconn_data_len, data, len);
            espconn_data_len += len;
        }
        else
        {
            ec_log("espconn buffer full\r\n");
            soc_connect_status_handler(-4);
        }
    }
}

// 创建连接
void ICACHE_FLASH_ATTR
e_soc_creat(char *host,
    int port, 
    soc_connect_status_callback connect_status_handler, 
    soc_recv_callback recv_handler)
{
    uint32 ipi = 0;
    if ((host == NULL) || os_strlen(host) <= 0 || connect_status_handler == NULL) 
    {
        return ;
    }
    ec_log("e_soc_creat\r\n");
    espconn_ptr = (struct espconn *)os_zalloc(sizeof(struct espconn));
    espconn_ptr->type = ESPCONN_TCP;
    espconn_ptr->state = ESPCONN_NONE;
    espconn_ptr->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
    espconn_ptr->proto.tcp->local_port = espconn_port();
    espconn_ptr->proto.tcp->remote_port = port;

    soc_connect_status_handler = connect_status_handler;
    soc_recv_handler = recv_handler;
    espconn_flag = FALSE;
    espconn_data_len = 0;
    // os_memset(soc_recv_buffer,0x0, 512);
    os_memset(espconn_buffer, 0x0, ESPCONN_BUFFER_SIZE);

    soc_connect_status_handler(0);
    // 1. 注册连接成功后的回调
    espconn_regist_connectcb(espconn_ptr, espconn_on_connect_cb);
    
    // 2. 连接失败重连回调
    // 注册 TCP 连接发生异常断开时的回调函数，可以在回调函数中进行重连。
    espconn_regist_reconcb(espconn_ptr, espconn_on_recon_cb);

    // 
    ipi = ipaddr_addr(host);
    ec_log("ipi hah %d\r\n",ipi);
    if (ipi > 0)
    {
        os_memcpy(espconn_ptr->proto.tcp->remote_ip, &ipi, sizeof(ipi));
        // MARK: 进行连接
        ipi = ipaddr_addr("www.baidu.com");
        ec_log("test hah %d \r\n",ipi);
        // TODO: 对返回值进行判断
        espconn_connect(espconn_ptr);
    }
    else 
    {
        espconn_gethostbyname(espconn_ptr, host, &espconn_ip, espconn_on_dns_cb);
    }
    
    //   ipi = ipaddr_addr("192.168.11.236");  /// 测试代码

    
    

    // MARK: 需要对ip地址进行判断
    // if (ip.addr != 0)
    // {
    //     // MARK: 直接进行连接
    //     os_memcpy(espconn_ptr->proto.tcp->remote_ip, &ip.addr, sizeof(ip.addr));
    //     // MARK: 进行连接
    //     // TODO: 对返回值进行判断
    //     espconn_connect(espconn_ptr);
    // }
    // else
    // {
    //     // MARK: 进行域名解析
    //     // ec_log("on gethostbyname %s\r\n",host);
    //     // MARK: 进行域名解析 - 解析后会回调结果
    //     // TODO: 对返回值进行判断
    //     espconn_gethostbyname(espconn_ptr, host, &espconn_ip, espconn_on_dns_cb);
    // }
}
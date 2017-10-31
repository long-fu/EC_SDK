
/**
 * ec 网络系统说明，在本文件有关全部网络配置
 * 本系统仅仅存在一个网络实体
 * 由于底层发送实体存在限制 只能在完成上次的数据发送才能发送下一帧数据 底层发送缓冲区只有(2920)字节
 */

#include "user_interface.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
#include "ip_addr.h"
#include "osapi.h"

#include "ec_io.h"

// 网络连接结构体 FALSE 表示数据不能进行发送
static bool ec_send_flag = FALSE;
static esp_tcp ec_tcp = { 0 };
static ip_addr_t ec_ip = { 0 };
static struct espconn ec_espconn = { 0 };

static ec_io_recv_callback ec_io_recv_handle = NULL;
static ec_io_notify_callback ec_io_notify_handle = NULL;

// MARK: 发送队列
// 采用循环队列来进行对数据的单条发送
// TODO: 后续可能会加上 加权值

#define EC_IO_QUEUE_SIZE 8

static int queue_count = 0;
static int queue_head = 0;
static int queue_font = 0;

struct queue_node {
	uint16 length;
	uint8 *psent;
};

static struct queue_node queue_list[EC_IO_QUEUE_SIZE];

static void ICACHE_FLASH_ATTR
init_queue()
{
	os_memset(queue_list, 0x0, sizeof(queue_list));
	queue_count = 0;
	queue_head = 0;
	queue_font = 0;
}

static bool ICACHE_FLASH_ATTR
queue_isEmty()
{
	if (queue_head == queue_font)
	{
		return TRUE;
	}
	return FALSE;
}

static int ICACHE_FLASH_ATTR
queue_push(uint8 *psent, uint16 length)
{
	int font = 0;
	if (queue_count == EC_IO_QUEUE_SIZE - 1)
	{
		// ec_log("QUEUE MAX");
		return 0;
	}

	queue_count ++ ;
	queue_font ++ ;
	font = queue_font % EC_IO_QUEUE_SIZE;
	queue_list[font].length = length;
	queue_list[font].psent = psent;
	queue_font = font;

	return 1;
}

struct queue_node * ICACHE_FLASH_ATTR
queue_pop()
{
	int head = 0;
	struct queue_node *t;
	if (queue_isEmty())
	{
		return NULL;
	}
	t = &queue_list[queue_head];
	head = queue_head;
	head ++;
	queue_count --;
	head = head % EC_IO_QUEUE_SIZE;
	queue_head = head;
	return t;
}

struct queue_node * ICACHE_FLASH_ATTR
queue_get()
{
	int head = 0;
	struct queue_node *t;
	if (queue_isEmty())
	{
		return NULL;
	}
	t = &queue_list[queue_head];
	return t;
}

static void ICACHE_FLASH_ATTR
deinit_queue()
{
	struct queue_node *t = NULL;
    while(t = queue_pop())
    {
    	os_free(t->psent);
    	t->psent = NULL;
    	t->length = 0;
    }
}

// ⽹络数据发送成功的回调函数，由 espconn_regist_sentcb 注册
static void ICACHE_FLASH_ATTR
ec_send_cb(void *arg)
{
	ec_send_flag = TRUE;
	struct espconn *esp = (struct espconn *)arg;
	if (queue_isEmty() == FALSE)
	{
		sint8 ret;
		ec_send_flag = FALSE;
		struct queue_node *t = queue_get();
		ret = espconn_send(esp, t->psent, t->length);
	    if (ret == 0)
	    {
	    	// 发送成功 删除数据
			t = queue_pop();
			os_free(t->psent);
		    t->length = 0;
		    t->psent = NULL;
	    }
	    else if (ret == ESPCONN_ARG)
	    {
	    	// 未找到参数 espconn 对应的⽹络传输
	    }
	    else if (ret == ESPCONN_MEM)
	    {
	    	// 空间不⾜
	    }
	    else if (ret == ESPCONN_MAXNUM)
	    {
	    	// 底层发包缓存已满，发包失败
	    }
	    else
	    {
	    	// FIXME: 需要要注意意外错误
	    }
	}
}

// 成功接收⽹络数据的回调函数，由 espconn_regist_recvcb 注册
static void ICACHE_FLASH_ATTR
ec_recv_cb(void *arg, char *pdata, unsigned short len)
{
	struct espconn *esp = (struct espconn *)arg;
	if (ec_io_recv_handle)
	{
		ec_io_recv_handle(pdata, len);
	}
}

// 域名解析后的回调
static void ICACHE_FLASH_ATTR
ec_dns_cb(const char *name, ip_addr_t *ipaddr, void *arg)
{
	struct espconn *esp = (struct espconn *)arg;
	if (ipaddr == NULL)
	{
		// TODO: 域名解析失败可能需要进行重连
		return;
	}

	if (ec_ip.addr == 0 && ipaddr->addr != 0)
	{
		sint8 ret = -99;
		os_memcpy(esp->proto.tcp->remote_ip, &ipaddr->addr, sizeof(ip_addr_t));
		ret = espconn_connect(esp);
		if(ret == 0)
        {
        	// 成功
        }
        else if (ret == ESPCONN_ARG)
        {
        	// 未找到参数 espconn 对应的 TCP 连接
        }
        else if (ret == ESPCONN_MEM)
        {
        	// 空间不⾜
        }
        else if (ret == ESPCONN_ISCONN)
        {
        	// 连接已经建⽴
        }
        else if (ret == ESPCONN_RTE)
        {
        	// 路由异常
        }
        else
        {
        	// FIXME: 未知错误
        }
	}
}

// TCP 连接异常断开时的回调函数，相当于出错处理回调，由 espconn_regist_reconcb 注册
static void ICACHE_FLASH_ATTR
ec_recon_cb(void *arg, sint8 errType)
{
	struct espconn *esp = (struct espconn *)arg;
	ec_send_flag = FALSE;
    // TODO: 对错误进行详细操作
    // if (errType == ESCONN_TIMEOUT)
    // {
    // 	// 超时出错断开
    // }
    // else
    if(errType == ESPCONN_ABRT)
    {
    	// TCP 连接异常断开
    }
    else if(errType == ESPCONN_RST)
    {
    	// TCP 连接复位断开
    }
    else if(errType == ESPCONN_CLSD)
    {
    	// TCP 连接在断开过程中出错，异常断开
    }
    else if(errType == ESPCONN_CONN)
    {
    	// TCP 未连接成功
    }
    else if(errType == ESPCONN_HANDSHAKE)
    {
    	// TCP SSL 握⼿失败
    }
    // else if(errType == ESPCONN_PROTO_MSG)
    // {
    // 	// SSL 应⽤数据处理异常
    // }
    else
    {
    	// FIXME: 此错误非常的严重
    }
}

// TCP 连接正常断开成功的回调函数
static void ICACHE_FLASH_ATTR
ec_discon_cb(void *arg)
{
	struct espconn *esp = (struct espconn *)arg;
	ec_send_flag = FALSE;
}

// 成功建⽴ TCP 连接的回调函数，由 espconn_regist_connectcb 注册。
static void ICACHE_FLASH_ATTR
ec_connect_cb(void *arg)
{
	sint8 ret = -99;
	struct espconn *esp = (struct espconn *)arg;
	ret = espconn_regist_disconcb(esp, ec_discon_cb);
	if (ret != 0)
	{
		// FIXME: 错误
		// TODO: 是否需要进行重连
	}
	ret = espconn_regist_recvcb(esp ,ec_recv_cb);
	if (ret != 0)
	{
		// FIXME: 错误
		// TODO: 是否需要进行重连
	}
	ret = espconn_regist_sentcb(esp ,ec_send_cb);
	if (ret != 0)
	{
		// FIXME: 错误
		// TODO: 是否需要进行重连
	}
	ec_io_notify_handle(EC_IO_CONNET, esp);
}

void *ICACHE_FLASH_ATTR
ec_io_connet(const char *host, int port)
{
	sint8 ret = -99;
	uint32 ip = 0;

	if (host == NULL || os_strlen(host) < 3)
	{
		return NULL;
	}
    ec_log("ec_io_connet \r\n");
	{
		deinit_queue();
        init_queue();

		espconn_disconnect(&ec_espconn);
        espconn_delete(&ec_espconn);

		os_memset(&ec_espconn, 0x0, sizeof(ec_espconn));
		os_memset(&ec_tcp, 0x0, sizeof(ec_tcp));
	}

	ec_espconn.type = ESPCONN_TCP;
	ec_espconn.state = ESPCONN_NONE;
	ec_espconn.proto.tcp = &ec_tcp;
    ec_espconn.proto.tcp->local_port = espconn_port();
    ec_espconn.proto.tcp->remote_port = port;

    ret = espconn_regist_connectcb(&ec_espconn, ec_connect_cb);
    if(ret == 0)
    {
    	// return &ec_espconn;
    }
    else
    {
    	// 未找到参数 espconn 对应的 TCP 连接
    	return NULL;
    }

    ret = espconn_regist_reconcb(&ec_espconn, ec_recon_cb);
    if(ret == 0)
    {
    	return &ec_espconn;
    }
    else
    {
    	// 未找到参数 espconn 对应的 TCP 连接
    	return NULL;
    }

    ec_send_flag = TRUE;

    ip = ipaddr_addr(host);
    if (ip > 0)
    {
        os_memcpy(&ec_espconn.proto.tcp->remote_ip, &ip, sizeof(uint32));
        ret = espconn_connect(&ec_espconn);
        if(ret == 0)
        {
        	return &ec_espconn;
        }
        else if (ret == ESPCONN_ARG)
        {
        	// 未找到参数 espconn 对应的 TCP 连接
        }
        else if (ret == ESPCONN_MEM)
        {
        	// 空间不⾜
        }
        else if (ret == ESPCONN_ISCONN)
        {
        	// 连接已经建⽴
            return &ec_espconn;
        }
        else if (ret == ESPCONN_RTE)
        {
        	// 路由异常
        }
        else
        {
        	// FIXME: 未知错误
        }
        return NULL;
    }
    else
    {
    	err_t et = -99;
        et = espconn_gethostbyname(&ec_espconn, host, &ec_ip, ec_dns_cb);
        if (ESPCONN_OK == et)
        {
            //成功
        	return &ec_espconn;
        }
        else if (ESPCONN_ISCONN == et)
        {
			// 已经连接
			return &ec_espconn;
        }
        else if (ESPCONN_ARG == et)
        {
        	// 未找到参数 espconn 对应的⽹络传输
        }
        else
        {
        	// FIXME: 未知错误
        }
    }
    return NULL;
}


sint8 ICACHE_FLASH_ATTR
ec_io_send (uint8 *psent, uint16 length)
{
	sint8 ret = -99;
	if (queue_isEmty() && ec_send_flag == TRUE)
	{
		ec_send_flag = FALSE;
		ret = espconn_send(&ec_espconn, psent, length);
	    if (ret == 0)
	    {
	    	// 发送成功
	    	return 1;
	    }
	    else if (ret == ESPCONN_ARG)
	    {
	    	// 未找到参数 espconn 对应的⽹络传输
	    }
	    else if (ret == ESPCONN_MEM)
	    {
	    	// 空间不⾜
	    }
	    else if (ret == ESPCONN_MAXNUM)
	    {
	    	// 底层发包缓存已满，发包失败
    		uint8 *data;
			data = os_malloc(length + 1);
			os_memset(data,0x0,length + 1);
			os_memcpy(data, psent, length);
			queue_push(data, length);
	    }
	    else
	    {
	    	// FIXME: 需要要注意意外错误
	    }
	}
	else
	{
		uint8 *data;
		data = os_malloc(length + 1);
		os_memset(data,0x0,length + 1);
		os_memcpy(data, psent, length);
		queue_push(data, length);
	}
	return ret;
}

void ICACHE_FLASH_ATTR
ec_io_close()
{
    deinit_queue();

    espconn_disconnect(&ec_espconn);
    espconn_delete(&ec_espconn);

	os_memset(&ec_espconn, 0x0, sizeof(ec_espconn));
	os_memset(&ec_tcp, 0x0, sizeof(ec_tcp));

    ec_io_recv_handle = NULL;
    ec_io_notify_handle = NULL;
}

void ICACHE_FLASH_ATTR
ec_io_regist_recvcb(ec_io_recv_callback handle)
{
	ec_io_recv_handle = handle;
}

void ICACHE_FLASH_ATTR
ec_io_regist_notifycb(ec_io_notify_callback handle)
{
	ec_io_notify_handle = handle;
}

int ICACHE_FLASH_ATTR
ec_io_reconnect()
{
    sint8 ret = -99;
	ret = espconn_connect(&ec_espconn);
	if(ret == 0)
    {
    	// 成功
    }
    else if (ret == ESPCONN_ARG)
    {
    	// 未找到参数 espconn 对应的 TCP 连接
    }
    else if (ret == ESPCONN_MEM)
    {
    	// 空间不⾜
    }
    else if (ret == ESPCONN_ISCONN)
    {
    	// 连接已经建⽴
    }
    else if (ret == ESPCONN_RTE)
    {
    	// 路由异常
    }
    else
    {
    	// FIXME: 未知错误
    }
    return ret;
}
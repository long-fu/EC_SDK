/* iksemel (XML parser for Jabber)
** Copyright (C) 2004 Gurer Ozen
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#include "common.h"
#include "iksemel.h"
#include "espconn.h"
#include "ip_addr.h"
#include "jabber_config.h"
#include "user_debug.h"

static struct espconn espconn_ptr;
static ip_addr_t espconn_ip;

#define ESPCONN_BUFFER_SIZE (2920)

static uint8 espconn_buffer[ESPCONN_BUFFER_SIZE];
static uint32 espconn_data_len = 0;
static bool espconn_flag = FALSE;

static int reconnect_status = 0;

static void ICACHE_FLASH_ATTR
on_recon_cb(void *arg, sint8 errType)
{
	struct espconn *esp = (struct espconn *)arg;
	ec_log("espconn reconnect error=%d\r\n",errType);
	espconn_flag = FALSE;
	espconn_connect(esp);
	reconnect_status = 1;
	// TODO: 标识连接错误 - 可以进行重连
}

static void ICACHE_FLASH_ATTR
on_discon_cb(void *arg)
{
	struct espconn *esp = (struct espconn *)arg;
	ec_log("espconn disconnected\r\n");
	espconn_flag = FALSE;
	reconnect_status = 0;
	espconn_connect(esp);
}

static void ICACHE_FLASH_ATTR
espconn_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
	if (ipaddr == NULL)
	{
		ec_log("DNS: Found, but got no ip, try to reconnect\r\n");
		// TODO: 标识DNS 解析失败 可以进行重连
		return;
	}

	// ec_log("DNS: found ip %d.%d.%d.%d\n",
	// 	   *((uint8 *)&ipaddr->addr),
	// 	   *((uint8 *)&ipaddr->addr + 1),
	// 	   *((uint8 *)&ipaddr->addr + 2),
	// 	   *((uint8 *)&ipaddr->addr + 3));

	os_memcpy(espconn_ptr.proto.tcp->remote_ip, &ipaddr->addr, 4);
	espconn_connect(&espconn_ptr);
	// TODO: 标识进行连接中
}

static void ICACHE_FLASH_ATTR
on_send_cb(void *arg)
{
	struct espconn *esp = (struct espconn *)arg;
	espconn_flag = TRUE;
	ec_log("on_send_cb -----\r\n");
	if (espconn_data_len)
	{
		int ret;
		ec_log("io send::%s\r\n=============================", espconn_buffer);
		ret = espconn_send(esp, espconn_buffer, espconn_data_len);
		ec_log("%d\r\n\rn", ret);
		if (ret == 0)
		{
			espconn_data_len = 0;
		}
		else if (ret == ESPCONN_MEM)
		{
			// TODO: 空间不足
		}
		else if (ret == ESPCONN_MAXNUM)
		{
			// TODO: 底层发包缓存已满，发包失败
		}
		else if (ret == ESPCONN_ARG)
		{
			// TODO: 未找到参数 espconn 对应的网络传输
		}
		// ec_log("espconn_send re %d\r\n", ret);
	}
}


static int ICACHE_FLASH_ATTR
io_send(void *socket, const char *data, size_t len)
{
	struct espconn *esp = (struct espconn *)socket;
    
	if ((data == NULL) || (len == 0))
	{
		return -1;
	}
    // ec_log("io send\r\n");
	if (espconn_flag)
	{
		int ret;
		ec_log("io send::%s\r\n===========", data);
		ret = espconn_send(&espconn_ptr, (uint8 *)data, len);
        ec_log("%d\r\n\r\n", ret);
		if (ret == 0)
		{
			espconn_flag = FALSE;
			return IKS_OK;
		}
		else if (ret == ESPCONN_MEM)
		{
			// TODO: 空间不足
		}
		else if (ret == ESPCONN_MAXNUM)
		{
			// TODO: 底层发包缓存已满，发包失败
		}
		else if (ret == ESPCONN_ARG)
		{
			// TODO: 未找到参数 espconn 对应的⽹络传输
		}
		// ec_log("espconn_send re %d\r\n", ret);
	}
	else
	{
		if (len <= (ESPCONN_BUFFER_SIZE - espconn_data_len))
		{
			ec_log("++++++++++++++++%s\r\n", data);
			os_memcpy(espconn_buffer + espconn_data_len, data, len);
			espconn_data_len += len;
		}
		else
		{
			ec_log("espconn buffer full\r\n");
			// TODO: 返回错误
		}
	}
	return IKS_OK;
}


struct stream_data
{
	iksparser *prs;
	ikstack *s;
	ikstransport *trans;
	char *name_space;
	void *user_data;
	const char *server;
	iksStreamHook *streamHook;
	iksLogHook *logHook;
	iks *current;
	char *buf;
	void *sock;
	unsigned int flags;
	char *auth_username;
	char *auth_pass;
	struct ikstls_data *tlsdata;
};
static char soc_recv_buffer[2048];
// MARK: 数据解析入口
static void ICACHE_FLASH_ATTR
on_recv(void *arg, char *pusrdata, unsigned short len)
{

	int ret;
	struct espconn *esp = (struct espconn *)arg;
	iksparser *prs = (iksparser *)esp->reverse;

	struct stream_data *data = iks_user_data (prs);
	
	if(len > NET_IO_BUF_SIZE - 1)
	{
		ec_log("io recv::buff full\r\n\r\n");	
	}
	
	os_memcpy(data->buf, pusrdata, len);

	if (len < 0) return ;
	if (len == 0) return ;

	data->buf[len] = '\0';
	
	if (data->logHook) data->logHook (data->user_data, pusrdata, len, 1);
	

	ec_log("io recv::%s\r\n\r\n", data->buf);
	
	ret = iks_parse(prs,(char *) data->buf, len, 0);

	if (ret != IKS_OK) return ;
	
	if (!data->trans) {
		/* stream hook called iks_disconnect */
		// MARK: 这里需要重连
		ec_log("disconnect\r\n");
	}
}
int keepalive = 10;
static void ICACHE_FLASH_ATTR
on_connect_cb(void *arg)
{
	int ret;
	struct espconn *esp = (struct espconn *)arg;
	iksparser *prs = (iksparser *)esp->reverse;
	ec_log("espconn connected\r\n");
	// espconn_set_opt(esp,ESPCONN_COPY);
	espconn_flag = TRUE;
	espconn_data_len = 0;

	// TODO: 标识TCP 连接成功
	espconn_regist_disconcb(esp, on_discon_cb);
	espconn_regist_recvcb(esp, on_recv);
	espconn_regist_sentcb(esp, on_send_cb);
	espconn_set_keepalive(esp, ESPCONN_KEEPIDLE, &keepalive);
	ret = espconn_set_keepalive(esp, ESPCONN_KEEPINTVL, &keepalive);
	// if ()
	// {

	// }
	ec_log("ret == %d \r\n",ret);
	// if (reconnect_status == 0)
	{
	    // TODO: 可以发送消息 这部分如果是没有断开一次连接可以不进行连接
	    iks_send_header(prs, j_config.domain);
	}
}

static void ICACHE_FLASH_ATTR
io_close(void *socket)
{
	struct espconn *esp = (struct espconn *)socket;
}


static esp_tcp tcp;
static int ICACHE_FLASH_ATTR
io_connect(iksparser *prs,
		   void **socketptr,
		   const char *server,
		   int port)
{
	int ret;

	espconn_disconnect(&espconn_ptr);
	espconn_delete(&espconn_ptr);
	
	os_memset(&espconn_ptr,0x0,sizeof(espconn_ptr));
	// espconn_ptr = (struct espconn *)os_zalloc(sizeof(struct espconn));
	espconn_ptr.type = ESPCONN_TCP;
	espconn_ptr.state = ESPCONN_NONE;
	espconn_ptr.proto.tcp = &tcp;
	espconn_ptr.proto.tcp->local_port = espconn_port();
	espconn_ptr.proto.tcp->remote_port = port;
	espconn_ptr.reverse = prs;
    // ec_log("start io connent port %d\r\n",port);
	espconn_regist_connectcb(&espconn_ptr, on_connect_cb);
	espconn_regist_reconcb(&espconn_ptr, on_recon_cb);
	if (server != NULL)
	{
		espconn_gethostbyname(&espconn_ptr, server, &espconn_ip, espconn_dns_found);
	}
	else
	{
		// ec_log("xmpp soc ip %d\r\n", j_config.ip.addr);
		os_memcpy(espconn_ptr.proto.tcp->remote_ip, &j_config.ip.addr, 4);
		espconn_connect(&espconn_ptr);
	}

	ret = espconn_connect(&espconn_ptr);
	if (ret)
	{
		// TODO: 连接成功
		*socketptr = (void*)&espconn_ptr;
		return IKS_OK;
	}
	else
	{
		// TODO: 连接失败
	}
	return IKS_OK;
}

static int ICACHE_FLASH_ATTR
io_recv(void *socket, char *buffer, size_t buf_len, int timeout)
{
    
	return 0;
}

ikstransport iks_default_transport = {
	IKS_TRANSPORT_V1,
	io_connect,
	io_send,
	io_recv,
	io_close,
	NULL};

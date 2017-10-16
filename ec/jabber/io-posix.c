/* iksemel (XML parser for Jabber)
** Copyright (C) 2004 Gurer Ozen
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#include "common.h"
#include "iksemel.h"



static struct espconn *espconn_ptr = NULL;
static ip_addr_t espconn_ip;

#define ESPCONN_BUFFER_SIZE (2920)

static uint8 espconn_buffer[AT_ESPCONN_DEMO_BUFFER_SIZE];
static uint32 espconn_data_len = 0;
static bool espconn_flag = FALSE;


static void ICACHE_FLASH_ATTR
on_recon_cb(void *arg, sint8 errType)
{
	struct espconn *esp = (struct espconn *)arg;
	ec_log("espconn reconnect\r\n");
	espconn_flag = FALSE;
	espconn_connect(esp);

	// TODO: 标识连接错误 - 可以进行重连
}

static void ICACHE_FLASH_ATTR
on_discon_cb(void *arg)
{
	struct espconn *esp = (struct espconn *)arg;
	ec_log("espconn disconnected\r\n");
	espconn_flag = FALSE;
	// espconn_connect(esp);
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

	ec_log("DNS: found ip %d.%d.%d.%d\n",
		   *((uint8 *)&ipaddr->addr),
		   *((uint8 *)&ipaddr->addr + 1),
		   *((uint8 *)&ipaddr->addr + 2),
		   *((uint8 *)&ipaddr->addr + 3));

	os_memcpy(espconn_ptr->proto.tcp->remote_ip, &ipaddr->addr, 4);
	espconn_connect(espconn_ptr);
	// TODO: 标识进行连接中
}

static void ICACHE_FLASH_ATTR
on_send_cb(void *arg)
{
	struct espconn *esp = (struct espconn *)arg;
	espconn_flag = TRUE;
	if (espconn_data_len)
	{
		int ret;
		ret = espconn_send(esp, espconn_buffer, espconn_data_len);
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
	}
}

static void ICACHE_FLASH_ATTR
on_recv(void *arg, char *pusrdata, unsigned short len)
{
	struct espconn *esp = (struct espconn *)arg;

	iksparser *prs = (iksparser *)esp->reverse;

	struct stream_data *data = iks_user_data(prs);

	if (len < 0)
		return;
	if (len == 0)
		break;
	pusrdata[len] = '\0';
	if (data->logHook)
	{
		data->logHook(data->user_data, pusrdata, len, 1);
	}
	ret = iks_parse(prs, pusrdata, len, 0);
	if (ret != IKS_OK)
	{
		return;
	}

	if (!data->trans)
	{
		/* stream hook called iks_disconnect */
		return;
	}
	// ec_log();
}

static void ICACHE_FLASH_ATTR
on_connect_cb(void *arg)
{
	struct espconn *esp = (struct espconn *)arg;
	iksparser *prs = (iksparser *)esp->reverse;
	os_printf("espconn connected\r\n");
	espconn_set_opt(esp,ESPCONN_COPY);
	espconn_flag = TRUE;
	espconn_data_len = 0;
	// TODO: 标识TCP 连接成功
	espconn_regist_disconcb(esp, on_discon_cb);
	espconn_regist_recvcb(esp, on_recv);
	espconn_regist_sentcb(esp, on_send_cb);
	
	// TODO: 可以发送消息
	iks_send_header(prs, server_name);
	
}

static void
io_close(void *socket)
{
	struct espconn *esp = (struct espconn *)arg;
}

static int
io_connect(iksparser *prs,
		   void *socketptr,
		   const char *server,
		   int port)
{
	int ret;
	espconn_ptr = (struct espconn *)os_zalloc(sizeof(struct espconn));
	espconn_ptr->type = ESPCONN_TCP;
	espconn_ptr->state = ESPCONN_NONE;
	espconn_ptr->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	espconn_ptr->proto.tcp->local_port = espconn_port();
	espconn_ptr->proto.tcp->remote_port = port;
	espconn_ptr->reverse = prs;

	espconn_regist_connectcb(espconn_ptr, on_connect_cb);
	espconn_regist_reconcb(espconn_ptr, on_recon_cb);
	if (server != NULL)
	{
		espconn_gethostbyname(espconn_ptr, server, &espconn_ip, espconn_dns_found);
	}
	else
	{
		os_memcpy(espconn_ptr->proto.tcp->remote_ip, ip, 4);
		espconn_connect(espconn_ptr);
	}

	ret = espconn_connect(espconn_ptr);
	if (ret)
	{
		// TODO: 连接成功
		socketptr = espconn_ptr;
		return IKS_OK;
	}
	else
	{
		// TODO: 连接失败
	}
	return IKS_OK;
}

static int
io_send(void *socket, const char *data, size_t len)
{
	struct espconn *esp = (struct espconn *)socket;

	if ((data == NULL) || (len == 0))
	{
		return;
	}

	if (espconn_flag)
	{
		int ret;
		ret = espconn_send(esp, (uint8 *)data, len);
		if (ret == 0)
		{
			espconn_flag = FALSE;
			return IKS_OK;
		}
		else if (ret == ESPCONN_MEM)
		{
			// TODO: 空间不⾜
		}
		else if (ret == ESPCONN_MAXNUM)
		{
			// TODO: 底层发包缓存已满，发包失败
		}
		else if (ret == ESPCONN_ARG)
		{
			// TODO: 未找到参数 espconn 对应的⽹络传输
		}
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
			// TODO: 返回错误
		}
	}
	return IKS_OK;
}

static int
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

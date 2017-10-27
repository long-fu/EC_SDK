/* iksemel (XML parser for Jabber)
** Copyright (C) 2004 Gurer Ozen
** This code is free software; you can redistribute it and/or
** modify it under the terms of GNU Lesser General Public License.
*/

#include "common.h"
#include "iksemel.h"

#include "user_interface.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"



// #include <errno.h>
#ifdef _WIN32
#include <winsock.h>
#else
// #include <netdb.h>
#include <sys/time.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
#endif

/////
#define QUEUE_MAX 8

static int queue_count = 0;
static int queue_head = 0;
static int queue_font = 0;

struct queue_node {
	int len;
	char *data;
};

static struct queue_node queue_list[QUEUE_MAX];

void ICACHE_FLASH_ATTR 
init_queue()
{
	os_memset(queue_list,0x0, sizeof(queue_list));
	queue_count = 0;
	queue_head = 0;
	queue_font = 0;
}

static int ICACHE_FLASH_ATTR
quque_isEmty()
{
	if (queue_head ==  queue_font) 
	{
		return 1;
	}
	return 0;
}

static int ICACHE_FLASH_ATTR
queue_push(char *data, int len) 
{
	int font = 0;
	if (queue_count == QUEUE_MAX - 1) 
	{
		INFO("QUEUE MAX");
		return 0;
	}
	queue_count ++ ;
	queue_font ++ ;
	font = queue_font % QUEUE_MAX;
	queue_list[font].len = len;
	queue_list[font].data = data;
	queue_font = font;
	return 1;
}

struct queue_node * ICACHE_FLASH_ATTR
queue_put() 
{
	int head = 0;
	struct queue_node *t;
	if (quque_isEmty()) 
	{
		INFO("queue is empty");
		return NULL;
	}	
	t = &queue_list[queue_head];
	head = queue_head;
	head ++;
	queue_count --;
	head = head % QUEUE_MAX;
	queue_head = head;
	return t; 
}


////

struct stream_data {
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




// TODO: 进行宏控制 对连接状态进行保存 WIFI TCP XMPP 
// WIFI:0 ~ 9 TCP:10 ~ 19 XMPP:20 ~ 29
// 10 进行TCP连接

int connect_status = -1;
ip_addr_t client_ip;
struct espconn *client_connect = NULL;


void e_tcpclient_sent_cb(void *arg);
// 域名解析回调
LOCAL void ICACHE_FLASH_ATTR
e_dns_found(const char *name, ip_addr_t *ipaddr, void *arg)
{
	struct espconn *pConn = (struct espconn *)arg;
    INFO("server name %s\r\n", name);
	// MQTT_Client* client = (MQTT_Client *)pConn->reverse;
	if (ipaddr == NULL)
	{
		INFO("DNS: Found , but got no ip, try to reconnect\r\n");
		// client->connState = TCP_RECONNECT_REQ;
		return;
	}

	INFO("DNS: found ip %d.%d.%d.%d\r\n",
	     *((uint8 *) &ipaddr->addr),
	     *((uint8 *) &ipaddr->addr + 1),
	     *((uint8 *) &ipaddr->addr + 2),
	     *((uint8 *) &ipaddr->addr + 3));

	if (client_ip.addr == 0 && ipaddr->addr != 0)
	{
		os_memcpy(client_connect->proto.tcp->remote_ip, &ipaddr->addr, 4);
		// MARK: 域名解析成功 进行连接
		INFO("TCP: connecting...\r\n");
		espconn_connect(client_connect);
	}

	
}

void ICACHE_FLASH_ATTR
e_tcpclient_delete(struct espconn * mqttClient)
{
	if (client_connect != NULL) {
		INFO("Free memory\r\n");
		espconn_delete(client_connect);
		if (client_connect->proto.tcp)
			os_free(client_connect->proto.tcp);
		os_free(client_connect);
		client_connect = NULL;
	}
}




void ICACHE_FLASH_ATTR
e_tcpclient_discon_cb(void *arg)
{


}



void ICACHE_FLASH_ATTR
e_tcpclient_recv(void *arg, char *pdata, unsigned short len)
{
	uint8_t msg_type;
	uint8_t msg_qos;
	uint16_t msg_id;
	int ret;
	struct espconn *pCon = (struct espconn *)arg;
	iksparser* prs = (iksparser *)pCon->reverse;
	struct stream_data *data = iks_user_data (prs);

	INFO("--------tcp recv start--------\r\n");
	INFO("%d:[%s] \r\n",len,pdata);
	INFO("--------tcp recv end----------\r\n");

	if (len < 0) return ;
	if (len == 0) return ;
	// data->buf[len] = '\0';
	if (data->logHook) data->logHook (data->user_data, pdata, len, 1);
	ret = iks_parse (prs, pdata, len, 0);
	if (ret != IKS_OK) return ;
	if (!data->trans) {
		/* stream hook called iks_disconnect */
		// MARK: 这里需要重连
		INFO("disconnect\r\n");
		// return IKS_NET_NOCONN;
	}
}

void ICACHE_FLASH_ATTR
e_tcpclient_connect_cb(void *arg)
{
	struct espconn *pCon = (struct espconn *)arg;
	iksparser* prs = (iksparser *)pCon->reverse;

	INFO("TCP connect success \r\n");
	espconn_regist_disconcb(pCon, e_tcpclient_discon_cb);
	espconn_regist_recvcb(pCon, e_tcpclient_recv);////////
	espconn_regist_sentcb(pCon, e_tcpclient_sent_cb);///////
	// INFO("MQTT: Connected to broker %s:%d\r\n", client->host, client->port);


	// 根据状态进行消息的发送 发送第一条消息头
	if (connect_status == 10) // 发送第一条消息
	{
		iks_send_header (prs, IKS_JABBER_DOMAIN);
	}

}

//////
static void ICACHE_FLASH_ATTR
io_close (void *socket)
{
	int sock = (int) socket;
// #ifdef _WIN32
// 	closesocket (sock);
// #else
// 	close (sock);
// #endif
}

static int ICACHE_FLASH_ATTR
io_connect (iksparser *prs, void **socketptr, const char *server, int port)
{
	INFO("TCP init ... %s:%d \r\n",server,port);

	if (client_connect) {
		e_tcpclient_delete(client_connect);
	}
    init_queue();
	
	client_connect = (struct espconn *)os_zalloc(sizeof(struct espconn));
	client_connect->type = ESPCONN_TCP;
	client_connect->state = ESPCONN_NONE;
	client_connect->proto.tcp = (esp_tcp *)os_zalloc(sizeof(esp_tcp));
	client_connect->proto.tcp->local_port = espconn_port();
	client_connect->proto.tcp->remote_port = port;
	client_connect->reverse = prs;
	//MARK: 注册连接成功的回调
	espconn_regist_connectcb(client_connect, e_tcpclient_connect_cb);
    espconn_gethostbyname(client_connect, server, &client_ip, e_dns_found);
	return IKS_OK;
}

// 1 发送成功
// 0 正在发送
static int send_isSuccess = 1;

void ICACHE_FLASH_ATTR
e_tcpclient_sent_cb(void *arg)
{
	INFO("TCP: Sent success and over #############\r\n");
	send_isSuccess = 1;
	if (quque_isEmty()) 
	{
    // TODO: 等待 不做任何处理
	INFO("send cb idel\r\n");
    } 
	else 
	{
	// TODO: 进行下一跳数据发送 
	    sint8 ret;
	 	struct queue_node *t;
		INFO("send next\r\n");

		t = queue_put();

		// INFO("send next data len  %x %d [%s] ", t, t->len, t->data);

		ret = espconn_send(client_connect,(uint8 *) t->data,(uint16) t->len);
		// INFO("\r\n--------------tcp send start-----------------\r\n");
		// INFO("\r\n%d:[%s]\r\n",t->len,t->data);
		// INFO("\r\n--------------tcp send end-----------------\r\n\r\n");
		os_free(t->data);
		t->len = 0;
		t->data = NULL;
		// INFO("#### espconn_send result %d\r\n",ret);
		if(ESPCONN_OK == ret) {
		// MARK: 发送成功
		
		// client->keepAliveTick = 0;
		// client->connState = MQTT_DATA;
		// system_os_post(MQTT_TASK_PRIO, 0, (os_param_t)client);

	
	}
	else {
		// MARK: 发送失败
		// TODO: 进行网络重连
		// client->connState = TCP_RECONNECT_DISCONNECTING;
		// system_os_post(MQTT_TASK_PRIO, 0, (os_param_t)client);
	}
	}
}

// TODO: 消息发送 
static int ICACHE_FLASH_ATTR
io_send (void *socket, const char *data, size_t len)
{
	sint8 ret = 0;
	// TODO: 实现消息队列 由于平台限制进行 必须等待上次数据发送完成才能发送下一条数据
	// char *my_data;
	// my_data = os_malloc(len);
	// os_memcpy(my_data, data, len);
	// queue_push(my_data, len)
	// INFO("io send soc empty  -- %d success --  %d\r\n ", quque_isEmty(),send_isSuccess);
	if (quque_isEmty() && send_isSuccess == 1) 
	{
		// TODO: 进行数据发送 
		send_isSuccess = 0;
		// INFO("queu is first send messsage\r\n");
		ret = espconn_send(client_connect,(uint8 *) data,(uint16) len);	
		// INFO("\r\n--------------tcp send start-----------------\r\n");
		// INFO("\r\n%d:[%s]\r\n",len,data);
		// INFO("\r\n--------------tcp send end-----------------\r\n\r\n");
		// INFO("#### espconn_send result %d\r\n",ret);
		
	
		if(ESPCONN_OK == ret) {
		// MARK: 发送成功
		// client->keepAliveTick = 0;
		// client->connState = MQTT_DATA;
		// system_os_post(MQTT_TASK_PRIO, 0, (os_param_t)client);
	    }
		else 
		{
		// MARK: 发送失败
		// TODO: 进行网络重连
		// client->connState = TCP_RECONNECT_DISCONNECTING;
		// system_os_post(MQTT_TASK_PRIO, 0, (os_param_t)client);
	    }
	} 
	else 
	{
		// TODO: 进行add_queue 
		char *my_data;
		INFO("init add queueu \r\n");
		my_data = os_malloc(len);
		os_memset(my_data,0x0,len);
		os_memcpy(my_data, data, len);
		queue_push(my_data, len);
	}

	return IKS_OK;
}

static int ICACHE_FLASH_ATTR
io_recv (void *socket, char *buffer, size_t buf_len, int timeout)
{
	// int sock = (int) socket;
	// fd_set fds;
	// struct timeval tv, *tvptr;
	// int len;

	// tv.tv_sec = 0;
	// tv.tv_usec = 0;

	// FD_ZERO (&fds);
	// FD_SET (sock, &fds);
	// tv.tv_sec = timeout;
	// if (timeout != -1) tvptr = &tv; else tvptr = NULL;
	// if (select (sock + 1, &fds, NULL, NULL, tvptr) > 0) {
	// 	do {
	// 		len = recv (sock, buffer, buf_len, 0);
	// 	} while (len == -1 && (errno == EAGAIN || errno == EINTR));
	// 	if (len > 0) {
	// 		return len;
	// 	} else if (len <= 0) {
	// 		return -1;
	// 	}
	// }
	return 0;
}

ikstransport iks_default_transport = {
	IKS_TRANSPORT_V1,
	io_connect,
	io_send,
	io_recv,
	io_close,
	NULL
};

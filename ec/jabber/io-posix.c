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

#include "user_debug.h"

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
		ec_log("QUEUE MAX");
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
		ec_log("queue is empty");
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
    ec_log("server name %s\r\n", name);
	// MQTT_Client* client = (MQTT_Client *)pConn->reverse;
	if (ipaddr == NULL)
	{
		ec_log("DNS: Found , but got no ip, try to reconnect\r\n");
		// client->connState = TCP_RECONNECT_REQ;
		return;
	}

	ec_log("DNS: found ip %d.%d.%d.%d\r\n",
	     *((uint8 *) &ipaddr->addr),
	     *((uint8 *) &ipaddr->addr + 1),
	     *((uint8 *) &ipaddr->addr + 2),
	     *((uint8 *) &ipaddr->addr + 3));

	if (client_ip.addr == 0 && ipaddr->addr != 0)
	{
		os_memcpy(client_connect->proto.tcp->remote_ip, &ipaddr->addr, 4);
		// MARK: 域名解析成功 进行连接
		ec_log("TCP: connecting...\r\n");
		espconn_connect(client_connect);
	}

	
}

void ICACHE_FLASH_ATTR
e_tcpclient_delete(struct espconn * mqttClient)
{
	if (client_connect != NULL) {
		ec_log("Free memory\r\n");
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

	ec_log("--------tcp recv start--------\r\n");
	ec_log("%d:[%s] \r\n",len,pdata);
	ec_log("--------tcp recv end----------\r\n");

	if (len < 0) return ;
	if (len == 0) return ;
	// data->buf[len] = '\0';
	if (data->logHook) data->logHook (data->user_data, pdata, len, 1);
	// 可能需要缓存数据
	ret = iks_parse (prs, pdata, len, 0);
	if (ret != IKS_OK) return ;
	if (!data->trans) {
		/* stream hook called iks_disconnect */
		// MARK: 这里需要重连
		ec_log("disconnect\r\n");
		// return IKS_NET_NOCONN;
	}
}

static void ICACHE_FLASH_ATTR
on_recon_cb(void *arg, sint8 errType)
{
	struct espconn *esp = (struct espconn *)arg;
	ec_log("espconn reconnect error=%d\r\n",errType);

	// espconn_flag = FALSE;
	// espconn_connect(esp);
	// reconnect_status = 1;
	// TODO: 标识连接错误 - 可以进行重连
}

void ICACHE_FLASH_ATTR
e_tcpclient_connect_cb(void *arg)
{
	struct espconn *pCon = (struct espconn *)arg;
	iksparser* prs = (iksparser *)pCon->reverse;

	ec_log("TCP connect success \r\n");
	espconn_regist_disconcb(pCon, e_tcpclient_discon_cb);
	espconn_regist_recvcb(pCon, e_tcpclient_recv);////////
	espconn_regist_sentcb(pCon, e_tcpclient_sent_cb);///////
	// ec_log("MQTT: Connected to broker %s:%d\r\n", client->host, client->port);


	// 根据状态进行消息的发送 发送第一条消息头
	// if (connect_status == 10) // 发送第一条消息
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
	ec_log("TCP init ... %s:%d \r\n",server,port);

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
	espconn_regist_reconcb(client_connect, on_recon_cb);
	if(server != NULL)
	{
		espconn_gethostbyname(client_connect, server, &client_ip, e_dns_found);
	}
	else
	{
		os_memcpy(client_connect->proto.tcp->remote_ip, &j_config.ip.addr, 4);
		// MARK: 域名解析成功 进行连接
		ec_log("TCP: connecting...\r\n");
		espconn_connect(client_connect);
	}
    
	return IKS_OK;
}

// 1 发送成功
// 0 正在发送
static int send_isSuccess = 1;

void ICACHE_FLASH_ATTR
e_tcpclient_sent_cb(void *arg)
{
	ec_log("TCP: Sent success and over #############\r\n");
	send_isSuccess = 1;
	if (quque_isEmty()) 
	{
    // TODO: 等待 不做任何处理
	ec_log("send cb idel\r\n");
    } 
	else 
	{
	// TODO: 进行下一跳数据发送 
	    sint8 ret;
	 	struct queue_node *t;
		

		t = queue_put();

		// ec_log("send next data len  %x %d [%s] ", t, t->len, t->data);
		ec_log("send next %s\r\n", t->data);
		ret = espconn_send(client_connect,(uint8 *) t->data,(uint16) t->len);
		// ec_log("\r\n--------------tcp send start-----------------\r\n");
		// ec_log("\r\n%d:[%s]\r\n",t->len,t->data);
		// ec_log("\r\n--------------tcp send end-----------------\r\n\r\n");
		os_free(t->data);
		t->len = 0;
		t->data = NULL;
		// ec_log("#### espconn_send result %d\r\n",ret);
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
	// ec_log("io send soc empty  -- %d success --  %d\r\n ", quque_isEmty(),send_isSuccess);
	if (quque_isEmty() && send_isSuccess == 1) 
	{
		// TODO: 进行数据发送 
		send_isSuccess = 0;
		// ec_log("queu is first send messsage\r\n");
		ret = espconn_send(client_connect,(uint8 *) data,(uint16) len);	
		// ec_log("\r\n--------------tcp send start-----------------\r\n");
		// ec_log("\r\n%d:[%s]\r\n",len,data);
		// ec_log("\r\n--------------tcp send end-----------------\r\n\r\n");
		// ec_log("#### espconn_send result %d\r\n",ret);
		
	
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
		ec_log("init add queueu \r\n");
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

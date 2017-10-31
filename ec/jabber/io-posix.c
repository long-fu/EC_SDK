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

#include <sys/time.h>
#include "ec_io.h"

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

static iksparser *io_prs = NULL;

static struct espconn *io_espconn = NULL;

static void ICACHE_FLASH_ATTR
ec_notify_cb(unsigned short status, void *arg)
{
	if(EC_IO_CONNET == status)
	{
		iks_send_header(io_prs, j_config.domain);
	}
}

static void ICACHE_FLASH_ATTR
ec_recv(char *pdata, unsigned short len)
{

	int ret;
	struct stream_data *data = iks_user_data (io_prs);

    os_memcpy(data->buf, pdata, len);

    if (len > NET_IO_BUF_SIZE - 1)
    {
    	// FIXME: 数据太大
    	return;
    }

	if (len < 0) return ;
	if (len == 0) return ;

	data->buf[len] = '\0';
    ec_log("recv:: %s\r\n\r\n",data->buf);
	if (data->logHook) data->logHook (data->user_data, pdata, len, 1);
	// 可能需要缓存数据
	ret = iks_parse (io_prs, data->buf, len, 0);
	if (ret != IKS_OK) return ;
	if (!data->trans) {
		/* stream hook called iks_disconnect */
		// MARK: 这里需要重连
		ec_log("disconnect\r\n");
	}
}

static void ICACHE_FLASH_ATTR
io_close (void *socket)
{
	io_prs = NULL;
	int sock = (int) socket;
	ec_io_close();
}

static int ICACHE_FLASH_ATTR
io_connect (iksparser *prs, void **socketptr, const char *server, int port)
{
    io_prs = prs;
    ec_log("io_connect \r\n");
    {
	    io_espconn = ec_io_connet(server, port);
	    ec_io_regist_recvcb(ec_recv);
	    ec_io_regist_notifycb(ec_notify_cb);
	    *socketptr = &io_espconn;
    }
	return IKS_OK;
}

static int ICACHE_FLASH_ATTR
io_send (void *socket, const char *data, size_t len)
{
	ec_log("send:: %s \r\n\r\n",data);
    ec_io_send ((uint8 *)data, (uint16)len);
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

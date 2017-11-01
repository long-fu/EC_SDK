#ifndef __EC_IO_H__
#define __EC_IO_H__

/// 连接成功
#define EC_IO_CONNET 1

typedef void (* ec_io_recv_callback)(char *pdata, unsigned short len);
typedef void (* ec_io_notify_callback)(unsigned short status, void *arg);

void ICACHE_FLASH_ATTR
ec_io_close();

sint8 ICACHE_FLASH_ATTR
ec_io_send (uint8 *psent, uint16 length);

void *ICACHE_FLASH_ATTR
ec_io_connet(const char *host, int port);

void ICACHE_FLASH_ATTR
ec_io_regist_recvcb(ec_io_recv_callback handle);

void ICACHE_FLASH_ATTR
ec_io_regist_notifycb(ec_io_notify_callback handle);

int ICACHE_FLASH_ATTR
ec_io_reconnect();
#endif

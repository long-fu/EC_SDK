
#ifndef __SERVER_H__
#define __SERVER_H__

#include "c_types.h"

typedef void (*server_recv_callback)(char *data, int len);
void ICACHE_FLASH_ATTR server_init(uint32 port, server_recv_callback handler);

#endif
#ifndef __USER_MUC_H__
#define __USER_MUC_H__

#include "osapi.h"

int ICACHE_FLASH_ATTR
ec_switch(int b);

int ICACHE_FLASH_ATTR
ec_get_asyncinfo(char *linkid);

int ICACHE_FLASH_ATTR
ec_get_info();

#endif
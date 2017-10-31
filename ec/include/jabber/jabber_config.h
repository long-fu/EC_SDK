#ifndef __JABBER_CONFIG_H__
#define __JABBER_CONFIG_H__

#include "ip_addr.h"

struct jabber_config
{
	int port;
	char ip[16];
	char resources[16];
	char username[16];
	char password[16];
	char app_username[16];
	char domain[16];
	char host_name[32];
};
extern struct jabber_config j_config;
bool ICACHE_FLASH_ATTR jabber_get_config(struct jabber_config * config);
bool ICACHE_FLASH_ATTR jabber_set_config(struct jabber_config * config);

#endif
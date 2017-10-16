#ifndef __JABBER_CONFIG_H__
#define __JABBER_CONFIG_H__

struct jabber_config
{
	int port;
	struct ip_addr;
	char username[16];
	char password[16];
	char app_username[16];
	char domain[16];
	char host_name[32];
};

bool jabber_get_config(struct jabber_config * config);
bool jabber_set_config(struct jabber_config * config);

#endif
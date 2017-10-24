
#ifndef __USER_JSON_H__
#define __USER_JSON_H__

#include "wifi.h"
#include "jabber_config.h"

int json_parse_config(char *json, 
    struct jabber_config *jconfig, 
    struct wifi_config *wconfig);

int json_parse_switch(char *json);
#endif


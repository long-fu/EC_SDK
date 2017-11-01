#ifndef __WIFI_H__
#define __WIFI_H__

#include "c_types.h"

struct wifi_config {
    char ssid[32];;
    char password[32];
};

extern struct wifi_config w_config;

typedef void (*WifiCallback)(uint8_t);
void ICACHE_FLASH_ATTR WIFI_Connect(uint8_t* ssid, uint8_t* pass, WifiCallback cb);
void ICACHE_FLASH_ATTR wifi_ap_set(uint8_t *ssid, uint8_t *pass);
#endif

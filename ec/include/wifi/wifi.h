#ifndef __WIFI_H__
#define __WIFI_H__

#include "c_types.h"

typedef void (*WifiCallback)(uint8_t);
void ICACHE_FLASH_ATTR WIFI_Connect(uint8_t* ssid, uint8_t* pass, WifiCallback cb);

#endif

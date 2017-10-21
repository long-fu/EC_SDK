#ifndef __WIFI_H__
#define __WIFI_H__

#include "c_types.h"

typedef void (*WifiCallback)(uint8_t);
void wifi_connect(uint8_t* ssid, uint8_t* pass, WifiCallback cb);
void wifi_ap_set(uint8_t *ssid, uint8_t *pass);
#endif

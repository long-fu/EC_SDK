
#include "wifi.h"
#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"
#include "os_type.h"
#include "mem.h"
#include "user_debug.h"
#include "user_config.h"

// TODO: 命名需要调整

static ETSTimer WiFiLinker;
WifiCallback wifiCb = NULL;
static uint8_t wifiStatus = STATION_IDLE, lastWifiStatus = STATION_IDLE;
static void ICACHE_FLASH_ATTR
wifi_check_ip(void *arg)
{
	struct ip_info ipConfig;

	os_timer_disarm(&WiFiLinker);
	wifi_get_ip_info(STATION_IF, &ipConfig);
	wifiStatus = wifi_station_get_connect_status();
	if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0)
	{

		os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
		os_timer_arm(&WiFiLinker, 2000, 0);
	}
	else
	{
		if (wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
		{

			ec_log("STATION_WRONG_PASSWORD\r\n");
			wifi_station_connect();
		}
		else if (wifi_station_get_connect_status() == STATION_NO_AP_FOUND)
		{

			ec_log("STATION_NO_AP_FOUND\r\n");
			wifi_station_connect();
		}
		else if (wifi_station_get_connect_status() == STATION_CONNECT_FAIL)
		{

			ec_log("STATION_CONNECT_FAIL\r\n");
			wifi_station_connect();
		}
		else
		{
			ec_log("STATION_IDLE\r\n");
		}

		os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
		os_timer_arm(&WiFiLinker, 500, 0);
	}
	if (wifiStatus != lastWifiStatus)
	{
		lastWifiStatus = wifiStatus;
		if (wifiCb)
			wifiCb(wifiStatus);
	}
}

void ICACHE_FLASH_ATTR
wifi_connect(uint8_t *ssid, uint8_t *pass, WifiCallback cb)
{
	struct station_config stationConf;

	ec_log("WIFI_INIT\r\n");
	wifi_set_opmode(STATION_MODE);
	os_memset(&stationConf, 0, sizeof(struct station_config));
	wifiCb = cb;
	if (ssid == NULL || pass == NULL)
	{
		wifi_station_get_config(&stationConf);
	}
	else
	{
		os_sprintf(stationConf.ssid, "%s", ssid);
		os_sprintf(stationConf.password, "%s", pass);
	}

	wifi_station_set_config(&stationConf);
	os_timer_disarm(&WiFiLinker);
	os_timer_setfn(&WiFiLinker, (os_timer_func_t *)wifi_check_ip, NULL);
	os_timer_arm(&WiFiLinker, 1000, 0);
	wifi_station_connect();
}

int ICACHE_FLASH_ATTR
get_random_string(int length, char *ouput)
{
	int flag, i;
	// srand((unsigned)time(NULL));
	for (i = 0; i < length - 1; i++)
	{
		flag = os_random() % 3;
		switch (flag)
		{
		case 0:
			ouput[i] = 'A' + os_random() % 26;
			break;
		case 1:
			ouput[i] = 'a' + os_random() % 26;
			break;
		case 2:
			ouput[i] = '0' + os_random() % 10;
			break;
		default:
			ouput[i] = 'x';
			break;
		}
	}
	return 0;
}

void ICACHE_FLASH_ATTR
wifi_ap_set(uint8_t *ssid, uint8_t *pass)
{
	char tmp_name[8] = {0};
	char wifi_name[16] = {0};
	struct softap_config config;

	get_random_string(AP_DEFINE_NAME_RAND_LEN, tmp_name);
	os_sprintf(wifi_name, "%s%s", AP_DEFINE_NAME, tmp_name);

	wifi_softap_get_config(&config);
	os_memset(config.ssid, 0, 32);
	os_memset(config.password, 0, 64);
	os_memcpy(config.ssid, wifi_name, os_strlen(wifi_name));
	os_memcpy(config.password, AP_DEFINE_PASSWORD, os_strlen(AP_DEFINE_PASSWORD));
	config.authmode = AUTH_WPA_WPA2_PSK;
	config.ssid_len = 0;
	config.beacon_interval = 100;
	config.max_connection = 4;
	wifi_softap_set_config(&config);
	wifi_set_opmode(SOFTAP_MODE);
}
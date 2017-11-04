#include "osapi.h"
#include "at_custom.h"
#include "user_interface.h"
#include "user_config.h"
#include "user_debug.h"
#include "wifi.h"
#include "user_at.h"

// #define TEXT_XMPP

char http_register_url[64];

#ifdef TEXT_XMPP
struct jabber_config x_config = {
    .port = 5222,
    .username = "18682435851",
    .password = "18682435851",
    .domain = "xsxwrd.com",
    .host_name = "gm.xsxwrd.com"
};
#endif

static os_event_t ec_task_queue[1];

static void ICACHE_FLASH_ATTR
server_recv_data(char *data, int len)
{
    char json[512] = { 0 };

    send_codec_decode(data, json);

    os_memset(&j_config, 0x0, sizeof(j_config));
    os_memset(&w_config, 0x0, sizeof(w_config));
    ec_log("debug %s\r\n", json);
    json_parse_config(json, &j_config, &w_config, http_register_url);

    system_os_post(USER_TASK_PRIO_2, SIG_ST, 0);
}

static void ICACHE_FLASH_ATTR
wifiConnectCb(uint8_t status)
{
    if (status == STATION_GOT_IP)
    {
        #ifdef TEXT_XMPP
        system_os_post(USER_TASK_PRIO_2, SIG_LG, 0);
        #else
        if (user_get_is_regisrer() == 1)
        {
            system_os_post(USER_TASK_PRIO_2, SIG_LG, 0);
        }
        else
        {
            system_os_post(USER_TASK_PRIO_2, SIG_RG, 0);
        }
        #endif
    }
    else
    {
        // TODO: 断开网络连接
    }

}

void ICACHE_FLASH_ATTR
ec_task(os_event_t *e)
{
    switch (e->sig)
    {
    case SIG_CG:
        at_port_print("\r\n--------------- ap model ---------------\r\n");
        wifi_ap_set(NULL, NULL);
        server_init(80, server_recv_data);
        break;
    case SIG_ST:
        at_port_print("\r\n--------------- station model ---------------\r\n");
        #ifdef TEXT_XMPP
        wifi_connect("JFF_2.4", "jff83224053", wifiConnectCb);
        #else
        ec_log("wifi --- %s %s ----\r\n", w_config.ssid , w_config.password);
        wifi_connect(w_config.ssid, w_config.password, wifiConnectCb);
        #endif
        break;
    case SIG_RG:
        at_port_print("\r\n--------------- register ---------------\r\n");
        http_register_jab(http_register_url, 0, j_config.app_username);
        break;
    case SIG_LG:
        at_port_print("\r\n--------------- login ---------------\r\n");
        #ifdef TEXT_XMPP
        xmpp_init(&x_config);
        #else
        xmpp_init(&j_config);
        #endif
    break;
    }
}

void ICACHE_FLASH_ATTR
system_on_done_cb(void)
{
    // MARK: 读取用户配置数据 必须在此处进行读取
    CFG_Load();
    at_port_print("system_on_done_cb\r\n");
    system_os_task(ec_task, USER_TASK_PRIO_2, ec_task_queue, 1);
#ifdef TEXT_XMPP
    system_os_post(USER_TASK_PRIO_2, SIG_ST, 0);
#else
    if (user_get_is_regisrer() == 1)
    {
        system_os_post(USER_TASK_PRIO_2, SIG_ST, 0);
    }
    else
    {
        system_os_post(USER_TASK_PRIO_2, SIG_CG, 0);
    }
#endif
}

/******************************************************************************
 * FunctionName : user_rf_cal_sector_set
 * Description  : SDK just reversed 4 sectors, used for rf init data and paramters.
 *                We add this function to force users to set rf cal sector, since
 *                we don't know which sector is free in user's application.
 *                sector map for last several sectors : ABCCC
 *                A : rf cal
 *                B : rf init data
 *                C : sdk parameters
 * Parameters   : none
 * Returns      : rf cal sector
 *******************************************************************************/
uint32 ICACHE_FLASH_ATTR
user_rf_cal_sector_set(void)
{
    enum flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;

    switch (size_map)
    {
    case FLASH_SIZE_4M_MAP_256_256:
        rf_cal_sec = 128 - 5;
        break;

    case FLASH_SIZE_8M_MAP_512_512:
        rf_cal_sec = 256 - 5;
        break;

    case FLASH_SIZE_16M_MAP_512_512:
    case FLASH_SIZE_16M_MAP_1024_1024:
        rf_cal_sec = 512 - 5;
        break;

    case FLASH_SIZE_32M_MAP_512_512:
    case FLASH_SIZE_32M_MAP_1024_1024:
        rf_cal_sec = 1024 - 5;
        break;

    case FLASH_SIZE_64M_MAP_1024_1024:
        rf_cal_sec = 2048 - 5;
        break;
    case FLASH_SIZE_128M_MAP_1024_1024:
        rf_cal_sec = 4096 - 5;
        break;
    default:
        rf_cal_sec = 0;
        break;
    }

    return rf_cal_sec;
}

void ICACHE_FLASH_ATTR
user_rf_pre_init(void)
{
    system_phy_freq_trace_enable(at_get_rf_auto_trace_from_flash());
}

void ICACHE_FLASH_ATTR
user_init(void)
{
    at_port_print("user_init\r\n");
#if AT_CUSTOM
    // MARK: 注册系统AT指令
    at_init();
    // MARK: 注册自定义AT指令
    at_cmd_array_regist(&at_custom_cmd[0], sizeof(at_custom_cmd) / sizeof(at_custom_cmd[0]));
#endif


    system_init_done_cb(system_on_done_cb);
}
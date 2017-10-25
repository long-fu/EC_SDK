
#include "osapi.h"
#include "at_custom.h"
#include "user_interface.h"
#include "user_config.h"
#include "user_debug.h"
#include "wifi.h"
#include "user_at.h"

#define SIG_CG 0 // AP模式 作为HTTP服务器 APP配置必须数据
#define SIG_ST 1 // station模式
#define SIG_RG 3 // 进行注册
#define SIG_LG 4 // 进行连接

#define S_CODE "smartswitch"

#define RIGISTER_BODY "{\"acc\":\"%s\",\
\"act\":\"%s\",\
\"reset\":\"%s\",\
\"phone\":\"%s\",\
\"sign\":\"%s\"}"

static os_event_t ec_task_queue[1];

static void ICACHE_FLASH_ATTR
http_success(char *data, int len)
{
    ec_log("HTTP SUCCESS %d: [%s]\r\n", len, data);
}

static void ICACHE_FLASH_ATTR
http_failure(int error)
{
    ec_log("HTTP ERROR %d \r\n", error);
}

void ICACHE_FLASH_ATTR
wifiConnectCb(uint8_t status)
{
    if (status == STATION_GOT_IP)
    {
        if (user_get_is_regisrer() == 1)
        {
            ec_log("===== login ==== \r\n");
            system_os_post(USER_TASK_PRIO_2, SIG_LG, NULL);
        }
        else
        {
            ec_log("===== register  ==== \r\n");
            system_os_post(USER_TASK_PRIO_2, SIG_RG, NULL);
        }
    }
    else
    {
        // TODO: 断开网络连接
    }
}
void ICACHE_FLASH_ATTR send_codec_encode(char *buff, int len, char *buff_out);
#include "user_json.h"

void ICACHE_FLASH_ATTR
server_recv_data(char *data, int len)
{
    char json[256] = {0};
    ec_log("server_recv_data [%s] \r\n", data);
    // send_codec_encode(data, os_strlen(data),json);
    ec_log("json_parse_config [%s] \r\n", json);
    // MARK: 这里解析配置数据
    // json_parse_config(json, &j_config, &w_config);
}

void ICACHE_FLASH_ATTR iks_md5(const char *data, char *buf);
static char register_body[512]  = { 0 };
static char body[256] = {0};
void ICACHE_FLASH_ATTR
http_register_jab(int re, char *appid)
{
    uint32 chipid = 0;
    char reset[4] = {0}, acc[16] = {0}, sig[64] = {0}, mdsig[128] = {0} ;
    
    chipid = system_get_chip_id();
    os_sprintf(acc, "%d", chipid);
    ec_log("http_register_jab sig acc %s\r\n", acc);
    os_sprintf(reset, "%d", re);
    os_sprintf(sig, "%s%s%s%s%s", acc, "regeditForClient", appid, reset, S_CODE);
    ec_log("http_register_jab sig %s %s\r\n", sig, acc);
    
    iks_md5(sig, mdsig);

    ec_log("http_register_jab mdsig %s \r\n", mdsig);
    
    os_memset(register_body, 0x0, sizeof(register_body));
    ec_log("http_register_jab acc acc %s\r\n" , acc);
    os_sprintf(body, RIGISTER_BODY, acc, "regeditForClient", reset, appid, mdsig);
    ec_log("http_register_jab body %s %s\r\n", body,acc);
    send_codec_encode(body, os_strlen(body), register_body);
    ec_log("http_register_jab register_body %s \r\n", register_body);
    http_request("http://52.80.97.230:9090/plugins/SmartSwitch/sev", 0, register_body, http_success, http_failure);
}

void ICACHE_FLASH_ATTR
ec_task(os_event_t *e)
{
    switch (e->sig)
    {
    case SIG_CG:
        ec_log("config ap model\r\n");
        wifi_ap_set(NULL, NULL);
        server_init(80, server_recv_data);
        break;
    case SIG_ST:
        ec_log("config station model\r\n");
        wifi_connect("JFF_2.4", "jff83224053", wifiConnectCb);
        break;
    case SIG_RG:

        ec_log(" register openfari\r\n");
        http_register_jab(0, "18627312312");

        break;
    case SIG_LG:
        ec_log("login openfair\r\n");
        xmpp_init(&j_config);
        break;
    }
}

void ICACHE_FLASH_ATTR
system_on_done_cb(void)
{
    ec_log("system_on_init_done \r\n");

    system_os_task(ec_task, USER_TASK_PRIO_2, ec_task_queue, 1);

    if (user_get_is_regisrer() == 0)
    {
        ec_log("===== start login ==== \r\n");
        system_os_post(USER_TASK_PRIO_2, SIG_ST, NULL);
    }
    else
    {
        ec_log("===== start ec sdk  ==== \r\n");
        system_os_post(USER_TASK_PRIO_2, SIG_CG, NULL);
    }
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
    ec_log("user init ok main ----\r\n");
    at_init();
#if AT_CUSTOM
// MARK: 注册系统AT指令
// at_init();
// // MARK: 注册自定义AT指令
// at_cmd_array_regist(&at_custom_cmd[0], sizeof(at_custom_cmd) / sizeof(at_custom_cmd[0]));
#endif

    // MARK: 读取用户配置数据 必须在此处进行读取
    CFG_Load();
    // timer_init();
    system_init_done_cb(system_on_done_cb);
}
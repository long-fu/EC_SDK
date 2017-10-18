
#include "osapi.h"
#include "at_custom.h"
#include "user_interface.h"
#include "user_config.h"
#include "user_debug.h"
#include "wifi.h"

#if AT_CUSTOM
/// MAKR: AT测试代码
// test :AT+TEST=1,"abc"<,3>
void ICACHE_FLASH_ATTR
at_setupCmdTest(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    char buffer[32] = { 0 };
    pPara++; // skip '='

    //get the first parameter
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    if (flag == FALSE)
    {
        at_response_error();
        return;
    }

    if (*pPara++ != ',')
    { // skip ','
        at_response_error();
        return;
    }

    ec_log("the first parameter:%d\r\n", result);
    //get the second parameter
    // string
    at_data_str_copy(buffer, &pPara, 10);
    ec_log("the second parameter:%s\r\n", buffer);

    if (*pPara == ',')
    {
        pPara++; // skip ','
        result = 0;
        //there is the third parameter
        // digit
        flag = at_get_next_int_dec(&pPara, &result, &err);
        // we donot care of flag
        ec_log("the third parameter:%d\r\n", result);
    }

    if (*pPara != '\r')
    {
        at_response_error();
        return;
    }
    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_testCmdTest(uint8_t id)
{
    ec_log("at_testCmdTest\r\n");
    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_queryCmdTest(uint8_t id)
{
    ec_log("at_queryCmdTest\r\n");
    at_response_ok();
}

void ICACHE_FLASH_ATTR
at_exeCmdTest(uint8_t id)
{
    ec_log("at_exeCmdTest\r\n");
    at_response_ok();
}

extern void at_exeCmdCiupdate(uint8_t id);
at_funcationType at_custom_cmd[] = {
    {"+TEST", 5, at_testCmdTest, at_queryCmdTest, at_setupCmdTest, at_exeCmdTest},
};
#endif

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
        // TODO: 发起网络连接
        // 1. HTTP
        // 2. XMPP
        xmpp_init();
    }
    else
    {
        // TODO: 断开网络连接
    }
}

void ICACHE_FLASH_ATTR
system_on_done_cb(void)
{
    ec_log("system_on_init_done \r\n");
    if (user_get_is_regisrer() > 0)
    {
        // TODO: 进入XMPP
        // MARK: station model 连接wifi
        ec_log("===== start login ==== \r\n");
        wifi_connect(NULL, NULL, wifiConnectCb);
    }
    else
    {
        // TODO: 完整的流程
        // 1. 获取到配置信息 http xmpp wifi
        wifi_ap_set(NULL, NULL);
        // server_init(80);
        ec_log("===== start ec sdk  ==== \r\n");
        // 2. 连接wifi
        // wifi_connect(NULL, NULL, wifiConnectCb);

        // 2. 通过http 注册xmpp
        // http_request("",0,"",http_success,http_failure);
        // 3. 连接xmpp
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
#if AT_CUSTOM
    // MARK: 注册系统AT指令
    at_init();
    // MARK: 注册自定义AT指令
    at_cmd_array_regist(&at_custom_cmd[0], sizeof(at_custom_cmd) / sizeof(at_custom_cmd[0]));
#endif
    // MARK: 读取用户配置数据 必须在此处进行读取
    CFG_Load();
    system_init_done_cb(system_on_done_cb);
}
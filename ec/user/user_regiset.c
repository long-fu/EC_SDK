

#include "osapi.h"

#include "user_interface.h"
#include "user_config.h"
#include "user_debug.h"

#define S_CODE "smartswitch"

#define RIGISTER_BODY "{\"acc\":\"%s\",\
\"act\":\"%s\",\
\"reset\":\"%s\",\
\"phone\":\"%s\",\
\"sign\":\"%s\"}"

// char http_register_url[64] = {0};

static void ICACHE_FLASH_ATTR
http_success(char *data, int len)
{
    ec_log("HTTP SUCCESS %d: [%s]\r\n", len, data);
    if (json_parse_register(data) == 0)
    {
        system_os_post(USER_TASK_PRIO_2, SIG_LG, NULL);
    }
}

static void ICACHE_FLASH_ATTR
http_failure(int error)
{
    ec_log("HTTP ERROR %d \r\n", error);
    // FIXME: 发送请求错误
}

static char register_body[512] = {0};
static char body[256] = {0};

void ICACHE_FLASH_ATTR
http_register_jab(char *url, int re, char *appid)
{
    uint32 chipid = 0;
    char reset[4] = {0}, acc[16] = {0}, sig[64] = {0}, mdsig[128] = {0};

    chipid = system_get_chip_id();
    os_sprintf(acc, "%d", chipid);
    os_sprintf(reset, "%d", re);
    os_sprintf(sig, "%s%s%s%s%s", acc, "regeditForClient", appid, reset, S_CODE);
    iks_md5(sig, mdsig);
    os_memset(register_body, 0x0, sizeof(register_body));
    os_memset(body, 0x0, sizeof(body));
    os_sprintf(body, RIGISTER_BODY, acc, "regeditForClient", reset, appid, mdsig);
    send_codec_encode(body, os_strlen(body), register_body);
    http_request(url, 0, register_body, http_success, http_failure);
}

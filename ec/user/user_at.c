#include "at_custom.h"
#include "user_config.h"
#include "user_at.h"

#include "user_debug.h"

#if AT_CUSTOM
/// MAKR: AT测试代码
// test :AT+TEST=1,"abc"<,3>
void ICACHE_FLASH_ATTR
at_test(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    char buffer[32] = {0};
    pPara++; // skip '='
    at_data_str_copy(buffer, &pPara, 16);
    at_response_ok();
    at_port_print(buffer);
}

void ICACHE_FLASH_ATTR
at_power_switch(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    char buffer[32] = {0};
    pPara++; // skip '='

    // get the first parameter
    // digit
    // flag = at_get_next_int_dec(&pPara, &result, &err);
    at_data_str_copy(buffer, &pPara, 10);
    // flag must be ture because there are more parameter
    // if (flag == FALSE)
    // {
    //     at_response_error();
    //     return;
    // }
    at_response_ok();
    ec_log("at_power_switch :%s\r\n", buffer);
}

void ICACHE_FLASH_ATTR
at_power(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    pPara++; // skip '='
    ec_log("at_power pPara %s\r\n", pPara);
    // get the first parameter
    // digit
    // TODO: 下面可以自己玩成数据解析
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    // if (flag == FALSE)
    // {
    //     at_response_error();
    //     return;
    // }
    at_response_ok();
    ec_log("at_power: %d \r\n", result);
}


void ICACHE_FLASH_ATTR
at_consumption(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    pPara++; // skip '='

    // get the first parameter
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    // if (flag == FALSE)
    // {
    //     at_response_error();
    //     return;
    // }
    at_response_ok();
    ec_log("at_consumption: %d \r\n", result);
}

void ICACHE_FLASH_ATTR
at_reconsumption(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    pPara++; // skip '='

    // get the first parameter
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    // if (flag == FALSE)
    // {
    //     at_response_error();
    //     return;
    // }
    at_response_ok();
    ec_log("at_reconsumption: %d \r\n", result);
}

void ICACHE_FLASH_ATTR
at_coo(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    pPara++; // skip '='

    // get the first parameter
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    // if (flag == FALSE)
    // {
    //     at_response_error();
    //     return;
    // }
    at_response_ok();
    ec_log("at_coo: %d \r\n", result);
}

void ICACHE_FLASH_ATTR
at_co(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    pPara++; // skip '='

    // get the first parameter
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    // if (flag == FALSE)
    // {
    //     at_response_error();
    //     return;
    // }
    at_response_ok();
    ec_log("at_co: %d \r\n", result);
}

void ICACHE_FLASH_ATTR
at_hcho(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    pPara++; // skip '='

    // get the first parameter
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    // if (flag == FALSE)
    // {
    //     at_response_error();
    //     return;
    // }
    at_response_ok();
    ec_log("at_hcho: %d \r\n", result);
}

void ICACHE_FLASH_ATTR
at_pm(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    pPara++; // skip '='

    // get the first parameter
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    // if (flag == FALSE)
    // {
    //     at_response_error();
    //     return;
    // }
    at_response_ok();
    ec_log("at_pm: %d \r\n", result);
}

void ICACHE_FLASH_ATTR
at_temp(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    pPara++; // skip '='

    // get the first parameter
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    // if (flag == FALSE)
    // {
    //     at_response_error();
    //     return;
    // }
    at_response_ok();
    ec_log("at_temp: %d \r\n", result);
}

void ICACHE_FLASH_ATTR
at_rh(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    pPara++; // skip '='

    // get the first parameter
    // digit
    flag = at_get_next_int_dec(&pPara, &result, &err);

    // flag must be ture because there are more parameter
    // if (flag == FALSE)
    // {
    //     at_response_error();
    //     return;
    // }
    at_response_ok();
    ec_log("at_rh: %d \r\n", result);
}

at_funcationType at_custom_cmd[11] = {
    {"+TEST", 5, NULL, NULL, at_test, NULL},
    {"+POWERS", 7, NULL, NULL, at_power_switch, NULL},
    {"+POWER", 6, NULL, NULL, at_power, NULL},
    {"+CONSUMPTION", 12, NULL, NULL, at_consumption, NULL},
    {"+RECONSUMPTION", 14, NULL, NULL, at_reconsumption, NULL},
    {"+CO2", 4, NULL, NULL, at_coo, NULL},
    {"+CO", 3, NULL, NULL, at_co, NULL},
    {"+HCHO", 5, NULL, NULL, at_hcho, NULL},
    {"+PM", 3, NULL, NULL, at_pm, NULL},
    {"+TEMP", 5, NULL, NULL, at_temp, NULL},
    {"+RH", 3, NULL, NULL, at_rh, NULL},
};
#endif
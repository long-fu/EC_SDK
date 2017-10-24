#include "at_custom.h"
#include "user_config.h"

#if AT_CUSTOM
/// MAKR: AT测试代码
// test :AT+TEST=1,"abc"<,3>
void ICACHE_FLASH_ATTR
at_setupCmdTest(uint8_t id, char *pPara)
{
    int result = 0, err = 0, flag = 0;
    char buffer[32] = {0};
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
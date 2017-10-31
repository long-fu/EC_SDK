#include "user_config.h"
#include "osapi.h"

int ICACHE_FLASH_ATTR
ec_switch(int b)
{
	if (b)
	{
		//MARK: 输出开的指令
		at_port_print("AT+EC_POWERS=OFF");
	}
	else
	{
		//MARK: 输出关的指令
		at_port_print("AT+EC_POWERS=ON");
	}
	return 1;
}
extern void ICACHE_FLASH_ATTR
on_asyncinfomation(int power, int totalPower, int co2, int co, 
	int pm25, int state, char *linkid);

int ICACHE_FLASH_ATTR
ec_get_asyncinfo(char *linkid)
{

    // MARK: 获取传感器数据
    // NOTE: 此数据为模拟数据
    on_asyncinfomation(20,20,20,20,20,1,linkid);
	return 1;
}

void ICACHE_FLASH_ATTR
on_infomation(int power, int totalPower, int co2, int co, int pm25, int state);

int ICACHE_FLASH_ATTR
ec_get_info()
{
    // MARK: 获取传感器数据
    // NOTE: 此数据为模拟数据
    on_infomation(20,20,20,20,20,1);
    return 0;
}
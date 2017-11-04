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
ec_get_info(void *arg)
{
    // MARK: 获取传感器数据
    // NOTE: 此数据为模拟数据
    on_infomation(20, 20, 20, 20, 20, 1);
    return 0;
}




static os_timer_t up_info_timer;
void ICACHE_FLASH_ATTR
init_info()
{
	ec_get_info(NULL);
  // os_timer_disarm(&up_info_timer);
  // os_timer_setfn(&up_info_timer, (os_timer_func_t *)ec_get_info, NULL);
  // os_timer_arm(&up_info_timer, 30000, 1); // ms 单位
}
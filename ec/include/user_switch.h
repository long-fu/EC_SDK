#ifndef __USER_SWITCH_H__
#define __USER_SWITCH_H__

#include "c_types.h"

void  ICACHE_FLASH_ATTR stop_all_user_alarm();
void  ICACHE_FLASH_ATTR add_user_alarm(int ing, int on, int hour, int min);
void  ICACHE_FLASH_ATTR add_user_delay(int on, int delay);
void  ICACHE_FLASH_ATTR  stop_all_user_delay(void);

#endif
#include "user_config.h"
#include "user_interface.h"
#include "user_debug.h"

#define TIME_S   1000
#define TIME_MIN 60 * 1000
#define TIME_HOUST 60 * 60 * 1000
#define TIME_DAY 24 * 60 * 60 * 1000

os_timer_t timer_ptr;


void ICACHE_FLASH_ATTR
timer_init(void) 
{
    ec_log("systime: %s %s", __DATE__, __TIME__);

}
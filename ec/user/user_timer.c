#include "user_config.h"
#include "user_interface.h"
#include "user_debug.h"

#define TIME_S 1000
#define TIME_MIN 60 * 1000
#define TIME_HOUST 60 * 60 * 1000
#define TIME_DAY 24 * 60 * 60 * 1000
// Alarm clock
static os_timer_t alarm_timer;

static int time_min = 0;

void ICACHE_FLASH_ATTR
timer_check_func(void *arg)
{
    time_min++;
    ec_log("time min %d\r\n", time_min);
}

void ICACHE_FLASH_ATTR
timer_init(void)
{
    ec_log("systime: %s %s\r\n", __DATE__, __TIME__);
    os_timer_disarm(&alarm_timer);
    os_timer_setfn(&alarm_timer, (os_timer_func_t *)timer_check_func, NULL);
    os_timer_arm(&alarm_timer, 60000, 1);
}
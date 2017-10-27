#ifndef __USER_TIME_H__
#define __USER_TIME_H__

#include "user_interface.h"
#include "c_types.h"

typedef uint32 time_t;

struct tm
{
  int tm_sec;   /*秒，正常范围0-59， 但允许至61*/
  int tm_min;   /*分钟，0-59*/
  int tm_hour;  /*小时， 0-23*/
  int tm_mday;  /*日，即一个月中的第几天，1-31*/
  int tm_mon;   /*月， 从一月算起，0-11 1+p->tm_mon;*/
  int tm_year;  /*年， 从1900至今已经多少年 1900＋ p->tm_year; */
  int tm_wday;  /*星期，一周中的第几天， 从星期日算起，0-6*/
  int tm_yday;  /*从今年1月1日到目前的天数，范围0-365*/
  // int tm_isdst; /*日光节约时间的旗标*/
};

uint32 ICACHE_FLASH_ATTR ec_get_timestamp();
struct tm *ICACHE_FLASH_ATTR ec_gmtime(const time_t *tim_p, struct tm *res);
time_t ICACHE_FLASH_ATTR ec_mktime(int year, int mon, int day, int hour, int min, int sec);
#endif


#include "user_config.h"
#include "user_interface.h"
#include "user_debug.h"
#include "osapi.h"


typedef uint32 time_t;

struct tm
{
    int tm_sec;  /*秒，正常范围0-59， 但允许至61*/
    int tm_min;  /*分钟，0-59*/
    int tm_hour; /*小时， 0-23*/
    int tm_mday; /*日，即一个月中的第几天，1-31*/
    int tm_mon;  /*月， 从一月算起，0-11*/  // 1+p->tm_mon;
    int tm_year;  /*年， 从1900至今已经多少年*/   // 1900＋ p->tm_year;
    int tm_wday; /*星期，一周中的第几天， 从星期日算起，0-6*/
    int tm_yday; /*从今年1月1日到目前的天数，范围0-365*/
    int tm_isdst; /*日光节约时间的旗标*/
};

#define SECSPERMIN 60L
#define MINSPERHOUR 60L
#define HOURSPERDAY 24L
#define SECSPERHOUR (SECSPERMIN * MINSPERHOUR)
#define SECSPERDAY (SECSPERHOUR * HOURSPERDAY)
#define DAYSPERWEEK 7
#define MONSPERYEAR 12

#define YEAR_BASE 1900
#define EPOCH_YEAR 1970
#define EPOCH_WDAY 4
#define EPOCH_YEARS_SINCE_LEAP 2
#define EPOCH_YEARS_SINCE_CENTURY 70
#define EPOCH_YEARS_SINCE_LEAP_CENTURY 370

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

// int __tznorth;
// int __tzyear;
char reult[100];
static const int mon_lengths[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

static const int year_lengths[2] = {
    365,
    366};

static struct tm res_buf;
static time_t system_timestam = 0;

struct tm *ICACHE_FLASH_ATTR
my_sntp_mktm_r(const time_t *tim_p, struct tm *res, int offset)
{
  long days, rem;
  time_t lcltime;
  int i;
  int y;
  int yleap;
  const int *ip;

  /* base decision about std/dst time on current time */
  lcltime = *tim_p;

  days = ((long)lcltime) / SECSPERDAY;
  rem = ((long)lcltime) % SECSPERDAY;
  while (rem < 0)
  {
    rem += SECSPERDAY;
    --days;
  }
  while (rem >= SECSPERDAY)
  {
    rem -= SECSPERDAY;
    ++days;
  }

  /* compute hour, min, and sec */
  res->tm_hour = (int)(rem / SECSPERHOUR);
  rem %= SECSPERHOUR;
  res->tm_min = (int)(rem / SECSPERMIN);
  res->tm_sec = (int)(rem % SECSPERMIN);

  /* compute day of week */
  if ((res->tm_wday = ((EPOCH_WDAY + days) % DAYSPERWEEK)) < 0)
    res->tm_wday += DAYSPERWEEK;

  /* compute year & day of year */
  y = EPOCH_YEAR;
  if (days >= 0)
  {
    for (;;)
    {
      yleap = isleap(y);
      if (days < year_lengths[yleap])
        break;
      y++;
      days -= year_lengths[yleap];
    }
  }
  else
  {
    do
    {
      --y;
      yleap = isleap(y);
      days += year_lengths[yleap];
    } while (days < 0);
  }

  res->tm_year = y - YEAR_BASE;
  res->tm_yday = days;
  ip = mon_lengths[yleap];
  for (res->tm_mon = 0; days >= ip[res->tm_mon]; ++res->tm_mon)
    days -= ip[res->tm_mon];
  res->tm_mday = days + 1;


  {

    int hours, mins, secs;
    res->tm_isdst = 0;

    // 时间偏移 8 * 60 * 60 东8 --
    // offset = (res->tm_isdst == 1 ? sntp__tzrule[1].offset : sntp__tzrule[0].offset);

    offset = -offset * SECSPERHOUR;
    hours = offset / SECSPERHOUR;
    offset = offset % SECSPERHOUR;

    mins = offset / SECSPERMIN;
    secs = offset % SECSPERMIN;

    res->tm_sec -= secs;
    res->tm_min -= mins;
    res->tm_hour -= hours;

    if (res->tm_sec >= SECSPERMIN)
    {
      res->tm_min += 1;
      res->tm_sec -= SECSPERMIN;
    }
    else if (res->tm_sec < 0)
    {
      res->tm_min -= 1;
      res->tm_sec += SECSPERMIN;
    }
    if (res->tm_min >= MINSPERHOUR)
    {
      res->tm_hour += 1;
      res->tm_min -= MINSPERHOUR;
    }
    else if (res->tm_min < 0)
    {
      res->tm_hour -= 1;
      res->tm_min += MINSPERHOUR;
    }
    if (res->tm_hour >= HOURSPERDAY)
    {
      ++res->tm_yday;
      ++res->tm_wday;
      if (res->tm_wday > 6)
        res->tm_wday = 0;
      ++res->tm_mday;
      res->tm_hour -= HOURSPERDAY;
      if (res->tm_mday > ip[res->tm_mon])
      {
        res->tm_mday -= ip[res->tm_mon];
        res->tm_mon += 1;
        if (res->tm_mon == 12)
        {
          res->tm_mon = 0;
          res->tm_year += 1;
          res->tm_yday = 0;
        }
      }
    }
    else if (res->tm_hour < 0)
    {
      res->tm_yday -= 1;
      res->tm_wday -= 1;
      if (res->tm_wday < 0)
        res->tm_wday = 6;
      res->tm_mday -= 1;
      res->tm_hour += 24;
      if (res->tm_mday == 0)
      {
        res->tm_mon -= 1;
        if (res->tm_mon < 0)
        {
          res->tm_mon = 11;
          res->tm_year -= 1;
          res->tm_yday = 365 + isleap(res->tm_year);
        }
        res->tm_mday = ip[res->tm_mon];
      }
    }
  }
  ec_log("res %d-%d-%d %d:%d:%d\r\n",1900 + res->tm_year, 
  1 + res->tm_mon,res->tm_mday, 
  res->tm_hour, res->tm_min,res->tm_sec);
  return (res);
}


static os_timer_t alarm_timer;

void ICACHE_FLASH_ATTR
timer_check_func(void *arg)
{
  system_timestam = system_timestam + 60;
  // ec_log("systime: %s %s\r\n", __DATE__, __TIME__);
  ec_log("time min %d\r\n", system_timestam);
  // ec_log("system_get_time %d\r\n", system_get_time()); // 这里是开机时间 us
  os_memset(&res_buf, 0x0, sizeof(res_buf));
  my_sntp_mktm_r(&system_timestam ,&res_buf ,8);
}

// system_get_rtc_time
// system_get_time

void al() 
{
   // 到现在是多久
   // 加上定时器
   //  
}


void ICACHE_FLASH_ATTR
timer_init(void)
{
  ec_log("systime: %s %s\r\n", __DATE__, __TIME__); // 代码编译时间
  system_timestam = 1508578384;// s 为单位
  os_timer_disarm(&alarm_timer);
  os_timer_setfn(&alarm_timer, (os_timer_func_t *)timer_check_func, NULL);
  os_timer_arm(&alarm_timer, 60000, 1); // ms 单位
}
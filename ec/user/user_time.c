#include "user_config.h"
#include "user_interface.h"
#include "user_debug.h"
#include "osapi.h"
#include "user_time.h"

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

#define SECSPEGMT8 (8 * SECSPERHOUR)

#define isleap(y) ((((y) % 4) == 0 && ((y) % 100) != 0) || ((y) % 400) == 0)

static const int mon_lengths[2][12] = {
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31},
    {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}};

static time_t mon_yday[2][12] =
    {
        {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334},
        {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335},
};

static const int year_lengths[2] = {
    365,
    366};

static os_timer_t system_time_timer;
static uint32 system_timestamp;
static struct tm system_datetime;

struct tm *ICACHE_FLASH_ATTR
ec_gmtime(const time_t *tim_p, struct tm *res)
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

    int hours, mins, secs,offset;
    // res->tm_isdst = 0;
    offset = -SECSPEGMT8;
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
  ec_log("res %d-%d-%d %d:%d:%d\r\n", 1900 + res->tm_year,
         1 + res->tm_mon, res->tm_mday,
         res->tm_hour, res->tm_min, res->tm_sec);
  return (res);
}

static long long ICACHE_FLASH_ATTR
get_day(int year)
{
  year = year - 1;
  int leap_year_num = year / 4 - year / 100 + year / 400;
  long long tol_day = year * 365 + leap_year_num;
  return tol_day;
}

time_t ICACHE_FLASH_ATTR
ec_mktime(int year, int mon, int day, int hour, int min, int sec)
{
  long long tol_day = 0;
  tol_day = get_day(year) - get_day(1970);
  tol_day += mon_yday[isleap(year)][mon - 1];
  tol_day += day - 1;

  long long ret = 0;
  ret += tol_day * SECSPERDAY;
  ret += hour * SECSPERHOUR;
  ret += min * SECSPERMIN;
  ret += sec;
  // 这部分是时区的 差值
  return ret - SECSPEGMT8;
}

void ec_set_timestamp(uint32 t)
{

}

uint32 ICACHE_FLASH_ATTR
ec_get_timestamp()
{
  // TODO: 返回时间戳
  return 0;
}



void ICACHE_FLASH_ATTR
check_time_func(void *arg)
{

  system_timestamp = system_timestamp + 60;
  ec_log("system_timestamp %d\r\n", system_timestamp);
  // ec_log("system_get_time %d\r\n", system_get_time()); // 这里是开机时间 us
  os_memset(&system_datetime, 0x0, sizeof(system_datetime));
  // my_sntp_mktm_r(&system_timestamp, &res_buf, -8);

  // 发送任务进行对 定时 和 延时 进行判断
  system_os_post(USER_TASK_PRIO_0, 0, 'a');
}

os_event_t time_task_queue;

void ICACHE_FLASH_ATTR 
al(os_event_t *e)
{
  // 到现在是多久
  // 加上定时器
  //
  ec_log("tash sldkjaskldsa\r\n");
}

void ICACHE_FLASH_ATTR
timer_init(void)
{
  ec_log("systime: %s %s\r\n", __DATE__, __TIME__); // 代码编译时间
  system_timestamp = 1508578384;                     // s 为单位
  os_timer_disarm(&system_time_timer);
  os_timer_setfn(&system_time_timer, (os_timer_func_t *)check_time_func, NULL);
  os_timer_arm(&system_time_timer, 60000, 1); // ms 单位
  system_os_task(al, USER_TASK_PRIO_0, &time_task_queue, 3);
}
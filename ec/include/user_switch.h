#ifndef __USER_SWITCH_H__
#define __USER_SWITCH_H__

#include "c_types.h"

void  stop_all_user_alarm();
void  add_user_alarm(int ing, int on, int hour, int min);
void  add_user_delay(int on, int delay);
void  stop_all_user_delay(void);

#endif
#ifndef __USER_DEBUG_H__
#define __USER_DEBUG_H__
#include "user_config.h"

#if DEBUG_ON

#ifdef HTTP_DEBUG_ON
    extern char log_buffer[512];
    ec_log(format, ...) os_sprintf(Log_Buff, format, ##__VA_ARGS__); at_port_print(Log_Buff)
#else
    ec_log(format, ...)
#endif

#ifdef XMPP_DEBUG_ON
    extern char log_buffer[512];
    ec_log(format, ...) os_sprintf(Log_Buff, format, ##__VA_ARGS__); at_port_print(Log_Buff)
#else
    ec_log(format, ...)
#endif

#ifdef ESPCONN_DEBUG_ON
    extern char log_buffer[512];
    ec_log(format, ...) os_sprintf(Log_Buff, format, ##__VA_ARGS__); at_port_print(Log_Buff)
#else
        ec_log(format, ...)
#endif

#if WIFI_DEBUG_ON
    ec_log(format, ...) os_sprintf(Log_Buff, format, ##__VA_ARGS__); at_port_print(Log_Buff)
#else
    ec_log(format, ...)
#endif

#else
ec_log(format, ...)
#endif

#endif
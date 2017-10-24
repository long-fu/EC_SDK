#ifndef __USER_DEBUG_H__
#define __USER_DEBUG_H__

#include "user_config.h"
#include "osapi.h"
#if DEBUG_ON
extern char log_buffer[512];
#ifdef HTTP_DEBUG_ON
    // extern char log_buffer[512];
#define ec_log(format, ...) os_sprintf(log_buffer, format, ##__VA_ARGS__); at_port_print(log_buffer)
#else
#define ec_log(format, ...)
#endif

#ifdef XMPP_DEBUG_ON
    // extern char log_buffer[512];
#define ec_log(format, ...) os_sprintf(log_buffer, format, ##__VA_ARGS__); at_port_print(log_buffer)
#else
#define ec_log(format, ...)
#endif

#ifdef ESPCONN_DEBUG_ON
    // extern char log_buffer[512];
#define ec_log(format, ...) os_sprintf(log_buffer, format, ##__VA_ARGS__); at_port_print(log_buffer)
#else
#define ec_log(format, ...)
#endif

#if WIFI_DEBUG_ON
#define ec_log(format, ...) os_sprintf(log_buffer, format, ##__VA_ARGS__); at_port_print(log_buffer)
#else
#define ec_log(format, ...)
#endif

#else
#define ec_log(format, ...)
#endif

#endif
#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#include "user_interface.h"
#include "jabber_config.h"
#define AP_DEFINE_NAME_RAND_LEN 5
#define AP_DEFINE_NAME "WSLP_"
#define AP_DEFINE_PASSWORD "12345678"

#define EC_VERSION "ec_v0.0.1"

/// 支持AT指令
#define AT_CUSTOM 1

/// LOG信息输出模式
#define DEBUG_ON 0

#define XMPP_DEBUG_ON 1

#define HTTP_DEBUG_ON 1

#define ESPCONN_DEBUG_ON 1

#define WIFI_DEBUG_ON 1

/// 用户信息存储起始位置
#define CFG_LOCATION 0xD0

// ec 任务
#define SIG_CG 0 // AP模式 作为HTTP服务器 APP配置必须数据
#define SIG_ST 1 // station模式
#define SIG_RG 3 // 进行注册
#define SIG_LG 4 // 进行登录连接

extern void ICACHE_FLASH_ATTR send_codec_encode(char *buff, int len, char *buff_out);
extern void ICACHE_FLASH_ATTR send_codec_decode(char *buf, char *decode_out);
extern void ICACHE_FLASH_ATTR iks_md5(const char *data, char *buf);

void ICACHE_FLASH_ATTR CFG_Save(void);
void ICACHE_FLASH_ATTR CFG_Load(void);

int ICACHE_FLASH_ATTR user_get_is_regisrer(void);

void ICACHE_FLASH_ATTR xmpp_init(struct jabber_config *config);
void ICACHE_FLASH_ATTR timer_init(uint32 t);

void ICACHE_FLASH_ATTR http_register_jab(char *url, int re, char *appid);
int ICACHE_FLASH_ATTR get_random_string(int length, char *ouput);
#endif
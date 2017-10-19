
#include "user_interface.h"
#include "user_config.h"
#include "user_debug.h"
#include "jabber_config.h"
#include "spi_flash.h"
#include "osapi.h"
#include "user_debug.h"

// #ifdef HTTP_DEBUG_ON
char log_buffer[512];
// #endif

static int user_isRegisrer;

/// 保存用户配置信息
void ICACHE_FLASH_ATTR
CFG_Save(void)
{
	// TODO: 保存用户配置信息

	spi_flash_erase_sector(CFG_LOCATION + 0);
	spi_flash_erase_sector(CFG_LOCATION + 1);

	// MARK: 保存注册信息
	// MARK: 保存XMPP配置信息
	if (user_isRegisrer == 1)
	{
		spi_flash_write((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&user_isRegisrer, sizeof(user_isRegisrer));

		spi_flash_write((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
						(uint32 *)&j_config, sizeof(j_config));
	}
}

/// 加载用户配置信息
void ICACHE_FLASH_ATTR
CFG_Load(void)
{
	// TODO: 读取用户配置信息
	user_isRegisrer = 0;
	spi_flash_read((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
				   (uint32 *)&user_isRegisrer, sizeof(user_isRegisrer));
    ec_log("\r\nload -- re   %d ---  \r\n", user_isRegisrer);
	os_memset(&j_config, 0x0, sizeof(j_config));
	spi_flash_read((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
				   (uint32 *)&j_config, sizeof(j_config));
}

/// 判断客户端是否被用户注册(也就是绑定) - 复位的时候才会发生值的一个变化
int ICACHE_FLASH_ATTR
user_get_is_regisrer(void)
{
	ec_log("user get register %d\r\n", user_isRegisrer);
	return user_isRegisrer = 1;
}
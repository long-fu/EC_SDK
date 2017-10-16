
#include "user_config.h"
#include "user_debug.h"

#ifdef HTTP_DEBUG_ON
char log_buffer[512];
#endif

// MARK: 保存用户配置信息
void ICACHE_FLASH_ATTR
CFG_Save()
{
	//  spi_flash_read((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
	//                    (uint32 *)&saveFlag, sizeof(SAVE_FLAG));

	// if (saveFlag.flag == 0) {
	// 	spi_flash_erase_sector(CFG_LOCATION + 1);
	// 	spi_flash_write((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
	// 					(uint32 *)&sysCfg, sizeof(SYSCFG));
	// 	saveFlag.flag = 1;
	// 	spi_flash_erase_sector(CFG_LOCATION + 3);
	// 	spi_flash_write((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
	// 					(uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	// } else {
	// 	spi_flash_erase_sector(CFG_LOCATION + 0);
	// 	spi_flash_write((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
	// 					(uint32 *)&sysCfg, sizeof(SYSCFG));
	// 	saveFlag.flag = 0;
	// 	spi_flash_erase_sector(CFG_LOCATION + 3);
	// 	spi_flash_write((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
	// 					(uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	// }
}

// MARK: 加载用户配置信息
void ICACHE_FLASH_ATTR
CFG_Load()
{

	// ec_log("\r\nload ...\r\n");
	// spi_flash_read((CFG_LOCATION + 3) * SPI_FLASH_SEC_SIZE,
	// 			   (uint32 *)&saveFlag, sizeof(SAVE_FLAG));
	// if (saveFlag.flag == 0) {
	// 	spi_flash_read((CFG_LOCATION + 0) * SPI_FLASH_SEC_SIZE,
	// 				   (uint32 *)&sysCfg, sizeof(SYSCFG));
	// } else {
	// 	spi_flash_read((CFG_LOCATION + 1) * SPI_FLASH_SEC_SIZE,
	// 				   (uint32 *)&sysCfg, sizeof(SYSCFG));
	// }
	// if(sysCfg.cfg_holder != CFG_HOLDER){
	// 	os_memset(&sysCfg, 0x00, sizeof sysCfg);


	// 	sysCfg.cfg_holder = CFG_HOLDER;

	// 	os_sprintf(sysCfg.device_id, MQTT_CLIENT_ID, system_get_chip_id());
	// 	sysCfg.device_id[sizeof(sysCfg.device_id) - 1] = '\0';
	// 	os_strncpy(sysCfg.sta_ssid, STA_SSID, sizeof(sysCfg.sta_ssid) - 1);
	// 	os_strncpy(sysCfg.sta_pwd, STA_PASS, sizeof(sysCfg.sta_pwd) - 1);
	// 	sysCfg.sta_type = STA_TYPE;

	// 	os_strncpy(sysCfg.mqtt_host, MQTT_HOST, sizeof(sysCfg.mqtt_host) - 1);
	// 	sysCfg.mqtt_port = MQTT_PORT;
	// 	os_strncpy(sysCfg.mqtt_user, MQTT_USER, sizeof(sysCfg.mqtt_user) - 1);
	// 	os_strncpy(sysCfg.mqtt_pass, MQTT_PASS, sizeof(sysCfg.mqtt_pass) - 1);

	// 	sysCfg.security = DEFAULT_SECURITY;	/* default non ssl */

	// 	sysCfg.mqtt_keepalive = MQTT_KEEPALIVE;

	// 	ec_log(" default configuration\r\n");

	// 	CFG_Save();
	// }

}

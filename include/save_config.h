/* save_config.h
*
*/
#ifndef SAVE_CONFIG_H
#define SAVE_CONFIG_H

#define CONFIG_LOCATION 0x3C

typedef struct{
	uint32_t timeout;
} config;

void ICACHE_FLASH_ATTR saveConfig();
void ICACHE_FLASH_ATTR loadConfig();

#endif

/*
/* save_config.c
*/
#include "user_interface.h"
#include "save_config.h"

config configuration;

void ICACHE_FLASH_ATTR saveConfig() {
    INFO("Save Config\n");
    if(!configuration.timeout) {
        configuration.timeout = 0;
    }
    spi_flash_erase_sector(CONFIG_LOCATION);
    spi_flash_write((CONFIG_LOCATION) * SPI_FLASH_SEC_SIZE,(uint32 *)&configuration, sizeof(config));
}

void ICACHE_FLASH_ATTR loadConfig(){
	INFO("Load Config\n");
    spi_flash_read((CONFIG_LOCATION) * SPI_FLASH_SEC_SIZE,(uint32 *)&configuration, sizeof(config));
}

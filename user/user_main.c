/* user_main.c -- Set a GPIO state based on MQTT message.
*
* Copyright (c) 2014-2015, Murf Mellgren (mikael@murf.se)
* All rights reserved.
*
*/
#include "ets_sys.h"
#include "osapi.h"
#include "debug.h"
#include "driver/uart_hw.h"
#include "esp_switch.h"

int GPIOS[6] = {2,12,13,14,15,16};


void ICACHE_FLASH_ATTR local_init_gpio(void) {
    int initstate = 0;
    gpio_init();
    //Set GPIOS to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
    if(NPN) {
        gpio_output_set(0, 0, 0, BIT2);
        gpio_output_set(0, 0, 0, BIT13);
    } else if(ACTIVE_LOW) {
        GPIO_OUTPUT_SET(2, 1);
        GPIO_OUTPUT_SET(13, 1);
    } else {
        GPIO_OUTPUT_SET(2, 0);
        GPIO_OUTPUT_SET(13, 0);
    }
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
}

void ICACHE_FLASH_ATTR user_init(void) {
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
    
    INFO("\nlocal_init_gpio\n");
    local_init_gpio();
    INFO("\ninit_WIFI\n");
    init_WIFI();
    INFO("\ninitmqtt\n");
    init_mqtt();
    INFO("\nReady\n");
}

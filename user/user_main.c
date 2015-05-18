/* main.c -- Set a GPIO state based on MQTT message.
*
* Copyright (c) 2014-2015, Murf Mellgren (mikael@murf.se)
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* * Redistributions of source code must retain the above copyright notice,
* this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above copyright
* notice, this list of conditions and the following disclaimer in the
* documentation and/or other materials provided with the distribution.
* * Neither the name of Redis nor the names of its contributors may be used
* to endorse or promote products derived from this software without
* specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*/
#include "ets_sys.h"
#include "osapi.h"
#include "debug.h"
#include "mqtt.h"
#include "wifi.h"
#include "gpio.h"
#include "mem.h"
#include "config.h"
#include "switch_mqtt.h"
#include "wifi_handler.h"

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
    /*
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
    
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
    // PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
    // PIN_FUNC_SELECT(PERIPHS_IO_MUX_XPD_DCDC_U, FUNC_GPIO16);
    
    //set GPIO to init state
    int i;
    for(i=0; i <= 5; i++) {
        GPIO_OUTPUT_SET(GPIOS[i],initstate);
    }
     */
}

void ICACHE_FLASH_ATTR user_init(void) {
    stdoutInit();
    wifi_set_event_handler_cb(btn_wifi_handle_event_cb);
    INFO("\nlocal_init_gpio\n");
    local_init_gpio();
    
    CFG_Load();
    os_delay_us(1000);
    INFO("\n\n");
    WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);
    INFO("\ninitmqtt\n");
    init_mqtt();
    INFO("\nReady\n");
}

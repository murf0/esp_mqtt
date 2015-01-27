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
#include "driver/uart.h"
#include "osapi.h"
#include "config.h"
#include "debug.h"
#include "mqtt.h"
#include "wifi.h"
#include "gpio.h"
#include "mem.h"
#include "ds18b20.c"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);
static volatile os_timer_t ds18b20_timer;


MQTT_Client mqttClient;
char subtopic[64];
char statustopic[64];
int GPIOS[6] = {2,12,13,14,15,16};


void ICACHE_FLASH_ATTR wifiConnectCb(uint8_t status) {
	if(status == STATION_GOT_IP){
		MQTT_Connect(&mqttClient);
	}
}

void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*)args;
    char temp[64];
    char temperature[128];
    os_sprintf(subtopic,"iot/%s/set",sysCfg.device_id);
    os_sprintf(statustopic,"iot/%s/status",sysCfg.device_id);

	MQTT_Subscribe(client, subtopic, 2);
    
    INFO("MQTT: Connected! subscribe to: %s\r\n", subtopic);
    INFO("MQTT: Connected! Statustopic to: %s\r\n", statustopic);
    os_sprintf(temp,"MQTT: Connected! subscribed to: %s\r\n", subtopic);
    MQTT_Publish(client, statustopic, temp, os_strlen(temp), 2, 0);

    if(dbg==1) INFO("TEST DS18B20\n");
    int r=do_ds18b20(2,temperature);
    MQTT_Publish(client, statustopic, temperature, os_strlen(temperature), 2, 0);
    
}

void ICACHE_FLASH_ATTR mqttDisconnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Disconnected\r\n");
}

void ICACHE_FLASH_ATTR mqttPublishedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Published\r\n");
}

void ICACHE_FLASH_ATTR mqttDataCb(uint32_t *args, const char* topic, uint32_t topic_len, const char *data, uint32_t data_len) {
	char *topicBuf = (char*)os_zalloc(topic_len+1),
			*dataBuf = (char*)os_zalloc(data_len+1);
	MQTT_Client* client = (MQTT_Client*)args;
    int i;
    char *tmp_gpios, *status, *out,*temp, *clean;
    
	os_memcpy(topicBuf, topic, topic_len);
	topicBuf[topic_len] = 0;
	os_memcpy(dataBuf, data, data_len);
	dataBuf[data_len] = 0;
    //Clean dataBuf
    clean = strstr(dataBuf,"{");
    INFO("Received on topic: %s ", topicBuf);
    INFO("Data: %s\n",clean);
    //PARSE MESSG
    /*
     STATUS:GET,GPIO02:1..
     */
    char *pch;
    pch = strtok (clean,",");
    while (pch != NULL) {
        INFO("%s \n",pch);
        if(os_strcmp(pch,"STATUS:GET")==0) {
            INFO("Send GPIOstatus to MQTT topic %s\n",statustopic);
            status="STATUS:SENT";
            for (i=0; i <= 5; i++) {
                INFO("Gpiostatus for pin: %d is %d\n",GPIOS[i],GPIO_INPUT_GET(GPIOS[i]));
                os_sprintf(status,"%s,GPIO%d:%d",status,GPIOS[i],GPIO_INPUT_GET(GPIOS[i]));
            }
            INFO("Publish status to topic: %s\n",statustopic);
            INFO("Sent string: %s\n",status);
            MQTT_Publish(client, statustopic, status, os_strlen(status), 2, 0);
        }
        //PIN2
        if(os_strcmp(pch,"GPIO2:1")==0) {
            INFO("ON\n");
            GPIO_OUTPUT_SET(2, 1);
        } else if(os_strcmp(pch,"GPIO2:0")==0) {
            INFO("OFF\n");
            GPIO_OUTPUT_SET(2, 0);
        }
        //PIN12
        if(os_strcmp(pch,"GPIO12:1")==0) {
            INFO("ON\n");
            GPIO_OUTPUT_SET(12, 1);
        } else if(os_strcmp(pch,"GPIO12:0")==0) {
            INFO("OFF\n");
            GPIO_OUTPUT_SET(12, 0);
        }
        //PIN13
        if(os_strcmp(pch,"GPIO13:1")==0) {
            INFO("ON\n");
            GPIO_OUTPUT_SET(13, 1);
        } else if(os_strcmp(pch,"GPIO13:0")==0) {
            INFO("OFF\n");
            GPIO_OUTPUT_SET(13, 0);
        }
        //PIN14
        if(os_strcmp(pch,"GPIO14:1")==0) {
            INFO("ON\n");
            GPIO_OUTPUT_SET(14, 1);
        } else if(os_strcmp(pch,"GPIO14:0")==0) {
            INFO("OFF\n");
            GPIO_OUTPUT_SET(14, 0);
        }
        //PIN15
        if(os_strcmp(pch,"GPIO15:1")==0) {
            INFO("ON\n");
            GPIO_OUTPUT_SET(15, 1);
        } else if(os_strcmp(pch,"GPIO15:0")==0) {
            INFO("OFF\n");
            GPIO_OUTPUT_SET(15, 0);
        }
        //PIN16
        if(os_strcmp(pch,"GPIO16:1")==0) {
            INFO("ON\n");
            GPIO_OUTPUT_SET(16, 1);
        } else if(os_strcmp(pch,"GPIO16:0")==0) {
            INFO("OFF\n");
            GPIO_OUTPUT_SET(16, 0);
        }
        
        pch = strtok (NULL, ",");
    }
    //Clean up.
    os_delay_us(1000);
    //delay(1);
    os_free(status);
    /*os_free(clean);
    os_free(topic);
    os_free(data);
    */
    //os_free(topicBuf);
    os_free(dataBuf);
}

void ICACHE_FLASH_ATTR local_init_gpio(void) {
    int initstate = 0;
    
    gpio_init();
    //Set GPIOS to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
    // PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
    // PIN_FUNC_SELECT(PERIPHS_IO_MUX_XPD_DCDC_U, FUNC_GPIO16);
    
    //set GPIO to init state
    int i;
    for (i=0; i <= 5; i++) {
        GPIO_OUTPUT_SET(GPIOS[i],initstate);
    }

}

//Setup some Temperature readings every 10 mins.
void ICACHE_FLASH_ATTR timer_send_temperature() {
    MQTT_Client* client = &mqttClient;
    char temperature[128];
    if(dbg==1) INFO("TEST DS18B20\n");
    int r=do_ds18b20(2,temperature);
    MQTT_Publish(client, statustopic, temperature, os_strlen(temperature), 2, 0);
}
//Do nothing function
static void ICACHE_FLASH_ATTR user_procTask(os_event_t *events) {
    os_delay_us(10);
}

void ICACHE_FLASH_ATTR user_init(void) {
    char temp[128];
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(1000000);
	CFG_Load();
    
    
    
	MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);
	//MQTT_InitConnection(&mqttClient, "192.168.11.122", 1880, 0);

	MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
	//MQTT_InitClient(&mqttClient, "client_id", "user", "pass", 120, 1);

	//MQTT_InitLWT(&mqttClient, "/lwt", "offline", 0, 0);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);
	WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);
    local_init_gpio();
    // Init timer to send DS18B20 Data
    os_timer_disarm(&ds18b20_timer);
    os_timer_setfn(&ds18b20_timer, (os_timer_func_t *)timer_send_temperature, NULL);
    os_timer_arm(&ds18b20_timer, 1000*60, 1);
    //Start os task
    //system_os_task(user_procTask, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);
    
	INFO("\r\nSystem started ...\r\n");
    

}

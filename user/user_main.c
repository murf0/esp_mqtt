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

MQTT_Client mqttClient;
char statustopic[64];
int GPIOS[6] = {2,12,13,14,15,16};
int deepsleep = 60; // Deep sleep when mqtt-msg recieved. After the timer is up ESP will wake as from boot (rst pin pulled from
int temp_sent = 0;

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void user_procTask(os_event_t *events);
static volatile os_timer_t watchdog_timer;

void ICACHE_FLASH_ATTR wifiConnectCb(uint8_t status) {
	if(status == STATION_GOT_IP){
		MQTT_Connect(&mqttClient);
	}
}

void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*)args;
    char tBuf[128];
    int temperature;
    os_sprintf(statustopic,"iot/%s/status/temperature",sysCfg.device_id);
    if(dbg==1) INFO("MQTT: Connected! Statustopic to: %s\r\n", statustopic);
    if(dbg==1) INFO("TEST DS18B20\n");
    //Subscribe to the statustopic. When message has been delivered we will deep-sleep to conserve energy
    // We do this in the mqttpublishedCB instead. using the global variable temp_sent as holder.
    //MQTT_Subscribe(client, statustopic, 0);
    
    //Send Temperature Data
    ds_get_temp(2,tBuf);
    if(dbg==1) INFO("Temperature: %s \n", tBuf);
    MQTT_Publish(client, statustopic, tBuf, os_strlen(tBuf), 2, 0);
    temp_sent=1;
    
}

void ICACHE_FLASH_ATTR mqttDisconnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Disconnected\r\n");
}

void ICACHE_FLASH_ATTR mqttPublishedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Published\r\n");
    if(sent==1) {
        uint32 runtime=system_get_time(); //us
        if(runtime > 15*1000000) {
            if(dbg==1) INFO("Runtime above 15s (%d) setting sleeptime to 45s\n",runtime/1000000);
            runtime=15*1000000;
        }
        if(dbg==1) INFO("Received on topic: %s Data: %s\n Going to Sleep for %d\n", topicBuf,dataBuf,deepsleep);
        deep_sleep_set_option(1); //Do RF_CAl. Stop the "no buf" bug? Costs a lot of current
        system_deep_sleep(1000000*deepsleep-runtime);
    }
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
    

}

void ICACHE_FLASH_ATTR user_init(void) {
    char temp[128];
	uart_init(BIT_RATE_115200, BIT_RATE_115200);
	os_delay_us(1000000);
    os_timer_disarm(&watchdog_timer);
    os_timer_setfn(&watchdog_timer, (os_timer_func_t *)system_restart, NULL);
    os_timer_arm(&watchdog_timer, 1000000*20, 1);
    
    CFG_Load();
    
	MQTT_InitConnection(&mqttClient, sysCfg.mqtt_host, sysCfg.mqtt_port, sysCfg.security);
	MQTT_InitClient(&mqttClient, sysCfg.device_id, sysCfg.mqtt_user, sysCfg.mqtt_pass, sysCfg.mqtt_keepalive, 1);
//	MQTT_InitLWT(&mqttClient, lwttopic, "offline", 0, 0);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);
    WIFI_Connect(sysCfg.sta_ssid, sysCfg.sta_pwd, wifiConnectCb);
}

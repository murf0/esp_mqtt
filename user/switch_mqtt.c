/* 
 * Adaption By Murf
*/
#include "switch_mqtt.h"

#include "ets_sys.h"
#include "osapi.h"
#include "debug.h"
#include "mqtt.h"
#include "gpio.h"
#include "mem.h"

static ETSTimer btnWiFiLinker;

MQTT_Client mqttClient;
char subtopic[64];
char statustopic[64];
MQTTCFG mqttcfg;
int ledslit=0;

void ICACHE_FLASH_ATTR wifiConnectCb(uint8_t status) {
    os_timer_disarm(&btnWiFiLinker);
    if(wifi_station_get_connect_status() == STATION_GOT_IP){
        MQTT_Connect(&mqttClient);
        INFO("****************\nTIMER: Connect MQTT\n****************\n");
    } else {
        MQTT_Disconnect(&mqttClient);
        INFO("TIMER: Disconnect MQTT\n");
        os_timer_setfn(&btnWiFiLinker, (os_timer_func_t *)wifiConnectCb, NULL);
        os_timer_arm(&btnWiFiLinker, 2000, 0);
    }
}

void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args) {
    MQTT_Client* client = (MQTT_Client*)args;
    char temp[64];
    char temperature[128];
    os_sprintf(subtopic,"iot/switch/%s/set",mqttcfg.client_id);
    os_sprintf(statustopic,"iot/switch/%s/status",mqttcfg.client_id);
    MQTT_Subscribe(client, subtopic, 2);
    INFO("MQTT: Connected! subscribe to: %s\r\n", subtopic);
    INFO("MQTT: Connected! Statustopic to: %s\r\n", statustopic);
    //os_sprintf(temp,"Switch: Connected! subscribed to: %s\r\n", subtopic);
    os_sprintf(temp,""{\"%s\":\"%s\"}", mqttcfg.client_id,subtopic);
    
    MQTT_Publish(client, statustopic, temp, os_strlen(temp), 2, 1);
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

    INFO("Received on topic: %s ", topicBuf);
    os_free(topicBuf);
    INFO("Data: %s\n",dataBuf);
    /*PARSE MESSG
     Commands available is: 
     {\"GPIO02\":\"ON\"}
     {\"GPIO02\":\"OFF\"}
     */
    if(os_strcmp(dataBuf,"{\"GPIO02\":\"ON\"}")==0) {
        INFO("ON\n");
        GPIO_OUTPUT_SET(2, 1);
    }
    if(os_strcmp(dataBuf,"{\"GPIO02\":\"OFF\"}")==0) {
        INFO("OFF\n");
        GPIO_OUTPUT_SET(2, 0);
    }
    os_free(dataBuf);
}

               
void ICACHE_FLASH_ATTR init_mqtt(void) {
    
    os_sprintf(mqttcfg.mqtt_host, "%s", "mqtt.murf.se");
    os_sprintf(mqttcfg.mqtt_user, "%s", "home_aut");
    os_sprintf(mqttcfg.mqtt_pass, "%s", "cakedoesnotwork");
    INFO("CHIPID: %08X\n",system_get_chip_id());
    os_sprintf(mqttcfg.client_id, "%08X", system_get_chip_id());
    mqttcfg.mqtt_port=8885;
    mqttcfg.mqtt_keepalive=120;
    mqttcfg.security = 1; // 1=ssl (max 1024bit certificate) 0=nonssl
    INFO("host: %s Port: %d Security: %d \n", mqttcfg.mqtt_host, mqttcfg.mqtt_port, mqttcfg.security);
	MQTT_InitConnection(&mqttClient, mqttcfg.mqtt_host, mqttcfg.mqtt_port, mqttcfg.security);
    
    INFO("ClientId: %s UserID: %s Password: %s Keepalive: %d\n ", mqttcfg.client_id, mqttcfg.mqtt_user, mqttcfg.mqtt_pass, mqttcfg.mqtt_keepalive);
	MQTT_InitClient(&mqttClient, mqttcfg.client_id, mqttcfg.mqtt_user, mqttcfg.mqtt_pass, mqttcfg.mqtt_keepalive, 1);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);
	
    //MQTT_Connect(&mqttClient);
    //Timer to make sure the MQTTclient is connected
    INFO("MQtt Arm timers?\n");
    os_timer_disarm(&btnWiFiLinker);
    os_timer_setfn(&btnWiFiLinker, (os_timer_func_t *)wifiConnectCb, NULL);
    os_timer_arm(&btnWiFiLinker, 2000, 0);
}

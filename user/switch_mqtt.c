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
int ledslit=0,ON=0;

ETSTimer btnDebounceTimer;

void ICACHE_FLASH_ATTR mqttConnectedCb(uint32_t *args) {
    MQTT_Client* client = (MQTT_Client*)args;
    char temp[128];
    char offline[64];
    os_sprintf(subtopic,"iot/switch/%s/set",mqttcfg.client_id);
    os_sprintf(statustopic,"iot/switch/%s/status",mqttcfg.client_id);
    os_sprintf(temp,CAPABILITY, mqttcfg.client_id,subtopic);
    os_sprintf(offline,"{\"%s\":{\"Adress\":\"%s\",\"Capability\":\"OFFLINE\"}}",mqttcfg.client_id,subtopic);
    MQTT_InitLWT(client,statustopic, offline, 2, 1);
    MQTT_Subscribe(client, subtopic, 2);
    INFO("MQTT: Connected! subscribe to: %s\r\n", subtopic);
    INFO("MQTT: Connected! Statustopic to: %s\r\n", statustopic);
    INFO("%s\n",temp);
    MQTT_Publish(client, statustopic, temp, os_strlen(temp), 2, 1);
}

void ICACHE_FLASH_ATTR mqttDisconnectedCb(uint32_t *args) {
	MQTT_Client* client = (MQTT_Client*)args;
	INFO("MQTT: Disconnected\r\n");
    os_timer_setfn(&btnWiFiLinker, (os_timer_func_t *)wifiConnectCb, NULL);
    os_timer_arm(&btnWiFiLinker, MQTT_RECONNECT_TIMEOUT*1000, 0);
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
    char *tmp_gpios, *status, *out;
    char temp[128];
    
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
        if(NPN) {
            // For the NPN transistor, Set to LOW (0V) to source current turning on the BC3906 NPN
            gpio_output_set(0, BIT2, BIT2, 0);
            INFO("ON - NPN\n");
        } else if(ACTIVE_LOW) {
            // GPIO set to LOW, sourcing current. 0V
            GPIO_OUTPUT_SET(2, 0);
            INFO("ON - ACTIVE_LOW\n");
        } else {
            // Setting to high (3.3V) , Logical for ON..
            GPIO_OUTPUT_SET(2, 1);
            INFO("ON - NORMAL\n");
        }
        os_sprintf(temp,"{\"%s\":{\"GPIO02\":\"ON\"}}", mqttcfg.client_id);
        MQTT_Publish(client, statustopic, temp, os_strlen(temp), 2, 0);
    }
    if(os_strcmp(dataBuf,"{\"GPIO02\":\"OFF\"}")==0) {
        if(NPN) {
            // For the NPN transistor, Set to FLOATING to stop source current turning off the BC3906 NPN, if is set to HIGH (3.3V) the transistor will not turn off.
            gpio_output_set(0, 0, 0, BIT2);
            INFO("OFF - NPN\n");
        } else if(ACTIVE_LOW) {
            // Setting to high (3.3V). providing current
            GPIO_OUTPUT_SET(2, 1);
            INFO("OFF - ACTIVE_LOW\n");
        } else {
            // GPIO set to LOW, sourcing current. 0V
            GPIO_OUTPUT_SET(2, 0);
            INFO("OFF - NORMAL\n");
        }
        os_sprintf(temp,"{\"%s\":{\"GPIO02\":\"OFF\"}}", mqttcfg.client_id);
        MQTT_Publish(client, statustopic, temp, os_strlen(temp), 2, 0);
    }
    
    if(os_strcmp(dataBuf,"{\"GPIO13\":\"ON\"}")==0) {
        if(NPN) {
            // For the NPN transistor, Set to LOW (0V) to source current turning on the BC3906 NPN
            gpio_output_set(0, BIT13, BIT13, 0);
            INFO("ON - NPN\n");
        } else if(ACTIVE_LOW) {
            // GPIO set to LOW, sourcing current. 0V
            GPIO_OUTPUT_SET(13, 0);
            INFO("ON - ACTIVE_LOW\n");
        } else {
            // Setting to high (3.3V) , Logical for ON..
            GPIO_OUTPUT_SET(13, 1);
            INFO("ON - NORMAL\n");
        }
        os_sprintf(temp,"{\"%s\":{\"GPIO13\":\"ON\"}}", mqttcfg.client_id);
        MQTT_Publish(client, statustopic, temp, os_strlen(temp), 2, 0);
    }
    if(os_strcmp(dataBuf,"{\"GPIO13\":\"OFF\"}")==0) {
        if(NPN) {
            // For the NPN transistor, Set to FLOATING to stop source current turning off the BC3906 NPN, if is set to HIGH (3.3V) the transistor will not turn off.
            gpio_output_set(0, 0, 0, BIT13);
            INFO("OFF - NPN\n");
        } else if(ACTIVE_LOW) {
            // Setting to high (3.3V). providing current
            GPIO_OUTPUT_SET(13, 1);
            INFO("OFF - ACTIVE_LOW\n");
        } else {
            // GPIO set to LOW, sourcing current. 0V
            GPIO_OUTPUT_SET(13, 0);
            INFO("OFF - NORMAL\n");
        }
        os_sprintf(temp,"{\"%s\":{\"GPIO13\":\"OFF\"}}", mqttcfg.client_id);
        MQTT_Publish(client, statustopic, temp, os_strlen(temp), 2, 0);
    }

    os_free(dataBuf);
}

void btnDebounceCb() {
    char temp[128];
    if(!GPIO_INPUT_GET(BTNGPIO)) {
        INFO("****************\nTIMER: CHECKDEBOUNCE BTN0 Pressed\n****************\n");
        //DO STUFF!
        if(ON==0) {
            if(NPN) {
                // For the NPN transistor, Set to LOW (0V) to source current turning on the BC3906 NPN
                gpio_output_set(0, BIT2, BIT2, 0);
                INFO("ON - NPN\n");
            } else if(ACTIVE_LOW) {
                // GPIO set to LOW, sourcing current. 0V
                GPIO_OUTPUT_SET(2, 0);
                INFO("ON - ACTIVE_LOW\n");
            } else {
                // Setting to high (3.3V) , Logical for ON..
                GPIO_OUTPUT_SET(2, 1);
                INFO("ON - NORMAL\n");
            }
            os_sprintf(temp,"{\"%s\":{\"GPIO02\":\"ON\"}}", mqttcfg.client_id);
            MQTT_Publish(&mqttClient, statustopic, temp, os_strlen(temp), 2, 0);
            ON=1;
        } else {
            if(NPN) {
                // For the NPN transistor, Set to FLOATING to stop source current turning off the BC3906 NPN, if is set to HIGH (3.3V) the transistor will not turn off.
                gpio_output_set(0, 0, 0, BIT2);
                INFO("OFF - NPN\n");
            } else if(ACTIVE_LOW) {
                // Setting to high (3.3V). providing current
                GPIO_OUTPUT_SET(2, 1);
                INFO("OFF - ACTIVE_LOW\n");
            } else {
                // GPIO set to LOW, sourcing current. 0V
                GPIO_OUTPUT_SET(2, 0);
                INFO("OFF - NORMAL\n");
            }
            os_sprintf(temp,"{\"%s\":{\"GPIO02\":\"OFF\"}}", mqttcfg.client_id);
            MQTT_Publish(&mqttClient, statustopic, temp, os_strlen(temp), 2, 0);
            ON=0;
        }
        
        //Done Stuff
        os_timer_disarm(&btnDebounceTimer);
        os_delay_us(1000000);
        os_timer_setfn(&btnDebounceTimer, (os_timer_func_t *)btnDebounceCb, NULL);
        os_timer_arm(&btnDebounceTimer, 100, 1);
    }
}
void wifiConnectCb(uint8_t status) {
    /*if(status == STATION_GOT_IP){
        MQTT_Connect(&mqttClient);
    } else {
        MQTT_Disconnect(&mqttClient);
    }
     */
}
void ICACHE_FLASH_ATTR init_mqtt(void) {
    
    os_sprintf(mqttcfg.mqtt_host, "%s", MQTT_HOST);
    os_sprintf(mqttcfg.mqtt_user, "%s", MQTT_USER);
    os_sprintf(mqttcfg.mqtt_pass, "%s", MQTT_PASS);
    INFO("CHIPID: %08X\n",system_get_chip_id());
    os_sprintf(mqttcfg.client_id, MQTT_CLIENT_ID, system_get_chip_id());
    mqttcfg.mqtt_port=MQTT_PORT;
    mqttcfg.mqtt_keepalive=MQTT_KEEPALIVE;
    mqttcfg.security = DEFAULT_SECURITY;
    INFO("host: %s Port: %d Security: %d \n", mqttcfg.mqtt_host, mqttcfg.mqtt_port, mqttcfg.security);
	MQTT_InitConnection(&mqttClient, mqttcfg.mqtt_host, mqttcfg.mqtt_port, mqttcfg.security);
    
    INFO("ClientId: %s UserID: %s Password: %s Keepalive: %d\n ", mqttcfg.client_id, mqttcfg.mqtt_user, mqttcfg.mqtt_pass, mqttcfg.mqtt_keepalive);
	MQTT_InitClient(&mqttClient, mqttcfg.client_id, mqttcfg.mqtt_user, mqttcfg.mqtt_pass, mqttcfg.mqtt_keepalive, 1);
	MQTT_OnConnected(&mqttClient, mqttConnectedCb);
	MQTT_OnDisconnected(&mqttClient, mqttDisconnectedCb);
	MQTT_OnPublished(&mqttClient, mqttPublishedCb);
	MQTT_OnData(&mqttClient, mqttDataCb);

    // Do button
    INFO("Button Arm timer\n");
    os_timer_disarm(&btnDebounceTimer);
    os_timer_setfn(&btnDebounceTimer, (os_timer_func_t *)btnDebounceCb, NULL);
    os_timer_arm(&btnDebounceTimer, 100, 1);
}

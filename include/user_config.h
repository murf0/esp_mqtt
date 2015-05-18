#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#define CFG_HOLDER	0x00FF55A4	/* Change this value to load default configurations */
#define CFG_LOCATION	0x2C	/* Please don't change or if you know what you doing */
#define CLIENT_SSL_ENABLE

/*DEFAULT CONFIGURATIONS*/

#define MQTT_HOST			"mqtt.murf.se" //or "mqtt.yourdomain.com"
#define MQTT_PORT			8885

#define MQTT_USER			"home_aut"
#define MQTT_PASS			"cakedoesnotwork"
#define MQTT_CLIENT_ID		"%08X"
#define MQTT_RECONNECT_TIMEOUT 	5	/*second*/
#define MQTT_BUF_SIZE		1024
#define MQTT_KEEPALIVE		120	 /*second*/

#define DEFAULT_SECURITY	1 /*1=SSL 0=None*/
#define QUEUE_BUFFER_SIZE		 		2048

#define PROTOCOL_NAMEv31	/*MQTT version 3.1 compatible with Mosquitto v0.15*/
//#define PROTOCOL_NAMEv311			/*MQTT version 3.11 compatible with https://eclipse.org/paho/clients/testing/*/



/* HOME */

#define STA_SSID "Murf_2"
#define STA_PASS "jagharenlitenkatt"

/* Caravan */
/*
#define STA_SSID "trackerd"
#define STA_PASS "tinycartrackrd"
*/
#define STA_TYPE AUTH_WPA2_PSK

/*Set up debug messages*/
#define dbg 1

/*Set up type of switch only 1 should be true Normal is Active High and Inactive Low*/
/*ACTIVE is active low and inactive High (Eg, Relay boards from Ebay)*/
#define ACTIVE_LOW false
/*NPN is active low and inactive Floating*/
#define NPN true

/*#define CAPABILITY "{\"%s\":{\"Adress\":\"%s\",\"Capability\":{\"GPIO02\": \"true\",\"GPIO13\": \"true\"}}}"*/
#define CAPABILITY "{\"%s\":{\"Adress\":\"%s\",\"Capability\":{\"GPIO02\": \"true\"}}}" 

#endif

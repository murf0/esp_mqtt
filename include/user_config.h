#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_

#define CFG_HOLDER	0x00FF55A4	/* Change this value to load default configurations */
#define CFG_LOCATION	0x3C	/* Please don't change or if you know what you doing */
#define CLIENT_SSL_ENABLE

/*DEFAULT CONFIGURATIONS*/

#define MQTT_HOST			"192.168.0.117" //or "mqtt.yourdomain.com"
#define MQTT_PORT			1883
#define MQTT_BUF_SIZE		1024
#define MQTT_KEEPALIVE		120	 /*second*/

#define MQTT_CLIENT_ID		"DVES_%08X"
#define MQTT_ROOT_TOPIC     "iot/power/"
#define MQTT_USER			"mqtt"
#define MQTT_PASS			"LongTime"

#define STA_SSID "Murf_2"
#define STA_PASS "jagharenlitenkatt"
#define STA_TYPE AUTH_WPA2_PSK

#define MQTT_RECONNECT_TIMEOUT 	5	/*second*/

#define DEFAULT_SECURITY	0 /*1=SSL 0=None*/
#define QUEUE_BUFFER_SIZE		 		2048
#endif

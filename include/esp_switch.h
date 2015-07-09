#ifndef _SWITCH_CONFIG_H_
#define _SWITCH_CONFIG_H_

#define STA_TYPE AUTH_WPA2_PSK
#define STA_SSID "Murf_2"
#define STA_PASS "jagharenlitenkatt"

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

/* 
 * Adaption By Murf
*/
#include "switch_mqtt.h"
#include "ets_sys.h"
#include "debug.h"
#include "osapi.h"
#include "esp_switch.h"

void btn_wifi_handle_event_cb(System_Event_t *evt){
    INFO("event %x\n", evt->event);
    switch (evt->event) {
        case EVENT_STAMODE_CONNECTED:
            INFO("connect to ssid %s, channel %d\n",
                      evt->event_info.connected.ssid,
                      evt->event_info.connected.channel);
            break;
        case EVENT_STAMODE_DISCONNECTED:
            INFO("disconnect from ssid %s, reason %d\n",
                      evt->event_info.disconnected.ssid,
                      evt->event_info.disconnected.reason);
            MQTT_Disconnect(&mqttClient);
            break;
        case EVENT_STAMODE_AUTHMODE_CHANGE:
            INFO("mode: %d -> %d\n",
                      evt->event_info.auth_change.old_mode,
                      evt->event_info.auth_change.new_mode);
            break;
        case EVENT_STAMODE_GOT_IP:
            INFO("ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
                      IP2STR(&evt->event_info.got_ip.ip),
                      IP2STR(&evt->event_info.got_ip.mask),
                      IP2STR(&evt->event_info.got_ip.gw));
            os_printf("\n");
             MQTT_Connect(&mqttClient);
            break;
        case EVENT_SOFTAPMODE_STACONNECTED:
            INFO("station: " MACSTR "join, AID = %d\n",
                      MAC2STR(evt->event_info.sta_connected.mac),
                      evt->event_info.sta_connected.aid);
            break;
        case EVENT_SOFTAPMODE_STADISCONNECTED:
            INFO("station: " MACSTR "leave, AID = %d\n",
                      MAC2STR(evt->event_info.sta_disconnected.mac),
                      evt->event_info.sta_disconnected.aid);
            break;
        default:
        break;
    }
}
void ICACHE_FLASH_ATTR init_WIFI(){
    struct station_config stationConf;
    wifi_set_event_handler_cb(btn_wifi_handle_event_cb);
    INFO("init_WIFI\n");
    wifi_set_opmode(STATION_MODE);
    wifi_station_set_auto_connect(FALSE);
    os_sprintf(stationConf.ssid, "%s", STA_SSID);
    os_sprintf(stationConf.password, "%s", STA_PASS);
    wifi_station_set_config(&stationConf);
    wifi_station_set_auto_connect(TRUE);
    wifi_station_connect();
}

#include <stdio.h>
#include "esp_mqtt_interface.h"
#include "esp_wifi_interface.h"

#define DEVICE_ID "esp32"
#define BROKER_URI "mqtt://esp32:esp32123@coiiote.com:1883" // Exemplo de URI do broker MQTT

#define LED_STATUS 2

void app_main(void)
{

    esp_wifi_interface_config_t wifi_inteface_config = {
        .channel = 1, // Access point channel
        .esp_max_retry = 10, // Maximum number of retries to connect to the AP         
        .wifi_sae_mode = WPA3_SAE_PWE_BOTH, // SAE mode for WPA3
        .esp_wifi_scan_auth_mode_treshold = WIFI_AUTH_WPA_WPA2_PSK, // Authentication mode threshold for Wi-Fi scan
        .status_io = LED_STATUS,  // Connection status. 
        .reset_io = 0,           // Reset pin.
    };
    
    WiFiInit (&wifi_inteface_config);

    WiFiSimpleConnection();

    mqtt_protocol_config_t client_mqtt = {
        .uri = BROKER_URI, //"mqtt://192.168.1.5:1883",//",//"mqtt://192.168.3.23:1883",
        .id = DEVICE_ID,
    };
    
    
    MQTT_connection_init(&client_mqtt);

}

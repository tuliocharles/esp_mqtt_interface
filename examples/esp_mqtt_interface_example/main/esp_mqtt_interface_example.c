#include <stdio.h>
#include "esp_mqtt_interface.h"
#include "esp_wifi_interface.h"

#define DEVICE_ID            "DEVICE_ID"
#define MQTT_BROKER_HOST     "MQTT_BROKER_HOST.com"
#define MQTT_BROKER_PORT     1883
#define MQTT_USERNAME        "MQTT_USERNAME"
#define MQTT_PASSWORD        "MQTT_PASSWORD"
#define LED_STATUS 2

static void evento_mqtt(uint32_t received_id,  const char *topic,  const char *data){
    
    switch((esp_mqtt_event_id_t)received_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI("MQTT_EVENT", "Conectado ao broker MQTT");
            esp_mqtt_interface_subscribe("tuliocharlescarvalho/Teste_Thing_Name/Time/response", 0); // Inscreve-se no tópico para receber mensagens
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI("MQTT_EVENT", "Desconectado do broker MQTT");
            break;
        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI("MQTT_EVENT", "Inscrito no tópico: %s", topic);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI("MQTT_EVENT", "Desinscrito do tópico: %s", topic);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI("MQTT_EVENT", "Publicado no tópico: %s", topic);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI("MQTT_EVENT", "Dados recebidos no tópico: %s, Dados: %s", topic, data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE("MQTT_EVENT", "Erro no evento MQTT");
            break;
        default:
            ESP_LOGI("MQTT_EVENT", "Outro evento: %lu", received_id);
    }

}
           

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

    esp_mqtt_interface_config_t client_mqtt = {
        .host = MQTT_BROKER_HOST, 
        .port = MQTT_BROKER_PORT, 
        .username = MQTT_USERNAME, 
        .password = MQTT_PASSWORD, 
        .id = DEVICE_ID,
    };

    esp_mqtt_interface_init(&client_mqtt);
    
    esp_mqtt_interface_register_cb(evento_mqtt); 

    uint64_t cont = 0; 
    
    while(1){
        cont++;     
        char topic[100];
        snprintf(topic, sizeof(topic), "topic_name"); 
        char data[50];
        snprintf(data, sizeof(data), "%llu seconds", cont); 
        esp_mqtt_interface_publish(topic, data, 0, 0); 
        vTaskDelay(pdMS_TO_TICKS(1000));     
    }


}

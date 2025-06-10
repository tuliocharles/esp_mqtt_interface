#ifndef _esp_mqtt_interface_H_
#define _esp_mqtt_interface_H_

#include "mqtt_client.h"
#include "freertos/queue.h"

typedef struct esp_mqtt_interface_t *esp_mqtt_interface_handle_t;

typedef struct {
    char *topic;  // Ponteiro para o tópico
    char *data;   // Ponteiro para os dados
} mqtt_received_message_t;

typedef struct{
    char *uri;
    char *id;
    
} mqtt_protocol_config_t;

void MQTT_connection_init(mqtt_protocol_config_t *mqtt_protocol_config);

void mqtt_protocol_change_uri (esp_mqtt_client_handle_t client, const char *uri);

#endif

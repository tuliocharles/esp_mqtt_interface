#include <stdio.h>
#include "esp_mqtt_interface.h"

#include "mqtt_client.h"
#include <stdio.h>

#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE // definição necessária para aumentar o nível de verbosidade -- definido antes de esp_log.h
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"

typedef struct esp_mqtt_interface_t esp_mqtt_interface_t;

struct esp_mqtt_interface_t{
    char *id;
    char uri[128];
    esp_mqtt_client_handle_t client;
    esp_mqtt_interface_cb_t s_callback; // Callback para eventos MQTT
};

static esp_mqtt_interface_handle_t esp_mqtt_interface_handle = NULL;

static const char  *TAGMQTT = "MQTTProtocol";

/***************************************/
/*DEFINE O NÍVEL DE ESP LOG PARA DEBUG*/
/*************************************/
#define ESP_LEVEL ESP_LOG_INFO //ESP_LOG_INF ESP_LOG_WARN ESP_LOG_ERROR
/************************************/

//static esp_mqtt_client_handle_t     client = NULL;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAGMQTT, "Last error %s: 0x%x", message, error_code);
    }
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    //mqtt_protocol_handle_t mqtt_protocol_handle = (mqtt_protocol_t *) handler_args;
    //QueueHandle_t  received_mqtt_queue = mqtt_protocol_handle->fila;
    
    ESP_LOGI(TAGMQTT, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAGMQTT, "MQTT_EVENT_CONNECTED");
        esp_mqtt_interface_handle->s_callback((esp_mqtt_event_id_t)event->event_id, NULL, NULL);
            

        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAGMQTT, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAGMQTT, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        //msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        //ESP_LOGI(TAGMQTT, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAGMQTT, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAGMQTT, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAGMQTT, "MQTT_EVENT_DATA");
        ESP_LOGI(TAGMQTT, "TOPIC=%.*s", event->topic_len, event->topic);
        ESP_LOGI(TAGMQTT, "DATA=%.*s", event->data_len, event->data);
        if (esp_mqtt_interface_handle && esp_mqtt_interface_handle->s_callback) {
            char *topic_received = strndup(event->topic, event->topic_len);
            char *data_received = strndup(event->data, event->data_len);
            
            // Chamar o callback com os dados recebidos
            esp_mqtt_interface_handle->s_callback((esp_mqtt_event_id_t)event->event_id, topic_received, data_received);
            
            // Liberar a memória alocada
            free(topic_received);
            free(data_received);
        }
        
        
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAGMQTT, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAGMQTT, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAGMQTT, "Other event id:%d", event->event_id);
        break;
    }
}

void esp_mqtt_interface_init(esp_mqtt_interface_config_t *esp_mqtt_interface_config)
{   
    
    esp_log_level_set("mqtt_client", ESP_LOG_VERBOSE);
    esp_log_level_set("MQTT_EXAMPLE", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT_BASE", ESP_LOG_VERBOSE);
    esp_log_level_set("esp-tls", ESP_LOG_VERBOSE);
    esp_log_level_set("TRANSPORT", ESP_LOG_VERBOSE);
    esp_log_level_set("outbox", ESP_LOG_VERBOSE);

    esp_log_level_set(TAGMQTT, ESP_LEVEL);

    esp_mqtt_interface_t *esp_mqtt_interface = NULL;

    esp_mqtt_interface = calloc(1,sizeof(esp_mqtt_interface_t));

    sprintf(esp_mqtt_interface->uri, "mqtt://%s:%s@%s:%d", 
        esp_mqtt_interface_config->username, esp_mqtt_interface_config->password, esp_mqtt_interface_config->host, 
        esp_mqtt_interface_config->port);
    esp_mqtt_interface->id = esp_mqtt_interface_config->id;

    esp_mqtt_client_config_t mqtt_cfg = 
        {
        .broker.address.uri = esp_mqtt_interface->uri,
        .credentials.client_id = esp_mqtt_interface->id,
    };
    
    esp_mqtt_interface->client = esp_mqtt_client_init(&mqtt_cfg);
  
    esp_mqtt_client_register_event(esp_mqtt_interface->client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(esp_mqtt_interface->client);
    

    esp_mqtt_interface_handle = esp_mqtt_interface;

}

void esp_mqtt_interface_register_cb(esp_mqtt_interface_cb_t cb){
    esp_mqtt_interface_handle->s_callback = cb;
    ESP_LOGI(TAGMQTT, "Callback registered at %p", cb);
}


void mqtt_protocol_change_uri (esp_mqtt_client_handle_t client, const char *uri){
    esp_mqtt_client_set_uri(client, uri);
}

void esp_mqtt_interface_publish(const char *topic, const char *data, int qos, int retain)
{
    if (esp_mqtt_interface_handle && esp_mqtt_interface_handle->client) {
        int msg_id = esp_mqtt_client_publish(esp_mqtt_interface_handle->client, topic, data, 0, qos, retain);
        ESP_LOGI(TAGMQTT, "sent publish successful, msg_id=%d", msg_id);
    } else {
        ESP_LOGE(TAGMQTT, "MQTT client not initialized");
    }
}

void esp_mqtt_interface_subscribe(const char *topic, int qos)
{
    if (esp_mqtt_interface_handle && esp_mqtt_interface_handle->client) {
        int msg_id = esp_mqtt_client_subscribe(esp_mqtt_interface_handle->client, topic, qos);
        ESP_LOGI(TAGMQTT, "sent subscribe from interface successful, msg_id=%d", msg_id);
    } else {
        ESP_LOGE(TAGMQTT, "MQTT client not initialized");
    }
}
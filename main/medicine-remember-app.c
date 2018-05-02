
/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "mqtt_client.h"
#include "jsmn.h"
#include "sdkconfig.h"
#include <time.h>
#include "apps/sntp/sntp.h"

static EventGroupHandle_t wifi_event_group;
const static int CONNECTED_BIT = BIT0;

//PROTOTIPE
static void wifi_init(void);
static void mqtt_app_start(void);
static bool jsonParser(char *payload);
static int findKey(const char *payload, jsmntok_t *tok, const char *s);
static void start_sntp(void);
void initHardware(void);
void alertaCliente(int _slot, int valor);
void beep();

time_t time1;
struct tm tm1;
struct cadastroremedio{
    char nome[50];
    int hora;
    int min;
    int id;
    int slot;
};

struct cadastroremedio rmd[25];

/*TASK DE RECEBIMENTO DA HORA ATUAL*/
void vTaskrecebehora(void * pvParameters){

    while(1){
        time(&time1);
        gmtime_r(&time1, &tm1);
        localtime_r(&time1, &tm1);
        //ESP_EARLY_LOGI("TIME", "%d %d %d %d %d %d", tm1.tm_year, tm1.tm_mon, tm1.tm_mday, tm1.tm_hour, tm1.tm_min, tm1.tm_sec);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}

/* TASK DE VARREDURA */
void vTaskVerificaRMD( void * pvParameters ){

    for( ;; )
    {
    for(int i = 0; i < 25; i++){                                                    //VARREDURA DAS ESTRUTURAS
      if(strlen(rmd[i].nome) != 0){                                                 //VERIFICA CONTEUDO DO NOME, SE FOR DIFERENTE DE ZERO SIGNIFICA QUE A ESTRUTURA POSSUI CADASTRO
        if(rmd[i].hora == tm1.tm_hour && rmd[i].min == tm1.tm_min){                 //VERIFICA HORARIO
            beep();
            alertaCliente(rmd[i].slot, 0);
            ESP_EARLY_LOGI("TASK_VARREDURA", "HORA DO REMEDIO: %s", rmd[i].nome);    
        }
        else{
            alertaCliente(rmd[i].slot, 1);
        }
      }
      else{
        //ESP_EARLY_LOGI("TASK_VARREDURA", "SLOT SEM CADASTRO: %d", i);
      }
    }
    //ESP_EARLY_LOGI("TASK_VARREDURA", "\n");
    //TODO: VERIFICAR REMEDIOS CADASTRADOS
    vTaskDelay(  500 / portTICK_PERIOD_MS );
    }
}

//MAIN APP
void app_main(){
    //MSG DE INICIALIZACAO
    ESP_LOGI("MAIN_APP", "STARTUP");

    //INICIALIZAÇÃO DO HW
    initHardware();

    //INICIALIZACAO WIFI E MQTT
    nvs_flash_init();
    wifi_init();
    mqtt_app_start();

    //ACERTO DO FUSORARIO
    setenv("TZ", "GMT+3", 1);
    tzset();
    start_sntp();       //RECEBE DATA E HORA ATUAL
    vTaskDelay(  1000 / portTICK_PERIOD_MS );

    //CRIA TASK DE VARREDURA DOS CADASTROS
    xTaskCreate(vTaskVerificaRMD, "taskVerifica", configMINIMAL_STACK_SIZE, (void *) 1, 1, NULL);
    xTaskCreate(vTaskrecebehora, "taskrecebehora", configMINIMAL_STACK_SIZE, (void *) 1, 1, NULL);
    }

//ACIONAMENTO DO BUZZER
void beep(){
  gpio_set_level( GPIO_NUM_23, 1);
  vTaskDelay(  100 / portTICK_PERIOD_MS );
  gpio_set_level( GPIO_NUM_23, 0);
  vTaskDelay(  20 / portTICK_PERIOD_MS );
  gpio_set_level( GPIO_NUM_23, 1);
  vTaskDelay(  100 / portTICK_PERIOD_MS );
  gpio_set_level( GPIO_NUM_23, 0);
}

//ACIONA O LED CORRESPONDENTE DO HORARIO DO REMEDIO
void alertaCliente(int _slot, int valor){
    switch (_slot){
        case 1:
            gpio_set_level( GPIO_NUM_21, valor);
            break;
        case 2:
            gpio_set_level( GPIO_NUM_18, valor);
            break;
        case 3:
            gpio_set_level( GPIO_NUM_4, valor);
            break;
        case 4:
            gpio_set_level( GPIO_NUM_27, valor);
            break;
        case 5:
            gpio_set_level( GPIO_NUM_22, valor);
            break;
        case 6:
            gpio_set_level( GPIO_NUM_19, valor);
            break;
        case 7:
            gpio_set_level( GPIO_NUM_5, valor);
            break;
        case 8:
            gpio_set_level( GPIO_NUM_2, valor);
            break;
    }
}

//INICIALIZAÇÃO DAS PORTAS
void initHardware(void){
    gpio_set_direction(GPIO_NUM_23, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_21, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_18, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_27, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_22, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_19, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_5, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_2, GPIO_MODE_OUTPUT);
    gpio_set_level( GPIO_NUM_23, 0);
    gpio_set_level( GPIO_NUM_21, 1);
    gpio_set_level( GPIO_NUM_18, 1);
    gpio_set_level( GPIO_NUM_4,  1);
    gpio_set_level( GPIO_NUM_27, 1);
    gpio_set_level( GPIO_NUM_22, 1);
    gpio_set_level( GPIO_NUM_19, 1);
    gpio_set_level( GPIO_NUM_5,  1);
    gpio_set_level( GPIO_NUM_2,  1);
}

//FUNCAO DE REBIMENTO DA HORA ATUAL
static void start_sntp(void){
    ESP_EARLY_LOGI("SNTP", "SNTP START");
    ip_addr_t addr;
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    inet_pton(AF_INET, "200.186.125.195", &addr);
    sntp_setserver(0, &addr);
    sntp_init();
}

//WIFI HANDLER
static esp_err_t wifi_event_handler(void *ctx, system_event_t *event){
    switch (event->event_id) {
        case SYSTEM_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);

            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            esp_wifi_connect();
            xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
            break;
        default:
            break;
    }
    return ESP_OK;
}

//WIFI BEGIN
static void wifi_init(void){

    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "AMNET85_4269",
            .password = "33aJvqrv",
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_LOGI("WIFI_INIT", "start the WIFI SSID:[%s] password:[%s]", "REDESOLVETRONICx", "******");
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI("WIFI_INIT", "Waiting for wifi");
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

//MQTT HANDLER
static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event){
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    // your_context_t *context = event->context;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI("MQTT", "MQTT_EVENT_CONNECTED");
            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0/dev/rafa_teste", 1);
            ESP_LOGI("MQTT", "sent subscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI("MQTT", "MQTT_EVENT_DISCONNECTED");
            break;

        case MQTT_EVENT_SUBSCRIBED:
            ESP_LOGI("MQTT", "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            //msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            //ESP_LOGI("MQTT", "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:
            ESP_LOGI("MQTT", "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI("MQTT", "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI("MQTT", "MQTT_EVENT_DATA");
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            event->data[event->total_data_len] = '\0';
            //jsonParser("{\"nomeRemedio\": \"1234567890123456\", \"abc\": 20}");
            jsonParser(event->data);
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGI("MQTT", "MQTT_EVENT_ERROR");
            break;
    }
    return ESP_OK;
}

//MQTT BEGIN
static void mqtt_app_start(void){
    const esp_mqtt_client_config_t mqtt_cfg = {
        .uri = "mqtt://iot.eclipse.org",
        .event_handle = mqtt_event_handler,
        // .user_context = (void *)your_context
    };

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_start(client);
}

/*
FUNCAO: jsonParser - Funcao desenvolvida para receber o payload mqtt e realizar a conversão para a unidade correta(int, float, char[])
Parametro(s): char* payload - mensagem recebida pelo mqtt
RETORNO:    false - conversao com falha
            verdadeiro - conversao com sucesso
*/

int count = 0;
static bool jsonParser(char *payload){

    #define JSON_MAX_TOKENS 100         //quantidade maxima de tokens
    jsmn_parser parser;                 //declaracao de objeto parser
    jsmntok_t   t[JSON_MAX_TOKENS];     
    jsmn_init(&parser);

    int ret = jsmn_parse(&parser, payload, strlen(payload), t, JSON_MAX_TOKENS);

    //ERRO DE PAYLOAD, PAYLOAD CORROMPIDA
    if(ret < 0){
        ESP_LOGE("jsonParser", "FALHA DE PAYLOAD, PAYLOAD CORROMPIDA OU INCORRETA");
        return false;
    }

    //PAYLOAD NAO É UM OBJETO JSON
    if(ret < 1 || t[0].type != JSMN_OBJECT){
        ESP_LOGE("jsonParser", "OBJETO JSON NAO ENCONTRADO");
        return false;
    }

    //PAYLOAD VALIDA
    else{
        ESP_LOGI("jsonParser", "Tokens encontrados: %d", ret);
        char buffer[10];
        int string_size;
        for(int i = 1; i < ret; i++){
            if(findKey(payload, &t[i], "nomeRemedio") == 0){
                string_size = t[i + 1].end - t[i + 1].start;
                memcpy(buffer, payload + t[i + 1].start, string_size);
                buffer[string_size] = '\0';
                printf("%s\n", buffer);
                strcpy(rmd[count].nome, buffer);
            }
            if(findKey(payload, &t[i], "hora") == 0){
                string_size = t[i + 1].end - t[i + 1].start;
                memcpy(buffer, payload + t[i + 1].start, string_size);
                buffer[string_size] = '\0';
                rmd[count].hora = atoi(buffer);
            }
            if(findKey(payload, &t[i], "min") == 0){
                string_size = t[i + 1].end - t[i + 1].start;
                memcpy(buffer, payload + t[i + 1].start, string_size);
                buffer[string_size] = '\0';
                rmd[count].min = atoi(buffer);
            }
            if(findKey(payload, &t[i], "id") == 0){
                string_size = t[i + 1].end - t[i + 1].start;
                memcpy(buffer, payload + t[i + 1].start, string_size);
                buffer[string_size] = '\0';
                rmd[count].id = atoi(buffer);
            }
            if(findKey(payload, &t[i], "slot") == 0){
                string_size = t[i + 1].end - t[i + 1].start;
                memcpy(buffer, payload + t[i + 1].start, string_size);
                buffer[string_size] = '\0';
                rmd[count].slot = atoi(buffer);
            }
        }
        count++;
        return true;
    }
}

/*
FUNCAO: findKey - Funcao desenvolvida para encontrar a palavra chave dentro do payload
PARAMETRO(S):   - const char *payload - Json completo recebido pelo mqtt
                - jsmntok_t *tok - token onde a palavra deve ser validada
                - const char *s - palavra a ser validada dentro do tokem
RETORNO:    0  - palavra validada
            -1 - palavra não validada 
*/
static int findKey(const char *payload, jsmntok_t *tok, const char *s){
    if(tok->type == JSMN_STRING && (int) strlen(s) == (tok->end - tok->start) && strncmp(payload + tok->start, s, (tok->end - tok->start)) == 0){
        return 0;
    }
    return -1;
}
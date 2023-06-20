#include "esp_system.h"
#include "esp_log.h"
#include "data_gather.h"
#include "driver/gpio.h"
#include "driver/twai.h"

//--------------------------------------------
// defines
//--------------------------------------------
#define DATA_QUEUE_LEN 10
#define DATA_GATHER_STACK_SIZE_KB (2)

//--------------------------------------------
// global variables
//--------------------------------------------
QueueHandle_t gathered_data_queue;

//--------------------------------------------
// private function prototypes
//--------------------------------------------
void data_gather_task(void* params);

//--------------------------------------------
// private variables
//--------------------------------------------
static const char *TAG = "data_gather"; // TAG for debug
static system_status_t system_status;

//--------------------------------------------
// public functions
//--------------------------------------------
esp_err_t data_gather_start(void){
    ESP_LOGI(TAG, "starting data gather");

    xTaskCreate(data_gather_task, "data_gather", DATA_GATHER_STACK_SIZE_KB*1024, NULL, 3, NULL);
    gathered_data_queue = xQueueCreate(DATA_QUEUE_LEN, sizeof(system_status_t));
    return ESP_OK;
}

//--------------------------------------------
// private functions
//--------------------------------------------
void data_gather_task(void* params){

    //Initialize configuration structures using macro initializers
    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_22, GPIO_NUM_23, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_100KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    //Install TWAI driver
    if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver installed\n");
    } else {
        ESP_LOGI(TAG, "Failed to install TWAI driver\n");
        return;
    }

    //Start TWAI driver
    if (twai_start() == ESP_OK) {
        ESP_LOGI(TAG, "TWAI driver started\n");
    } else {
        ESP_LOGI(TAG, "Failed to start TWAI driver\n");
        return;
    }

    system_status.pump_stat = false;
    system_status.alarm_stat = false;

    while(1){
        // system_status.pump_stat = !system_status.pump_stat;
        // system_status.alarm_stat = !system_status.alarm_stat;
        // xQueueSend(gathered_data_queue, &system_status, 0);
        // vTaskDelay(1000);
        //Wait for message to be received
        twai_message_t message;
        if (twai_receive(&message, pdMS_TO_TICKS(10000)) == ESP_OK) {
            ESP_LOGI(TAG, "Message received\n");
            //Process received message
            if (message.extd) {
                ESP_LOGI(TAG, "Message is in Extended Format\n");
            } else {
                ESP_LOGI(TAG, "Message is in Standard Format\n");
            }
            printf("ID is %ld\n", message.identifier);
            if (!(message.rtr)) {
                for (int i = 0; i < message.data_length_code; i++) {
                    printf("Data byte %d = %d\n", i, message.data[i]);
                }
            }
        } else {
            ESP_LOGI(TAG, "Failed to receive message\n");
}
    }
}
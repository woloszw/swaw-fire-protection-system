#include "esp_system.h"
#include "esp_log.h"
#include "data_gather.h"
#include "driver/gpio.h"
#include "driver/twai.h"
#include <string.h>
#include <sys/time.h>

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
static nodes_status_t system_status;

//--------------------------------------------
// public functions
//--------------------------------------------
esp_err_t data_gather_start(void){
    ESP_LOGI(TAG, "starting data gather");

    xTaskCreate(data_gather_task, "data_gather", DATA_GATHER_STACK_SIZE_KB*1024, NULL, 3, NULL);
    gathered_data_queue = xQueueCreate(DATA_QUEUE_LEN, sizeof(nodes_status_t));
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

    for(uint8_t i = 0; i < NODES_MAX_NUM; i++){
        system_status.nodes[i].online = false;
    }

    while(1){
        twai_message_t message;
        uint8_t slave_id;
        payload_slv_t slave_data;
        if (twai_receive(&message, pdMS_TO_TICKS(1000)) == ESP_OK) {
            struct timeval tv_now;
            gettimeofday(&tv_now, NULL);
            int64_t time_us = (int64_t)tv_now.tv_sec * 1000000L + (int64_t)tv_now.tv_usec;
            ESP_LOGI(TAG, "Message received");
            if (!(message.rtr)) {
                memcpy(&slave_data, message.data, sizeof(payload_slv_t));
                slave_id = message.identifier >> 8;
                system_status.nodes[slave_id].online = true;
                system_status.nodes[slave_id].fire = slave_data.flame_val;
                system_status.nodes[slave_id].smoke = slave_data.smoke_val;
                system_status.nodes[slave_id].temp = (float)slave_data.temp_val/16.0f;
                system_status.nodes[slave_id].last_message_time = time_us/1000;
                // printf("data_gather: slave ID: %d, smoke: %d, flame: %d temp: %d, time: %ld\n", 
                // slave_id, slave_data.smoke_val, slave_data.flame_val, (int16_t)(slave_data.temp_val/16),
                //  (uint32_t)system_status.nodes[slave_id].last_message_time);
            }
        } else {
            ESP_LOGI(TAG, "Failed to receive message\n");
        }
        xQueueSend(gathered_data_queue, &system_status, 0);
    }
}
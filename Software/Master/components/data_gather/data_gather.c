#include "esp_system.h"
#include "esp_log.h"
#include "data_gather.h"
#include "driver/gpio.h"
#include "driver/twai.h"
#include <string.h>
#include <sys/time.h>
#include "system_status.h"

//--------------------------------------------
// defines
//--------------------------------------------
#define DATA_QUEUE_LEN 10
#define DATA_GATHER_STACK_SIZE_KB (2)

//--------------------------------------------
// global variables
//--------------------------------------------

//--------------------------------------------
// private function prototypes
//--------------------------------------------
void data_gather_task(void* params);

//--------------------------------------------
// private variables
//--------------------------------------------
static const char *TAG = "data_gather"; // TAG for debug

//--------------------------------------------
// public functions
//--------------------------------------------
esp_err_t data_gather_start(void){
    ESP_LOGI(TAG, "starting data gather");

    system_status_init();
    xTaskCreate(data_gather_task, "data_gather", DATA_GATHER_STACK_SIZE_KB*1024, NULL, 3, NULL);
    return ESP_OK;
}

//--------------------------------------------
// private functions
//--------------------------------------------

int64_t curr_time_ms(void){
    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    return (int64_t)tv_now.tv_sec * 1000L + (int64_t)tv_now.tv_usec;
}

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
        sys_stat_set_online(i, false);
    }

    while(1){
        twai_message_t message;
        uint8_t slave_id;
        bool ID_bit_0_set;
        bool payload_len_6B;
        payload_slv_t slave_data;

        if (twai_receive(&message, pdMS_TO_TICKS(3500)) == ESP_OK) {
            ESP_LOGI(TAG, "Message received");

            ID_bit_0_set = (message.identifier & 0x01) == 0x01;  // bit 0 indicates that frame carries data
            payload_len_6B = message.data_length_code == sizeof(payload_slv_t);
            if (!(message.rtr) && ID_bit_0_set && payload_len_6B) {
                memcpy(&slave_data, message.data, sizeof(payload_slv_t));
                slave_id = message.identifier >> 8;
                if(slave_id >= 0 && slave_id < 8){
                    sys_stat_set_online(slave_id, true);
                    sys_stat_set_fire(slave_id, slave_data.flame_val);
                    sys_stat_set_smoke(slave_id, slave_data.smoke_val);
                    sys_stat_set_temp(slave_id, slave_data.temp_val/16.0f);
                    sys_stat_set_last_message_time(slave_id, curr_time_ms());
                }
            }
        } else {
            ESP_LOGI(TAG, "Failed to receive message\n");
        }
    }
}
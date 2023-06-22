#include "effectors.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "system_status.h"
#include <stdbool.h>
#include "esp_log.h"

#define PUMP_PIN GPIO_NUM_18
#define VALVE_PIN GPIO_NUM_19
#define ALARM_PIN GPIO_NUM_32

#define TEMP_THRESHOLD 50
#define SMOKE_THRESHOLD 100
#define FIRE_THRESHOLD 10

static const char *TAG = "effectors"; // TAG for debug

void effectors_task(void* params);

void effectors_start(){
    xTaskCreate(effectors_task, "eff", 1024, NULL, 3, NULL);
}

void effectors_task(void* params){
    gpio_set_direction(PUMP_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(VALVE_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ALARM_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(PUMP_PIN, 0);
    gpio_set_level(VALVE_PIN, 0);
    gpio_set_level(ALARM_PIN, 0);

    while(1){
        for(int i = 0; i < NODES_MAX_NUM; i++){
            if(sys_stat_get_online(i)){
                if(sys_stat_get_temp(i) > TEMP_THRESHOLD){
                    sys_stat_set_fire_detected(true);
                    sys_stat_set_alarm(true);
                    sys_stat_set_pump(true);
                    ESP_LOGI(TAG, "over temeprature detected: %d", sys_stat_get_temp(i));
                }else if(sys_stat_get_smoke(i) > SMOKE_THRESHOLD){
                    sys_stat_set_fire_detected(true);
                    sys_stat_set_alarm(true);
                    sys_stat_set_pump(true);
                    ESP_LOGI(TAG, "over smoke detected: %d", sys_stat_get_smoke(i));
                } else if(sys_stat_get_fire(i) < FIRE_THRESHOLD){
                    sys_stat_set_fire_detected(true);
                    sys_stat_set_alarm(true);
                    sys_stat_set_pump(true);
                    ESP_LOGI(TAG, "over fire detected: %d", sys_stat_get_fire(i));
                }
            }
        }

        if(sys_stat_get_pump()){
            gpio_set_level(PUMP_PIN, 1);
            gpio_set_level(VALVE_PIN, 1);
        }else{
            gpio_set_level(PUMP_PIN, 0);
            gpio_set_level(VALVE_PIN, 0);
        }
        
        if(sys_stat_get_alarm()){
            gpio_set_level(ALARM_PIN, 1);
        }else{
            gpio_set_level(ALARM_PIN, 0);
        }

        vTaskDelay(100);
    }
}
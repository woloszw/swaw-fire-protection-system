#include "system_status.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

typedef struct __attribute__((__packed__)){
    bool online;
    uint8_t fire;
    uint8_t smoke;
    float temp;
    uint64_t last_message_time;
} node_status_t;

typedef struct __attribute__((__packed__)){
    bool fire_detected;
    bool pump_status;
    bool alarm_status;
    node_status_t nodes[NODES_MAX_NUM];
}nodes_status_t;

static nodes_status_t system_status;
static SemaphoreHandle_t sys_stat_mutex;
static const char *TAG = "sys_stat"; // TAG for debug

void system_status_init(){
    sys_stat_mutex = xSemaphoreCreateMutex();
    sys_stat_set_pump(false);
    sys_stat_set_alarm(false);
}

bool sys_stat_get_fire_detected(){
    xSemaphoreTake(sys_stat_mutex, 100);
    bool fire_detected = system_status.fire_detected;
    xSemaphoreGive(sys_stat_mutex);
    return fire_detected;
}

bool sys_stat_get_pump(){
    xSemaphoreTake(sys_stat_mutex, 100);
    bool pump = system_status.pump_status;
    xSemaphoreGive(sys_stat_mutex);
    return pump;
}

bool sys_stat_get_alarm(){
    xSemaphoreTake(sys_stat_mutex, 100);
    bool alarm = system_status.alarm_status;
    xSemaphoreGive(sys_stat_mutex);
    return alarm;
}

bool sys_stat_get_online(uint8_t node_id){
    xSemaphoreTake(sys_stat_mutex, 100);
    bool online = false;
    if(node_id < NODES_MAX_NUM){
        online = system_status.nodes[node_id].online;
    }else{
        ESP_LOGE(TAG, "invalid node ID: %d", node_id);
    }
    xSemaphoreGive(sys_stat_mutex);
    return online;
}

int sys_stat_get_temp(uint8_t node_id){
    xSemaphoreTake(sys_stat_mutex, 100);
    int temp;
    if(node_id < NODES_MAX_NUM){
        temp = system_status.nodes[node_id].temp;
    }else{
        temp = -15000;
        ESP_LOGE(TAG, "invalid node ID: %d", node_id);
    }
    xSemaphoreGive(sys_stat_mutex);
    return temp;
}

int sys_stat_get_smoke(uint8_t node_id){
    xSemaphoreTake(sys_stat_mutex, 100);
    int smoke;
    if(node_id < NODES_MAX_NUM){
        smoke = system_status.nodes[node_id].smoke;
    }else{
        smoke = -15000;
        ESP_LOGE(TAG, "invalid node ID: %d", node_id);
    }
    xSemaphoreGive(sys_stat_mutex);
    return smoke;
}

int sys_stat_get_fire(uint8_t node_id){
    xSemaphoreTake(sys_stat_mutex, 100);
    int fire;
    if(node_id < NODES_MAX_NUM){
        fire = system_status.nodes[node_id].fire;
    }else{
        fire = -15000;
        ESP_LOGE(TAG, "invalid node ID: %d", node_id);
    }
    xSemaphoreGive(sys_stat_mutex);
    return fire;
}

int64_t sys_stat_get_last_message_time(uint8_t node_id){
    xSemaphoreTake(sys_stat_mutex, 100);
    int64_t time;
    if(node_id < NODES_MAX_NUM){
        time = system_status.nodes[node_id].last_message_time;
    }else{
        time = -15000;
        ESP_LOGE(TAG, "invalid node ID: %d", node_id);
    }
    xSemaphoreGive(sys_stat_mutex);
    return time;
}

void sys_stat_set_fire_detected(bool fire_detected){
    xSemaphoreTake(sys_stat_mutex, 100);
    system_status.fire_detected = fire_detected;
    xSemaphoreGive(sys_stat_mutex);
}

void sys_stat_set_pump(bool pump_stat){
    xSemaphoreTake(sys_stat_mutex, 100);
    system_status.pump_status = pump_stat;
    xSemaphoreGive(sys_stat_mutex);
}

void sys_stat_set_alarm(bool alarm_stat){
    xSemaphoreTake(sys_stat_mutex, 100);
    system_status.alarm_status = alarm_stat;
    xSemaphoreGive(sys_stat_mutex);
}

void sys_stat_set_online(uint8_t node_id, bool online){
    xSemaphoreTake(sys_stat_mutex, 100);
    if(node_id < NODES_MAX_NUM){
        system_status.nodes[node_id].online = online;
    }else{
        ESP_LOGE(TAG, "invalid node ID: %d", node_id);
    }
    xSemaphoreGive(sys_stat_mutex);
}

void sys_stat_set_temp(uint8_t node_id, int temp){
    xSemaphoreTake(sys_stat_mutex, 100);
    if(node_id < NODES_MAX_NUM){
        system_status.nodes[node_id].temp = temp;
    }else{
        ESP_LOGE(TAG, "invalid node ID: %d", node_id);
    }
    xSemaphoreGive(sys_stat_mutex);
}

void sys_stat_set_smoke(uint8_t node_id, int smoke){
    xSemaphoreTake(sys_stat_mutex, 100);
    if(node_id < NODES_MAX_NUM){
        system_status.nodes[node_id].smoke = smoke;
    }else{
        ESP_LOGE(TAG, "invalid node ID: %d", node_id);
    }
    xSemaphoreGive(sys_stat_mutex);
}

void sys_stat_set_fire(uint8_t node_id, int fire){
    xSemaphoreTake(sys_stat_mutex, 100);
    if(node_id < NODES_MAX_NUM){
        system_status.nodes[node_id].fire = fire;
    }else{
        ESP_LOGE(TAG, "invalid node ID: %d", node_id);
    }
    xSemaphoreGive(sys_stat_mutex);
}

void sys_stat_set_last_message_time(uint8_t node_id, int64_t time){
    xSemaphoreTake(sys_stat_mutex, 100);
    if(node_id < NODES_MAX_NUM){
        system_status.nodes[node_id].last_message_time = time;
    }else{
        ESP_LOGE(TAG, "invalid node ID: %d", node_id);
    }
    xSemaphoreGive(sys_stat_mutex);
}
#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define NODES_MAX_NUM 8

typedef struct __attribute__((__packed__)){
    bool online;
    float fire;
    float smoke;
    float temp;
} node_status_t;

typedef struct __attribute__((__packed__)){
    bool pump_stat;
    bool alarm_stat;
    node_status_t nodes[NODES_MAX_NUM];
}system_status_t;

extern QueueHandle_t gathered_data_queue;

esp_err_t data_gather_start(void);
#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#define NODES_MAX_NUM 8

typedef struct __attribute__((__packed__)){
    bool online;
    uint8_t fire;
    uint8_t smoke;
    float temp;
    uint64_t last_message_time;
} node_status_t;

typedef struct __attribute__((__packed__)){
    node_status_t nodes[NODES_MAX_NUM];
}nodes_status_t;

typedef struct __attribute__((__packed__)){
	uint16_t smoke_val;
	uint16_t flame_val;
	int16_t	 temp_val;
}payload_slv_t;

extern QueueHandle_t gathered_data_queue;

esp_err_t data_gather_start(void);
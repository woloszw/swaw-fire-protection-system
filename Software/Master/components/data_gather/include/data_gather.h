#include "stdbool.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

typedef struct __attribute__((__packed__)){
	uint16_t smoke_val;
	uint16_t flame_val;
	int16_t	 temp_val;
}payload_slv_t;

esp_err_t data_gather_start(void);
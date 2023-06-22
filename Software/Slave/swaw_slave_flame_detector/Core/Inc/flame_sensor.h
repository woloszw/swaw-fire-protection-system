#pragma once

#include "stm32f0xx.h"
//read value from analog flame sensor
// flame_ptr ptr to value from adc - 12 bit
HAL_StatusTypeDef flame_sensor_read(uint16_t* flame_ptr);

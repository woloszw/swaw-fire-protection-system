#pragma once

#include "stm32f0xx.h"


//read value from analog smoke sensor
// smoke_ptr ptr to  value from adc - 12 bit
HAL_StatusTypeDef smoke_sensor_read(uint16_t* smoke_ptr);

#pragma once

#include "stm32f0xx.h"

#define DS18B20_ROM_CODE_SIZE		8

// Init 1 wire
// return - HAL_OK/HAL_ERROR
HAL_StatusTypeDef ds18b20_init(void);

// read sensor address and calculate crc sum
// rom_code - place where save incoming data
// return - HAL_OK/HAL_ERROR
HAL_StatusTypeDef ds18b20_read_address(uint8_t* rom_code);

// Start temperature measure
// rom_code - sensor address or NULL
// return - HAL_OK/HAL_ERROR
HAL_StatusTypeDef ds18b20_start_measure(const uint8_t* rom_code);

// Take temperature
// rom_code - sensor address or NULL
// return - temperature (to celsius divide by 16)
int16_t ds18b20_get_temp(const uint8_t* rom_code);

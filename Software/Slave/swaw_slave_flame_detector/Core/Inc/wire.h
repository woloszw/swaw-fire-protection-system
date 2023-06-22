#pragma once

#include "stm32f0xx.h"

// Init 1 wire
// Start timer to delay
// return - HAL_OK/HAL_ERROR
HAL_StatusTypeDef wire_init(void);

// Send reset sequence
// return - HAL_OK/HAL_ERROR
HAL_StatusTypeDef wire_reset(void);

// Read data from sensor
// return - read byte
uint8_t wire_read(void);

// Sent data to sensor
// byte - byte to send
void wire_write(uint8_t byte);

// Calculate crc sum
// data - data block
// len - data length
uint8_t wire_crc(const uint8_t* data, int len);

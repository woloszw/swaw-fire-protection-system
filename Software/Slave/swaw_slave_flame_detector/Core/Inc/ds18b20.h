#pragma once

#include "stm32f0xx.h"

#define DS18B20_ROM_CODE_SIZE		8

// Inicjalizacja 1-wire
// return - HAL_OK/HAL_ERROR
HAL_StatusTypeDef ds18b20_init(void);

// Odczytaj adres czujnika i policz CRC
// rom_code - miejsce do zapisu odczytanych danych
// return - HAL_OK/HAL_ERROR
HAL_StatusTypeDef ds18b20_read_address(uint8_t* rom_code);

// Rozpocznij pomiar temperatury
// rom_code - adres czujnika lub NULL
// return - HAL_OK/HAL_ERROR
HAL_StatusTypeDef ds18b20_start_measure(const uint8_t* rom_code);

// Pobierz temperaturę
// rom_code - adres czujnika lub NULL
// return - temperatura w stopniach Celsjusza
int16_t ds18b20_get_temp(const uint8_t* rom_code);

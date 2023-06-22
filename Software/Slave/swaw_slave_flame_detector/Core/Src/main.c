/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "can.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "ds18b20.h"
#include "flame_sensor.h"
#include "smoke_sensor.h"
#include<string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
	struct Slv_payload{
		uint16_t smoke_val;
		uint16_t flame_val;
		int16_t	 temp_val;

	}payload_slv;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
	uint8_t slave_id = 0;
  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC_Init();
  MX_CAN_Init();
  MX_USART2_UART_Init();
  MX_TIM17_Init();
  /* USER CODE BEGIN 2 */

  //init temperature sensor
  if (ds18b20_init() != HAL_OK) {
    Error_Handler();
  }
  uint8_t ds1[DS18B20_ROM_CODE_SIZE];

  if (ds18b20_read_address(ds1) != HAL_OK) {
    Error_Handler();
  }

  HAL_CAN_Start(&hcan);

  //dip switch slave id read
  int tmp_id;
  tmp_id =HAL_GPIO_ReadPin(ID_0_PIN_GPIO_Port, ID_0_PIN_Pin);
  slave_id |= tmp_id;
  tmp_id =HAL_GPIO_ReadPin(ID_1_PIN_GPIO_Port, ID_1_PIN_Pin);
  slave_id |= (tmp_id<<1);
  tmp_id =HAL_GPIO_ReadPin(ID_2_PIN_GPIO_Port, ID_2_PIN_Pin);
  slave_id |= (tmp_id<<2);
  HAL_Delay(1000);


  // struct for can
  CAN_TxHeaderTypeDef txHeader;
  uint32_t              TxMailbox;
  uint32_t can_node_id_base = slave_id << 8;



  uint8_t tx_can_data[6];

  txHeader.StdId = can_node_id_base | 0x1;  // Node ID - LSB == 1 - payload id

  txHeader.ExtId = 0;      // // Standard frame
  txHeader.RTR = CAN_RTR_DATA;  //  trans mode - data
  txHeader.IDE = CAN_ID_STD;    //standart id format
  txHeader.DLC = 6;  // length (in bytes)




  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

	  while (1)
	  {
		  //temp measure
		  ds18b20_start_measure(NULL);
		  HAL_Delay(750);
		  payload_slv.temp_val = ds18b20_get_temp(NULL);


		  //flame measure
		  flame_sensor_read(&payload_slv.flame_val);
		  //smoke measure
		  smoke_sensor_read(&payload_slv.smoke_val);
		  //send to master via can
		  memcpy(tx_can_data, &payload_slv, sizeof(payload_slv));

		  if (HAL_CAN_AddTxMessage(&hcan, &txHeader, tx_can_data, &TxMailbox) == HAL_OK) {
			  // Successfully add frame to queue
		  } else {
			  //Error when try add frame to queue
		  }

		  // wait for send
		  if (HAL_CAN_GetTxMailboxesFreeLevel(&hcan) == 3) {
			  // Send box is free, all frames are send
		  }

		  HAL_Delay(1000);

	  }


    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI14|RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.HSI14State = RCC_HSI14_ON;
  RCC_OscInitStruct.HSI14CalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI48;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL5;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV6;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */


/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

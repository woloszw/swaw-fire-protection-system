#include "adc.h"

HAL_StatusTypeDef flame_sensor_read(uint16_t* flame_ptr){
	  ADC_ChannelConfTypeDef sConfig_4 = {0};
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	  */
	  sConfig_4.Channel = ADC_CHANNEL_4;
	  sConfig_4.Rank = 1;
	  sConfig_4.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc, &sConfig_4) != HAL_OK)
	  {
		Error_Handler();
	  }
	  HAL_ADC_Start(&hadc);
	  HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
	  while(HAL_ADC_GetState(&hadc)== HAL_ADC_STATE_BUSY){}
	  uint16_t adcValue = (uint16_t)HAL_ADC_GetValue(&hadc);
	  HAL_ADC_Stop(&hadc);

	  sConfig_4.Rank = ADC_RANK_NONE;
	  if (HAL_ADC_ConfigChannel(&hadc, &sConfig_4) != HAL_OK)
	  {
		Error_Handler();
	  }


	  HAL_Delay(100);
	  *flame_ptr = adcValue;

	  return HAL_OK;

}

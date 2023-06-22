#include "adc.h"

#define NUM_OF_AVERAGE_SAMPL 10

HAL_StatusTypeDef smoke_sensor_read(uint16_t* smoke_ptr){
	  ADC_ChannelConfTypeDef sConfig = {0};
	  uint32_t avr_value = 0;
	  uint16_t adcValue= 0;
	  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	  */
	  sConfig.Channel = ADC_CHANNEL_5;
	  sConfig.Rank = ADC_RANK_CHANNEL_NUMBER;
	  sConfig.SamplingTime = ADC_SAMPLETIME_28CYCLES_5;
	  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	  {
		Error_Handler();
	  }

	  HAL_ADC_Start(&hadc);
	  HAL_ADC_PollForConversion(&hadc, HAL_MAX_DELAY);
	  for(int i=0; i<NUM_OF_AVERAGE_SAMPL;i++){
		  avr_value += (uint32_t)HAL_ADC_GetValue(&hadc);
	  }
	  HAL_ADC_Stop(&hadc);


	  sConfig.Rank = ADC_RANK_NONE;
	  if (HAL_ADC_ConfigChannel(&hadc, &sConfig) != HAL_OK)
	  {
		Error_Handler();
	  }

	  avr_value = avr_value/NUM_OF_AVERAGE_SAMPL;

	  adcValue = avr_value;

	  *smoke_ptr = adcValue;

	  return HAL_OK;

}

/**
  ******************************************************************************
  * File Name          : ADC.c
  * Description        : This file provides code for the configuration
  *                      of the ADC instances.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "adc.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

ADC_HandleTypeDef hadc3;
/*RANK1、RANK2 adc value*/
uint16_t ar_get_voltage_value[V_TWO] = {I_ZERO};
/* ADC3 init function */
void MX_ADC3_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  /** Common config 
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc3.Init.Resolution = ADC_RESOLUTION_12B;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.NbrOfConversion = 2;
  hadc3.Init.DiscontinuousConvMode = ENABLE;
  hadc3.Init.NbrOfDiscConversion = 1;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  sConfig.Channel = ADC_CHANNEL_6;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_2CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure Regular Channel 
  */
  HAL_Delay(10);
  sConfig.Channel = ADC_CHANNEL_13;
  sConfig.Rank = ADC_REGULAR_RANK_2;
#if 1
  sConfig.SamplingTime = ADC_SAMPLETIME_247CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
#endif
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(adcHandle->Instance==ADC3)
  {
  /* USER CODE BEGIN ADC3_MspInit 0 */

  /* USER CODE END ADC3_MspInit 0 */
    /* ADC3 clock enable */
    __HAL_RCC_ADC_CLK_ENABLE();
  
    __HAL_RCC_GPIOF_CLK_ENABLE();
    /**ADC3 GPIO Configuration    
    PF3     ------> ADC3_IN6
    PF10     ------> ADC3_IN13 
    */
    GPIO_InitStruct.Pin = BAT_ADC_IN_Pin|BAT_VOLTAGE_COM_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG_ADC_CONTROL;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /* USER CODE BEGIN ADC3_MspInit 1 */

  /* USER CODE END ADC3_MspInit 1 */
  }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle)
{

  if(adcHandle->Instance==ADC3)
  {
  /* USER CODE BEGIN ADC3_MspDeInit 0 */

  /* USER CODE END ADC3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_ADC_CLK_DISABLE();
  
    /**ADC3 GPIO Configuration    
    PF3     ------> ADC3_IN6
    PF10     ------> ADC3_IN13 
    */
    HAL_GPIO_DeInit(GPIOF, BAT_ADC_IN_Pin|BAT_VOLTAGE_COM_Pin);

  /* USER CODE BEGIN ADC3_MspDeInit 1 */

  /* USER CODE END ADC3_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
void get_adc3_value(void)
{
    uint16_t ar_get_adc_value[V_TWO] = {I_ZERO};
    uint16_t m_coin_value = 0;
    for(uint8_t i = V_ONE; i <= V_TWO; i++)
    {
        HAL_ADC_Start(&hadc3);
        HAL_ADC_PollForConversion(&hadc3, 0xffff);
        ar_get_adc_value[i] = HAL_ADC_GetValue(&hadc3);
//        printf("rank %d ADC %04x\r\n", i, ar_get_adc_value[i]);
    }
    ar_get_voltage_value[V_ZERO] = ar_get_adc_value[V_ONE]*6*330/4096;  /*the voltage value is increased by 100 times*/
    //ar_get_voltage_value[V_ZERO] += 50;
//    printf("bat pack voltage : %d.%dV\r\n", ar_get_voltage_value[V_ZERO]/100,ar_get_voltage_value[V_ZERO]%100);
    m_coin_value = ar_get_adc_value[V_TWO]*2*330/4096; 
    m_coin_value %= 10;
    if(m_coin_value > 4)
    {
        ar_get_voltage_value[V_ONE] = ar_get_adc_value[V_TWO]*2*33/4096 + 2;   /*the voltage value is increased by 10 times*/      
    }
    else
    { 
        ar_get_voltage_value[V_ONE] = ar_get_adc_value[V_TWO]*2*33/4096;
    } 
    HAL_Delay(5);
//    printf("button bat voltage : %04x\r\n", ar_get_voltage_value[V_ONE]);
    HAL_ADC_Stop(&hadc3);
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

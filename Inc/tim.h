/**
  ******************************************************************************
  * File Name          : TIM.h
  * Description        : This file provides code for the configuration
  *                      of the TIM instances.
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __tim_H
#define __tim_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#define LED_TOGGLE_CNT     200U
#define LED_RUN     HAL_GPIO_TogglePin(LED_RUN_GPIO_Port,LED_RUN_Pin)
#define WDI_REFRESH     HAL_GPIO_TogglePin(DONE_GPIO_Port,DONE_Pin)
#define DOOR_UNLOCK_EN      0x01
#define RATIO_START_INDEX     44
#define IOT_POWER_ON_DELAY_TIME     1000    /*iot上电后20S再操作指令*/
#define MIN_VOLTAGE 600
#define VOLTAGE_CHECK_CNT     60000U

extern volatile uint16_t g_unlock_tick;
extern volatile uint8_t g_unlock_flag;
extern uint8_t g_ble_disconnect_flag;
extern uint8_t g_min_voltage_flag;
extern uint8_t g_voltage_check_flag;
/* USER CODE END Includes */

extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim6;

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

void MX_TIM5_Init(void);
void MX_TIM6_Init(void);
extern void start_pwm_buzzer(void);
extern void stop_pwm_buzzer(void);

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);
                    
/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__ tim_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

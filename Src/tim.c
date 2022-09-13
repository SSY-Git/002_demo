/**
  ******************************************************************************
  * File Name          : TIM.c
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

/* Includes ------------------------------------------------------------------*/
#include "tim.h"
#include "global.h"
#include "user_gpio.h"
#include "ble_e95.h"
#include "8k.h"
#include "iot.h"
#include "adc.h"
#include "main.h"
#include <stdio.h>
/* USER CODE BEGIN 0 */
/*运行指示灯计数，1S闪烁一次*/
uint16_t g_led_tick = I_ZERO;
/*开锁计数，开锁时间700ms*/
volatile uint16_t g_unlock_tick = I_ZERO;
/*开锁结束标志*/
volatile uint8_t g_unlock_flag = I_ZERO;
/*蓝牙断开连接标志位*/
uint8_t g_ble_disconnect_flag = I_ZERO;
uint32_t time = I_ZERO;
volatile uint32_t data_cnt = RATIO_START_INDEX;
/* 电池低电量标志*/
uint8_t g_min_voltage_flag = I_ZERO;
uint16_t g_voltage_check_tick = I_ZERO;
/* 电量检测标志*/
uint8_t g_voltage_check_flag = V_ZERO;

TIM_HandleTypeDef htim5;
TIM_HandleTypeDef htim6;

/* TIM5 init function */
void MX_TIM5_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 5-1;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 125-1;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 60;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim5, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim5);

}
/* TIM6 init function */
void MX_TIM6_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 10-1;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 5000-1;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM5)
  {
  /* USER CODE BEGIN TIM5_MspInit 0 */

  /* USER CODE END TIM5_MspInit 0 */
    /* TIM5 clock enable */
    __HAL_RCC_TIM5_CLK_ENABLE();

    /* TIM5 interrupt Init */
    HAL_NVIC_SetPriority(TIM5_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM5_IRQn);
  /* USER CODE BEGIN TIM5_MspInit 1 */

  /* USER CODE END TIM5_MspInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM6)
  {
  /* USER CODE BEGIN TIM6_MspInit 0 */

  /* USER CODE END TIM6_MspInit 0 */
    /* TIM6 clock enable */
    __HAL_RCC_TIM6_CLK_ENABLE();

    /* TIM6 interrupt Init */
    HAL_NVIC_SetPriority(TIM6_DAC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);
  /* USER CODE BEGIN TIM6_MspInit 1 */

  /* USER CODE END TIM6_MspInit 1 */
  }
}
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(timHandle->Instance==TIM5)
  {
  /* USER CODE BEGIN TIM5_MspPostInit 0 */

  /* USER CODE END TIM5_MspPostInit 0 */
  
    __HAL_RCC_GPIOF_CLK_ENABLE();
    /**TIM5 GPIO Configuration    
    PF7     ------> TIM5_CH2 
    */
    GPIO_InitStruct.Pin = STM_BUZZER_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init(STM_BUZZER_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN TIM5_MspPostInit 1 */

  /* USER CODE END TIM5_MspPostInit 1 */
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle)
{

  if(tim_baseHandle->Instance==TIM5)
  {
  /* USER CODE BEGIN TIM5_MspDeInit 0 */

  /* USER CODE END TIM5_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM5_CLK_DISABLE();

    /* TIM5 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM5_IRQn);
  /* USER CODE BEGIN TIM5_MspDeInit 1 */

  /* USER CODE END TIM5_MspDeInit 1 */
  }
  else if(tim_baseHandle->Instance==TIM6)
  {
  /* USER CODE BEGIN TIM6_MspDeInit 0 */

  /* USER CODE END TIM6_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_TIM6_CLK_DISABLE();

    /* TIM6 interrupt Deinit */
    HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);
  /* USER CODE BEGIN TIM6_MspDeInit 1 */

  /* USER CODE END TIM6_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */
/* USER CODE BEGIN 1 */
/*********************************************************************
 *	function name:  USER_PWM_SetDutyRatio
 *	describe: 设置占空比
 *	input:TIM_HandleTypeDef *htim：定时器；uint32_t Channel：通道；uint8_t value：占空比
 *	output:void
 *	return:void
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
void USER_PWM_SetDutyRatio(TIM_HandleTypeDef *htim, uint32_t Channel, uint8_t value)
{
    TIM_OC_InitTypeDef sConfigOC;
    uint32_t pluse = value;
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = pluse;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(htim, &sConfigOC, Channel);
    HAL_TIM_PWM_Start(htim, Channel);
}
/*********************************************************************
 *	function name:  start_pwm_buzzer
 *	describe: 开启pwm输出
 *	input:void
 *	output:void
 *	return:void
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
void start_pwm_buzzer(void)
{
    data_cnt = 44;
    HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_2);
}
/*********************************************************************
 *	function name:  stop_pwm_buzzer
 *	describe: 关闭PWM输出
 *	input:void
 *	output:void
 *	return:void
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
void stop_pwm_buzzer(void)
{
	HAL_TIM_PWM_Stop(&htim5, TIM_CHANNEL_2);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if(htim->Instance == htim5.Instance)
    {

        if(data_cnt < (sizeof(__8k_wav)-7000))
        {
            time = (__8k_wav[data_cnt]+0x80) * 125 / 256;
            data_cnt++;
            USER_PWM_SetDutyRatio(&htim5, TIM_CHANNEL_2, time);
        }
        else
        {
            stop_pwm_buzzer();
            OPAMP_POWER_DIS;
            data_cnt = 44;
        }
    }
    if(htim->Instance == htim6.Instance)
    {
        g_led_tick++;
        if(g_led_tick >= LED_TOGGLE_CNT)  
        {
            g_led_tick = V_ZERO;
            LED_RUN;
            WDI_REFRESH;
            HAL_UART_Receive_IT(&huart5, ar_usart5_rx_buffer, 1);    
        }
        g_voltage_check_tick++;
        if(g_voltage_check_tick >= VOLTAGE_CHECK_CNT)/*5分钟检测一次*/
        {
            g_voltage_check_tick = V_ZERO;        
            g_voltage_check_flag = V_ONE;
        }
        if(g_lock_open_flag == OPEN_DOOR)
        {
            g_unlock_tick++;
            if(g_unlock_tick >= (g_open_luck_time/5))
            {
                g_lock_open_flag = V_ZERO;
                g_unlock_tick = V_ZERO;
                g_open_luck_time = V_ZERO;
                g_unlock_flag = DOOR_UNLOCK_EN;
            }
        }
        if(g_ble_connect_flag == BLE_E95_CONNECTED)
        {
            g_ble_connect_count++;
            if(g_ble_connect_count >= (BLE_E95_CONNECTED_TIME_OUT/V_FIVE))
            {
                g_ble_connect_count = V_ZERO;
                g_ble_disconnect_flag = V_ONE;
            }
        }
        if(g_set_param_timeout_flag)
        {
            g_set_param_count++;
            if(g_set_param_count >= (SET_BLE_CMD_TIMEOUT/V_FIVE))
            {
                g_set_param_timeout_flag = V_ZERO;
                g_set_param_count = V_ZERO;
                g_set_step = g_current_step;
            }
        }
        if(g_nb_send_cmd_star_time)
        {
            g_nb_send_cmd_overtime++;
            if(g_nb_send_cmd_overtime >= (IOT_SEND_CMD_OVERTIME/V_FIVE))
            {
                g_nb_ctrl_step = g_nb_curren_step;
                g_nb_send_cmd_overtime = V_ZERO;
                g_nb_send_cmd_star_time = V_ZERO;
            }
        }
        if(g_start_open_door_flag)
        {
            g_start_open_door_cnt++;
            if(g_start_open_door_cnt >= (DOOR_OPEN_INTERVAL/V_FIVE))
            {
                g_start_open_door_flag = V_ZERO;
                g_start_open_door_cnt = V_ZERO;
                g_open_one_door_flag = V_ONE;
            }
        }
        if(g_iot_power_on_delay)
        {
            g_iot_power_on_delay_cnt++;
            if(g_iot_power_on_delay_cnt >= 2000)//IOT_POWER_ON_DELAY_TIME)
            {
                g_iot_power_on_delay_cnt = V_ZERO;
                g_iot_power_on_delay = V_ZERO;
                g_iot_wakeup_flag = V_ONE;
            }
        }
        if(g_uart5_tick)
        {
            g_uart5_tick--;
        }
    }
}
/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_hal.h"
#include "global.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define IOT_RESET_H_Pin GPIO_PIN_2
#define IOT_RESET_H_GPIO_Port GPIOE
#define SMBUS1_INT_Pin GPIO_PIN_3
#define SMBUS1_INT_GPIO_Port GPIOE
#define IOT_STATUS_Pin GPIO_PIN_4
#define IOT_STATUS_GPIO_Port GPIOE
#define IOT_POWER_KEY_Pin GPIO_PIN_5
#define IOT_POWER_KEY_GPIO_Port GPIOE
#define IOT_EN_Pin GPIO_PIN_6
#define IOT_EN_GPIO_Port GPIOE
#define SMBUS1_SDA_Pin GPIO_PIN_0
#define SMBUS1_SDA_GPIO_Port GPIOF
#define SMBUS1_SCL_Pin GPIO_PIN_1
#define SMBUS1_SCL_GPIO_Port GPIOF
#define BAT_ADC_IN_Pin GPIO_PIN_3
#define BAT_ADC_IN_GPIO_Port GPIOF
#define STM_BUZZER_Pin GPIO_PIN_7
#define STM_BUZZER_GPIO_Port GPIOF
#define PCF85263_INT_Pin GPIO_PIN_8
#define PCF85263_INT_GPIO_Port GPIOF
#define BAT_VOLTAGE_COM_ON_Pin GPIO_PIN_9
#define BAT_VOLTAGE_COM_ON_GPIO_Port GPIOF
#define BAT_VOLTAGE_COM_Pin GPIO_PIN_10
#define BAT_VOLTAGE_COM_GPIO_Port GPIOF
#define I2C3_SCL_Pin GPIO_PIN_0
#define I2C3_SCL_GPIO_Port GPIOC
#define I2C3_SDA_Pin GPIO_PIN_1
#define I2C3_SDA_GPIO_Port GPIOC
#define VIBRATION_INT_Pin GPIO_PIN_2
#define VIBRATION_INT_GPIO_Port GPIOC
#define CONN_IND_Pin GPIO_PIN_0
#define CONN_IND_GPIO_Port GPIOA
#define RS485_EN_Pin GPIO_PIN_1
#define RS485_EN_GPIO_Port GPIOA
#define SMBUS0_SDA_Pin GPIO_PIN_4
#define SMBUS0_SDA_GPIO_Port GPIOA
#define SMBUS0_INT_Pin GPIO_PIN_5
#define SMBUS0_INT_GPIO_Port GPIOA
#define SMBUS0_SCL_Pin GPIO_PIN_6
#define SMBUS0_SCL_GPIO_Port GPIOA
#define LOCK_EN_Pin GPIO_PIN_7
#define LOCK_EN_GPIO_Port GPIOA
#define INFRARED_EN_Pin GPIO_PIN_4
#define INFRARED_EN_GPIO_Port GPIOC
#define LOCK_DC12V_EN_Pin GPIO_PIN_5
#define LOCK_DC12V_EN_GPIO_Port GPIOC
#define BAT_V_ADC_EN_Pin GPIO_PIN_0
#define BAT_V_ADC_EN_GPIO_Port GPIOB
#define DETECT_DC12V_Pin GPIO_PIN_2
#define DETECT_DC12V_GPIO_Port GPIOB
#define DETECT_DC12V_EN_Pin GPIO_PIN_11
#define DETECT_DC12V_EN_GPIO_Port GPIOF
#define LED_RUN_Pin GPIO_PIN_12
#define LED_RUN_GPIO_Port GPIOF
#define GOODS_A0_Pin GPIO_PIN_13
#define GOODS_A0_GPIO_Port GPIOF
#define GOODS_A1_Pin GPIO_PIN_14
#define GOODS_A1_GPIO_Port GPIOF
#define GOODS_A2_Pin GPIO_PIN_15
#define GOODS_A2_GPIO_Port GPIOF
#define GOODS_A3_Pin GPIO_PIN_0
#define GOODS_A3_GPIO_Port GPIOG
#define GOODS_A4_Pin GPIO_PIN_1
#define GOODS_A4_GPIO_Port GPIOG
#define GOODS_A5_Pin GPIO_PIN_7
#define GOODS_A5_GPIO_Port GPIOE
#define GOODS_A6_Pin GPIO_PIN_8
#define GOODS_A6_GPIO_Port GPIOE
#define GOODS_A7_Pin GPIO_PIN_9
#define GOODS_A7_GPIO_Port GPIOE
#define GOODS_ENA_Pin GPIO_PIN_10
#define GOODS_ENA_GPIO_Port GPIOE
#define GOODS_ENB_Pin GPIO_PIN_11
#define GOODS_ENB_GPIO_Port GPIOE
#define SPI_CS_Pin GPIO_PIN_12
#define SPI_CS_GPIO_Port GPIOE
#define SPI_CLK_Pin GPIO_PIN_13
#define SPI_CLK_GPIO_Port GPIOE
#define SPI_MISO_Pin GPIO_PIN_14
#define SPI_MISO_GPIO_Port GPIOE
#define SPI_MOSI_Pin GPIO_PIN_15
#define SPI_MOSI_GPIO_Port GPIOE
#define UART3_TX_BLE_Pin GPIO_PIN_10
#define UART3_TX_BLE_GPIO_Port GPIOB
#define UART3_RX_BLE_Pin GPIO_PIN_11
#define UART3_RX_BLE_GPIO_Port GPIOB
#define GOODS_ENC_Pin GPIO_PIN_12
#define GOODS_ENC_GPIO_Port GPIOB
#define CTRL_BUS_A7_Pin GPIO_PIN_13
#define CTRL_BUS_A7_GPIO_Port GPIOB
#define CTRL_BUS_A6_Pin GPIO_PIN_14
#define CTRL_BUS_A6_GPIO_Port GPIOB
#define CTRL_BUS_A5_Pin GPIO_PIN_15
#define CTRL_BUS_A5_GPIO_Port GPIOB
#define CTRL_BUS_A4_Pin GPIO_PIN_8
#define CTRL_BUS_A4_GPIO_Port GPIOD
#define CTRL_BUS_A3_Pin GPIO_PIN_9
#define CTRL_BUS_A3_GPIO_Port GPIOD
#define CTRL_BUS_A2_Pin GPIO_PIN_10
#define CTRL_BUS_A2_GPIO_Port GPIOD
#define CTRL_BUS_A1_Pin GPIO_PIN_11
#define CTRL_BUS_A1_GPIO_Port GPIOD
#define CTRL_BUS_A0_Pin GPIO_PIN_12
#define CTRL_BUS_A0_GPIO_Port GPIOD
#define CHIP_SELECT_01_Pin GPIO_PIN_13
#define CHIP_SELECT_01_GPIO_Port GPIOD
#define CTRL_BUS_POWER_Pin GPIO_PIN_14
#define CTRL_BUS_POWER_GPIO_Port GPIOD
#define CHIP_SELECT_02_Pin GPIO_PIN_15
#define CHIP_SELECT_02_GPIO_Port GPIOD
#define TEMP_Pin GPIO_PIN_2
#define TEMP_GPIO_Port GPIOG
#define H_T_POWER_CTRL_Pin GPIO_PIN_3
#define H_T_POWER_CTRL_GPIO_Port GPIOG
#define HUMI_Pin GPIO_PIN_4
#define HUMI_GPIO_Port GPIOG
#define BT_CTRL_Pin GPIO_PIN_5
#define BT_CTRL_GPIO_Port GPIOG
#define BLE_POWER_CTRL_Pin GPIO_PIN_6
#define BLE_POWER_CTRL_GPIO_Port GPIOG
#define BLE_MODE_CTRL_Pin GPIO_PIN_7
#define BLE_MODE_CTRL_GPIO_Port GPIOG
#define PWR_IND_Pin GPIO_PIN_8
#define PWR_IND_GPIO_Port GPIOG
#define DOUT_IND_Pin GPIO_PIN_6
#define DOUT_IND_GPIO_Port GPIOC
#define BLE_RESET_Pin GPIO_PIN_7
#define BLE_RESET_GPIO_Port GPIOC
#define BUSY_IND_Pin GPIO_PIN_8
#define BUSY_IND_GPIO_Port GPIOC
#define BCD1_IN8_Pin GPIO_PIN_9
#define BCD1_IN8_GPIO_Port GPIOC
#define BCD1_IN4_Pin GPIO_PIN_8
#define BCD1_IN4_GPIO_Port GPIOA
#define BCD1_IN2_Pin GPIO_PIN_9
#define BCD1_IN2_GPIO_Port GPIOA
#define BCD1_IN1_Pin GPIO_PIN_10
#define BCD1_IN1_GPIO_Port GPIOA
#define OPAMP_POWER_EN_Pin GPIO_PIN_10
#define OPAMP_POWER_EN_GPIO_Port GPIOC
#define LED_DC12V_EN_Pin GPIO_PIN_11
#define LED_DC12V_EN_GPIO_Port GPIOC
#define UART5_TX_IOT_Pin GPIO_PIN_12
#define UART5_TX_IOT_GPIO_Port GPIOC
#define CHIP_SELECT_03_Pin GPIO_PIN_0
#define CHIP_SELECT_03_GPIO_Port GPIOD
#define CHIP_SELECT_04_Pin GPIO_PIN_1
#define CHIP_SELECT_04_GPIO_Port GPIOD
#define UART5_RX_IOT_Pin GPIO_PIN_2
#define UART5_RX_IOT_GPIO_Port GPIOD
#define DONE_Pin GPIO_PIN_5
#define DONE_GPIO_Port GPIOB
#define GOODS_END_Pin GPIO_PIN_6
#define GOODS_END_GPIO_Port GPIOB
#define GOODS_ENE_Pin GPIO_PIN_7
#define GOODS_ENE_GPIO_Port GPIOB
#define GOODS_ENF_Pin GPIO_PIN_8
#define GOODS_ENF_GPIO_Port GPIOB
#define POWER_5V_CTRL_Pin GPIO_PIN_9
#define POWER_5V_CTRL_GPIO_Port GPIOB
#define GOODS_POWER_CTRL_Pin GPIO_PIN_0
#define GOODS_POWER_CTRL_GPIO_Port GPIOE
#define LPUART1_RX_Pin GPIO_PIN_10
#define LPUART1_RX_GPIO_Port GPIOB
#define LPUART1_TX_Pin GPIO_PIN_11
#define LPUART1_TX_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */
#define IOT_POWER_ON_CNT    3
extern volatile uint8_t g_start_http_flag;
extern uint8_t g_iot_power_on_cnt;
extern uint8_t g_iot_wakeup_flag;
extern uint16_t g_iot_power_on_delay_cnt;
extern uint8_t g_iot_power_on_delay;
extern const uint8_t ar_resp_iot_test_success[27];
/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

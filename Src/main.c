/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "user_gpio.h"
#include "ble_e95.h"
#include "pcf85263.h"
#include "iot.h"
#include "aes.h"
#include <stdio.h>
#include <errno.h>
#include <sys/unistd.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

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
uint8_t g_reset_flag = V_ONE;
RTC_DATE_TIME_T rtc_alarm_time = {0,0,59,23,1,1,2021};
RTC_COUNT_TIME_T default_rtc_alarm_time = {0,0,0,0,0,0};
RTC_COUNT_TIME_T rtc_read_time;
RTC_COUNT_TIME_T rtc_set_time = {0,0,0,1,0,0};

/*iot唤醒标志*/
uint8_t g_iot_wakeup_flag = V_ZERO;
uint16_t g_iot_power_on_delay_cnt = V_ZERO;
uint8_t g_iot_power_on_delay = V_ZERO;
uint8_t g_iot_first_send_cmd = V_ZERO;     /*iot开机后板子会收到一些数据，所以发at指令前先清楚下buff*/

/*iot测试结果返回协议数据*/
const uint8_t ar_resp_iot_test_success[27] = {0xC5, 0xAA, 0x1B, 0x00, 0x03, 0x10, 0x10, 0x5E, 0x3E, 0x52, \
                                            0x0F, 0x4B, 0xAC, 0xA0, 0x2B, 0x7F, 0x75, 0xD2, 0x42, 0x70, \
                                            0x07, 0x40, 0x5A, 0x00, 0x00, 0xD1, 0xAA};
const uint8_t ar_resp_iot_test_fail[27] = {0xC5,0xAA,0x1B,0x00,0x03,0x10,0x10,0xE3,0xA0,0xCB,0xF6,0x74, \
                                         0x74,0x91,0x93,0x81,0xE9,0xDF,0x43,0x11,0x3B,0x33,0x62,0x00, \
                                         0x00,0x84,0xAA};
/*闹钟中断设置小时数计数*/
uint8_t g_alarm_cnt = V_ZERO;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/*重新定义__write函数*/

int _write(int fd, char *ptr, int len)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)ptr, len, 0xFFFF);
    return len;
}
#if 0
int _write(int file, char *data, int len)
{
    if ((file != STDOUT_FILENO) && (file != STDERR_FILENO))
    {
        errno = EBADF;
        return -1;
    }
    HAL_StatusTypeDef status = HAL_UART_Transmit(&huart2, (uint8_t *)data, len, 1000);
    return (status == HAL_OK ? len : 0);
}
#endif
/*iot上电联、网标志位*/
volatile uint8_t g_start_http_flag = I_ZERO;
/*iot联网失败次数，大于三次判断失败*/
uint8_t g_iot_power_on_cnt = I_ZERO;

void low_voltage_check(void)
{
    if(g_voltage_check_flag)
    {
        pcf85263_query_time(&rtc_read_time);
        g_voltage_check_flag = I_ZERO;
        if(LOCK_POWER_SOURCE == PIN_LEVEL_HIGH)    /*判读供电方式：市电或者电池包*/
        {
            BAT_ADC_EN;    /*使能电池包电压读取*/
            HAL_Delay(10);
            get_adc3_value();    /*电池包供电的话就读取adc值*/;        
            printf("voltage: %d\n", ar_get_voltage_value[V_ZERO]);                    
            if(ar_get_voltage_value[V_ZERO] < (MIN_VOLTAGE - DEVICE_VOLTAGE_DROP))/*电压低于6V上报服务器*/
            {         
                g_min_voltage_flag = V_ONE;    
                g_start_http_flag = V_ONE;
            } 
            BAT_ADC_DIS;                    
        }         
    }  
}

void iot_http_get_process(void)
{
    if(g_start_http_flag == V_ONE)
    {  
        printf("g_iot_power_on_cnt %d\r\n", g_iot_power_on_cnt);
        if(g_iot_power_on_cnt <= IOT_POWER_ON_CNT)
        {
            g_start_http_flag = V_ZERO;
            printf("iot power on!\r\n");
            iot_power_on_ctrl();
            g_iot_power_on_delay = V_ONE;
            g_iot_power_on_delay_cnt = V_ZERO;
            g_iot_first_send_cmd = V_ONE;
        }
        else
        {
            if(g_iot_start_test == V_ONE)
            {
                g_iot_start_test = V_ZERO;
                HAL_UART_Transmit(&hlpuart1, ar_resp_iot_test_fail, 0x1B, 0xffffff);
                printf("IOT TEST FAILED!\r\n");
            }
            g_start_http_flag = V_ZERO;
            g_iot_power_on_cnt = V_ZERO;
            printf("iot networking failed!\r\n");
        }
    }
    if(g_iot_wakeup_flag)
    {
        if(g_iot_first_send_cmd == V_ONE)
        {
            g_iot_first_send_cmd = V_ZERO;
            g_usart5_rx_finish = V_ZERO;
        }
        g_nb_ctrl_step = IOT_TEST_UART;
        g_iot_wakeup_flag = V_ZERO;
    }
    if((g_uart5_receive_finish == V_ONE) && (g_uart5_tick == V_ZERO))
    {
        uart5_recv_data_fun();
        receive_nb_data_handle();
        g_uart5_receive_finish = V_ZERO;
        g_uart_tmp5.readpos = g_uart_tmp5.writepos;
    }
    nb_http_set_param();
}

/*定义_sys_exit()以避免使用半主机模式*/
void _sys_exit(int x)
{
    x = x;
}
uint8_t rtc_read_alarm = V_ZERO;
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
  MX_DMA_Init();
  MX_ADC3_Init();
  MX_I2C3_Init();
  MX_SPI1_Init();
  MX_TIM5_Init();
  MX_UART5_Init();
  MX_USART2_UART_Init();
  MX_LPUART1_UART_Init();
  MX_TIM6_Init();
  AES_init_ctx(&aes_test, aes_key);
  pcf85263_check_inta();
  init_pcf85263_interrupt();
  //pcf85263_set_time(rtc_set_time); 
  pcf85263_set_sw_mode_alarm(V_ONE, default_rtc_alarm_time);
  pcf85263_query_time(&rtc_read_time);
  lis2dh_init();
  /* USER CODE BEGIN 2 */
  if(HAL_TIM_Base_Start_IT(&htim5) != HAL_OK)
  {
      Error_Handler();
  }
  if(HAL_TIM_Base_Start_IT(&htim6) != HAL_OK)
  {
      Error_Handler();
  }
  /* USER CODE END 2 */
  HAL_UART_Receive_DMA(&hlpuart1, ar_usart1_rx_buffer, MAX_RECEIVE_DATA);
  __HAL_UART_ENABLE_IT(&hlpuart1, UART_IT_IDLE); 

  if (HAL_UART_Receive_IT(&huart5, ar_usart5_rx_buffer, 1) != HAL_OK)
  {
      Error_Handler();
  }
  /* Infinite loop */
  ble_load_name();
  ble_e95_reset();
  printf("ctrl board start!");
  //g_start_http_flag = V_ONE;
  /* USER CODE BEGIN WHILE */
  SENSOR_POWER_EN;
  while (1)
  {
    /* USER CODE END WHILE */
      if(g_unlock_flag == DOOR_UNLOCK_EN)
      {
          g_unlock_flag = V_ZERO;
          door_control_index(g_lock_number, OPEN_DOOR_TIME, CLOSE_DOOR);
          g_lock_number = V_ZERO;
          OPAMP_POWER_EN;
          start_pwm_buzzer();
      }
      /*BLE DISCONNECTED*/
      if(g_ble_disconnect_flag)
      {
          E95_BROADCAST_DIS;
          HAL_Delay(50);
          E95_BROADCAST_EN;
          g_ble_disconnect_flag = V_ZERO;
          //g_ble_connect_flag = BLE_E95_DISCONNECTED;
          g_ble_connect_count = V_ZERO;
      }
      if(g_usart1_rx_finish == 1)
      {
          usart1_protocol_handle();
      }
      if(g_open_one_door_flag)
      {
          open_multiple_doors_fun();
      }
      ble_e92_set_fun();
      if(g_rtc_alarm_flag)
      {
          pcf85263_read_data(PCF8563_ADDR_FLAGS, &rtc_read_alarm, V_ONE);
          printf("alarm flag = %d\n",rtc_read_alarm);
          pcf85263_clear_alarm_flag();
          g_rtc_alarm_flag = V_ZERO;
          pcf85263_query_time(&rtc_read_time);
          if(rtc_read_time.minute == default_rtc_alarm_time.minute)
          {
              printf("alarm\r\n");            
              g_alarm_cnt++;
              printf("g_alarm_cnt = %d\r\n", g_alarm_cnt);  
              printf("g_alarm_set_num = %d\r\n", g_alarm_set_num); 
              if(g_alarm_set_num >= V_SIX)
              {
                  if(g_alarm_cnt == g_alarm_set_num)
                  {
                      g_start_http_flag = V_ONE; 
                      g_alarm_cnt = V_ZERO;
                      printf("start http\r\n");
                  }                  
              }
              else
              {
                  if(g_alarm_cnt == V_SIX)
                  {
                      g_start_http_flag = V_ONE; 
                      g_alarm_cnt = V_ZERO;
                      printf("start http\r\n");
                  }                   
              }            
          }
          pcf85263_set_sw_mode_alarm(V_ONE, default_rtc_alarm_time);         
      }
      low_voltage_check();
      iot_http_get_process();
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
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV8;  /*system clock 5M*/
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_USART3
                              |RCC_PERIPHCLK_UART5|RCC_PERIPHCLK_I2C3
                              |RCC_PERIPHCLK_ADC;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Usart3ClockSelection = RCC_USART3CLKSOURCE_PCLK1;
  PeriphClkInit.Uart5ClockSelection = RCC_UART5CLKSOURCE_PCLK1;
  PeriphClkInit.I2c3ClockSelection = RCC_I2C3CLKSOURCE_PCLK1;
  PeriphClkInit.AdcClockSelection = RCC_ADCCLKSOURCE_PLLSAI1;
  PeriphClkInit.PLLSAI1.PLLSAI1Source = RCC_PLLSOURCE_HSE;
  PeriphClkInit.PLLSAI1.PLLSAI1M = 1;
  PeriphClkInit.PLLSAI1.PLLSAI1N = 8;
  PeriphClkInit.PLLSAI1.PLLSAI1P = RCC_PLLP_DIV7;
  PeriphClkInit.PLLSAI1.PLLSAI1Q = RCC_PLLQ_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1R = RCC_PLLR_DIV2;
  PeriphClkInit.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_ADC1CLK;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure the main internal regulator output voltage 
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
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
void assert_failed(char *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

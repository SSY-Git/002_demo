
/*
    Copyright 2019 SHENZHEN ZHILAI SCI AND TECH CO., LTD,
    15th floor, C3 building, Nanshan ipark,Xueyuan road 1001,
	Nanshan District, Shenzhen 518052, China.
    All rights are reserved. Reproduction in whole or in part is prohibited
    without the prior written consent of the copyright owner.

    COMPANY CONFIDENTIAL

Filename  :  user_gpio.h


Rev     Date    Author  Comments
-------------------------------------------------------------------------------
001  2021.3.27   zjs
002
003
004
-------------------------------------------------------------------------------

@Begin
@module
        Module name
        Module description ()
@end
*/

/* Includes ------------------------------------------------------------------*/
#ifndef __user_gpio_H
#define __user_gpio_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "global.h"

#define HC245_BUS_EN     0x01
#define HC245_BUS_DIS     0x00
#define OPEN_DOOR     0x01
#define CLOSE_DOOR     0x00

#define DC12V_DETECT_EN     HAL_GPIO_WritePin(DETECT_DC12V_EN_GPIO_Port, DETECT_DC12V_EN_Pin, GPIO_PIN_SET)
#define DC12V_DETECT_DIS     HAL_GPIO_WritePin(DETECT_DC12V_EN_GPIO_Port, DETECT_DC12V_EN_Pin, GPIO_PIN_RESET)
#define LOCK_POWER_SOURCE     HAL_GPIO_ReadPin(DETECT_DC12V_GPIO_Port, DETECT_DC12V_Pin)
#define LOCK_POWER_SOURCE_BAT     0x01
#define LOCK_POWER_SOURCE_DC12V     0x00
#define BAT_POWER_EN     HAL_GPIO_WritePin(LOCK_EN_GPIO_Port, LOCK_EN_Pin, GPIO_PIN_RESET)     /*赢家设计全都用这个电源开锁*/
#define BAT_POWER_DIS     HAL_GPIO_WritePin(LOCK_EN_GPIO_Port, LOCK_EN_Pin, GPIO_PIN_SET)
#define DC12V_POWER_EN     HAL_GPIO_WritePin(LOCK_DC12V_EN_GPIO_Port, LOCK_DC12V_EN_Pin, GPIO_PIN_SET)
#define DC12V_POWER_DIS     HAL_GPIO_WritePin(LOCK_DC12V_EN_GPIO_Port, LOCK_DC12V_EN_Pin, GPIO_PIN_RESET)
#define CHIP_SELECT_01EN     HAL_GPIO_WritePin(CHIP_SELECT_01_GPIO_Port, CHIP_SELECT_01_Pin, GPIO_PIN_RESET)
#define CHIP_SELECT_01DIS     HAL_GPIO_WritePin(CHIP_SELECT_01_GPIO_Port, CHIP_SELECT_01_Pin, GPIO_PIN_SET)
#define CHIP_SELECT_02EN     HAL_GPIO_WritePin(CHIP_SELECT_02_GPIO_Port, CHIP_SELECT_02_Pin, GPIO_PIN_RESET)
#define CHIP_SELECT_02DIS     HAL_GPIO_WritePin(CHIP_SELECT_02_GPIO_Port, CHIP_SELECT_02_Pin, GPIO_PIN_SET)
#define CHIP_SELECT_03EN     HAL_GPIO_WritePin(CHIP_SELECT_03_GPIO_Port, CHIP_SELECT_03_Pin, GPIO_PIN_RESET)
#define CHIP_SELECT_03DIS     HAL_GPIO_WritePin(CHIP_SELECT_03_GPIO_Port, CHIP_SELECT_03_Pin, GPIO_PIN_SET)
#define CHIP_SELECT_04EN     HAL_GPIO_WritePin(CHIP_SELECT_04_GPIO_Port, CHIP_SELECT_04_Pin, GPIO_PIN_RESET)
#define CHIP_SELECT_04DIS     HAL_GPIO_WritePin(CHIP_SELECT_04_GPIO_Port, CHIP_SELECT_04_Pin, GPIO_PIN_SET)
#define CTRL_BUS_POWER_EN     HAL_GPIO_WritePin(CTRL_BUS_POWER_GPIO_Port, CTRL_BUS_POWER_Pin, GPIO_PIN_SET)
#define CTRL_BUS_POWER_DIS     HAL_GPIO_WritePin(CTRL_BUS_POWER_GPIO_Port, CTRL_BUS_POWER_Pin, GPIO_PIN_RESET)
#define CTRL_BUS_AO_HIGH     HAL_GPIO_WritePin(CTRL_BUS_A0_GPIO_Port, CTRL_BUS_A0_Pin, GPIO_PIN_SET)
#define CTRL_BUS_AO_LOW     HAL_GPIO_WritePin(CTRL_BUS_A0_GPIO_Port, CTRL_BUS_A0_Pin, GPIO_PIN_RESET)
#define CTRL_BUS_A1_HIGH     HAL_GPIO_WritePin(CTRL_BUS_A1_GPIO_Port, CTRL_BUS_A1_Pin, GPIO_PIN_SET)
#define CTRL_BUS_A1_LOW     HAL_GPIO_WritePin(CTRL_BUS_A1_GPIO_Port, CTRL_BUS_A1_Pin, GPIO_PIN_RESET)
#define CTRL_BUS_A2_HIGH     HAL_GPIO_WritePin(CTRL_BUS_A2_GPIO_Port, CTRL_BUS_A2_Pin, GPIO_PIN_SET)
#define CTRL_BUS_A2_LOW     HAL_GPIO_WritePin(CTRL_BUS_A2_GPIO_Port, CTRL_BUS_A2_Pin, GPIO_PIN_RESET)
#define CTRL_BUS_A3_HIGH     HAL_GPIO_WritePin(CTRL_BUS_A3_GPIO_Port, CTRL_BUS_A3_Pin, GPIO_PIN_SET)
#define CTRL_BUS_A3_LOW     HAL_GPIO_WritePin(CTRL_BUS_A3_GPIO_Port, CTRL_BUS_A3_Pin, GPIO_PIN_RESET)
#define CTRL_BUS_A4_HIGH     HAL_GPIO_WritePin(CTRL_BUS_A4_GPIO_Port, CTRL_BUS_A4_Pin, GPIO_PIN_SET)
#define CTRL_BUS_A4_LOW     HAL_GPIO_WritePin(CTRL_BUS_A4_GPIO_Port, CTRL_BUS_A4_Pin, GPIO_PIN_RESET)
#define CTRL_BUS_A5_HIGH     HAL_GPIO_WritePin(CTRL_BUS_A5_GPIO_Port, CTRL_BUS_A5_Pin, GPIO_PIN_SET)
#define CTRL_BUS_A5_LOW     HAL_GPIO_WritePin(CTRL_BUS_A5_GPIO_Port, CTRL_BUS_A5_Pin, GPIO_PIN_RESET)
#define CTRL_BUS_A6_HIGH     HAL_GPIO_WritePin(CTRL_BUS_A6_GPIO_Port, CTRL_BUS_A6_Pin, GPIO_PIN_SET)
#define CTRL_BUS_A6_LOW     HAL_GPIO_WritePin(CTRL_BUS_A6_GPIO_Port, CTRL_BUS_A6_Pin, GPIO_PIN_RESET)
#define CTRL_BUS_A7_HIGH     HAL_GPIO_WritePin(CTRL_BUS_A7_GPIO_Port, CTRL_BUS_A7_Pin, GPIO_PIN_SET)
#define CTRL_BUS_A7_LOW     HAL_GPIO_WritePin(CTRL_BUS_A7_GPIO_Port, CTRL_BUS_A7_Pin, GPIO_PIN_RESET)
#define SN74_A_EN     HAL_GPIO_WritePin(GOODS_ENA_GPIO_Port, GOODS_ENA_Pin, GPIO_PIN_RESET)
#define SN74_A_DIS     HAL_GPIO_WritePin(GOODS_ENA_GPIO_Port, GOODS_ENA_Pin, GPIO_PIN_SET)
#define SN74_B_EN     HAL_GPIO_WritePin(GOODS_ENB_GPIO_Port, GOODS_ENB_Pin, GPIO_PIN_RESET)
#define SN74_B_DIS     HAL_GPIO_WritePin(GOODS_ENB_GPIO_Port, GOODS_ENB_Pin, GPIO_PIN_SET)
#define SN74_C_EN     HAL_GPIO_WritePin(GOODS_ENC_GPIO_Port, GOODS_ENC_Pin, GPIO_PIN_RESET)
#define SN74_C_DIS     HAL_GPIO_WritePin(GOODS_ENC_GPIO_Port, GOODS_ENC_Pin, GPIO_PIN_SET)
#define SN74_D_EN     HAL_GPIO_WritePin(GOODS_END_GPIO_Port, GOODS_END_Pin, GPIO_PIN_RESET)
#define SN74_D_DIS     HAL_GPIO_WritePin(GOODS_END_GPIO_Port, GOODS_END_Pin, GPIO_PIN_SET)
#define SN74_E_EN     HAL_GPIO_WritePin(GOODS_ENE_GPIO_Port, GOODS_ENE_Pin, GPIO_PIN_RESET)
#define SN74_E_DIS     HAL_GPIO_WritePin(GOODS_ENE_GPIO_Port, GOODS_ENE_Pin, GPIO_PIN_SET)
#define SN74_F_EN     HAL_GPIO_WritePin(GOODS_ENF_GPIO_Port, GOODS_ENF_Pin, GPIO_PIN_RESET)
#define SN74_F_DIS     HAL_GPIO_WritePin(GOODS_ENF_GPIO_Port, GOODS_ENF_Pin, GPIO_PIN_SET)
#define GOODS_POWER_3V3_EN     HAL_GPIO_WritePin(GOODS_POWER_CTRL_GPIO_Port, GOODS_POWER_CTRL_Pin, GPIO_PIN_RESET)
#define GOODS_POWER_3V3_DIS     HAL_GPIO_WritePin(GOODS_POWER_CTRL_GPIO_Port, GOODS_POWER_CTRL_Pin, GPIO_PIN_SET)
#define POWER_5V_EN     HAL_GPIO_WritePin(POWER_5V_CTRL_GPIO_Port, POWER_5V_CTRL_Pin, GPIO_PIN_SET)
#define POWER_5V_DIS     HAL_GPIO_WritePin(POWER_5V_CTRL_GPIO_Port, POWER_5V_CTRL_Pin, GPIO_PIN_RESET)
#define GOODS_A7_STATUS     HAL_GPIO_ReadPin(GOODS_A7_GPIO_Port, GOODS_A7_Pin)
#define GOODS_A6_STATUS     HAL_GPIO_ReadPin(GOODS_A6_GPIO_Port, GOODS_A6_Pin)
#define GOODS_A5_STATUS     HAL_GPIO_ReadPin(GOODS_A5_GPIO_Port, GOODS_A5_Pin)
#define GOODS_A4_STATUS     HAL_GPIO_ReadPin(GOODS_A4_GPIO_Port, GOODS_A4_Pin)
#define GOODS_A3_STATUS     HAL_GPIO_ReadPin(GOODS_A3_GPIO_Port, GOODS_A3_Pin)
#define GOODS_A2_STATUS     HAL_GPIO_ReadPin(GOODS_A2_GPIO_Port, GOODS_A2_Pin)
#define GOODS_A1_STATUS     HAL_GPIO_ReadPin(GOODS_A1_GPIO_Port, GOODS_A1_Pin)
#define GOODS_A0_STATUS     HAL_GPIO_ReadPin(GOODS_A0_GPIO_Port, GOODS_A0_Pin)
#define INFRARED_POWER_EN     HAL_GPIO_WritePin(INFRARED_EN_GPIO_Port, INFRARED_EN_Pin, GPIO_PIN_SET)
#define INFRARED_POWER_DIS     HAL_GPIO_WritePin(INFRARED_EN_GPIO_Port, INFRARED_EN_Pin, GPIO_PIN_RESET)
#define E95_RESET_HIGH     HAL_GPIO_WritePin(BLE_RESET_GPIO_Port, BLE_RESET_Pin, GPIO_PIN_SET)
#define E95_RESET_LOW     HAL_GPIO_WritePin(BLE_RESET_GPIO_Port, BLE_RESET_Pin, GPIO_PIN_RESET)
#define E95_MODE_CMD     HAL_GPIO_WritePin(BLE_MODE_CTRL_GPIO_Port, BLE_MODE_CTRL_Pin, GPIO_PIN_RESET)   /*default mode*/
#define E95_MODE_TRANSPARENT     HAL_GPIO_WritePin(BLE_MODE_CTRL_GPIO_Port, BLE_MODE_CTRL_Pin, GPIO_PIN_SET)
#define E95_BROADCAST_EN     HAL_GPIO_WritePin(BT_CTRL_GPIO_Port, BT_CTRL_Pin, GPIO_PIN_RESET)   /*default mode*/
#define E95_BROADCAST_DIS     HAL_GPIO_WritePin(BT_CTRL_GPIO_Port, BT_CTRL_Pin, GPIO_PIN_SET)
#define BAT_ADC_EN     HAL_GPIO_WritePin(BAT_V_ADC_EN_GPIO_Port, BAT_V_ADC_EN_Pin, GPIO_PIN_SET)
#define BAT_ADC_DIS     HAL_GPIO_WritePin(BAT_V_ADC_EN_GPIO_Port, BAT_V_ADC_EN_Pin, GPIO_PIN_RESET)
#define BAT_VOLTAGE_COM_EN     HAL_GPIO_WritePin(BAT_VOLTAGE_COM_ON_GPIO_Port, BAT_VOLTAGE_COM_ON_Pin, GPIO_PIN_SET)
#define BAT_VOLTAGE_COM_DIS     HAL_GPIO_WritePin(BAT_VOLTAGE_COM_ON_GPIO_Port, BAT_VOLTAGE_COM_ON_Pin, GPIO_PIN_RESET)

#define SENSOR_POWER_EN     HAL_GPIO_WritePin(H_T_POWER_CTRL_GPIO_Port, H_T_POWER_CTRL_Pin, GPIO_PIN_RESET)
#define SENSOR_POWER_DIS     HAL_GPIO_WritePin(H_T_POWER_CTRL_GPIO_Port, H_T_POWER_CTRL_Pin, GPIO_PIN_SET)
#define DHT22_Port     HAL_GPIO_ReadPin(HUMI_GPIO_Port, HUMI_Pin)
#define DHT22_High     HAL_GPIO_WritePin(HUMI_GPIO_Port, HUMI_Pin, GPIO_PIN_SET)
#define DHT22_Low     HAL_GPIO_WritePin(HUMI_GPIO_Port, HUMI_Pin, GPIO_PIN_RESET)
#define READ_DOORS_STATUS     0x01
#define READ_GOODS_STATUS     0x02

#define IOT_ON_OFF_EN     HAL_GPIO_WritePin(IOT_POWER_KEY_GPIO_Port, IOT_POWER_KEY_Pin, GPIO_PIN_SET)     /*iot_on_off*/
#define IOT_ON_OFF_DIS     HAL_GPIO_WritePin(IOT_POWER_KEY_GPIO_Port, IOT_POWER_KEY_Pin, GPIO_PIN_RESET)

#define IOT_POWER_EN     HAL_GPIO_WritePin(IOT_EN_GPIO_Port, IOT_EN_Pin, GPIO_PIN_RESET)
#define IOT_POWER_DIS     HAL_GPIO_WritePin(IOT_EN_GPIO_Port, IOT_EN_Pin, GPIO_PIN_SET)

#define IOT_RESET_H     HAL_GPIO_WritePin(IOT_EN_GPIO_Port, IOT_RESET_H_Pin, GPIO_PIN_SET)     
#define IOT_RESET_L     HAL_GPIO_WritePin(IOT_EN_GPIO_Port, IOT_RESET_H_Pin, GPIO_PIN_RESET)

#define IOT_STATUS_READ     HAL_GPIO_ReadPin(IOT_STATUS_GPIO_Port, IOT_STATUS_Pin)

#define OPAMP_POWER_EN     HAL_GPIO_WritePin(OPAMP_POWER_EN_GPIO_Port, OPAMP_POWER_EN_Pin, GPIO_PIN_SET)
#define OPAMP_POWER_DIS     HAL_GPIO_WritePin(OPAMP_POWER_EN_GPIO_Port, OPAMP_POWER_EN_Pin, GPIO_PIN_RESET)

#define BLE_E95_CONNECTED     0x01
#define BLE_E95_DISCONNECTED     0x00
#define BLE_E95_CONNECTED_TIME_OUT     180000   //60s

extern void door_status_index(uint8_t index);
extern void door_status_refresh(void);
extern void get_one_good_state_fun(uint8_t index);
extern void get_all_goods_state_fun(void);
extern lock_ctrl_return_type_t door_control_index(uint8_t inputval, uint16_t opentime, uint8_t ctrl_door_action);

extern uint8_t g_door_stat[V_THREE];
extern uint8_t g_stdoor_stat[MAX_DOOR_NUM];
extern uint8_t g_all_goods_status[V_THREE];
extern uint8_t g_item_stat[MAX_DOOR_NUM];
extern volatile uint8_t g_lock_number;
extern volatile uint16_t g_open_luck_time;
extern volatile uint8_t g_lock_open_flag;
extern volatile uint32_t g_ble_connect_count;
extern volatile uint8_t g_ble_connect_flag;
extern uint8_t g_rtc_alarm_flag;

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


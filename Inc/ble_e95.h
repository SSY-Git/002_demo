
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
#ifndef __ble_e95_H
#define __ble_e95_H

#ifdef __cplusplus
extern "C" {
#endif

#include "global.h"

#define E95_SET_MODE     0x00
#define E95_WORK_MODE     0x01

#define FRAM_FIRST_HEAD  0x04
#define FRAM_SECOND_HEAD 0xFC

#define BLE_NAME_SET     0x07
#define BLE_BROADCAST_SET     0X09
#define BLE_POWER_SET     0x0b
#define BLE_CHECK_MODEL     0x18

#define SET_E95_NAME     0x01
#define SET_E95_BROADCAST     0x02
#define SET_E95_POWER     0x03
#define GET_E95_MODEL     0x04

#define BLE_LEN_INDEX     0x03
#define BLE_STATUS_INDEX     0x04
#define BLE_INFOR_INDEX     0x05
#define BLE_STATUS_SUCCESS     0x00

#define BLE_SEND_CMD_MAX_CNT     0x03

#define AES_D_E_DATA_LEN     0x40

#define OPEN_SLOT_SUCCESS          0x00
#define OPEN_SLOT_FAILED           0x01

#define MAINS_VOLTAGE    900
#define PIN_LEVEL_HIGH    0x01
#define PIN_LEVEL_LOW     0x00


#define FRAME_FIRST_HAED           0xCA
#define FRAME_FIRST_SECOND_HEAD    0x55
#define FRAME_RES_FIRST_HEAD       0xC5
#define FRAME_RES_SECOND_HEAD      0xAA
#define FRAME_TERMINATOR           0xAA
#define CUSTOMER_NUM               0x1010 /*default*/
#define BOARD_NUM                  0x01   /*default*/
#define PACKAGE_NUM                0x01   /*default*/
#define DEFAULT_MAX_SLOT_NUM       0x05

#define GET_ALL_SLOT_STATUS        0x01
#define GET_ONE_SLOT_STATUS        0x02
#define GET_ALL_SLOT_GOODS_STATUS  0x03
#define GET_ONE_SLOT_GOODS_STATUS  0x04
#define OPEN_ONE_SLOT_DOOR         0x05
#define GET_SOFTWARE_VER           0x06
#define GET_BLE_NAME               0x07
#define SET_BLE_NAME               0x08
#define GET_BAT_VOLTAGE            0x09
#define SET_RTC_TIME               0x0A
#define GET_RTC_TIME               0x0B
#define SET_BROADCAST_PARA         0x0C
#define GET_BROADCAST_PARA         0x0D
#define GET_TEMP                   0x0E
#define GET_HUM                    0x0F
#define SET_SN                     0x10
#define GET_SN                     0x11
#define GET_BUTTON_BAT             0x12
#define RES_DOOR_INT               0x13
#define IOT_TEST                   0x14
#define SET_LIS2DH_PARA            0x15
#define GET_BLE_PRODUCT_MODEL      0x16
#define BATCH_OPENING              0x17
#define SET_RTC_ALARM_TIME         0x19
#define RES_ERR_CODE               0xff

#define OPEN_DOOR_TIME     500
#define BLE_NAME_MAX_LEN     20
#define DOOR_OPEN_INTERVAL     1000
#define BLE_HEAD_ONE                    0xaa
#define BLE_HEAD_TWO                    0x55

#define FLASH_BLE_NAME_ADDR                   0x001000
#define FLASH_BLE_BOARDCOST_PARA              0x002000
#define FLASH_SN_ADDR                         0x003000
#define FLASH_LIS2DH_THRESHOLD_ADDR           0x004000

#define BASIC_YEAR                 2000

#define FLASH_WRITE_FLAG           0x55
#define DEFAULT_BROADCAST     300U
#define DEFAULT_TRANSMIT_POWER     0x07  /*0db*/

#define SET_BLE_CMD_TIMEOUT     2000
#define SN_DATA_LEN                36
#define SET_SN_SUCCESS             0x00
#define SET_SN_FAILED              0x01

#define INFO_LEN_ERR     0x00F6
#define SLOT_NUM_ERR     0x00FA

#define DEVICE_VOLTAGE_DROP     74

typedef struct 
{
    uint8_t first_head;
    uint8_t second_head;
    uint16_t data_len;
    uint8_t encrypt_type;
    uint16_t customer_num;
    uint8_t board_num;
    uint8_t package_num;
    uint8_t Product_line;
    uint8_t cmd_type;
    uint16_t info_len;
    uint8_t info_buf[MAX_RECEIVE_DATA];
    uint8_t sign_l;
    uint8_t sign_h;
    uint8_t bcc;
    uint8_t terminator;
}protocol_fram_t;

typedef enum
{
	UART_RECEIVE_SUCCESS,
	UART_RECEIVE_BCC_ERR,
	UART_RECEIVE_MD5_ERR,
    UART_RECEIVE_HEAD_ERR,
    UART_RECEIVE_LEN_ERR,
    UART_RECEIVE_ENCRYPT_TYPE_ERR,
    UART_RECEIVE_NO_DATA,
    UART_RECEIVE_CUST_NUM_ERR,
    UART_RECEIVE_BOARD_NUM_ERR,
    UART_RECEIVE_PACK_NUM_ERR,
    UART_RECEIVE_CMD_TYPE_ERR,
    UART_RECEIVE_INFO_CMD_ERR,
    UART_RECEIVE_INFO_LEN_ERR,
    UART_RECEIVE_TERMINATOR_ERR,
    UART_RECEIVE_SIGN_ERR,
    UART_RECEIVE_DECRYPT_ERR
} uart_receive_data_check;

typedef enum
{
    ENPT_PLAINTEXT = 0x01,
    ENPT_CRC_16,
    ENPT_TINA_AES,
    ENPT_MD5C,
    ENPT_SHA256,
    ENPTCHACHA20
}ENCRYPT_TYPE;

typedef enum
{
    UNIVERSAL,
    DEPOSIT_EXPRESS,
    VENDING_MACHINE,
    FREEZER,
    INFRARED
}CMD_TYPE;

static struct{
    uint8_t head1;
    uint8_t head2;
    uint8_t value;
    uint8_t crc;
}lis2dh_config_t __attribute__ ((aligned (4)));

extern uint8_t ar_usart1_tx_buffer[MAX_RECEIVE_DATA];
extern uint8_t ar_usart1_rx_buffer[MAX_RECEIVE_DATA];
extern uint8_t ar_sn_num[SN_DATA_LEN];
extern uint8_t g_usart1_tx_len;
extern uint8_t g_usart1_rx_finish;
extern uint8_t g_soft_reset;
extern uint8_t g_open_multiple_doors_num;
extern uint8_t g_open_one_door_flag;
extern uint8_t g_start_open_door_flag;
extern uint16_t g_start_open_door_cnt;
extern uint8_t g_iot_start_test;
extern struct AES_ctx aes_test;
extern const uint8_t aes_key[];
extern volatile uint8_t g_set_param_timeout_flag;
extern volatile uint16_t g_set_param_count;
extern volatile uint8_t g_current_step;
extern uint8_t g_set_step;
extern uint8_t g_sn_data_len;
extern void usart1_protocol_handle(void);
extern void ble_e92_set_fun(void);
extern void ble_load_name(void);
extern void ble_e95_reset(void);
extern uint8_t g_alarm_set_num;
extern uint8_t g_read_dht22_flag;

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */


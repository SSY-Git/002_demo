/*
    Copyright 2019 SHENZHEN ZHILAI SCI AND TECH CO., LTD,
    15th floor, C3 building, Nanshan ipark,Xueyuan road 1001,
	Nanshan District, Shenzhen 518052, China.
    All rights are reserved. Reproduction in whole or in part is prohibited
    without the prior written consent of the copyright owner.

    COMPANY CONFIDENTIAL

Filename  :  iot.h


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
        Module description (Driving iot module ma510)
@end
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __iot_H
#define __iot_H

#include "main.h"
#include "global.h"
#include "usart.h"

typedef struct
{
    uint8_t writepos;
    uint8_t readpos;
}Uart_pos;


#define IOT_TEST_UART     0x01
#define IOT_TEST_SIM     0x02
#define IOT_QUERY_RSSI     0x03
#define IOT_QUERY_NET     0x04
#define IOT_CONNETC_NET     0x05
#define IOT_HTTP_SET_PARAM     0x06
#define IOT_HTTP_GET_PARAM     0x07
#define IOT_CFUN     0x08
#define IOT_CEREG     0x09
#define IOT_CREAT_HTTP_CLIENT     0x0a
#define IOT_CREAT_HTTP_SOCKET     0x0b
#define IOT_HTTP_GET     0x0c
#define IOT_SET_HTTP_PARA     0x0d
#define IOT_START_HTTP_SERVICE     0x0e
#define IOT_READ_HTTP_DATA     0x0f

#define IOT_SEND_CMD_OVERTIME     5000
#define IOT_SEND_CMD_TIME     0x05
#define IOT_SEND_QUERY_NET_TIME     10
#define IOT_MAX_RECV_DATA     256
#define UART_RECV_DATA_TIMEOUT     8     /*超时时间40ms*/
extern uint8_t ar_usart5_tx_buffer[IOT_MAX_RECV_DATA];
extern uint8_t ar_usart5_rx_buffer[IOT_MAX_RECV_DATA];
extern uint8_t g_usart5_tx_len;
extern uint8_t g_usart5_rx_finish;
extern uint8_t g_nb_ctrl_step;
extern uint8_t g_nb_curren_step;
extern uint8_t g_nb_send_cmd_cnt;
extern uint8_t g_uart5_tick;
extern uint8_t g_uart5_receive_finish;
extern Uart_pos g_uart_tmp5;
extern const uint8_t ar_nb_uart_connect[];
extern uint8_t ar_get_http_server[150];
extern uint16_t g_nb_send_cmd_overtime;
extern uint8_t g_nb_send_cmd_star_time;
extern void nb_http_set_param(void);
extern void receive_nb_data_handle(void);
extern void iot_power_on_ctrl(void);
extern void iot_power_off_ctrl(void);
extern void uart5_recv_data_fun(void);
extern void test(void);
#endif
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

/*
    Copyright 2019 SHENZHEN ZHILAI SCI AND TECH CO., LTD,
    15th floor, C3 building, Nanshan ipark,Xueyuan road 1001,
	Nanshan District, Shenzhen 518052, China.
    All rights are reserved. Reproduction in whole or in part is prohibited
    without the prior written consent of the copyright owner.

    COMPANY CONFIDENTIAL

Filename  :  iot.c


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

/* Includes ------------------------------------------------------------------*/
#include "iot.h"
#include "stm32l4xx_it.h"
#include "user_gpio.h"
#include "ble_e95.h"
#include "adc.h"
#include "tim.h"
extern UART_HandleTypeDef huart5;
/*uart4 接收、处理数据定位指针*/
Uart_pos g_uart_tmp5 = {I_ZERO};
/*uart5收发数据存储，最大接收字节255*/
uint8_t ar_usart5_tx_buffer[IOT_MAX_RECV_DATA] = {I_ZERO};
uint8_t ar_usart5_rx_buffer[IOT_MAX_RECV_DATA] = {I_ZERO};
uint8_t g_usart5_tx_len = I_ZERO;

/*接收完成标志位*/
uint8_t g_usart5_rx_finish = I_ZERO;
/*测试iot串口通断*/
const uint8_t ar_nb_uart_connect[] = "AT\r";     /*AT OK*/

/*查询iot是否插卡*/
const uint8_t ar_nb_cpin[] = "AT+CPIN?\r";     /*+CPIN：READY  OK*/
/*查询信号强度*/
const uint8_t ar_nb_csq[] = "AT+CSQ\r";    /*+COPS: 0 OK  +CSQ: 30,99  OK*, RSSI:0-31,99/
/*查询可用网络*/
const uint8_t ar_nb_cops[] = "AT+COPS?\r";    /*+COPS: 0 OK,没有可用网络，+COPS: 0,0,"CHINA MOBILE",0 OK，中国移动网*/
/*连接中国移动网*/
uint8_t ar_nb_connnect_china_mobile[70] = "0";     /*AT+MIPCALL=1,"CHINA MOBILE" OK +MIPCALL: 100.126.217.237*/
/*开启iot卡的全功能*/
const uint8_t ar_nb_cfun[] = "AT+CFUN=1\r";     /*回复OK*/
/*查询网络附着状态*/
const uint8_t ar_nb_cereg[] = "AT+CEREG?\r";    /*+CEREG: 2,1,"3B00","08A29A69",9 OK  第二个参数为1或者5表示附着成功*/
/*创建http客户端*/
const uint8_t ar_creat_http_client[] = "AT+HTTPCREATE=0,\"http://118.31.125.61:8167\"\r";     /*回复+HTTPCREATE: 0 OK 其中0是http client id号，下创建socket时需要用到这个id号*/
/*创建httpsocket*/
 uint8_t ar_creat_http_socket[15] = "AT+HTTPCON=0\r";
/*请求服务器*/
uint8_t ar_get_http_server[150] = "AT+HTTPSEND=0,0,74,\"/btserver/bt/?PW=zhilai&TC=1104&SI=222-0000002&BS=111111111110000100000000\"\r";
const uint8_t ar_start_http_service[] = "AT+HTTPACT=0\r";
uint8_t ar_http_server_len[] = "00";
uint8_t ar_goods_status[] = "111111111110000100000000";
uint8_t ar_voltage_status[] = "low voltage: ";
//uint8_t ar_goods_status[] = "1111111111100001";

/*回复+HTTPRESPH: 0,200,111,Content-Type: application/json;charset=UTF-8
Transfer-Encoding: chunked
Date: Thu, 13 May 2021 02:20:21 GMT

+HTTPRESPC: 0,0,4,4,0000*/
/*http 默认client id = 0*/
char g_httpclient_id = '0';
/*串口5字节中断接收数据，字节超时时间为40ms*/
uint8_t g_uart5_tick = V_ZERO;
uint8_t g_uart5_receive_finish = V_ZERO;
/*iot操作步骤*/
uint8_t g_nb_ctrl_step = V_ZERO;
uint8_t g_nb_curren_step = V_ZERO;
uint8_t g_nb_send_cmd_cnt = V_ZERO;
uint16_t g_nb_send_cmd_overtime = V_ZERO;
uint8_t g_nb_send_cmd_star_time = V_ZERO;
/*nb模块接收数据*/
uint8_t g_nb_receive_step = V_ZERO;
/*搜索到网络名称长度*/
uint8_t g_network_len = V_ZERO;
/*存储网络名*/
uint8_t ar_network[50] = "0";

void test(void)
{
    sprintf(ar_get_http_server, 
            "%s%c,0,%s%s%s%s%s\"\r",
            (char *)"AT+HTTPSEND=",
            g_httpclient_id,
            ar_http_server_len,
            (char *)",\"/btserver/bt/?PW=zhilai&TC=1104&SI=",
            ar_sn_num,
            (char *)"&BS=",
            ar_goods_status);
    ar_get_http_server[16] = ((strlen(ar_get_http_server) - 22)/10) + 0x30;
    ar_get_http_server[17] = ((strlen(ar_get_http_server) - 22)%10) + 0x30;
    printf("string : %s\r\n", ar_get_http_server);
    printf("data_len : %d\r\n", strlen(ar_get_http_server));
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if((huart == &huart5) && (HAL_UART_STATE_READY == HAL_UART_GetState(&huart5)))
    {
        g_uart_tmp5.writepos++;
        HAL_UART_Receive_IT(&huart5, &ar_usart5_rx_buffer[g_uart_tmp5.writepos], L_ONE);
        g_uart5_tick = UART_RECV_DATA_TIMEOUT;
        g_uart5_receive_finish = V_ONE;
    }
}

void uart5_recv_data_fun(void)
{
    /*判断接收、处理数据的指针是否一致，并将接收到的数据保存到tmp数组中*/
    if(g_uart_tmp5.readpos <= g_uart_tmp5.writepos)
    {
        g_usart5_tx_len = g_uart_tmp5.writepos - g_uart_tmp5.readpos;
        memcpy(ar_usart5_tx_buffer, &ar_usart5_rx_buffer[g_uart_tmp5.readpos], g_usart5_tx_len);
    }
    else
    {
        g_usart5_tx_len = g_uart_tmp5.writepos + IOT_MAX_RECV_DATA - g_uart_tmp5.readpos;
        memcpy(ar_usart5_tx_buffer, &ar_usart5_tx_buffer[g_uart_tmp5.readpos], IOT_MAX_RECV_DATA - g_uart_tmp5.readpos);
        memcpy(&ar_usart5_tx_buffer[IOT_MAX_RECV_DATA - g_uart_tmp5.readpos], ar_usart5_rx_buffer, g_uart_tmp5.writepos);
    }
}

void iot_power_on_ctrl(void)
{
    IOT_POWER_EN;     /*电源使能*/
    HAL_Delay(100);
    IOT_ON_OFF_EN;     /*开机*/
    HAL_Delay(1000);    /*on-off给500ms-1.7s高电平，iot开机*/
    IOT_ON_OFF_DIS;
    //IOT_RESET_H;
    //HAL_Delay(2);
    //IOT_RESET_L;
}

void iot_power_off_ctrl(void)
{

    IOT_ON_OFF_EN;     /*开机*/
    HAL_Delay(5000);
    IOT_ON_OFF_DIS;
    //IOT_RESET_H;
    IOT_POWER_DIS;     /*电源使能*/
}

/**/

void nb_http_set_param(void)
{
    switch(g_nb_ctrl_step)
    {
        case IOT_TEST_UART:
        {
            g_nb_send_cmd_cnt++;
            if(g_nb_send_cmd_cnt <= IOT_SEND_CMD_TIME)     /*重复发5次，每次间隔5S都没收到回复就报异常*/
            {
                HAL_UART_Transmit(&huart5, ar_nb_uart_connect, strlen(ar_nb_uart_connect), 1000);
                HAL_Delay(1000);
                #if 0
                HAL_UART_Transmit(&huart5, ar_nb_cpin, strlen(ar_nb_cpin), 1000);
                HAL_Delay(1000);
                HAL_UART_Transmit(&huart5, "AT+CGREG?\r", strlen("AT+CGREG?\r"), 1000);
                HAL_Delay(1000);
                HAL_UART_Transmit(&huart5, "AT+CEREG?\r", strlen("AT+CEREG?\r"), 1000);
                HAL_Delay(1000);
                HAL_UART_Transmit(&huart5, "AT+COPS?\r", strlen("AT+COPS?\r"), 1000);
                HAL_Delay(1000);
                HAL_UART_Transmit(&huart5, "AT+CESQ\r", strlen("AT+CESQ\r"), 1000);
                HAL_Delay(1000);
                HAL_UART_Transmit(&huart5, "AT+CREG?\r", strlen("AT+CREG?\r"), 1000);
                HAL_Delay(1000);
                #endif
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ONE;
                //g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = IOT_TEST_UART;
                g_nb_send_cmd_overtime = V_ZERO;//IOT_SEND_CMD_OVERTIME;/*超时5s*/
                printf("send at cmd!\r\n");
            }
            else
            {
                g_start_http_flag = V_ONE;
                g_iot_power_on_cnt++;
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ZERO;
                g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = V_ZERO;
                g_nb_send_cmd_overtime = V_ZERO;
                iot_power_off_ctrl();
                printf("iot uart error!\r\n");
            }

            break;
        }
        case IOT_TEST_SIM:
        {
            g_nb_send_cmd_cnt++;
            if(g_nb_send_cmd_cnt <= IOT_SEND_CMD_TIME)     /*重复发10次，每次间隔5S都没收到回复就报异常*/
            {
                HAL_UART_Transmit(&huart5, ar_nb_cpin, strlen(ar_nb_cpin), 1000);
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ONE;
                //g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = IOT_TEST_SIM;
                g_nb_send_cmd_overtime = V_ZERO;//IOT_SEND_CMD_OVERTIME;/*超时5s*/
            }
            else
            {
                g_start_http_flag = V_ONE;
                g_iot_power_on_cnt++;
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ZERO;
                g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = V_ZERO;
                g_nb_send_cmd_overtime = V_ZERO;
                iot_power_off_ctrl();
                printf("iot no sim card!\r\n");
            }
            break;
        }
        case IOT_QUERY_RSSI:
        {
            /*干掉，直接查询可用网络*/
            // g_nb_send_cmd_cnt++;
            // if(g_nb_send_cmd_cnt <= IOT_SEND_QUERY_NET_TIME)     /*重复发10次，每次间隔5S都没收到回复就报异常*/
            // {
            //     HAL_UART_Transmit(&huart5, ar_nb_csq, strlen(ar_nb_csq), 1000);
            //     HAL_Delay(1000);
            //     g_nb_ctrl_step = V_ZERO;
            //     g_nb_send_cmd_star_time = V_ONE;
            //     //g_nb_send_cmd_cnt = V_ZERO;
            //     g_nb_curren_step = IOT_QUERY_RSSI;
            //     g_nb_send_cmd_overtime = V_ZERO;//IOT_SEND_CMD_OVERTIME;/*超时5s*/
            // }
            // else
            // {
            //     g_start_http_flag = V_ONE;
            //     g_iot_power_on_cnt++;
            //     g_nb_ctrl_step = V_ZERO;
            //     g_nb_send_cmd_star_time = V_ZERO;
            //     g_nb_send_cmd_cnt = V_ZERO;
            //     g_nb_curren_step = V_ZERO;
            //     g_nb_send_cmd_overtime = V_ZERO;
            //     iot_power_off_ctrl();
            //     printf("iot no rssi!\r\n");
            // }
            // break;
        }
        case IOT_QUERY_NET:
        {
            g_nb_send_cmd_cnt++;
            if(g_nb_send_cmd_cnt <= IOT_SEND_QUERY_NET_TIME)     /*重复发10次，每次间隔5S都没收到回复就报异常*/
            {
                HAL_UART_Transmit(&huart5, ar_nb_cops, strlen(ar_nb_cops), 1000);
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ONE;
                //g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = IOT_QUERY_NET;
                g_nb_send_cmd_overtime = V_ZERO;//IOT_SEND_CMD_OVERTIME;/*超时5s*/
                HAL_Delay(5000);
            }
            else
            {
                g_start_http_flag = V_ONE;
                g_iot_power_on_cnt++;
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ZERO;
                g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = V_ZERO;
                g_nb_send_cmd_overtime = V_ZERO;
                iot_power_off_ctrl();
                printf("iot not attached!\r\n");
            }
            break;
        }
        case IOT_CONNETC_NET:
        {
            g_nb_send_cmd_cnt++;
            if(g_nb_send_cmd_cnt <= IOT_SEND_CMD_TIME)     /*重复发10次，每次间隔5S都没收到回复就报异常*/
            {
                uint8_t ar_network_buf[50] = "0";
                memcpy(ar_network_buf, ar_network, g_network_len);
                // printf("ar_network_buf:");
                // for (int i = 0; i < g_network_len; i++)
                // {
                //     printf("%c",ar_network_buf[i]);
                // }
                // printf("\n");
                
                sprintf(ar_nb_connnect_china_mobile, "AT+MIPCALL=1,\"%s\"\r", ar_network_buf);
                // printf("ar_nb_connnect_china_mobile:");
                // for (int i = 0; i < strlen(ar_nb_connnect_china_mobile); i++)
                // {
                //     printf("%c",ar_nb_connnect_china_mobile[i]);
                // }
                // printf("\n");
                
                //HAL_UART_Transmit(&huart5, ar_nb_connnect_china_mobile, strlen(ar_nb_connnect_china_mobile), 1000);
                HAL_UART_Transmit(&huart5, "AT+MIPCALL=1\r", strlen("AT+MIPCALL=1\r"), 1000);
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ONE;
                //g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = IOT_CONNETC_NET;
                g_nb_send_cmd_overtime = V_ZERO;//IOT_SEND_CMD_OVERTIME;/*超时5s*/
                HAL_Delay(3000);
            }
            else
            {
                g_start_http_flag = V_ONE;
                g_iot_power_on_cnt++;
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ZERO;
                g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = V_ZERO;
                g_nb_send_cmd_overtime = V_ZERO;
                iot_power_off_ctrl();
                printf("IOT SET CREAT CLIENT FAILED!\r\n");
            }
            break;
        }
        case IOT_SET_HTTP_PARA:
        {
            uint8_t ar_sn_test_buf[SN_DATA_LEN] = "0";
            HAL_Delay(1000);
            if(g_min_voltage_flag)    /*电池电量不足上报电量信息*/
            {
                g_min_voltage_flag = V_ZERO;
                ar_get_voltage_value[V_ZERO] += DEVICE_VOLTAGE_DROP;
                sprintf(ar_goods_status, "%s%d", ar_voltage_status, ar_get_voltage_value[V_ZERO]);
                printf("%s\n",ar_goods_status);
            }
            else    /*上报格口状态*/
            {
                get_all_goods_state_fun();
                for(uint8_t i = V_ZERO; i < MAX_DOOR_NUM; i++)
                {
                    g_item_stat[i] += 0x30;
                    ar_goods_status[i] = g_item_stat[i];
                }
            }            
            memcpy(ar_sn_test_buf, ar_sn_num, g_sn_data_len);
            sprintf(ar_get_http_server, 
                    "%s%s%s%s\"\r",
                    (char *)"AT+HTTPSET=\"URL\",\"http://118.31.125.61:8167/btserver/bt/?PW=zhilai&TC=1104&SI=",
                    ar_sn_test_buf,
                    (char *)"&BS=",
                    ar_goods_status);
            g_nb_send_cmd_cnt++;
            printf("%s\r\n", ar_get_http_server);
            
            if(g_nb_send_cmd_cnt <= IOT_SEND_CMD_TIME)     /*重复发10次，每次间隔5S都没收到回复就报异常*/
            {
                HAL_UART_Transmit(&huart5, ar_get_http_server, strlen(ar_get_http_server), 1000);
                
                HAL_Delay(300);
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ONE;
                //g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = IOT_SET_HTTP_PARA;
                g_nb_send_cmd_overtime = V_ZERO;//IOT_SEND_CMD_OVERTIME;/*超时5s*/
            }
            else
            {
                g_start_http_flag = V_ONE;
                g_iot_power_on_cnt++;
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ZERO;
                g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = V_ZERO;
                g_nb_send_cmd_overtime = V_ZERO;
                iot_power_off_ctrl();
                printf("IOT SET CREAT CLIENT FAILED!\r\n");
            }
            break;
        }
        case IOT_START_HTTP_SERVICE:
        {
            g_nb_send_cmd_cnt++;
            if(g_nb_send_cmd_cnt <= IOT_SEND_CMD_TIME)     /*重复发10次，每次间隔5S都没收到回复就报异常*/
            {
                HAL_UART_Transmit(&huart5, ar_start_http_service, strlen(ar_start_http_service), 1000);
                //HAL_UART_Transmit(&huart5, "AT+MIPCALL=1\r", strlen("AT+MIPCALL=1\r"), 1000);
                HAL_Delay(2000);
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ONE;
                //g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = IOT_START_HTTP_SERVICE;
                g_nb_send_cmd_overtime = V_ZERO;
                HAL_Delay(8000);
            }
            else
            {
                g_start_http_flag = V_ONE;
                g_iot_power_on_cnt++;
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ZERO;
                g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = V_ZERO;
                g_nb_send_cmd_overtime = V_ZERO;
                iot_power_off_ctrl();
                printf("IOT HTTPCLIENT GET FAILED!\r\n");
            }
            break;
        }
        case IOT_READ_HTTP_DATA:
        {
            g_nb_send_cmd_cnt++;
            if(g_nb_send_cmd_cnt <= IOT_SEND_CMD_TIME)     /*重复发10次，每次间隔5S都没收到回复就报异常*/
            {
                //HAL_UART_Transmit(&huart5, ar_start_http_service, strlen(ar_start_http_service), 1000);
                HAL_UART_Transmit(&huart5, "AT+HTTPREAD\r", strlen("AT+HTTPREAD\r"), 1000);
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ONE;
                //g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = IOT_READ_HTTP_DATA;
                g_nb_send_cmd_overtime = V_ZERO;
                HAL_Delay(8000);
            }
            else
            {
                g_start_http_flag = V_ONE;
                g_iot_power_on_cnt++;
                g_nb_ctrl_step = V_ZERO;
                g_nb_send_cmd_star_time = V_ZERO;
                g_nb_send_cmd_cnt = V_ZERO;
                g_nb_curren_step = V_ZERO;
                g_nb_send_cmd_overtime = V_ZERO;
                iot_power_off_ctrl();
                printf("IOT HTTPCLIENT GET FAILED!\r\n");
            }
            break;
        }
    }
}


void receive_nb_data_handle(void)
{
    //if(g_usart5_rx_finish)
    {
        printf("iot recv data len %d\r\n", g_usart5_tx_len);
        printf("iot recv data:");
        for(uint8_t i = V_ZERO; i < g_usart5_tx_len; i++)
        {
            printf("%c", ar_usart5_tx_buffer[i]);
        }
        //memset(&ar_usart5_tx_buffer[g_usart5_tx_len], V_ZERO, IOT_MAX_RECV_DATA-g_usart5_tx_len);
        switch(g_nb_curren_step)
        {
            case IOT_TEST_UART:
            {
                if(strstr((char*)ar_usart5_tx_buffer,"OK") != NULL)
                {
                    g_nb_ctrl_step = IOT_TEST_SIM;
                    g_nb_curren_step = V_ZERO;
                    g_nb_send_cmd_cnt = V_ZERO;
                    //g_iot_power_on_cnt = V_ZERO;
                    printf("IOT UART OK!\r\n");
                    HAL_Delay(1000);
                }
                else
                {
                    g_nb_ctrl_step = g_nb_curren_step;
                    //g_nb_send_cmd_cnt++;
                    g_nb_send_cmd_overtime = V_ZERO;
                    printf("receive iot uart data err!\r\n");
                }
                break;
            }
            case IOT_TEST_SIM:
            {
                if(strstr((char*)ar_usart5_tx_buffer,"READ") != NULL)
                {
                    g_nb_ctrl_step = IOT_QUERY_RSSI;
                    g_nb_curren_step = V_ZERO;
                    g_nb_send_cmd_cnt = V_ZERO;
                    //g_iot_power_on_cnt = V_ZERO;
                    printf("IOT SIM CARD OK!\r\n");
                    HAL_Delay(1000);
                }
                else
                {
                    g_nb_ctrl_step = g_nb_curren_step;
                    //g_nb_send_cmd_cnt++;
                    g_nb_send_cmd_overtime = V_ZERO;
                    printf("receive iot sim card err!\r\n");
                }
                break;
            }
            case IOT_QUERY_RSSI:
            {
                if(strstr((char*)ar_usart5_tx_buffer,"OK") != NULL)
                {
                    uint8_t j = V_ZERO;
                    for(uint8_t i = V_ZERO; i < g_usart5_tx_len; i++)
                    {
                        if(ar_usart5_tx_buffer[i] == ':')
                        {
                            j = i;
                            break;
                        }
                    }
                    if(j < g_usart5_tx_len)
                    {
                        if((ar_usart5_tx_buffer[j+L_TWO] > '0') && (ar_usart5_tx_buffer[j+L_TWO] <= '3'))
                        {
                            printf("rssi = %c%c\r\n", ar_usart5_tx_buffer[j+L_TWO], ar_usart5_tx_buffer[j+L_THREE]);
                            g_nb_ctrl_step = IOT_QUERY_NET;
                            g_nb_curren_step = V_ZERO;
                            g_nb_send_cmd_cnt = V_ZERO;
                            //g_iot_power_on_cnt = V_ZERO;
                            printf("IOT RSSI OK!\r\n");
                        }
                        else
                        {
                            g_nb_ctrl_step = g_nb_curren_step;
                            //g_nb_send_cmd_cnt++;
                            g_nb_send_cmd_overtime = V_ZERO;
                            printf("rssi query err!\r\n");
                        }
                    }
                    else
                    {
                        g_nb_ctrl_step = g_nb_curren_step;
                        //g_nb_send_cmd_cnt++;
                        g_nb_send_cmd_overtime = V_ZERO;
                        printf("rssi query err err!\r\n");
                    }
                }
                else
                {
                    g_nb_ctrl_step = g_nb_curren_step;
                    //g_nb_send_cmd_cnt++;
                    g_nb_send_cmd_overtime = V_ZERO;
                    printf("rssi query err err!\r\n");
                }
                break;
            }
            case IOT_QUERY_NET:
            {
                uint8_t j = V_ZERO;
                uint8_t i = V_ZERO;
                if(strstr((char*)ar_usart5_tx_buffer,"OK") != NULL)
                {
                    for(i = V_SEVEN; i < g_usart5_tx_len; i++)
                    {
                        if(ar_usart5_tx_buffer[i] == '"')
                        {
                            j = i;
                            break;
                        }
                    }
                    if(i == g_usart5_tx_len)
                    {
                        g_nb_ctrl_step = g_nb_curren_step;
                        //g_nb_send_cmd_cnt++;
                        g_nb_send_cmd_overtime = V_ZERO;
                        printf("no network to connect!\r\n");
                    }
                    else
                    {
                        printf("Search the Internet:");
                        for(i = j+V_ONE; i < g_usart5_tx_len; i++)
                        {
                            printf("%c", ar_usart5_tx_buffer[i]);
                            if(ar_usart5_tx_buffer[i] == '"')
                            {
                                break;
                            }
                        }
                        if(i != g_usart5_tx_len)
                        {
                            g_network_len = i-j-L_ONE;
                            if(g_network_len < 50)
                            {
                                memcpy(ar_network, &ar_usart5_tx_buffer[j+V_ONE], g_network_len);
                                g_nb_ctrl_step = IOT_CONNETC_NET;
                                g_nb_curren_step = V_ZERO;
                                g_nb_send_cmd_cnt = V_ZERO;
                                printf("search the Internet successfully!\r\n");
                            }
                            else
                            {
                                g_nb_ctrl_step = g_nb_curren_step;
                                //g_nb_send_cmd_cnt++;
                                g_nb_send_cmd_overtime = V_ZERO;
                                printf("networrk name length err!\r\n");
                            }
                        }
                    }
                }
                else
                {
                    g_nb_ctrl_step = g_nb_curren_step;
                    //g_nb_send_cmd_cnt++;
                    g_nb_send_cmd_overtime = V_ZERO;
                    printf("receive iot query networrk err!\r\n");
                }
                break;
            }
            case IOT_CONNETC_NET:
            {
                uint8_t j = V_ZERO;
                uint8_t k = V_ZERO;
                if(strstr((char*)ar_usart5_tx_buffer,"OK") != NULL)
                {
                    printf("receive ok\n");
                    for(uint8_t i = V_ZERO; i < g_usart5_tx_len; i++)
                    {
                        if((memcmp(&ar_usart5_tx_buffer[i], "MIPCALL:", L_EIGHT) == V_ZERO))
                        {
                            //printf("");
                            j = i+L_EIGHT;
                            break;
                        }
                    }
                    if(j < g_usart5_tx_len)
                    {
                        for(uint8_t i = j; i < g_usart5_tx_len; i++)
                        {
                            printf("%c", ar_usart5_tx_buffer[i]);
                            if(ar_usart5_tx_buffer[i] == '.')   /*  .   .   .*/
                            {
                                k++;
                            }
                        }
                        if(k == V_THREE)
                        {

                            g_nb_ctrl_step = IOT_SET_HTTP_PARA;
                            g_nb_curren_step = V_ZERO;
                            g_nb_send_cmd_cnt = V_ZERO;
                            printf("IOT MIPCALL SUCCESS!\r\n");
                            HAL_Delay(1000);
                        }
                        else
                        {
                            g_nb_ctrl_step = g_nb_curren_step;
                            //g_nb_send_cmd_cnt++;
                            g_nb_send_cmd_overtime = V_ZERO;
                            printf("IOT MIPCALL FAILED 1!\r\n");
                        }
                    }
                    else
                    {
                        g_nb_ctrl_step = g_nb_curren_step;
                        //g_nb_send_cmd_cnt++;
                        g_nb_send_cmd_overtime = V_ZERO;
                        printf("IOT MIPCALL FAILED 2!\r\n");
                    }
                }
                else
                {
                    g_nb_ctrl_step = g_nb_curren_step;
                    //g_nb_send_cmd_cnt++;
                    g_nb_send_cmd_overtime = V_ZERO;
                    printf("IOT MIPCALL FAILED!\r\n");
                }
                break;
            }
            case IOT_SET_HTTP_PARA:
            {
                if(strstr((char*)ar_usart5_tx_buffer,"OK") != NULL)
                {
                    g_nb_ctrl_step = IOT_START_HTTP_SERVICE;
                    g_nb_curren_step = V_ZERO;
                    g_nb_send_cmd_cnt = V_ZERO;
                    //g_iot_power_on_cnt = V_ZERO;
                    printf("IOT SET HTTP PARA OK!\r\n");
                    HAL_Delay(1000);
                }
                else
                {
                    uint8_t i;
                    for(i = 0; i < g_usart5_tx_len; i++)
                    {
                        if(ar_usart5_tx_buffer[i] == 'O')
                        {
                            break;
                        }
                    }
                    if(ar_usart5_tx_buffer[i+L_ONE] == 'K')
                    {
                        g_nb_ctrl_step = IOT_START_HTTP_SERVICE;
                        g_nb_curren_step = V_ZERO;
                        g_nb_send_cmd_cnt = V_ZERO;
                        //g_iot_power_on_cnt = V_ZERO;
                        printf("IOT SET HTTP PARA OK!\r\n");
                        HAL_Delay(1000);
                    }
                    else
                    {
                        g_nb_ctrl_step = g_nb_curren_step;
                        //g_nb_send_cmd_cnt++;
                        g_nb_send_cmd_overtime = V_ZERO;
                        printf("receive set http param err!\r\n");
                    }
                }
                break;
            }
            case IOT_START_HTTP_SERVICE:
            {
                uint8_t j = V_ZERO;
                
                if(strstr((char*)ar_usart5_tx_buffer,"OK") != NULL)
                {
                    for(uint8_t i = V_ZERO; i < g_usart5_tx_len; i++)
                    {
                        if((memcmp(&ar_usart5_tx_buffer[i], "HTTP:", L_FIVE) == V_ZERO))
                        {
                            j = i+L_SIX;
                            break;
                        }
                    }
                    if(j < g_usart5_tx_len)
                    {
                        if(ar_usart5_tx_buffer[j] == '1')     /*connect success*/
                        {
                            g_nb_ctrl_step = IOT_READ_HTTP_DATA;
                            g_nb_curren_step = V_ZERO;
                            g_nb_send_cmd_cnt = V_ZERO;
                            //g_iot_power_on_cnt = V_ZERO;
                            printf("IOT HTTP CONNECT SUCCESS OK!\r\n");
                            HAL_Delay(1000);
                        }
                        else
                        {
                            g_nb_ctrl_step = g_nb_curren_step;
                            //g_nb_send_cmd_cnt++;
                            g_nb_send_cmd_overtime = V_ZERO;
                            printf("IOT START HTTP SERVICE ERR!\r\n");
                        }
                    }
                    else
                    {
                        g_nb_ctrl_step = g_nb_curren_step;
                        //g_nb_send_cmd_cnt++;
                        g_nb_send_cmd_overtime = V_ZERO;
                        printf("IOT START HTTP SERVICE ERR!\r\n");
                    }
                }
                else
                {
                    g_nb_ctrl_step = g_nb_curren_step;
                    //g_nb_send_cmd_cnt++;
                    g_nb_send_cmd_overtime = V_ZERO;
                    printf("IOT START HTTP SERVICE ERR!\r\n");
                }
                break;
            }
            case IOT_READ_HTTP_DATA:
            {
                uint8_t j = V_ZERO;
                //printf("step IOT_READ_HTTP_DATA\n");
                //if(strstr((char*)ar_usart5_tx_buffer,"OK") != NULL)
                {
                    for(uint8_t i = V_ZERO; i < g_usart5_tx_len; i++)
                    {
                        if((memcmp(&ar_usart5_tx_buffer[i], "0000", L_FOUR) == V_ZERO))
                        {
                            j = i;
                            break;
                        }
                    }
                    if(j < g_usart5_tx_len && j > V_ZERO)
                    {
                        g_nb_ctrl_step = V_ZERO;
                        g_nb_curren_step = V_ZERO;
                        g_nb_send_cmd_cnt = V_ZERO;
                        g_iot_power_on_cnt = V_ZERO;
                        printf("IOT HTTP GET SUCCESS!\r\n");
                        HAL_Delay(1000);
                        iot_power_off_ctrl();
                        if(g_iot_start_test == V_ONE)
                        {
                            g_iot_start_test = V_ZERO;
                            HAL_UART_Transmit(&hlpuart1, ar_resp_iot_test_success, 0x1B, 0xffffff);
                            printf("IOT TEST SUCCESSED!\r\n");
                        }
                    }
                    else
                    {
                        g_nb_ctrl_step = g_nb_curren_step;
                        //g_nb_send_cmd_cnt++;
                        g_nb_send_cmd_overtime = V_ZERO;
                        printf("HTTP GET ERR!\r\n");
                    }
                }
                #if 0
                else
                {
                    g_nb_ctrl_step = g_nb_curren_step;
                    //g_nb_send_cmd_cnt++;
                    g_nb_send_cmd_overtime = V_ZERO;
                    printf("HTTP GET ERR!\r\n");
                }
                #endif
                break;
            }
            default:
                break;
        }
        //g_usart5_rx_finish = V_ZERO;
    }
}
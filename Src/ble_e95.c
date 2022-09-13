/*
    Copyright 2019 SHENZHEN ZHILAI SCI AND TECH CO., LTD,
    15th floor, C3 building, Nanshan ipark,Xueyuan road 1001,
	Nanshan District, Shenzhen 518052, China.
    All rights are reserved. Reproduction in whole or in part is prohibited
    without the prior written consent of the copyright owner.

    COMPANY CONFIDENTIAL

Filename  :  ble_e95.c


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
        Module description (Driving Bluetooth module E95)
@end
*/

/* Includes ------------------------------------------------------------------*/
#include "ble_e95.h"
#include "user_gpio.h"
#include "aes.h"
#include "usart.h"
#include "w25q64.h"
#include "adc.h"
#include "pcf85263.h"
#include "DHT22.h"
#include "lis2dh.h"

extern UART_HandleTypeDef hlpuart1;
struct AES_ctx aes_test;
const uint8_t aes_key[] = {0xE7,0xB9,0xF9,0xBD,0xEC,0x14,0xB0,0x27,0x49,0x03,0xAC,0x18,0x1C,0x08,0x1F,0xCA};
uint8_t ar_usart1_tx_buffer[MAX_RECEIVE_DATA] = {I_ZERO};
uint8_t ar_usart1_rx_buffer[MAX_RECEIVE_DATA] = {I_ZERO};
uint8_t g_usart1_tx_len = I_ZERO;
uint8_t g_usart1_rx_finish = I_ZERO;
uint8_t ar_response_buffer[MAX_RECEIVE_DATA] = {I_ZERO};
/*the name length is limited to 20 bytes*/
uint8_t ar_ble_name_buf[BLE_NAME_MAX_LEN] = "ZL_ZY";
uint8_t ar_software_ver[14] = "V1.1.4.210820";
/*1:Command mode；2：Transparent transmission mode；
 First work in the command mode, set the parameters 
 and then enter the transparent mode*/
volatile uint8_t g_ble_e95_work_mode = E95_SET_MODE;
uint8_t g_ble_e92_cmd = I_ZERO;
/*set step-1:set ble name;2:set broadcast;3:set Transmitting power；4：get model*/
uint8_t g_set_step = I_ZERO;
/*每次重启就初始化一次*/
uint8_t g_e95_start_init = V_ONE;
/*发送指令次数，重复发送3次数据无回复后就报硬件异常*/
uint8_t g_send_cmd_cnt = I_ZERO;
/*每次发数据后开始计时标志*/
volatile uint8_t g_set_param_timeout_flag = V_ZERO;
/*每次发送设置协议后2s内未收到数据就超时*/
volatile uint16_t g_set_param_count = V_ZERO;
/*记录当前设置蓝牙的步骤：1：设置蓝牙名称；2：设置广播间隔；3：设置发射功率；4:查询产品名称*/
volatile uint8_t g_current_step = V_ZERO;
/*用于存储AES加解密后的数据*/
uint8_t ar_ble_decrypt_buf[AES_D_E_DATA_LEN] = {I_ZERO};
protocol_fram_t new_protocol_fram = {V_ZERO,V_ZERO,V_ZERO,V_ZERO,V_ZERO,V_ZERO,V_ZERO,\
V_ZERO,V_ZERO,V_ZERO,{V_ZERO},V_ZERO,V_ZERO,V_ZERO};
uint8_t g_ble_boardcost_time = I_ZERO;
uint8_t g_ble_transmit_power = I_ZERO;
RTC_COUNT_TIME_T get_rtc_date_time;
RTC_COUNT_TIME_T set_rtc_date_time;
RTC_COUNT_TIME_T rec_rtc_count_time;
/*广播参数*/
uint8_t ar_set_boardcast_params[L_SIX] = {V_ZERO};
/*蓝牙名称*/
uint8_t ar_set_name_params[L_TWENTY_FOUR] = {V_ZERO};
/*发射功率*/
uint8_t ar_set_transmit_power[L_FIVE] = {V_ZERO};
uint8_t xor_check(uint8_t *pdata, uint16_t data_len);
/*E95产品型号*/
uint8_t ar_ble_model[L_TWENTY] = {V_ZERO};
/*查询e95产品型号：LSD4BT-E95ALSP001*/
const uint8_t ar_e95_get_model[L_FIVE] = {0x01, 0xfc, 0x18, 0x01, 0x00};
/*存放SN码,SN36字节+1字节帧头+1字节bcc校验，通过帧头跟校验位来判断SN是否被设置*/
uint8_t ar_sn_buf[SN_DATA_LEN+L_TWO] = {I_ZERO};
/*将设置的震动传感器阈值存在外部flash 0x004000R*/
void lis2dh_config_save(uint8_t value);
/*批量开门格口号bit 0为不开门，bit1需要开门，
  byte0：bit0-bit7表示1-8号格口
  byte1：bit0-bit7表示9-16号格口
  byte2：bit0-bit7表示17-24号格口*/
uint8_t ar_open_slot_num[V_THREE] = {I_ZERO};
/*依次开锁的格口号，0-23*/
uint8_t g_open_multiple_doors_num = I_ZERO;
/*一次开锁的标志位*/
uint8_t g_open_one_door_flag = V_ZERO;
/*开锁开始计时标志，开锁间隔1s*/
uint8_t g_start_open_door_flag = I_ZERO;
/*开锁计时*/
uint16_t g_start_open_door_cnt = I_ZERO;
uint8_t g_package_num = I_ZERO;

/*iot测试标志*/
uint8_t g_iot_start_test = V_ZERO;
/*sn长度*/
uint8_t g_sn_data_len = V_ZERO;
uint8_t g_ble_name_len = V_ZERO;
/*sn号*/
uint8_t ar_sn_num[SN_DATA_LEN] = "000-0000000";
/*信息上报时间设置量*/
uint8_t g_alarm_set_num = V_SIX;

axis_info_t acc_sample;
filter_avg_t acc_data;

uint8_t g_read_dht22_flag = 0;
/*********************************************************************
 *	function name:  ble_load_name
 *	describe: read setting parameters from external flash:ble name、broadcast、transmit power and SN
 *	input: void
 *	output: void
 *	return: void
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
void ble_load_name(void)
{
    uint8_t ble_name_buf[L_TWENTY_FOUR];
    uint8_t ar_boardcast_time[V_FOUR] = {I_ZERO};
    uint8_t bcc = I_ZERO;
    uint8_t m_data_len = I_ZERO;

    W25Q16_HighSpeedBufferRead(FLASH_BLE_NAME_ADDR, ble_name_buf, L_TWENTY_FOUR);     /*读取蓝牙名称*/
    if((ble_name_buf[V_ZERO] >= L_TWO) && (ble_name_buf[V_ZERO] <= L_TWENTY_TWO))     /*判读长度范围*/
    {
        bcc = xor_check(ble_name_buf, ble_name_buf[V_ZERO]);
        if(ble_name_buf[ble_name_buf[V_ZERO]] == bcc)     /*蓝牙名称被正确设置过*/
        {
            memset(ar_ble_name_buf, V_ZERO, L_TWENTY);
            memcpy(ar_ble_name_buf, &ble_name_buf[V_ONE], ble_name_buf[V_ZERO]-L_ONE);
            g_ble_name_len = ble_name_buf[V_ZERO]-L_ONE;
            ar_set_name_params[V_THREE] = ble_name_buf[V_ZERO];
            memcpy(&ar_set_name_params[V_FOUR], ar_ble_name_buf, strlen(ar_ble_name_buf));
            printf("ble name : %s\r\n", ar_ble_name_buf);
        }
        else     /*数据写入检验出错就用默认蓝牙名称：ZL_ZY*/
        {
            memcpy(ar_ble_name_buf, (uint8_t *)"ZL_ZY", L_FIVE);
            g_ble_name_len = L_FIVE;
            ar_set_name_params[V_THREE] = L_FIVE;
            memcpy(&ar_set_name_params[V_FOUR], ar_ble_name_buf, L_FIVE);
            printf("bcc err use default ble name:ZL_ZY!\r\n");
        }
    }/*蓝牙名称没被设置过就用默认名称：ZL_ZY*/
    else
    {
        memcpy(ar_ble_name_buf, (uint8_t *)"ZL_ZY", L_FIVE);
        g_ble_name_len = L_FIVE;
        ar_set_name_params[V_THREE] = L_FIVE;
        memcpy(&ar_set_name_params[V_FOUR], ar_ble_name_buf, L_FIVE);
        printf("head err use default ble name:ZL_ZY!\r\n");
    }

    W25Q16_HighSpeedBufferRead(FLASH_BLE_BOARDCOST_PARA, ar_boardcast_time, L_FOUR);     /*读取广播参数*/
    if(ar_boardcast_time[V_ZERO] == FLASH_WRITE_FLAG)     /*判读参数被写标志*/
    {
        bcc = xor_check(ar_boardcast_time, L_THREE);
        if(bcc == ar_boardcast_time[V_THREE])     /*广播参数被正确设置*/
        {
            ar_set_boardcast_params[V_FOUR] = (ar_boardcast_time[V_ONE]*100)%256;
            ar_set_boardcast_params[V_FIVE] = (ar_boardcast_time[V_ONE]*100)/256;
            g_ble_boardcost_time = ar_boardcast_time[V_ONE];
            ar_set_transmit_power[V_FOUR] = ar_boardcast_time[V_TWO];
            if(ar_set_transmit_power[V_FOUR] < V_ZERO)
            {
                ar_set_transmit_power[V_FOUR] = V_ZERO;
            }
            if(ar_set_transmit_power[V_FOUR] >= V_EIGHT)
            {
                ar_set_transmit_power[V_FOUR] = V_EIGHT;
            }
            g_ble_transmit_power = ar_set_transmit_power[V_FOUR];
        }
        else     /*设置检验出错，就用默认参数广播间隔300ms，发射功率0db*/
        {
            ar_set_boardcast_params[V_FOUR] = DEFAULT_BROADCAST%256;
            ar_set_boardcast_params[V_FIVE] = DEFAULT_BROADCAST/256;   /*默认广播间隔300ms*/
            g_ble_boardcost_time = ((ar_set_boardcast_params[V_FOUR] << 8) + ar_set_boardcast_params[V_FIVE])/100;
            g_ble_transmit_power = DEFAULT_TRANSMIT_POWER;
            ar_set_transmit_power[V_FOUR] = g_ble_transmit_power;
        }
    }
    else/*参数未被设置，就用默认参数广播间隔300ms，发射功率0db*/
    {
        ar_set_boardcast_params[V_FOUR] = DEFAULT_BROADCAST;
        ar_set_boardcast_params[V_FIVE] = DEFAULT_BROADCAST/256;   /*默认广播间隔300ms*/
        g_ble_boardcost_time = (ar_set_boardcast_params[V_FOUR] + (ar_set_boardcast_params[V_FIVE] << V_EIGHT))/100;
        g_ble_transmit_power = DEFAULT_TRANSMIT_POWER;
        ar_set_transmit_power[V_FOUR] = g_ble_transmit_power;
    }
    printf("boardcast time : %d00ms\r\n", g_ble_boardcost_time);
    printf("transmit power : %d\r\n", g_ble_transmit_power);

    W25Q16_HighSpeedBufferRead(FLASH_LIS2DH_THRESHOLD_ADDR, (uint8_t *)&lis2dh_config_t, sizeof(lis2dh_config_t));
    if((lis2dh_config_t.head1 == BLE_HEAD_ONE) && (lis2dh_config_t.head2 == BLE_HEAD_TWO))
    {
        bcc = xor_check((uint8_t *)&lis2dh_config_t,sizeof(lis2dh_config_t) - 1);
        if(lis2dh_config_t.crc == bcc)
        { 
            printf("lis2dh_config_load success,threshold:%d\r\n", lis2dh_config_t.value);
            // return lis2dh_config_t.value;
        }
        else
        {
            lis2dh_config_t.value = LIS2DH_DEFAULT_THRESHOLD;
            printf("lis2dh_config_load crc check fail\r\n");
        } 
    }
    else
    {
            lis2dh_config_t.value = LIS2DH_DEFAULT_THRESHOLD;
            printf("lis2dh_config_load head check fail\r\n");
    }
    lis2dh_set_threshold(lis2dh_config_t.value);

    W25Q16_HighSpeedBufferRead(FLASH_SN_ADDR, ar_sn_buf, SN_DATA_LEN+L_THREE);
    m_data_len = ar_sn_buf[L_ONE];
    // printf("----------------------\n");
    if(m_data_len <= SN_DATA_LEN)
    {
        // printf("m_data_len <= SN_DATA_LEN\n");
        bcc = xor_check(ar_sn_buf, m_data_len+L_TWO);
    }
    else
    {
        // printf("m_data_len > 36\n");
        m_data_len = L_ZERO;
    }
    if((ar_sn_buf[V_ZERO] == FLASH_WRITE_FLAG) && (ar_sn_buf[m_data_len+L_TWO] == bcc))    /*判读SN是否被写入*/
    {
        // printf("sn be writed\n");
        g_sn_data_len = m_data_len;
        memcpy(ar_sn_num, &ar_sn_buf[V_TWO], m_data_len);
        printf("sn=");
        for(uint8_t i = V_ZERO; i < g_sn_data_len; i++)
        {
            printf("%c", ar_sn_num[i]);
        }
    }
    else
    {
        printf("default sn=%s\r\n", ar_sn_num);
        g_sn_data_len = sizeof(ar_sn_num)/sizeof(ar_sn_num[0]);
    }
}

/*********************************************************************
 *	function name:  ble_e95_reset
 *	describe: reset e95 modular
 *	input:
 *	output:
 *	return:null
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
void ble_e95_reset(void)
{
    /*E95硬件复位*/
    E95_RESET_LOW;
    HAL_Delay(10);
    E95_RESET_HIGH;
    HAL_Delay(500);     /*需要个比较大的延时，有个放电时长*/
}

/***************************************************************
* brief  Response to upper computer
* param  unsigned char *response_buf; uint16_t buf_len:buf_len <= 1013; 
* return :null
**************************************************************/
void uart1_response_computer(uint8_t *input, \
                            uint16_t input_buf_len, \
                            unsigned char *out_put)
{
    uint16_t out_data_len = I_ZERO;
    uint8_t data_buf[MAX_RECEIVE_DATA];
    memset(data_buf, V_ZERO, MAX_RECEIVE_DATA);
    /*将需要发送的数据进行加密并且组包成正常的协议格式发送出去*/
    if((input != NULL) && (input_buf_len <= MAX_RECEIVE_DATA))
    {
        out_data_len = Encrypt(input, input_buf_len, out_put);    /*加密信息部分内容*/
        memcpy(&data_buf[V_SEVEN], out_put, out_data_len);
        data_buf[L_ZERO] = FRAME_RES_FIRST_HEAD;     /*帧头1：0xc5*/
        data_buf[L_ONE] = FRAME_RES_SECOND_HEAD;     /*帧头2：0xAA*/
        data_buf[L_TWO] = (uint8_t)(out_data_len + L_ELEVEN);     /*长度*/
        data_buf[L_THREE] = (out_data_len + L_ELEVEN) >> V_EIGHT;
        data_buf[L_FOUR] = ENPT_TINA_AES;     /*加密类型：3为AES加密*/
        data_buf[L_FIVE] = (uint8_t)CUSTOMER_NUM;     /*用户代码默认0x101*/
        data_buf[L_SIX] = CUSTOMER_NUM >> V_EIGHT;
        data_buf[L_SEVEN+out_data_len] = V_ZERO;     /*签名:不需要签名，签名位都为0*/
        data_buf[L_EIGHT+out_data_len] = V_ZERO;
        data_buf[L_NINE+out_data_len] = xor_check(data_buf, out_data_len+V_SEVEN);
        data_buf[L_TEN+out_data_len] = FRAME_TERMINATOR;     /*结束符：0xaa*/
        //g_res_e92_data_len = V_ELEVEN+out_data_len;i++
        //memcpy(input, data_buf, g_res_e92_data_len);
        HAL_UART_Transmit(&hlpuart1, data_buf, V_ELEVEN+out_data_len, 0xffffff);
        printf("send data:");
        for(uint8_t i = V_ZERO; i < (out_data_len+V_ELEVEN); i++)
        {
            printf("%02x ", data_buf[i]);
        }
    }
    else
    {
        printf("pst err!");
    }
}

/*********************************************************************
 *	function name:  xor_check
 *	describe: Calculate XOR value
 *	input:char *padta:buffer; uint16_t data_len:buffer length; 
*         char *str：Store converted String
 *	output:null
 *	return:crc value
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
uint8_t xor_check(uint8_t *pdata, uint16_t data_len)
{
    uint8_t m_bcc = 0;

    if(pdata != NULL)
    {
        if(data_len != 0)
        {
            for(uint16_t i = 0; i < data_len; i++)
            {
                m_bcc ^= pdata[i];
            }
        }
        else
        {
            /* code */
        }
        
    }
    else
    {
        /* code */
    }

    return m_bcc;
}

/***************************************************************
* brief  memcpy receive data to struct
* param  uint8_t *rev_buf:ble data; uint8_t counter:ble data length;
* unsigned char *output:data decrypted by AES.
* return ：null
**************************************************************/
uint8_t memcpy_buf_to_strcut(uint8_t *rev_buf, \
                         uint8_t counter, \
                         unsigned char *output)
{
    uint16_t m_data_len = V_ZERO;
    /*接收到的数据存到协议结构体中，并解密相关内容*/
    if((rev_buf != NULL) && (output != NULL) && (counter != L_ZERO))
    {
        new_protocol_fram.first_head = rev_buf[V_ZERO];
        new_protocol_fram.second_head = rev_buf[V_ONE];
        new_protocol_fram.data_len = (uint16_t)(rev_buf[V_THREE] << V_EIGHT) + rev_buf[V_TWO];
        if((new_protocol_fram.data_len == g_usart1_tx_len) && (g_usart1_tx_len <= MAX_RECEIVE_DATA))
        {
            if((new_protocol_fram.first_head == FRAME_FIRST_HAED) && (new_protocol_fram.second_head == FRAME_FIRST_SECOND_HEAD))
            {
                for(uint8_t i = V_SEVEN; i <= 22; i++)
                {
                    printf("%d ", rev_buf[i]);
                }
                printf("\n");
                new_protocol_fram.encrypt_type = rev_buf[V_FOUR];
                new_protocol_fram.customer_num = (uint16_t)(rev_buf[V_SIX] << V_EIGHT)+ rev_buf[V_FIVE];
                new_protocol_fram.sign_l = rev_buf[new_protocol_fram.data_len-L_FOUR];
                new_protocol_fram.sign_h = rev_buf[new_protocol_fram.data_len-L_THREE];
                new_protocol_fram.bcc = rev_buf[new_protocol_fram.data_len-L_TWO];
                new_protocol_fram.terminator = rev_buf[new_protocol_fram.data_len-L_ONE];
                m_data_len = Decrypt(&rev_buf[V_SEVEN], new_protocol_fram.data_len - L_ELEVEN, output);  /*解密数据*/
                printf("m_data_len = %d\n", m_data_len);
                if(m_data_len == 0)     /*解密出来的长度不可能为0*/
                {
                    printf("decrypt error!");
                }
                else
                {
                    printf("decrypt data:");
                    for(uint8_t i = 0; i < m_data_len; i++)
                    {
                        printf("%02x",output[i]);
                    }
                    new_protocol_fram.board_num = output[V_ZERO];
                    new_protocol_fram.package_num = output[V_ONE];
                    new_protocol_fram.Product_line = output[V_TWO];
                    new_protocol_fram.cmd_type = output[V_THREE];
                    new_protocol_fram.info_len = (uint16_t)(output[V_FIVE] << V_EIGHT)+ output[V_FOUR];
                    memcpy(new_protocol_fram.info_buf,&output[V_SIX],new_protocol_fram.info_len);
                }
            }
            else
            {
                printf("head err!\r\n");
            }
        }
        else 
        {
            printf("data length error!\r\n");
        }
    }
    else
    {
        printf("pst err！");
    }
    return m_data_len;
}

/*********************************************************************
 *	function name:  usart3_recv_data_check
 *	describe: analysis of Bluetooth data
 *	input:uint8_t *pdata:ble data;uint8_t data_len:ble data length; 
 *  unsigned char *output:data decrypted by AES.
 *	output:null:
 *	return:uart_receive_data_check:	UART_RECEIVE_SUCCESS,
 *  UART_RECEIVE_BCC_ERR,
 *   UART_RECEIVE_MD5_ERR,
 *    UART_RECEIVE_HEAD_ERR,
 *     UART_RECEIVE_LEN_ERR,
 *      UART_RECEIVE_ENCRYPT_TYPE_ERR,
 *       UART_RECEIVE_NO_DATA,
 *        UART_RECEIVE_CUST_NUM_ERR,
 *         UART_RECEIVE_BOARD_NUM_ERR,
 *          UART_RECEIVE_PACK_NUM_ERR,
 *           UART_RECEIVE_CMD_TYPE_ERR,
 *            UART_RECEIVE_INFO_CMD_ERR,
 *             UART_RECEIVE_INFO_LEN_ERR,
 *              UART_RECEIVE_TERMINATOR_ERR,
 *               UART_RECEIVE_SIGN_ERR
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
uart_receive_data_check usart1_recv_data_check(uint8_t *pdata, uint8_t data_len, unsigned char *output)
{
    uart_receive_data_check ret = UART_RECEIVE_SUCCESS;
    uint16_t m_info_len = V_ZERO;
    uint8_t m_bcc = V_ZERO;
    uint8_t m_decrypt_data_len = V_ZERO;

    printf("receive data:");
    for(uint8_t i = V_ZERO; i < data_len; i++)
    {
        printf("%02x\n", pdata[i]);
    }
    if((pdata != NULL) && (data_len != V_ZERO) && (output != NULL))     /*判读长度及指针有效性*/
    {
        if(g_ble_e95_work_mode == E95_SET_MODE)     /*指令模式下就用E95指令格式解析数据，上电后先是指令模式，设置E95参数完成后MCU才会控制E95进入透传模式*/
        {
            if((pdata[0] == FRAM_FIRST_HEAD) && (pdata[1] == FRAM_SECOND_HEAD))     /*E95回复数据格式 01 FC LEN STATUS ...*/
            {
                if(data_len == (pdata[3] + 4))
                {
                    ret = UART_RECEIVE_SUCCESS;
                    g_ble_e92_cmd = pdata[2];
                    printf("USART_PROTOCOL_OK!\n");
                }
                else
                {
                    printf("USART_PROTOCOL_LEN_ERR!\n");
                    ret = UART_RECEIVE_LEN_ERR;
                }
            }
            else
            {
                printf("USART_PROTOCOL_HEAD_ERR!\n");
                ret = UART_RECEIVE_HEAD_ERR;
            }
        }
        else if(g_ble_e95_work_mode == E95_WORK_MODE)/*透传模式下就用与APP间的协议格式解析数据*/
        {
            printf("memcpy to struct!");
            m_bcc = xor_check(pdata, data_len-L_FOUR);
            printf("m_bcc=%02X", m_bcc);
            m_decrypt_data_len = memcpy_buf_to_strcut(pdata, data_len, output);
            if(m_decrypt_data_len != 0)
            {
                if((new_protocol_fram.first_head == FRAME_FIRST_HAED) && (new_protocol_fram.second_head == FRAME_FIRST_SECOND_HEAD))     /*帧头CA 55*/
                {
                    if(new_protocol_fram.data_len == data_len)
                    {
                        printf("new_protocol_fram.data_len=%04X", new_protocol_fram.data_len);
                        if(new_protocol_fram.encrypt_type == ENPT_TINA_AES)     /*AES加密方式*/
                        {
                            printf("new_protocol_fram.encrypt_type=%02X", new_protocol_fram.encrypt_type);
                            if(new_protocol_fram.customer_num == CUSTOMER_NUM)    /*用户编码*/
                            {
                                printf("new_protocol_fram.customer_num=%02X", new_protocol_fram.customer_num);
                                if(new_protocol_fram.board_num == BOARD_NUM)     /*板号*/
                                {
                                    printf("new_protocol_fram.board_num=%02X", new_protocol_fram.board_num);
                                    //if(new_protocol_fram.package_num == PACKAGE_NUM)
                                    {
                                        if(new_protocol_fram.Product_line == DEPOSIT_EXPRESS)     /*产品线*/
                                        {
                                            printf("new_protocol_fram.Product_line=%02X", new_protocol_fram.Product_line);
                                            printf("new_protocol_fram.cmd_type=%02X", new_protocol_fram.cmd_type);
                                            switch(new_protocol_fram.cmd_type)
                                            {
                                                case GET_ALL_SLOT_STATUS:
                                                case GET_ALL_SLOT_GOODS_STATUS:
                                                case GET_SOFTWARE_VER:
                                                case GET_BLE_NAME:
                                                case GET_BAT_VOLTAGE:
                                                case GET_RTC_TIME:
                                                case GET_BROADCAST_PARA:
                                                case GET_TEMP:
                                                case GET_HUM:
                                                case GET_SN:
                                                case GET_BUTTON_BAT:
                                                case IOT_TEST:
                                                case GET_BLE_PRODUCT_MODEL:
                                                    m_info_len = L_ZERO;
                                                    break;
                                                case OPEN_ONE_SLOT_DOOR:
                                                case GET_ONE_SLOT_STATUS:
                                                case GET_ONE_SLOT_GOODS_STATUS:
                                                case SET_RTC_ALARM_TIME:
                                                    m_info_len = L_ONE;
                                                    break;
                                                case SET_BROADCAST_PARA:
                                                    m_info_len = L_TWO;
                                                    break;
                                                case SET_LIS2DH_PARA:
                                                case BATCH_OPENING:
                                                    m_info_len = L_THREE;
                                                    break;
                                                case SET_RTC_TIME:
                                                    m_info_len = L_FIVE;
                                                    break;
                                                case SET_BLE_NAME:
                                                case SET_SN:
                                                    m_info_len = new_protocol_fram.info_len;
                                                    break;
                                                default:
                                                {
                                                    return UART_RECEIVE_INFO_CMD_ERR;
                                                }
                                            }
                                            printf("new_protocol_fram.info_len=%02x\r\n", new_protocol_fram.info_len);
                                            printf("m_info_len = %02x\r\n", m_info_len);
                                            if(new_protocol_fram.info_len == m_info_len)
                                            {
                                                if(new_protocol_fram.bcc == m_bcc)
                                                {
                                                    if(new_protocol_fram.terminator == FRAME_TERMINATOR)     /*结束符*/
                                                    {
                                                        if((V_ZERO == new_protocol_fram.sign_l) || \
                                                            (V_ZERO == new_protocol_fram.sign_h))
                                                        {
                                                            printf("UART_RECEIVE_SUCCESS\r\n");
                                                        }
                                                        else
                                                        {
                                                            printf("UART_RECEIVE_SIGN_ERR\r\n");
                                                            ret = UART_RECEIVE_SIGN_ERR;
                                                        }
                                                    }
                                                    else
                                                    {
                                                        printf("UART_RECEIVE_TERMINATOR_ERR\r\n");
                                                        ret = UART_RECEIVE_TERMINATOR_ERR;
                                                    }
                                                }
                                                else
                                                {
                                                    printf("UART_RECEIVE_BCC_ERR\r\n");
                                                    ret = UART_RECEIVE_BCC_ERR;
                                                }
                                            }
                                            else
                                            {
                                                printf("UART_RECEIVE_INFO_LEN_ERR\r\n");
                                                ret = UART_RECEIVE_INFO_LEN_ERR;
                                            }
                                        }
                                        else
                                        {
                                            printf("UART_RECEIVE_CMD_TYPE_ERR\r\n");
                                            ret = UART_RECEIVE_CMD_TYPE_ERR;
                                        }
                                    }
                                    #if 0
                                    else
                                    {
                                        data_check = UART_RECEIVE_PACK_NUM_ERR;
                                    }
                                    #endif
                                }
                                else
                                {
                                    printf("UART_RECEIVE_BOARD_NUM_ERR\r\n");
                                    ret = UART_RECEIVE_BOARD_NUM_ERR;
                                } 
                            }
                            else
                            {
                                printf("UART_RECEIVE_CUST_NUM_ERR\r\n");
                                ret = UART_RECEIVE_CUST_NUM_ERR;
                            }
                        }
                        else
                        {
                            printf("UART_RECEIVE_ENCRYPT_TYPE_ERR\r\n");
                            ret = UART_RECEIVE_ENCRYPT_TYPE_ERR;
                        }
                    }
                    else
                    {
                        printf("UART_RECEIVE_LEN_ERR\r\n");
                        ret = UART_RECEIVE_LEN_ERR;
                    }
                }
                else
                {
                    ret = UART_RECEIVE_HEAD_ERR;
                    printf("UART_RECEIVE_HEAD_ERR\r\n");
                }
            }
            else
            {
                ret = UART_RECEIVE_DECRYPT_ERR;
            }
        }
        else
        {
            printf("unknown e95 work mode!");
        }
    }
    else
    {
        printf("USART_PROTOCOL_DATA_NULL!\n");
        ret = USART_PROTOCOL_DATA_NULL;
    }
    
    return ret;
}

/*********************************************************************
 *	function name:  usart3_protocol_handle
 *	describe: analysis of Bluetooth data and E95 data
 *	input:void
 *	output:void
 *	return:void
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
void usart1_protocol_handle(void)
{
    uart_receive_data_check handle_cmd;
    uint16_t m_sn_info_len = L_ZERO;
    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
    handle_cmd = usart1_recv_data_check(ar_usart1_tx_buffer, g_usart1_tx_len, ar_ble_decrypt_buf);
    if(handle_cmd == UART_RECEIVE_SUCCESS)     /*收取数据格式正确*/
    {
        if(g_ble_e95_work_mode == E95_SET_MODE)     /*E95在指令模式下解析MCU与E95通讯的数据，指令模式下只设置E95参数*/
        {
            switch(g_ble_e92_cmd)
            {
                case BLE_NAME_SET:                      /*设置E95名称回复*/
                {
                    g_set_param_timeout_flag = V_ZERO;
                    g_current_step = V_ZERO;
                    g_set_param_count = V_ZERO;
                    g_send_cmd_cnt = V_ZERO;
                    if(ar_usart1_tx_buffer[BLE_STATUS_INDEX] == BLE_STATUS_SUCCESS)     /*状态位为0，说明设置成功*/
                    {
                        printf("SET NAME SUCCESS!\n");
                        g_set_step = SET_E95_BROADCAST;
                    }
                    else
                    {
                        g_set_step = V_ZERO;
                        printf("SET NAME FAILED!\n");
                    }
                    break;
                }
                case BLE_BROADCAST_SET:     /*设置E95广播间隔回复*/
                {
                    g_set_param_timeout_flag = V_ZERO;
                    g_current_step = V_ZERO;
                    g_set_param_count = V_ZERO;
                    g_send_cmd_cnt = V_ZERO;
                    if(ar_usart1_tx_buffer[BLE_STATUS_INDEX] == BLE_STATUS_SUCCESS)    /*状态位为0，说明设置成功*/
                    {
                        printf("SET BROADCAST SUCCESS!\n");
                        g_set_step = SET_E95_POWER;
                        g_send_cmd_cnt = V_ZERO;
                    }
                    else
                    {
                        g_set_step = V_ZERO;
                        printf("SET BROADCAST FAILED!\n");
                    }
                    break;
                }
                case BLE_POWER_SET:     /*设置E95发射功率回复*/
                {
                    g_set_param_timeout_flag = V_ZERO;
                    g_current_step = V_ZERO;
                    g_set_param_count = V_ZERO;
                    g_send_cmd_cnt = V_ZERO;
                    if(ar_usart1_tx_buffer[BLE_STATUS_INDEX] == BLE_STATUS_SUCCESS)     /*状态位为0，说明设置成功*/
                    {
                        printf("SET POWER SUCCESS!\n");
                        printf("SET BLE PARA FINISH!\n");
                        g_set_step = GET_E95_MODEL;
                        g_send_cmd_cnt = V_ZERO;
                    }
                    else
                    {
                        g_set_step = V_ZERO;
                        printf("SET POWER FAILED!\n");
                    }
                    break;
                }
                case BLE_CHECK_MODEL:     /*读取产品型号回复*/
                {
                    g_set_param_timeout_flag = V_ZERO;
                    g_current_step = V_ZERO;
                    g_set_param_count = V_ZERO;
                    g_send_cmd_cnt = V_ZERO;
                    if(ar_usart1_tx_buffer[BLE_STATUS_INDEX] == BLE_STATUS_SUCCESS)    /*状态位为0，说明获取成功*/
                    {
                        printf("GET MODEL SUCCESS!\n");
                        printf("SET BLE PARA FINISH!\n");
                        ar_ble_model[V_ZERO] = ar_usart1_tx_buffer[BLE_LEN_INDEX];
                        memcpy(&ar_ble_model[V_ONE], &ar_usart1_tx_buffer[BLE_INFOR_INDEX], ar_ble_model[V_ZERO]);     /*将获取到的产品型号存在buffer中，方便在透传模式下APP端读取*/
                        g_set_step = V_ZERO;
                        g_send_cmd_cnt = V_ZERO;
                        E95_MODE_TRANSPARENT;
                        E95_BROADCAST_EN;
                        g_ble_e95_work_mode = E95_WORK_MODE;
                    }
                    else
                    {
                        g_set_step = V_ZERO;
                        printf("GET E95 MODEL FAILED!\n");
                    }
                    break;
                }
                default:
                {
                    printf("unknown error!");
                    break;
                }
            }
        }
        else if(g_ble_e95_work_mode == E95_WORK_MODE)     /*E95在透传模式下解析与app端的通讯数据*/
        {
            g_ble_connect_count = V_ZERO;
            uint8_t ar_ble_load_name[L_TWENTY_FOUR] = {I_ZERO};
            uint8_t ar_sn_test_buf[SN_DATA_LEN+L_TWO] = {I_ZERO};   /*SN码36字节+帧头1字节+校验1字节*/

            switch(new_protocol_fram.cmd_type)
            {
                case GET_ALL_SLOT_STATUS:     /*获取所有格口门磁状态*/
                {
                    door_status_refresh();
                    ar_response_buffer[L_ZERO] = BOARD_NUM;
                    ar_response_buffer[L_ONE] = new_protocol_fram.package_num;  
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[L_FIVE] = GET_ALL_SLOT_STATUS;
                    ar_response_buffer[L_SIX] = L_FOUR;
                    ar_response_buffer[L_SEVEN] = L_ZERO;
                    ar_response_buffer[L_EIGHT] = g_door_stat[L_ZERO];
                    ar_response_buffer[L_NINE] = g_door_stat[L_ONE];
                    ar_response_buffer[L_TEN] = g_door_stat[L_TWO];
                    ar_response_buffer[L_ELEVEN] = V_ZERO;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_TWELVE, ar_ble_decrypt_buf);
                    printf("GET_ALL_SLOT_STATUS!\r\n");
                    break;
                }
                case GET_ONE_SLOT_STATUS:     /*获取一个格口门磁状态*/
                {
                    uint8_t m_slot_num = I_ZERO;
                    m_slot_num = ar_ble_decrypt_buf[V_SIX];
                    if(m_slot_num <= MAX_DOOR_NUM)
                    {
                        m_slot_num = m_slot_num - V_ONE;
                        door_status_index(m_slot_num);
                        ar_response_buffer[L_TWO] = V_ZERO;
                        ar_response_buffer[L_THREE] = V_ZERO;
                        ar_response_buffer[L_EIGHT] = g_stdoor_stat[m_slot_num];
                    }
                    else
                    {
                        ar_response_buffer[L_TWO] = SLOT_NUM_ERR%256;
                        ar_response_buffer[L_THREE] = SLOT_NUM_ERR/256;
                        ar_response_buffer[L_EIGHT] = V_ZERO;
                    }
                    ar_response_buffer[L_ZERO] = BOARD_NUM;
                    ar_response_buffer[L_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[L_FIVE] = GET_ONE_SLOT_STATUS;
                    ar_response_buffer[L_SIX] = L_ONE;
                    ar_response_buffer[L_SEVEN] = L_ZERO;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_NINE, ar_ble_decrypt_buf);
                    printf("GET_ONE_SLOT_STATUS!\r\n");
                    break;
                }
                case GET_ALL_SLOT_GOODS_STATUS:     /*获取所有格口物品状态*/
                {
                    get_all_goods_state_fun();
                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = GET_ALL_SLOT_GOODS_STATUS;
                    ar_response_buffer[V_SIX] = L_FOUR;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    ar_response_buffer[V_EIGHT] = g_all_goods_status[L_ZERO];
                    ar_response_buffer[V_NINE] = g_all_goods_status[L_ONE];
                    ar_response_buffer[V_TEN] = g_all_goods_status[L_TWO];
                    ar_response_buffer[L_ELEVEN]= V_ZERO;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_TWELVE, ar_ble_decrypt_buf);
                    printf("GET_ALL_SLOT_GOODS_STATUS!\r\n");
                    break;
                }
                case GET_ONE_SLOT_GOODS_STATUS:     /*获取一个格口物品状态*/
                {
                    uint8_t m_slot_num = I_ZERO;
                    m_slot_num = ar_ble_decrypt_buf[V_SIX];
                    if(m_slot_num <= MAX_DOOR_NUM)
                    {
                        m_slot_num = m_slot_num - V_ONE;
                        get_one_good_state_fun(m_slot_num);
                        ar_response_buffer[L_TWO] = V_ZERO;
                        ar_response_buffer[L_THREE] = V_ZERO;
                        ar_response_buffer[L_EIGHT] = g_item_stat[m_slot_num];
                    }
                    else
                    {
                        ar_response_buffer[L_TWO] = SLOT_NUM_ERR%256;
                        ar_response_buffer[L_THREE] = SLOT_NUM_ERR/256;
                        ar_response_buffer[L_EIGHT] = V_ZERO;
                    }

                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = GET_ONE_SLOT_GOODS_STATUS;
                    ar_response_buffer[V_SIX] = L_ONE;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_NINE, ar_ble_decrypt_buf);
                    printf("GET_ONE_SLOT_GOODS_STATUS!\r\n");
                    break;
                }
                case OPEN_ONE_SLOT_DOOR:     /*开锁*/
                {
                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = OPEN_ONE_SLOT_DOOR;
                    ar_response_buffer[V_SIX] = L_ONE;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    ar_response_buffer[V_EIGHT] = OPEN_SLOT_SUCCESS;
                    g_lock_number = ar_ble_decrypt_buf[V_SIX];
                    if((g_lock_number != V_ZERO) && (g_lock_number <= MAX_DOOR_NUM))
                    {
                        uart1_response_computer(ar_response_buffer, L_NINE, ar_ble_decrypt_buf);
                        g_lock_number = g_lock_number - V_ONE;
                        door_control_index(g_lock_number, OPEN_DOOR_TIME, OPEN_DOOR);
                    }
                    else
                    {
                        ar_response_buffer[V_EIGHT] = OPEN_SLOT_FAILED;
                        uart1_response_computer(ar_response_buffer, L_NINE, ar_ble_decrypt_buf);
                    }
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    printf("OPEN_ONE_SLOT_DOOR!\r\n");
                    break;
                }
                case GET_SOFTWARE_VER:     /*获取软件版本号*/
                {
                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = GET_SOFTWARE_VER;
                    ar_response_buffer[V_SIX] = L_THIRTEEN;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    ar_response_buffer[V_EIGHT] = 0x76;
                    memcpy(&ar_response_buffer[V_EIGHT], ar_software_ver, 13);
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_TWENTY_ONE, ar_ble_decrypt_buf);
                    printf("GET_SOFTWARE_VER!\r\n");
                    break;
                }
                case GET_BLE_NAME:     /*获取蓝牙名称*/
                {
                    /*mcu上后会从flash中将数据读取到ar_ble_name_buf中*/
                    ar_response_buffer[L_ZERO] = BOARD_NUM;
                    ar_response_buffer[L_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[L_FIVE] = GET_BLE_NAME;
                    ar_response_buffer[L_SIX] = g_ble_name_len;
                    ar_response_buffer[L_SEVEN] = L_ZERO;
                    memcpy(&ar_response_buffer[L_EIGHT], ar_ble_name_buf,  ar_response_buffer[L_SIX]);
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, ar_response_buffer[V_SIX]+L_EIGHT, ar_ble_decrypt_buf);
                    printf("GET_BLE_NAME!\r\n");
                    break;
                }
                case SET_BLE_NAME:     /*设置蓝牙名称*/
                {
                    /*先将名称保存到flash中，然后重启MCU，mcu重启
                      后先从flash中读出名称，然后再设置到E95中*/
                    if((ar_ble_decrypt_buf[V_FOUR] >= L_ONE) && (ar_ble_decrypt_buf[V_FOUR] <= L_TWENTY))     /*判断名称长度范围1-20字节*/
                    {
                        memcpy(&ar_ble_load_name[V_ONE], &ar_ble_decrypt_buf[V_SIX], ar_ble_decrypt_buf[V_FOUR]);
                        ar_ble_load_name[V_ZERO] = ar_ble_decrypt_buf[V_FOUR] + L_ONE;
                        ar_ble_load_name[ar_ble_load_name[V_ZERO]] = xor_check(ar_ble_load_name, ar_ble_load_name[V_ZERO]);
                        W25Q16_SectorErase_4KByte(FLASH_BLE_NAME_ADDR);
                        W25Q16_PageWrite(FLASH_BLE_NAME_ADDR, ar_ble_load_name, ar_ble_load_name[V_ZERO]+L_ONE);
                        ar_response_buffer[V_EIGHT] = V_ZERO;
                    }
                    else     /*否则上报设置失败*/
                    {
                        ar_response_buffer[V_EIGHT] = V_ONE;
                    }
                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = SET_BLE_NAME;
                    ar_response_buffer[V_SIX] = L_ONE;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_NINE, ar_ble_decrypt_buf);    /*如果发送完数据后马上重启mcu会导致app
                                                                                                  端收不到数据，所有发送数据后延时100ms*/
                    printf("SET_BLE_NAME!\r\n");
                    HAL_Delay(100);     /*等待发送的蓝牙数据被app接收到*/
                    E95_BROADCAST_DIS;
                    __set_FAULTMASK(1); /*close all interrupts*/
                    NVIC_SystemReset();
                    break;
                }
                case GET_BAT_VOLTAGE:     /*获取电池包电压*/
                {
                    BAT_ADC_EN;    /*使能电池包电压读取*/
                    HAL_Delay(5);
                    if(LOCK_POWER_SOURCE == PIN_LEVEL_HIGH)    /*判读供电方式：市电或者电池包*/
                    {
                        get_adc3_value();    /*电池包供电的话就读取adc值*/
                        ar_get_voltage_value[V_ZERO] += DEVICE_VOLTAGE_DROP;
                    }
                    else    /*市电供电的话，上传电压值为定值：9V，即满电压*/
                    {
                        ar_get_voltage_value[V_ZERO] = MAINS_VOLTAGE;
                    }
                    BAT_ADC_DIS;
                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = GET_BAT_VOLTAGE;
                    ar_response_buffer[V_SIX] = L_TWO;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    ar_response_buffer[V_EIGHT] = ar_get_voltage_value[V_ZERO]%256;
                    ar_response_buffer[V_NINE] = ar_get_voltage_value[V_ZERO]/256;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_TEN, ar_ble_decrypt_buf);
                    printf("GET_BAT_VOLTAGE!\r\n");
                    break;
                }
                case SET_RTC_TIME:     /*设置RTC*/
                {
                    for(uint8_t i = V_SIX; i <= V_TEN; i++)
                    {
                        printf("%d ", ar_ble_decrypt_buf[i]);
                    }
                    printf(" ar_ble_decrypt_buf\n");
                    set_rtc_date_time.hundredkilo = ar_ble_decrypt_buf[V_SIX];
                    set_rtc_date_time.kilo = ar_ble_decrypt_buf[V_SEVEN];
                    set_rtc_date_time.ten = ar_ble_decrypt_buf[V_EIGHT];
                    set_rtc_date_time.minute = ar_ble_decrypt_buf[V_NINE];
                    set_rtc_date_time.second = ar_ble_decrypt_buf[V_TEN];
                    pcf85263_set_time(set_rtc_date_time);
                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = SET_RTC_TIME;
                    ar_response_buffer[V_SIX] = L_ONE;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    ar_response_buffer[V_EIGHT] = V_ZERO;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_NINE, ar_ble_decrypt_buf);
                    printf("SET_RTC_TIME!\r\n");
                    break;
                }
                case GET_RTC_TIME:     /*获取RTC值*/
                {
                    uint8_t m_fun_data = 0x90;
                    pcf85263_write_data(PCF8563_ADDR_FUN, &m_fun_data, L_ONE);
                    pcf85263_query_time(&get_rtc_date_time);
                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = GET_RTC_TIME;
                    ar_response_buffer[V_SIX] = L_FIVE;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    ar_response_buffer[V_EIGHT] = get_rtc_date_time.hundredkilo;
                    ar_response_buffer[V_NINE] = get_rtc_date_time.kilo;
                    ar_response_buffer[V_TEN] = get_rtc_date_time.ten;
                    ar_response_buffer[L_ELEVEN] = get_rtc_date_time.minute;
                    ar_response_buffer[L_TWELVE] = get_rtc_date_time.second;
//                    ar_response_buffer[L_THIRTEEN] = get_rtc_date_time.millis;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_THIRTEEN, ar_ble_decrypt_buf);
                    printf("GET_RTC_TIME!\r\n");
                    break;
                }
                case SET_RTC_ALARM_TIME:     /*设置RTC ALARM时间*/
                {
                    g_alarm_set_num = ar_ble_decrypt_buf[V_SIX];
                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = SET_RTC_ALARM_TIME;
                    ar_response_buffer[V_SIX] = L_ONE;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    ar_response_buffer[V_EIGHT] = V_ZERO;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_NINE, ar_ble_decrypt_buf);
                    printf("SET_RTC_ALARM_TIME!\r\n");
                    break;
                }                
                case SET_BROADCAST_PARA:     /*设置广播参数*/
                {
                    /*将设置参数保存到外部flash中，然后重启mcu，
                      重启后mcu从flash中读出参数并设置到e95中*/
                    uint8_t ar_ble_set_boardcost_para[V_FOUR] = {I_ZERO}; 
                    g_ble_boardcost_time = ar_ble_decrypt_buf[V_SIX];   /*设置时间单位100ms*/
                    g_ble_transmit_power = ar_ble_decrypt_buf[V_SEVEN];  /*设置蓝牙发射功率*/
                    ar_ble_set_boardcost_para[V_ZERO] = FLASH_WRITE_FLAG;
                    ar_ble_set_boardcost_para[V_ONE] = g_ble_boardcost_time;
                    ar_ble_set_boardcost_para[V_TWO] = g_ble_transmit_power;
                    ar_ble_set_boardcost_para[V_THREE] = xor_check(ar_ble_set_boardcost_para, L_THREE);
                    W25Q16_SectorErase_4KByte(FLASH_BLE_BOARDCOST_PARA);
                    W25Q16_PageWrite(FLASH_BLE_BOARDCOST_PARA, ar_ble_set_boardcost_para, L_FOUR);
                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = SET_BROADCAST_PARA;
                    ar_response_buffer[V_SIX] = L_ONE;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    ar_response_buffer[V_EIGHT] = V_ZERO;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_NINE, ar_ble_decrypt_buf);
                    printf("SET_BROADCAST_PARA!\r\n");
                    HAL_Delay(100);
                    E95_BROADCAST_DIS;
                    __set_FAULTMASK(1); /*close all interrupts*/
                    NVIC_SystemReset();
                    break;
                }
                case GET_BROADCAST_PARA:     /*获取广播参数：广播间隔、发射功率*/
                {
                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = GET_BROADCAST_PARA;
                    ar_response_buffer[V_SIX] = L_TWO;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    ar_response_buffer[V_EIGHT] = g_ble_boardcost_time;
                    ar_response_buffer[V_NINE] = g_ble_transmit_power;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_TEN, ar_ble_decrypt_buf);
                    printf("GET_BROADCAST_PARA!\r\n");
                    break;
                }
                case GET_TEMP:     /*获取温度*/
                {
                    HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);     /*读取温湿度值时需先关闭中断，否则读取过程中有可能会被中断打断，导致读取数据不正确*/
                    //__set_FAULTMASK(1);
                    for(uint8_t i = 0; i < 8; i++)/*多次读取，防止DHT22出现收到起始信号未返回响应信号情况*/
                    {
                        printf("read count = %d\r\n", i);
                        read_dht22_RH();    
                        if(g_read_dht22_flag)   
                        {
                            g_read_dht22_flag = V_ZERO;
                            break;
                        }  
                        HAL_Delay(400);              
                    }
                    //__set_FAULTMASK(0);
                    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);

                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = GET_TEMP;
                    ar_response_buffer[V_SIX] = L_TWO;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    ar_response_buffer[V_EIGHT] = g_temp%256;     /*温度值实际扩大了10倍，保留小数点后两位*/
                    ar_response_buffer[V_NINE] = g_temp/256;
                    HAL_Delay(10);
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_TEN, ar_ble_decrypt_buf);
                    printf("GET_TEMP:%04x!\r\n", g_temp);
                    break;
                }
                case GET_HUM:     /*获取湿度*/
                {
                    HAL_NVIC_DisableIRQ(TIM6_DAC_IRQn);     /*读取温湿度值时需先关闭中断，否则读取过程中有可能会被中断打断，导致读取数据不正确*/
                    //__set_FAULTMASK(1);
                    for(uint8_t i = 1; i < 8; i++)/*多次读取，防止DHT22出现收到起始信号未返回响应信号情况*/
                    {
                        printf("read count = %d\r\n", i);
                        read_dht22_RH();    
                        if(g_read_dht22_flag)   
                        {
                            g_read_dht22_flag = V_ZERO;
                            break;
                        }  
                        HAL_Delay(400);              
                    }
                    //__set_FAULTMASK(0);
                    HAL_NVIC_EnableIRQ(TIM6_DAC_IRQn);


                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = GET_HUM;
                    ar_response_buffer[V_SIX] = L_TWO;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    ar_response_buffer[V_EIGHT] = g_hump%256;     /*温度值实际扩大了10倍*/
                    ar_response_buffer[V_NINE] = g_hump/256;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_TEN, ar_ble_decrypt_buf);
                    printf("GET_HUM:%04x\r\n", g_hump);
                    break;
                }
                case SET_SN:    /*设置SN，SN长度36字节*/
                {
                    m_sn_info_len = (ar_ble_decrypt_buf[V_FIVE] << V_EIGHT) + ar_ble_decrypt_buf[V_FOUR];
                    if((m_sn_info_len <= SN_DATA_LEN) && (m_sn_info_len != L_ZERO))
                    {
                        g_sn_data_len = m_sn_info_len;
                        memcpy(ar_sn_num, &ar_ble_decrypt_buf[V_SIX], m_sn_info_len);
                        printf("sn=");
                        for(uint8_t i = V_ZERO; i < g_sn_data_len; i++)
                        {
                            printf("%c", ar_sn_num[i]);
                        }
                        printf("\r\n");
                        memcpy(&ar_sn_buf[V_TWO], &ar_ble_decrypt_buf[V_SIX], m_sn_info_len);
                        ar_sn_buf[V_ZERO] = FLASH_WRITE_FLAG;    /*数据存入flash中时增加帧头0x55与结尾bcc校验，作为SN被设置的标记*/
                        ar_sn_buf[V_ONE] = m_sn_info_len;
                        ar_sn_buf[m_sn_info_len + V_TWO] = xor_check(ar_sn_buf, m_sn_info_len+L_TWO);
                        W25Q16_SectorErase_4KByte(FLASH_SN_ADDR);
                        W25Q16_PageWrite(FLASH_SN_ADDR, ar_sn_buf, m_sn_info_len+L_THREE);
                        for(uint8_t i = V_ZERO; i < (m_sn_info_len+L_THREE); i++)
                        {
                            printf("%02x\r\n", ar_sn_buf[i]);
                        }
                        ar_response_buffer[L_TWO] = V_ZERO;
                        ar_response_buffer[L_THREE] = V_ZERO;
                    }
                    else
                    {
                        ar_response_buffer[L_TWO] = INFO_LEN_ERR%256;
                        ar_response_buffer[L_THREE] = INFO_LEN_ERR/256;
                    }
                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;

                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = SET_SN;
                    ar_response_buffer[V_SIX] = L_ONE;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    ar_response_buffer[V_EIGHT] = SET_SN_SUCCESS;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_NINE, ar_ble_decrypt_buf);
                    printf("SET_SN!\r\n");
                    break;
                }
                case GET_SN:    /*读取SN*/
                {
                    uint8_t m_xor_value = I_ZERO;
                    uint8_t m_sn_len = I_ZERO;
                    W25Q16_HighSpeedBufferRead(FLASH_SN_ADDR, ar_sn_test_buf, SN_DATA_LEN+L_THREE);
                    m_sn_len = ar_sn_test_buf[L_ONE];
                    printf("m_sn_len = %d\r\n", m_sn_len);
                    if(m_sn_len <= SN_DATA_LEN)
                    {
                        m_xor_value = xor_check(ar_sn_test_buf, m_sn_len+L_TWO);
                        if((ar_sn_test_buf[V_ZERO] == FLASH_WRITE_FLAG) && (ar_sn_test_buf[m_sn_len+L_TWO] == m_xor_value))    /*判读SN是否被写入*/
                        {
                            memcpy(&ar_response_buffer[V_EIGHT], &ar_sn_test_buf[V_TWO], m_sn_len);
                        }
                        else
                        {
                            m_sn_len = L_ELEVEN;
                            memset(&ar_response_buffer[V_EIGHT], ar_sn_num, m_sn_len);    /*默认SN全为0xff*/
                        }
                    }
                    else
                    {
                        m_sn_len = L_ELEVEN;
                        memcpy(&ar_response_buffer[V_EIGHT], ar_sn_num, m_sn_len);    /*默认SN全为0xff*/
                        printf("ar_sn_num = %s\r\n", ar_sn_num);
                    }

                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = GET_SN;
                    ar_response_buffer[V_SIX] = m_sn_len;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, m_sn_len+L_EIGHT, ar_ble_decrypt_buf);
                    printf("GET_SN!\r\n");
                    break;
                }
                case GET_BUTTON_BAT:     /*获取纽扣电池电压*/
                {
                    BAT_VOLTAGE_COM_EN;     /*使能纽扣电池电压*/
                    HAL_Delay(5);
                    get_adc3_value();
                    printf("coin voltage = %d\r\n",ar_get_voltage_value[V_ONE]);
                    BAT_VOLTAGE_COM_DIS;
                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = GET_BUTTON_BAT;
                    ar_response_buffer[V_SIX] = L_ONE;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    ar_response_buffer[V_EIGHT] = ar_get_voltage_value[V_ONE];
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_NINE, ar_ble_decrypt_buf);
                    printf("GET_BUTTON_BAT!\r\n");
                    break;
                }
                case IOT_TEST:
                {
                    g_iot_start_test = V_ONE;
                    g_start_http_flag = V_ONE;
                    printf("IOT_TEST!\r\n");
                    break;
                }
                case SET_LIS2DH_PARA:     /*设置震动传感器阈值以及信息查询*/
                {
                    uint8_t ret = V_ZERO;
                    uint8_t m_lis2dh_threshold = ar_ble_decrypt_buf[L_SIX];
                    uint8_t m_lis2dh_query = ar_ble_decrypt_buf[L_SEVEN];
                    printf("threshold = %d\r\n", m_lis2dh_threshold);
                    printf("m_lis2dh_query = %d\r\n", m_lis2dh_query);
                    ar_response_buffer[V_ZERO] = BOARD_NUM;
                    ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                    ar_response_buffer[L_TWO] = V_ZERO;
                    ar_response_buffer[L_THREE] = V_ZERO;
                    ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                    ar_response_buffer[V_FIVE] = SET_LIS2DH_PARA;
                    ar_response_buffer[V_SIX] = L_EIGHT;
                    ar_response_buffer[V_SEVEN] = L_ZERO;
                    if(m_lis2dh_threshold != V_ZERO)
                    {
                        ret = lis2dh_set_threshold(m_lis2dh_threshold);
                        if(ret == LIS2DH_SUCCESS)
                        {
                            printf("set threshold successed!");
                        }
                        else
                        {
                            printf("set threshold failed");
                        }
                        ret = lis2dh_get_threshold(&lis2dh_config_t.value);
                        if(ret == LIS2DH_SUCCESS)
                        {
                            printf("get threshold successed!");
                        }
                        else
                        {
                            printf("get threshold failed");
                        }
                        printf("read threshold = %d\r\n", lis2dh_config_t.value);
                        if(lis2dh_config_t.value == m_lis2dh_threshold)
                        {
                            lis2dh_config_save(m_lis2dh_threshold);
                            ar_response_buffer[V_EIGHT] = LIS2DH_SUCCESS;
                        }
                        else
                        {
                            ar_response_buffer[V_EIGHT] = LIS2DH_ERROR;
                        }
                        if(m_lis2dh_query == V_ONE)     /*需要获取震动传感器信息*/
                        {
                            filter_acc();
                            ar_response_buffer[V_NINE] = acc_sample.x;
                            ar_response_buffer[V_TEN] = acc_sample.x >> V_EIGHT;
                            ar_response_buffer[L_ELEVEN] = acc_sample.y;
                            ar_response_buffer[L_TWELVE] = acc_sample.y >> V_EIGHT;
                            ar_response_buffer[L_THIRTEEN] = acc_sample.z;
                            ar_response_buffer[L_FOURTEEN] = acc_sample.z >> V_EIGHT;
                        }
                        else     /*不需要获取震动传感器信息*/
                        {
                            memset(&ar_response_buffer[V_NINE], V_ZERO, L_SEVEN);
                        }
                    }
                    else
                    {
                        printf("threshold error!\r\n");
                    }
                    memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                    uart1_response_computer(ar_response_buffer, L_SIXTEEN, ar_ble_decrypt_buf);
                    printf("SET_LIS2DH_PARA!\r\n");
                    break;
                }
                case GET_BLE_PRODUCT_MODEL:     /*获取产品型号*/
                {
                    if(ar_ble_model[V_ZERO] != L_ZERO)     /*判断在设置参数时获取到的产品型号长度不为0*/
                    {
                        ar_response_buffer[V_ZERO] = BOARD_NUM;
                        ar_response_buffer[V_ONE] = new_protocol_fram.package_num;
                        ar_response_buffer[L_TWO] = V_ZERO;
                        ar_response_buffer[L_THREE] = V_ZERO;
                        ar_response_buffer[L_FOUR] = DEPOSIT_EXPRESS;
                        ar_response_buffer[V_FIVE] = GET_BLE_PRODUCT_MODEL;
                        ar_response_buffer[V_SIX] = L_SEVENTEEN;
                        ar_response_buffer[V_SEVEN] = L_ZERO;
                        memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
                        memcpy(&ar_response_buffer[V_SEVEN], &ar_ble_model[V_ONE], ar_ble_model[V_ZERO]);
                        uart1_response_computer(ar_response_buffer, L_SEVENTEEN+L_EIGHT, ar_ble_decrypt_buf);
                        printf("GET_BLE_PRODUCT_MODEL!\r\n");
                    }
                    else
                    {
                        printf("GET MODEL ERROR!\r\n");
                    }

                    break;
                }
                case BATCH_OPENING:
                {
                    memcpy(ar_open_slot_num, &ar_ble_decrypt_buf[L_SIX], L_THREE);
                    g_open_one_door_flag = V_ONE;
                    g_open_multiple_doors_num = V_ZERO;
                    g_package_num = new_protocol_fram.package_num;
                    break;
                }
                default:
                    break;
            }
        }
        else
        {
            printf("unknown e95 work mode!");
        }
    }
    else
    {
        if(g_ble_e95_work_mode == E95_WORK_MODE)
        {

            ar_response_buffer[V_ZERO] = V_ONE;
            ar_response_buffer[V_ONE] = V_ZERO;
            ar_response_buffer[V_TWO] = RES_ERR_CODE;
            ar_response_buffer[V_THREE] = L_ONE;
            ar_response_buffer[V_FOUR] = L_ZERO;
            ar_response_buffer[V_FIVE] = handle_cmd;
            memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
            uart1_response_computer(ar_response_buffer, L_SIX, ar_ble_decrypt_buf);
            printf("response err code!\r\n");
        }
    }
    g_ble_e92_cmd = V_ZERO;
    g_usart1_tx_len = V_ZERO;
    g_usart1_rx_finish = V_ZERO;
    memset(ar_usart1_tx_buffer, V_ZERO, g_usart1_tx_len);
}

void open_multiple_doors_fun(void)
{
    uint8_t i = I_ZERO;
    uint8_t ar_res_buf[L_FORTY_ONE] = {I_ZERO};

    if(g_open_multiple_doors_num <= (MAX_DOOR_NUM - V_ONE))
    {
        i = g_open_multiple_doors_num % V_EIGHT;
        if(((ar_open_slot_num[g_open_multiple_doors_num/V_EIGHT] >> i) & V_ONE) == V_ONE)
        {
            printf("open door num = %d \r\n", g_open_multiple_doors_num+V_ONE);
            door_control_index(g_open_multiple_doors_num, OPEN_DOOR_TIME, OPEN_DOOR);
            g_start_open_door_flag = V_ONE;
            g_start_open_door_cnt = V_ZERO;
            g_open_one_door_flag = V_ZERO;
        }
        else
        {
            g_start_open_door_flag = V_ZERO;
            g_start_open_door_cnt = V_ZERO;
            g_open_one_door_flag = V_ONE;
        }
        g_open_multiple_doors_num++;
    }
    else
    {
        g_open_one_door_flag = V_ZERO;
        g_start_open_door_flag = V_ZERO;
        g_start_open_door_cnt = V_ZERO;
        g_open_multiple_doors_num = V_ZERO;
        ar_res_buf[V_ZERO] = BOARD_NUM;
        ar_res_buf[V_ONE] = g_package_num;
        ar_res_buf[L_TWO] = V_ZERO;
        ar_res_buf[L_THREE] = V_ZERO;
        ar_res_buf[L_FOUR] = DEPOSIT_EXPRESS;
        ar_res_buf[V_FIVE] = BATCH_OPENING;
        ar_res_buf[V_SIX] = L_ONE;
        ar_res_buf[V_SEVEN] = L_ZERO;
        ar_res_buf[V_EIGHT] = L_ZERO;
        memset(ar_ble_decrypt_buf, V_ZERO, AES_D_E_DATA_LEN);
        uart1_response_computer(ar_res_buf, L_ONE+L_EIGHT, ar_ble_decrypt_buf);
        printf("BATCH_OPENING!\r\n");
    }
}

/*********************************************************************
 *	function name:  lis2dh_config_save
 *	describe: 保存震动传感器阈值
 *	input:void
 *	output:void
 *	return:void
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
void lis2dh_config_save(uint8_t value)
{
    lis2dh_config_t.head1 = BLE_HEAD_ONE;
    lis2dh_config_t.head2 = BLE_HEAD_TWO;
    lis2dh_config_t.value = value;
    lis2dh_config_t.crc = xor_check((uint8_t *)&lis2dh_config_t, L_THREE);
    W25Q16_SectorErase_4KByte(FLASH_LIS2DH_THRESHOLD_ADDR);
    W25Q16_PageWrite(FLASH_LIS2DH_THRESHOLD_ADDR, (uint8_t *)&lis2dh_config_t, sizeof(lis2dh_config_t));
}

/*********************************************************************
 *	function name:  ble_e92_set_fun
 *	describe: set ble name、broadcast、transmit power and get ble model
 *	input:void
 *	output:void
 *	return:void
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
void ble_e92_set_fun(void)
{
    if(g_e95_start_init == V_ONE)     /*初始化标志，重启后初始化一次*/
    {
        g_set_step = SET_E95_NAME;
        g_e95_start_init = V_ZERO;;
    }
    
    switch(g_set_step)
    {
        case SET_E95_NAME:    /*设置蓝牙名称,长度在20字节以内*/
        {
            g_send_cmd_cnt++;
            g_set_param_count = V_ZERO;
            if(g_send_cmd_cnt <= BLE_SEND_CMD_MAX_CNT)   /*读取超时次数3次*/
            {
                ar_set_name_params[V_ZERO] = V_ONE;
                ar_set_name_params[V_ONE] = FRAM_SECOND_HEAD;
                ar_set_name_params[V_TWO] = BLE_NAME_SET;     /*名称从flash中读取*/
                for(uint8_t i = V_ZERO; i < (ar_set_name_params[L_THREE]+L_FOUR); i++)
                {
                    printf("%02x\n", ar_set_name_params[i]);
                }
                HAL_UART_Transmit(&hlpuart1, ar_set_name_params, ar_set_name_params[L_THREE]+L_FOUR, 0xfff); /*set e95 name*/
                memset(ar_set_name_params, I_ZERO, L_TWENTY_FOUR);
                g_set_step = V_ZERO;
                g_set_param_timeout_flag = V_ONE;    /*开始计时标志*/
                g_current_step = SET_E95_NAME;     /*记录当前步骤*/
                printf("SET_E95_NAME!");
            }
            else
            {
                g_set_param_timeout_flag = V_ZERO;
                g_set_step = V_ZERO;
                g_send_cmd_cnt = V_ZERO;
                g_current_step = V_ZERO;
                printf("SET NAME FAILED,PLEASE CHECK BLE HARDWARE!\n");
            }
            break;
        }
        case SET_E95_BROADCAST:     /*设置广播间隔，范围20-10240ms*/
        {
            /*set e95 broadcast*/
            g_send_cmd_cnt++;
            g_set_param_count = V_ZERO;
            if(g_send_cmd_cnt <= BLE_SEND_CMD_MAX_CNT)
            {
                ar_set_boardcast_params[V_ZERO] = V_ONE;
                ar_set_boardcast_params[V_ONE] = FRAM_SECOND_HEAD;
                ar_set_boardcast_params[V_TWO] = BLE_BROADCAST_SET;     /*广播参数从flash中读取*/
                ar_set_boardcast_params[V_THREE] = L_TWO;
                for(uint8_t i = V_ZERO; i < (ar_set_boardcast_params[L_THREE]+L_FOUR); i++)
                {
                    printf("%02x\n", ar_set_boardcast_params[i]);
                }
                HAL_UART_Transmit(&hlpuart1, ar_set_boardcast_params, ar_set_boardcast_params[L_THREE]+L_FOUR, 0xfff);
                g_set_step = V_ZERO;
                g_set_param_timeout_flag = V_ONE;     /*超时开始标志*/
                g_current_step = SET_E95_BROADCAST;     /*记录当前步骤*/
                printf("SET_E95_BROADCAST!");
            }
            else
            {
                g_set_param_timeout_flag = V_ZERO;
                g_set_step = V_ZERO;
                g_send_cmd_cnt = V_ZERO;
                g_current_step = V_ZERO;
                printf("SET BROADCAST FAILED,PLEASE CHECK BLE HARDWARE!\n");
            }
            break;
        }
        case SET_E95_POWER:    /*设置发射功率：-40db -20db -16db -12db -8db
                                               -4db 0db 3db 4db*/
        {
            /*set e95 power*/
            g_send_cmd_cnt++;
            g_set_param_count = V_ZERO;
            if(g_send_cmd_cnt <= BLE_SEND_CMD_MAX_CNT)
            {
                ar_set_transmit_power[V_ZERO] = V_ONE;
                ar_set_transmit_power[V_ONE] = FRAM_SECOND_HEAD;
                ar_set_transmit_power[V_TWO] = BLE_POWER_SET;     /*发射功率从flash中读取*/
                ar_set_transmit_power[V_THREE] = L_ONE;
                for(uint8_t i = V_ZERO; i < (ar_set_transmit_power[L_THREE]+L_FOUR); i++)
                {
                    printf("%02x\n", ar_set_transmit_power[i]);
                }
                HAL_UART_Transmit(&hlpuart1, ar_set_transmit_power, ar_set_transmit_power[L_THREE]+L_FOUR, 0xfff);
                g_set_step = V_ZERO;
                g_set_param_timeout_flag = V_ONE;    /*超时开始标志*/
                g_current_step = SET_E95_POWER;    /*记录当前步骤*/
                printf("SET_E95_POWER!");
            }
            else
            {
                g_set_param_timeout_flag = V_ZERO;
                g_set_step = V_ZERO;
                g_send_cmd_cnt = V_ZERO;
                g_current_step = V_ZERO;
                printf("SET POWER FAILED,PLEASE CHECK BLE HARDWARE!\n");
            }
            break;
        }
        case GET_E95_MODEL:
        {
            /*set e95 power*/
            g_send_cmd_cnt++;
            g_set_param_count = V_ZERO;
            if(g_send_cmd_cnt <= BLE_SEND_CMD_MAX_CNT)
            {
                HAL_UART_Transmit(&hlpuart1, ar_e95_get_model, L_FIVE, 0xfff);
                g_set_step = V_ZERO;
                printf("SET_E95_POWER!");
                g_set_param_timeout_flag = V_ONE;    /*超时开始标志*/
                g_current_step = GET_E95_MODEL;     /*记录当前步骤*/
            }
            else
            {
                g_set_param_timeout_flag = V_ZERO;
                g_set_step = V_ZERO;
                g_send_cmd_cnt = V_ZERO;
                g_current_step = V_ZERO;
                printf("GET MODEL FAILED,PLEASE CHECK BLE HARDWARE!\n");
            }
            break;
        }
        #if 0
        default:
        {
            g_set_step = V_ZERO;
            g_send_cmd_cnt = V_ZERO;
            g_current_step = V_ZERO;
            g_set_param_timeout_flag = V_ZERO;
            g_set_param_count = V_ZERO;
            printf("unknown error!");
            break;
        }
        #endif
    }
}
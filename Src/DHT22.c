#include "DHT22.h"
#include "main.h"
#include "gpio.h"
#include "user_gpio.h"
#include "ble_e95.h"

extern UART_HandleTypeDef huart2;
unsigned char  U8RH_data_H = 0,U8RH_data_L = 0;
unsigned char  U8T_data_H  = 0, U8T_data_L  = 0;
char Temperature = 0;
unsigned char Humidity = 0;
uint16_t g_temp = V_ZERO;
uint16_t g_hump = V_ZERO;

void Set_HumPin_Output(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = HUMI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(HUMI_GPIO_Port, &GPIO_InitStruct);
}

void Set_HumPin_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = HUMI_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(HUMI_GPIO_Port, &GPIO_InitStruct);
}

void delay_us(uint32_t i)
{
    uint32_t count;
    for(count = i; count > 0; count--)
    {
        for(uint16_t j = 0; j < 2; j++)
        {
            __NOP();
        }
    }
}
unsigned char ReadDht22_8bit(void)
{
    unsigned char i,u8data,value = 0;
    uint16_t flag;
    for(i=0;i<8;i++)
    { 
        flag = 2;
        Set_HumPin_Input();
        while(!DHT22_Port&&flag++ );
        delay_us(20);
        u8data = 0;
        if(DHT22_Port)
        u8data = 1;
        flag = 2;
        while(DHT22_Port&&flag++ );
        if(flag == 1) 
        break;   
        value <<= 1;
        value |= u8data;
    }
    return value;
}
void read_dht22_RH(void)
{
    unsigned char U8RH_data_H_temp,U8RH_data_L_temp;
    uint16_t flag;
    unsigned char U8T_data_H_temp,U8T_data_L_temp,U8checkdata_temp,U8temp;
    uint16_t cnt;
    uint8_t data4 = 33;

//    SENSOR_POWER_EN;
    HAL_Delay(10);
    Set_HumPin_Output();
    DHT22_High;
    delay_us(10);
    DHT22_Low;
    delay_us(500);
    DHT22_High;
    Set_HumPin_Input();
    cnt = 1;   
    while(DHT22_Port&&cnt++);  
    if(!DHT22_Port)
    {
        flag = 2;
        while(!DHT22_Port&&flag++ );
        flag = 2;
        while(DHT22_Port&&flag++ );
        U8RH_data_H_temp = ReadDht22_8bit();
        U8RH_data_L_temp = ReadDht22_8bit();
        U8T_data_H_temp  = ReadDht22_8bit();
        U8T_data_L_temp  = ReadDht22_8bit();
        U8checkdata_temp = ReadDht22_8bit();
        Set_HumPin_Output();
        //DHT22_High;
        DHT22_Low;
        U8temp = (U8T_data_H_temp+U8T_data_L_temp+U8RH_data_H_temp+U8RH_data_L_temp);
        #if 0
        printf("U8RH_data_H_temp = %d U8RH_data_L_temp = %d U8T_data_H_temp = %d U8T_data_L_temp = %d \r\n",
                U8RH_data_H_temp, U8RH_data_L_temp, U8T_data_H_temp, U8T_data_L_temp);
        printf("U8temp = %d.\r\n", U8temp);
        printf("U8checkdata_temp = %d.\r\n", U8checkdata_temp);
        #endif
        if(U8temp == U8checkdata_temp)
        {
            g_read_dht22_flag = V_ONE;   //读取成功标志位置位        
            U8RH_data_H = U8RH_data_H_temp;
            U8RH_data_L = U8RH_data_L_temp;
            g_hump = (U8RH_data_H_temp << 8) + U8RH_data_L_temp;
            printf("hum :%d\r\n", g_hump);
            if(U8T_data_H_temp&0x80)
            {
                U8T_data_H = 0xFF;
                U8T_data_L = 0xFF - U8T_data_L_temp;
            }
            else 
            {
                U8T_data_H  = U8T_data_H_temp;
                U8T_data_L  = U8T_data_L_temp;
            }
            g_temp = (U8T_data_H_temp << 8) + U8T_data_L_temp;
            printf("temp :%d\r\n", g_temp);
        }    
    }
    else
    {
        U8RH_data_H = 0xFF;
        U8RH_data_L = 0xFF;
        U8T_data_H  = 0xFF;
        U8T_data_L  = 0xFF;
        Set_HumPin_Output();
        //DHT22_High;
        DHT22_Low;
    } 
//    SENSOR_POWER_DIS;
}


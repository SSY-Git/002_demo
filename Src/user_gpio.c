/*
    Copyright 2019 SHENZHEN ZHILAI SCI AND TECH CO., LTD,
    15th floor, C3 building, Nanshan ipark,Xueyuan road 1001,
	Nanshan District, Shenzhen 518052, China.
    All rights are reserved. Reproduction in whole or in part is prohibited
    without the prior written consent of the copyright owner.

    COMPANY CONFIDENTIAL

Filename  :  user_gpio.c


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
        Module description (bottom GPIO pin package)
@end
*/

/* Includes ------------------------------------------------------------------*/
#include "user_gpio.h"


/* USER CODE BEGIN 0 */
uint8_t read_sn74lvc_statua(uint8_t bus_num, uint8_t goods_or_doors);
/*storage door status*/
uint8_t g_stdoor_stat[MAX_DOOR_NUM] = {I_ZERO};
/*storage all doors status, One bit denotes a slot status*/
uint8_t g_door_stat[V_THREE] = {I_ZERO};
/*storage goods status*/
uint8_t g_item_stat[MAX_DOOR_NUM] = {I_ZERO};
/*storage all goods status, One bit denotes a slot status*/
uint8_t g_all_goods_status[V_THREE] = {I_ZERO};
/*open door count*/
volatile uint16_t g_open_luck_time = I_ZERO;

/*lock number*/
volatile uint8_t g_lock_number = I_ZERO;
volatile uint8_t g_lock_open_flag = I_ZERO;

/*0:e95 disconnected;1:e95 connected*/
volatile uint8_t g_ble_connect_flag = BLE_E95_DISCONNECTED;
/*ble connection timeout,no data in 30s*/
volatile uint32_t g_ble_connect_count = I_ZERO;

/*rtc闹钟中断标志位*/
uint8_t g_rtc_alarm_flag = I_ZERO;

/**
  * @brief  The application entry point.
  * @retval int
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == GPIO_PIN_0)     /*双边沿触发*/
    { 
        if(HAL_GPIO_ReadPin(GPIOA, CONN_IND_Pin) == GPIO_PIN_SET) /*E95连接指令引脚*/
        {
            g_ble_connect_flag = BLE_E95_DISCONNECTED;     /*未连接*/
            printf("ble disconnected!\n");
        }
        else    /*e95 connected*/
        {
            g_ble_connect_flag = BLE_E95_CONNECTED;     /*连接*/
            printf("ble connected!\n");
        }
    }
    if(GPIO_Pin == GPIO_PIN_2)     /*震动传感器中断*/
    {
        printf("lis2dh interrupt!\n");
    }
    if(GPIO_Pin == PCF85263_INT_Pin)
    {
        printf("alarm interrupt!\n");
        g_rtc_alarm_flag = V_ONE;
    }
}

/*********************************************************************
 *	function name: ctrl extend in 74hc245
 *	describe:  
 *	input: door_num 0-23; en_or_dis:open or close slot
 *	output:
 *	return:lock_ctrl_return_type_t:	LOCK_CTRL_OK,
 *                                    LOCK_CTRL_FIALED,
                                        LOCK_NUM_ERR
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
lock_ctrl_return_type_t extend_ic_74hc245_ctrl_lock(uint8_t door_num, uint8_t en_or_dis)
{
    /*74HC245的B1、B2、B3、B4、B6、B7用于控制开锁，B0、B5用于控制红外电源
      将格口锁号对应到相应的控制线上*/
    lock_ctrl_return_type_t m_door_num_result = LOCK_CTRL_OK;
    uint8_t m_select_ctrl = I_ZERO;
    if(door_num <= V_FIVE)
    {
        m_select_ctrl = V_ONE;
    }
    else if(door_num <= V_ELEVEN)
    {
        door_num -= V_SIX;
        m_select_ctrl = V_TWO;
    }
    else if(door_num <= V_SEVENTEEN)
    {
        door_num -= V_TWELVE;
        m_select_ctrl = V_ONE;
    }
    else if(door_num <= V_TWENTY_THREE)
    {
        door_num -= V_EIGHTTEEN;
        m_select_ctrl = V_TWO;
    }
    else
    {
        printf("door number error!");
        return LOCK_NUM_ERR;
    }
    if(door_num < EXTEND_IC_CTRL_MAX_LOCK_NUM)     /*一片74HC245控制6个格口锁*/
    {
        if((en_or_dis == HC245_BUS_EN) || (en_or_dis == HC245_BUS_DIS))     /*开锁或者关锁*/
        {
            switch(door_num)     /*锁号*/
            {
                case V_ZERO:
                {
                    if(en_or_dis == HC245_BUS_EN)
                    {
                        if(m_select_ctrl == V_ONE)
                        {
                            CTRL_BUS_A1_HIGH;
                        }
                        else
                        {
                            CTRL_BUS_AO_HIGH;
                        }
                    }
                    else if(en_or_dis == HC245_BUS_DIS)
                    {
                        if(m_select_ctrl == V_ONE)
                        {
                            CTRL_BUS_A1_LOW;
                        }
                        else
                        {
                            CTRL_BUS_AO_LOW;
                        }
                    }
                    else
                    {
                        printf("unknown error!");
                    }
                    break;
                }
                case V_ONE:
                {
                    if(en_or_dis == HC245_BUS_EN)
                    {
                        if(m_select_ctrl == V_ONE)
                        {
                            CTRL_BUS_A2_HIGH;
                        }
                        else
                        {
                            CTRL_BUS_A1_HIGH;
                        }
                    }
                    else if(en_or_dis == HC245_BUS_DIS)
                    {
                        if(m_select_ctrl == V_ONE)
                        {
                            CTRL_BUS_A2_LOW;
                        }
                        else
                        {
                            CTRL_BUS_A1_LOW;
                        }
                    }
                    else
                    {
                        printf("unknown error!");
                    }
                    break;
                }
                case V_TWO:
                {
                    if(en_or_dis == HC245_BUS_EN)
                    {
                        CTRL_BUS_A3_HIGH;
                    }
                    else if(en_or_dis == HC245_BUS_DIS)
                    {
                        CTRL_BUS_A3_LOW;
                    }
                    else
                    {
                        printf("unknown error!");
                    }
                    break;
                }
                case V_THREE:
                {
                    if(en_or_dis == HC245_BUS_EN)
                    {
                        CTRL_BUS_A4_HIGH;
                    }
                    else if(en_or_dis == HC245_BUS_DIS)
                    {
                        CTRL_BUS_A4_LOW;
                    }
                    else
                    {
                        printf("unknown error!");
                    }
                    break;
                }
                case V_FOUR:
                {
                    if(en_or_dis == HC245_BUS_EN)
                    {
                        if(m_select_ctrl == V_ONE)
                        {
                            CTRL_BUS_A6_HIGH;
                        }
                        else
                        {
                            CTRL_BUS_A5_HIGH;
                        }
                    }
                    else if(en_or_dis == HC245_BUS_DIS)
                    {
                        if(m_select_ctrl == V_ONE)
                        {
                            CTRL_BUS_A6_LOW;
                        }
                        else
                        {
                            CTRL_BUS_A5_LOW;
                        }
                    }
                    else
                    {
                        printf("unknown error!");
                    }
                    break;
                }
                case V_FIVE:
                {
                    if(en_or_dis == HC245_BUS_EN)
                    {
                        if(m_select_ctrl == V_ONE)
                        {
                            CTRL_BUS_A7_HIGH;
                        }
                        else
                        {
                            CTRL_BUS_A6_HIGH;
                        }
                    }
                    else if(en_or_dis == HC245_BUS_DIS)
                    {
                        if(m_select_ctrl == V_ONE)
                        {
                            CTRL_BUS_A7_LOW;
                        }
                        else
                        {
                            CTRL_BUS_A6_LOW;
                        }
                    }
                    else
                    {
                        printf("unknown error!");
                    }
                    break;
                }
                default:
                    break;
            }
        }
        else
        {
            printf("bus ctrl error!");
            m_door_num_result = LOCK_CTRL_FIALED;
        }
    }
    else
    {
        printf("lock number error!");
        m_door_num_result = LOCK_NUM_ERR;
    }

    return m_door_num_result;
}



/*********************************************************************
 *	function name: door_control_index
 *	describe:  control the opening and closing of lock
 *	input: inputval:door numbet 0-23；opentime：open lock time,default 500ms；
 *         ctrl_door_action：open or close
 *	output: void
 *	return:lock_ctrl_return_type_t:	LOCK_CTRL_OK,
	                                 LOCK_CTRL_FIALED,
                                       LOCK_NUM_ERR
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
lock_ctrl_return_type_t door_control_index(uint8_t inputval, uint16_t opentime, uint8_t ctrl_door_action)
{
    /*控制开、关锁*/
    uint8_t m_index  = inputval;
    lock_ctrl_return_type_t m_lock_ctrl_result = LOCK_CTRL_OK;
    g_open_luck_time = opentime;
    g_lock_number = inputval;

    if((m_index <  MAX_DOOR_NUM) && (g_open_luck_time != V_ZERO))     /*判断锁号已经开锁时间有效性*/
    {
        if((ctrl_door_action == OPEN_DOOR) || (ctrl_door_action == CLOSE_DOOR))     /*控制锁动有效性：开锁或者关锁*/
        {
            if(ctrl_door_action == OPEN_DOOR)     /*开锁*/
            {
                g_lock_open_flag = START_OPEN_DOOR_FLAG;
                BAT_POWER_EN;    /*先开启锁电源*/
                CTRL_BUS_POWER_EN;    /*开启74HC245芯片电源*/   
                if(m_index <= V_FIVE)     /*0-5号锁，就使能第一片74hc245*/
                {
                    CHIP_SELECT_01EN;    
                }
                else if(m_index <= V_ELEVEN)/*6-11号锁，就使能第二片74hc245*/
                {
                    CHIP_SELECT_02EN;
                    m_index -= V_SIX;     /*因为每片74hc245只控6把锁，所以需要把格口号转换成74hc245对应的线号控制，每片的线号在6以内*/
                    printf("m_index = %d\r\n", m_index);
                }
                else if(m_index <= V_SEVENTEEN)/*12-17号锁，就使能第三片74hc245*/
                {
                    CHIP_SELECT_03EN;
                    m_index -= V_TWELVE;     /*因为每片74hc245只控6把锁，所以需要把格口号转换成74hc245对应的线号控制，每片的线号在6以内*/
                }
                else if(m_index <= V_TWENTY_THREE)/*18-23号锁，就使能第四片74hc245*/
                {
                    CHIP_SELECT_04EN;
                    m_index -= V_EIGHTTEEN;    /*因为每片74hc245只控6把锁，所以需要把格口号转换成74hc245对应的线号控制，每片的线号在6以内*/
                }
                else
                {
                    printf("open door num err!");
                    return LOCK_NUM_ERR;
                }
                HAL_Delay(5); /*开启电源以及使能芯片后延时5ms等待稳定，然后再控制相应格口开关*/
                extend_ic_74hc245_ctrl_lock(inputval, HC245_BUS_EN);
            }
            if(ctrl_door_action == CLOSE_DOOR)     /*关锁*/
            {
                extend_ic_74hc245_ctrl_lock(inputval, HC245_BUS_DIS);     /*控制关锁*/
                if(m_index <= V_FIVE)    /*0-5号锁，就失能第一片74hc245*/
                {
                    CHIP_SELECT_01DIS;
                }
                else if(m_index <= V_ELEVEN)     /*6-11号锁，就失能第二片74hc245*/
                {
                    CHIP_SELECT_02DIS;
                    m_index -= V_FIVE;
                }
                else if(m_index <= V_SEVENTEEN)     /*12-17号锁，就失能第三片74hc245*/
                {
                    CHIP_SELECT_03DIS;
                    m_index -= V_ELEVEN;
                }
                else if(m_index <= V_TWENTY_THREE)     /*18-23号锁，就失能第四片74hc245*/
                {
                    CHIP_SELECT_04DIS;
                    m_index -= V_SEVENTEEN;
                }
                else
                {
                    printf("close door num err!");
                    BAT_POWER_DIS;
                    CTRL_BUS_POWER_DIS;
                    return LOCK_NUM_ERR;
                }
                BAT_POWER_DIS;     /*关闭锁电源*/
                CTRL_BUS_POWER_DIS;     /*关闭74HC245芯片电源*/
            }
        }
        else
        {
            m_lock_ctrl_result = LOCK_CTRL_FIALED;
            printf("door action err!");
        }
    }
    else
    {
        m_lock_ctrl_result = LOCK_NUM_ERR;
        printf("door num err!");
    }

    return m_lock_ctrl_result;
}

/*********************************************************************
 *	function name:read doors status
 *	describe:voltage low means door is open,
 *            voltage high means door is close
 *	input:index：slot number :0-23
 *	output:void
 *	return:void
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
void door_status_index(uint8_t index)
{
    /*获取单个门磁状态*/
    POWER_5V_EN;     /*开启SN74LV片子的5V电源*/
    GOODS_POWER_3V3_EN;     /*开启SN74LV片子的3.3V电源*/
    /*一共6片sn74lv片子分别控制24路门磁检测、24路物品检测，
      每片的线1、3、5、7为读取物品状态；0、2、4、6为读取门磁状态*/
    if(index <= V_THREE)     /*0-3号格口就使能第一片sn74*/
    {
        SN74_A_EN;  /*enable 74LVC4245*/
        HAL_Delay(50);/*延时50ms等待电源稳定后再读取状态*/
        g_stdoor_stat[index] = !read_sn74lvc_statua(index, READ_DOORS_STATUS);     /*实际状态与协议状态相反，所有读取的状态进行一次取反*/
        HAL_Delay(10);/*根据上一版本蓝牙柜的调试经验，读完状态后需要延时一段时间，否则读取的状态不正确*/
        SN74_A_DIS;  /*disable 74LVC4245*/
    }
    else if(index <= V_SEVEN)    /*4-7号格口就使能第二片sn74*/
    {
        SN74_B_EN;  /*enable 74LVC4245*/
        HAL_Delay(50);/*延时50ms等待电源稳定后再读取状态*/
        g_stdoor_stat[index] = !read_sn74lvc_statua(index-V_FOUR, READ_DOORS_STATUS);    /*实际状态与协议状态相反，所有读取的状态进行一次取反*/
        HAL_Delay(10);/*根据上一版本蓝牙柜的调试经验，读完状态后需要延时一段时间，否则读取的状态不正确*/
        SN74_B_DIS;  /*disable 74LVC4245*/
    }
    else if(index <= V_ELEVEN)    /*8-11号格口就使能第三片sn74*/
    {
        SN74_C_EN;  /*enable 74LVC4245*/
        HAL_Delay(50);/*延时50ms等待电源稳定后再读取状态*/
        g_stdoor_stat[index] = !read_sn74lvc_statua(index-V_EIGHT, READ_DOORS_STATUS);    /*实际状态与协议状态相反，所有读取的状态进行一次取反*/
        HAL_Delay(10);/*根据上一版本蓝牙柜的调试经验，读完状态后需要延时一段时间，否则读取的状态不正确*/
        SN74_C_DIS;  /*disable 74LVC4245*/
    }
    else if(index <= V_FIFTEEN)    /*12-15号格口就使能第四片sn74*/
    {
        SN74_D_EN;  /*enable 74LVC4245*/
        HAL_Delay(50);/*延时50ms等待电源稳定后再读取状态*/
        g_stdoor_stat[index] = !read_sn74lvc_statua(index-V_TWELVE, READ_DOORS_STATUS);   /*实际状态与协议状态相反，所有读取的状态进行一次取反*/
        HAL_Delay(10);/*根据上一版本蓝牙柜的调试经验，读完状态后需要延时一段时间，否则读取的状态不正确*/
        SN74_D_DIS;  /*disable 74LVC4245*/
    }
    else if(index <= V_NINETEEN)    /*16-19号格口就使能第五片sn74*/
    {
        SN74_E_EN;  /*enable 74LVC4245*/
        HAL_Delay(50);/*延时50ms等待电源稳定后再读取状态*/
        g_stdoor_stat[index] = !read_sn74lvc_statua(index-V_SIXTEEN, READ_DOORS_STATUS);    /*实际状态与协议状态相反，所有读取的状态进行一次取反*/
        HAL_Delay(10);/*根据上一版本蓝牙柜的调试经验，读完状态后需要延时一段时间，否则读取的状态不正确*/
        SN74_E_DIS;  /*disable 74LVC4245*/
    }
    else if(index <= V_TWENTY_THREE)    /*20-23号格口就使能第六片sn74*/
    {
        SN74_F_EN;  /*enable 74LVC4245*/
        HAL_Delay(50);/*延时50ms等待电源稳定后再读取状态*/
        g_stdoor_stat[index] = !read_sn74lvc_statua(index-V_TWENTY, READ_DOORS_STATUS);    /*实际状态与协议状态相反，所有读取的状态进行一次取反*/
        HAL_Delay(10);/*根据上一版本蓝牙柜的调试经验，读完状态后需要延时一段时间，否则读取的状态不正确*/
        SN74_F_DIS;  /*disable 74LVC4245*/
    }
    else
    {
        printf("door num  err!");
    }

    POWER_5V_DIS;    /*关闭5V电源*/
    GOODS_POWER_3V3_DIS;    /*关闭3.3V电源*/
}

/*********************************************************************
 *	function name:read sn74lvc bus status
 *	describe:read sn74 chip bus  status
 *	input:bus_num：1-4;goods_or_doors:goods status or doors status;
 *                   1、3、5、7 bus read doors status,0、2、4、6 bus read goods status
 *	output:void
 *	return:uint8_t 1 or 0 or 0xff，0xff:mean failed
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
uint8_t read_sn74lvc_statua(uint8_t bus_num, uint8_t goods_or_doors)
{
    /*读取sn74芯片的线状态*/
    uint8_t m_read_bus_satus = I_ZERO;
    if((goods_or_doors == READ_DOORS_STATUS) || (goods_or_doors == READ_GOODS_STATUS))    /*判断是读取门磁还是物品状态*/
    {
        if(goods_or_doors == READ_DOORS_STATUS)    /*每一片sn74的1、3、5、7线为物品状态*/
        {
            if(bus_num == 0)
            {
                m_read_bus_satus = GOODS_A0_STATUS;
            }
            else if(bus_num == 1)
            {
                m_read_bus_satus = GOODS_A2_STATUS;
            }
            else if(bus_num == 2)
            {
                m_read_bus_satus = GOODS_A4_STATUS;
            }
            else if(bus_num == 3)
            {
                m_read_bus_satus = GOODS_A6_STATUS;
            }
            else
            {
                m_read_bus_satus = UINT8_MAX;     /*bus number error!*/
            }
        }
        if(goods_or_doors == READ_GOODS_STATUS)    /*读取门磁状态*/
        {
            if(bus_num == 0)    /*每一片的0、2、4、6线为物品状态*/
            {
                m_read_bus_satus = GOODS_A1_STATUS;
            }
            else if(bus_num == 1)
            {
                m_read_bus_satus = GOODS_A3_STATUS;
            }
            else if(bus_num == 2)
            {
                m_read_bus_satus = GOODS_A5_STATUS;
            }
            else if(bus_num == 3)
            {
                m_read_bus_satus = GOODS_A7_STATUS;
            }
            else
            {
                m_read_bus_satus = UINT8_MAX;     /*bus number error!*/
            }
        }
    }
    else
    {
        m_read_bus_satus = UINT8_MAX;    /*返回状态为0xff时说明读取失败*/
    }

    return m_read_bus_satus;
}

/*********************************************************************
 *	function name:door_status_refresh
 *	describe:read all doors status
 *	input:void
 *	output:void
 *	return:void
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
void door_status_refresh(void)
{
    uint8_t door_stat[MAX_DOOR_NUM] = {I_ZERO};
    /*读取所有物品状态，先开电源，然后轮流使能sn74读取物品状态*/
    POWER_5V_EN;    /*使能5V电源*/
    GOODS_POWER_3V3_EN;    /*使能3.3V电源*/
    SN74_A_EN;  /*enable 74LVC4245*/
    HAL_Delay(50);/*50ms*/
    /*读1-4路格口门磁状态*/
    door_stat[V_ZERO] = !read_sn74lvc_statua(V_ZERO, READ_DOORS_STATUS);
    door_stat[V_ONE] = !read_sn74lvc_statua(V_ONE, READ_DOORS_STATUS);
    door_stat[V_TWO] = !read_sn74lvc_statua(V_TWO, READ_DOORS_STATUS);
    door_stat[V_THREE] = !read_sn74lvc_statua(V_THREE, READ_DOORS_STATUS);
    HAL_Delay(10);/*10ms*/
    SN74_A_DIS;  /*disable 74LVC4245*/
    SN74_B_EN;  /*enable 74LVC4245*/
    HAL_Delay(50);/*50ms*/
    /*读5-8路格口门磁状态*/
    door_stat[V_ZERO+V_FOUR] = !read_sn74lvc_statua(V_ZERO, READ_DOORS_STATUS);
    door_stat[V_ONE+V_FOUR] = !read_sn74lvc_statua(V_ONE, READ_DOORS_STATUS);
    door_stat[V_TWO+V_FOUR] = !read_sn74lvc_statua(V_TWO, READ_DOORS_STATUS);
    door_stat[V_THREE+V_FOUR] = !read_sn74lvc_statua(V_THREE, READ_DOORS_STATUS);
    HAL_Delay(10);/*10ms*/
    SN74_B_DIS;  /*disable 74LVC4245*/
    SN74_C_EN;  /*enable 74LVC4245*/
    HAL_Delay(50);/*50ms*/
    /*读9-12路格口门磁状态*/
    door_stat[V_ZERO+V_EIGHT] = !read_sn74lvc_statua(V_ZERO, READ_DOORS_STATUS);
    door_stat[V_ONE+V_EIGHT] = !read_sn74lvc_statua(V_ONE, READ_DOORS_STATUS);
    door_stat[V_TWO+V_EIGHT] = !read_sn74lvc_statua(V_TWO, READ_DOORS_STATUS);
    door_stat[V_THREE+V_EIGHT] = !read_sn74lvc_statua(V_THREE, READ_DOORS_STATUS);
    HAL_Delay(10);/*10ms*/
    SN74_C_DIS;  /*disable 74LVC4245*/
    SN74_D_EN;  /*enable 74LVC4245*/
    HAL_Delay(50);/*50ms*/
    /*读13-16路格口门磁状态*/
    door_stat[V_ZERO+V_TWELVE] = !read_sn74lvc_statua(V_ZERO, READ_DOORS_STATUS);
    door_stat[V_ONE+V_TWELVE] = !read_sn74lvc_statua(V_ONE, READ_DOORS_STATUS);
    door_stat[V_TWO+V_TWELVE] = !read_sn74lvc_statua(V_TWO, READ_DOORS_STATUS);
    door_stat[V_THREE+V_TWELVE] = !read_sn74lvc_statua(V_THREE, READ_DOORS_STATUS);
    HAL_Delay(10);/*10ms*/
    SN74_D_DIS;  /*disable 74LVC4245*/
    SN74_E_EN;  /*enable 74LVC4245*/
    HAL_Delay(50);/*50ms*/
    /*读17-20路格口门磁状态*/
    door_stat[V_ZERO+V_SIXTEEN] = !read_sn74lvc_statua(V_ZERO, READ_DOORS_STATUS);
    door_stat[V_ONE+V_SIXTEEN] = !read_sn74lvc_statua(V_ONE, READ_DOORS_STATUS);
    door_stat[V_TWO+V_SIXTEEN] = !read_sn74lvc_statua(V_TWO, READ_DOORS_STATUS);
    door_stat[V_THREE+V_SIXTEEN] = !read_sn74lvc_statua(V_THREE, READ_DOORS_STATUS);
    HAL_Delay(10);/*10ms*/
    SN74_E_DIS;  /*disable 74LVC4245*/
    SN74_F_EN;  /*enable 74LVC4245*/
    HAL_Delay(50);/*50ms*/
    /*读21-24路格口门磁状态*/
    door_stat[V_ZERO+V_TWENTY] = !read_sn74lvc_statua(V_ZERO, READ_DOORS_STATUS);
    door_stat[V_ONE+V_TWENTY] = !read_sn74lvc_statua(V_ONE, READ_DOORS_STATUS);
    door_stat[V_TWO+V_TWENTY] = !read_sn74lvc_statua(V_TWO, READ_DOORS_STATUS);
    door_stat[V_THREE+V_TWENTY] = !read_sn74lvc_statua(V_THREE, READ_DOORS_STATUS);
    HAL_Delay(10);/*10ms*/
    SN74_F_DIS;  /*disable 74LVC4245*/
    POWER_5V_DIS;    /*关5V电源*/
    GOODS_POWER_3V3_DIS;    /*关3.3v电源*/
    /*将24路格口状态转换成一个bit表示一个格口状态*/
    g_door_stat[V_ZERO] = door_stat[V_ZERO]|(door_stat[V_ONE]<<V_ONE)|(door_stat[V_TWO]<<V_TWO)|(door_stat[V_THREE]<<V_THREE)|\
                (door_stat[V_FOUR]<<V_FOUR)|(door_stat[V_FIVE]<<V_FIVE)|(door_stat[V_SIX]<<V_SIX)|(door_stat[V_SEVEN]<<V_SEVEN);
    g_door_stat[V_ONE] = door_stat[V_ZERO+V_EIGHT]|(door_stat[V_ONE+V_EIGHT]<<V_ONE)|(door_stat[V_TWO+V_EIGHT]<<V_TWO)|(door_stat[V_THREE+V_EIGHT]<<V_THREE)|\
                (door_stat[V_FOUR+V_EIGHT]<<V_FOUR)|(door_stat[V_FIVE+V_EIGHT]<<V_FIVE)|(door_stat[V_SIX+V_EIGHT]<<V_SIX)|(door_stat[V_SEVEN+V_EIGHT]<<V_SEVEN);
    g_door_stat[V_TWO] = door_stat[V_ZERO+V_SIXTEEN]|(door_stat[V_ONE+V_SIXTEEN]<<V_ONE)|(door_stat[V_TWO+V_SIXTEEN]<<V_TWO)|(door_stat[V_THREE+V_SIXTEEN]<<V_THREE)|\
                (door_stat[V_FOUR+V_SIXTEEN]<<V_FOUR)|(door_stat[V_FIVE+V_SIXTEEN]<<V_FIVE)|(door_stat[V_SIX+V_SIXTEEN]<<V_SIX)|(door_stat[V_SEVEN+V_SIXTEEN]<<V_SEVEN);
}


/*********************************************************************
 *	function name:  get_one_good_state_fun
 *	describe: read one slot goods status
 *	input:index：solt number:0-23
 *	output:
 *	return:null
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
void get_one_good_state_fun(uint8_t index)
{
    /*74HC控制B0、B5分别控制着4路红外板电源，红外状态读取步骤
      1.使能红外电源；2.开启74HC电源也即开启红外板电源；
        3.开启sn74电源；4.读取物品状态*/
    INFRARED_POWER_EN;        /*使能红外电源*/
    CTRL_BUS_POWER_EN;    /*开启74HC电源*/
    POWER_5V_EN;    /*开启sn74片子的5V电源*/
    GOODS_POWER_3V3_EN;    /*开启sn74片子的3.3V电源*/
    if(index <= V_THREE)    /*0-3号格口就使能第一片sn74*/
    {
        CHIP_SELECT_01EN;    /*使能第一片74HC电源*/
        HAL_Delay(5);
        CTRL_BUS_AO_HIGH;    /*线A0控制着0-3路红外电源*/
        SN74_A_EN;    /*使能sn745*/
        HAL_Delay(60);
        g_item_stat[index] = read_sn74lvc_statua(index, READ_GOODS_STATUS);    /*读取红外状态*/
        SN74_A_DIS;    /*失能sn74*/
        CTRL_BUS_AO_LOW;    /*关闭0-3路红外电源*/
        CHIP_SELECT_01DIS;    /*失能第一片74hc*/
    }
    else if(index <= V_SEVEN)    /*4-7号格口就使能第二片sn74*/
    {
        CHIP_SELECT_01EN;    /*使能第一片74HC电源*/
        HAL_Delay(5);
        CTRL_BUS_A5_HIGH;    /*线A5控制着4-7路红外电源*/
        SN74_B_EN;    /*使能sn74*/
        HAL_Delay(60); 
        g_item_stat[index] = read_sn74lvc_statua(index-V_FOUR, READ_GOODS_STATUS);     /*读取红外状态*/
        SN74_B_DIS;    /*失能sn74*/
        CTRL_BUS_A5_LOW;    /*关闭4-7路红外电源*/
        CHIP_SELECT_01DIS;    /*失能第一片74hc*/
    }
    else if(index <= V_ELEVEN)    /*8-11号格口就使能第三片sn74*/
    {
        CHIP_SELECT_02EN;    /*使能第二片74HC电源*/
        HAL_Delay(5);
        CTRL_BUS_A2_HIGH;    /*线A0控制着8-11路红外电源*/
        SN74_C_EN;  /*使能sn74*/
        HAL_Delay(60);
        g_item_stat[index] = read_sn74lvc_statua(index-V_EIGHT, READ_GOODS_STATUS);    /*读取红外状态*/
        SN74_C_DIS;    /*失能sn74*/
        CTRL_BUS_A2_LOW;    /*关闭8-11路红外电源*/
        CHIP_SELECT_02DIS;    /*失能第二片74hc*/

    }
    else if(index <= V_FIFTEEN)    /*12-15号格口就使能第四片sn74*/
    {
        CHIP_SELECT_03EN;    /*使能第二片74HC电源*/
        HAL_Delay(5);
        CTRL_BUS_AO_HIGH;    /*线A5控制着12-15路红外电源*/
        SN74_D_EN;  /*使能sn74*/
        HAL_Delay(60);
        g_item_stat[index] = read_sn74lvc_statua(index-V_TWELVE, READ_GOODS_STATUS);    /*读取红外状态*/
        SN74_D_DIS;    /*失能sn74*/
        CTRL_BUS_AO_LOW;    /*关闭12-15路红外电源*/
        CHIP_SELECT_03DIS;    /*失能第二片74hc*/
    }
    else if(index <= V_NINETEEN)     /*16-19号格口就使能第五片sn74*/
    {
        CHIP_SELECT_03EN;    /*使能第三片74HC电源*/
        HAL_Delay(5);
        CTRL_BUS_A5_HIGH;    /*线A0控制着16-19路红外电源*/
        SN74_E_EN;  /*使能sn74*/
        HAL_Delay(60);
        g_item_stat[index] = read_sn74lvc_statua(index-V_SIXTEEN, READ_GOODS_STATUS);    /*读取红外状态*/
        SN74_E_DIS;     /*失能sn74*/
        CTRL_BUS_A5_LOW;    /*关闭16-19路红外电源*/
        CHIP_SELECT_03DIS;    /*失能第三片74hc*/
    }
    else if(index <= V_TWENTY_THREE)    /*20-23号格口就使能第六片sn74*/
    {
        CHIP_SELECT_04EN;    /*使能第三片74HC电源*/
        HAL_Delay(5);
        CTRL_BUS_A2_HIGH;    /*线A5控制着20-23路红外电源*/
        SN74_F_EN;  /*使能sn74*/
        HAL_Delay(60);
        g_item_stat[index] = read_sn74lvc_statua(index-V_TWENTY, READ_GOODS_STATUS);    /*读取红外状态*/
        SN74_F_DIS;    /*失能sn74*/
        CTRL_BUS_A2_LOW;    /*关闭20-23路红外电源*/
        CHIP_SELECT_04DIS;    /*失能第三片74hc*/
    }
    else
    {
        printf("get one goods status err!");
    }
    INFRARED_POWER_DIS;    /*失能红外电源*/
    CTRL_BUS_POWER_DIS;    /*关闭74hc电源*/
    POWER_5V_DIS;    /*关闭5V电源*/
    GOODS_POWER_3V3_DIS;    /*关闭3.3V电源*/
}

/*********************************************************************
 *	function name:  get_all_goods_state_fun
 *	describe: get all goods status
 *	input:
 *	output:
 *	return:null
 *	revision histroy :   1.
 *                      2.
 **********************************************************************/
void get_all_goods_state_fun(void)
{
    /*获取所有物品状态，先开使能红外电源、74HC电源也即红外电源、
      开启sn74电源，然后轮流使能75hc、sn74分别读取物品状态*/
    memset(g_all_goods_status, I_ZERO, L_THREE);
    INFRARED_POWER_EN;        /*使能红外电源*/
    CTRL_BUS_POWER_EN;    /*开启74hc电源*/
    POWER_5V_EN;    /*开启5v电源*/
    GOODS_POWER_3V3_EN;    /*开启3.3v电源*/
    
    CHIP_SELECT_01EN;    /*使能第一片74HC,读取0-7路物品状态*/
    CTRL_BUS_AO_HIGH;    /*开启0-3路红外电源*/
    SN74_A_EN;  /*使能第一片SN74*/
    HAL_Delay(60);
    g_item_stat[V_ZERO] = read_sn74lvc_statua(V_ZERO, READ_GOODS_STATUS);    /*读取0-3路物品状态*/
    g_item_stat[V_ONE] = read_sn74lvc_statua(V_ONE, READ_GOODS_STATUS);
    g_item_stat[V_TWO] = read_sn74lvc_statua(V_TWO, READ_GOODS_STATUS);
    g_item_stat[V_THREE] = read_sn74lvc_statua(V_THREE, READ_GOODS_STATUS);
    SN74_A_DIS;    /*失能第一片SN74*/
    CTRL_BUS_AO_LOW;    /*关闭0-3路红外电源*/
    //CHIP_SELECT_01DIS;    
    //CHIP_SELECT_01EN;
    //HAL_Delay(5);
    CTRL_BUS_A5_HIGH;    /*开启4-7路红外电源*/
    SN74_B_EN;  /*使能第二片SN74*/
    HAL_Delay(60);
    g_item_stat[V_ZERO+V_FOUR] = read_sn74lvc_statua(V_ZERO, READ_GOODS_STATUS);    /*读取4-7路物品状态*/
    g_item_stat[V_ONE+V_FOUR] = read_sn74lvc_statua(V_ONE, READ_GOODS_STATUS);
    g_item_stat[V_TWO+V_FOUR] = read_sn74lvc_statua(V_TWO, READ_GOODS_STATUS);
    g_item_stat[V_THREE+V_FOUR] = read_sn74lvc_statua(V_THREE, READ_GOODS_STATUS);
    SN74_B_DIS;    /*失能第二片SN74*/
    CTRL_BUS_A5_LOW;    /*关闭4-7路红外电源*/
    CHIP_SELECT_01DIS;    /*失能第一片74HC*/


    CHIP_SELECT_02EN;    /*使能第二片74HC,读取8-15路物品状态*/
    CTRL_BUS_A2_HIGH;    /*开启8-11路红外电源*/
    SN74_C_EN;    /*使能第三片SN74*/
    HAL_Delay(60);
    g_item_stat[V_ZERO+V_EIGHT] = read_sn74lvc_statua(V_ZERO, READ_GOODS_STATUS);    /*读取8-11路物品状态*/
    g_item_stat[V_ONE+V_EIGHT] = read_sn74lvc_statua(V_ONE, READ_GOODS_STATUS);
    g_item_stat[V_TWO+V_EIGHT] = read_sn74lvc_statua(V_TWO, READ_GOODS_STATUS);
    g_item_stat[V_THREE+V_EIGHT] = read_sn74lvc_statua(V_THREE, READ_GOODS_STATUS);
    SN74_C_DIS;    /*失能第三片SN74*/
    CTRL_BUS_A2_LOW;    /*关闭8-11路红外电源*/
    CHIP_SELECT_02DIS;
    CHIP_SELECT_03EN;
    CTRL_BUS_AO_HIGH;    /*开启12-15路红外电源*/
    SN74_D_EN;  /*使能第四片SN74*/
    HAL_Delay(60);
    g_item_stat[V_ZERO+V_TWELVE] = read_sn74lvc_statua(V_ZERO, READ_GOODS_STATUS);    /*读取12-15路物品状态*/
    g_item_stat[V_ONE+V_TWELVE] = read_sn74lvc_statua(V_ONE, READ_GOODS_STATUS);
    g_item_stat[V_TWO+V_TWELVE] = read_sn74lvc_statua(V_TWO, READ_GOODS_STATUS);
    g_item_stat[V_THREE+V_TWELVE] = read_sn74lvc_statua(V_THREE, READ_GOODS_STATUS);
    SN74_D_DIS;    /*失能第四片SN74*/
    CTRL_BUS_AO_LOW;    /*关闭12-15路红外电源*/
    CHIP_SELECT_03DIS;    /*失能第二片74HC*/

    CHIP_SELECT_03EN;    /*使能第三片74HC,读取16-23路物品状态*/
    CTRL_BUS_A5_HIGH;    /*开启16-19路红外电源*/
    SN74_E_EN;  /*使能第五片SN74*/
    HAL_Delay(60);
    g_item_stat[V_ZERO+V_SIXTEEN] = read_sn74lvc_statua(V_ZERO, READ_GOODS_STATUS);    /*读取16-19路物品状态*/
    g_item_stat[V_ONE+V_SIXTEEN] = read_sn74lvc_statua(V_ONE, READ_GOODS_STATUS);
    g_item_stat[V_TWO+V_SIXTEEN] = read_sn74lvc_statua(V_TWO, READ_GOODS_STATUS);
    g_item_stat[V_THREE+V_SIXTEEN] = read_sn74lvc_statua(V_THREE, READ_GOODS_STATUS);
    SN74_E_DIS;    /*失能第五片SN74*/
    CTRL_BUS_A5_LOW;    /*关闭16-19路红外电源*/
    CHIP_SELECT_03DIS;
    CHIP_SELECT_04EN;
    CTRL_BUS_A2_HIGH;    /*开启20-23路红外电源*/
    SN74_F_EN;  /*使能第六片SN74*/
    HAL_Delay(60);
    g_item_stat[V_ZERO+V_TWENTY] = read_sn74lvc_statua(V_ZERO, READ_GOODS_STATUS);    /*读取20-23路物品状态*/
    g_item_stat[V_ONE+V_TWENTY] = read_sn74lvc_statua(V_ONE, READ_GOODS_STATUS);
    g_item_stat[V_TWO+V_TWENTY] = read_sn74lvc_statua(V_TWO, READ_GOODS_STATUS);
    g_item_stat[V_THREE+V_TWENTY] = read_sn74lvc_statua(V_THREE, READ_GOODS_STATUS);
    SN74_F_DIS;    /*失能第六片SN74*/
    CTRL_BUS_A2_LOW;    /*关闭20-23路红外电源*/
    CHIP_SELECT_04DIS;    /*失能第三片74HC*/


    INFRARED_POWER_DIS;    /*失能红外电源*/
    CTRL_BUS_POWER_DIS;    /*关闭74HC电源*/
    POWER_5V_DIS;    /*关闭5V电源*/
    GOODS_POWER_3V3_DIS;    /*关闭3.3V电源*/
    /*用1bit表示一个格口物品状态*/
    g_all_goods_status[V_ZERO] = g_item_stat[V_ZERO]|(g_item_stat[V_ONE]<<V_ONE)|(g_item_stat[V_TWO]<<V_TWO)|(g_item_stat[V_THREE]<<V_THREE)|\
                (g_item_stat[V_FOUR]<<V_FOUR)|(g_item_stat[V_FIVE]<<V_FIVE)|(g_item_stat[V_SIX]<<V_SIX)|(g_item_stat[V_SEVEN]<<V_SEVEN);
    g_all_goods_status[V_ONE] = g_item_stat[V_ZERO+V_EIGHT]|(g_item_stat[V_ONE+V_EIGHT]<<V_ONE)|(g_item_stat[V_TWO+V_EIGHT]<<V_TWO)|(g_item_stat[V_THREE+V_EIGHT]<<V_THREE)|\
                (g_item_stat[V_FOUR+V_EIGHT]<<V_FOUR)|(g_item_stat[V_FIVE+V_EIGHT]<<V_FIVE)|(g_item_stat[V_SIX+V_EIGHT]<<V_SIX)|(g_item_stat[V_SEVEN+V_EIGHT]<<V_SEVEN);
    g_all_goods_status[V_TWO] = g_item_stat[V_ZERO+V_SIXTEEN]|(g_item_stat[V_ONE+V_SIXTEEN]<<V_ONE)|(g_item_stat[V_TWO+V_SIXTEEN]<<V_TWO)|(g_item_stat[V_THREE+V_SIXTEEN]<<V_THREE)|\
                (g_item_stat[V_FOUR+V_SIXTEEN]<<V_FOUR)|(g_item_stat[V_FIVE+V_SIXTEEN]<<V_FIVE)|(g_item_stat[V_SIX+V_SIXTEEN]<<V_SIX)|(g_item_stat[V_SEVEN+V_SIXTEEN]<<V_SEVEN);
}

/* USER CODE END 0 */



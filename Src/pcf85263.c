#include "pcf85263.h"
#include "global.h"
#include "i2c.h"
extern I2C_HandleTypeDef hi2c3;

/***************************************************************
* brief  Conversion from hexadecimal to BCD
* param  BINValue    Hexadecimal value to be converted
* return Converted BCD code
* note   2020.10.16
* Sample tao
**************************************************************/
static unsigned char bin_to_bcd(unsigned char BINValue)
{
    unsigned char bcdhigh = I_ZERO;

    while(BINValue >= V_TEN)
    {
        bcdhigh++;
        BINValue -= V_TEN;
    }

    return ((unsigned char)(bcdhigh << V_FOUR) | BINValue);
}

/***************************************************************
* brief  Conversion from BCD code to hexadecimal code
* param  BINValue    BCD code to be converted
* return hexadecimal number after conversion
* note   2017.10.26
* Sample tao
**************************************************************/
static unsigned char bcd_to_bin(unsigned char BCDValue)
{
    unsigned char tmp = I_ZERO;

    tmp = ((unsigned char)(BCDValue & (unsigned char)HIGH_FOUR_BITS_ENABLE) >> (unsigned char)V_FOUR) * V_TEN;
    return (tmp + (BCDValue & (unsigned char)LOW_FOUR_BITS_ENABLE));
}

/***************************************************************
* @brief  To specify length data to PCF8563
* @param
*       add     Write address
*       write_buf Write data
*       len     data lenth
* @return ：0-> success          1-> fail
**************************************************************/

uint8_t pcf85263_write_data(uint8_t addr, uint8_t *write_buf, uint8_t len)
{
    uint8_t ret = I_ZERO;
    //MX_I2C3_Init();
    //i2c3_gpio_init();
    //HAL_Delay(5);
	if(HAL_OK == HAL_I2C_Mem_Write(&hi2c3, PCF85263_WRITE_ADDR, addr, I2C_MEMADD_SIZE_8BIT, write_buf, len, 1000))
    {
        ret = V_ZERO;
        //SEGGER_RTT_printf(0, "pcf85263 write data success!\n");
    }
    else
    {
        ret = V_ONE;
        //SEGGER_RTT_printf(0, "pcf85263 write data failed!\n");
    }
    //HAL_Delay(5);
    //HAL_I2C_MspDeInit(I2C3);
    //set_gpio_mode_input_up(GPIOC, GPIO_PIN_0);
    //set_gpio_mode_input_up(GPIOC, GPIO_PIN_1);
    return ret;
}


/***************************************************************
*@brief  Read the specified length data from PCF8563
*@param
*       add      read addr 
*       read_buf read data 
*       len      data lenth 
*@return ：0-> success          1-> fail
**************************************************************/
uint8_t pcf85263_read_data(uint8_t addr, uint8_t *read_buf, uint8_t len)
{
    uint8_t ret = I_ZERO;
    //MX_I2C3_Init();
    //i2c3_gpio_init();
    //HAL_Delay(5);
	if(HAL_OK == HAL_I2C_Mem_Read(&hi2c3, PCF85263_READ_ADDR, addr, I2C_MEMADD_SIZE_8BIT, read_buf, len, 1000))
    {
        //SEGGER_RTT_printf(0, "pcf85263 read data success!\n");
        ret = V_ZERO;
    }
    else
    {
        ret = V_ONE;
        //SEGGER_RTT_printf(0, "pcf85263 read data failed!\n");
    }
    //HAL_Delay(5);
    //HAL_I2C_MspDeInit(I2C3);
    //set_gpio_mode_input_up(GPIOC, GPIO_PIN_0);
    //set_gpio_mode_input_up(GPIOC, GPIO_PIN_1);
    return ret;
}

/***************************************************************
*@brief  This api is the underlying driver interface to get Date and Time from the related register in external rtc chip 
*@param[out]  
*           pcf8563_query_timer
* @return
*            RTC_OP_SUCCESS,                
*            RTC_OP_COMMUNICATION_FAILURE,  
*            RTC_OP_INVALID_DATE_TIME       
**************************************************************/
rtc_op_return_type_t pcf85263_query_time(RTC_COUNT_TIME_T *count_time)
{
    uint8_t m_rtc_mode_read = 0;
    uint8_t ret = I_ZERO;
    uint8_t ram_flag = I_ZERO;
    uint8_t PCF8563_Read_buf[V_SIX] = {I_ZERO, I_ZERO, I_ZERO, I_ZERO,\
                                             I_ZERO, I_ZERO};
    // uint8_t m_rtc_mode_bit = SW_MODE;   
    // ret = pcf85263_write_data(PCF8563_ADDR_FUN, &m_rtc_mode_bit, L_ONE);    /*rtc mode enable*/
    // printf("rtc mode enable\n");

    ret = pcf85263_read_data(PCF8563_ADDR_RAM_BYTE, &ram_flag, L_ONE);    /*read flag*/
    if(ret != V_ZERO)
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }
    ret = pcf85263_read_data(PCF8563_ADDR_FUN, &m_rtc_mode_read, L_ONE);    /*read rtc mode */
    printf("m_rtc_mode_read = %d\n", m_rtc_mode_read);
    HAL_Delay(10);
    ret = pcf85263_read_data(PCF8563_ADDR_MS, PCF8563_Read_buf, L_SIX);   /*read time from pcf8563*/ 

    // PCF8563_Read_buf[V_ZERO] &= LOW_SEVEN_BITS_ENABLE;           /*millisecond*/
    PCF8563_Read_buf[V_ONE] &= LOW_SEVEN_BITS_ENABLE;           /*second*/
    PCF8563_Read_buf[V_TWO] &= LOW_SEVEN_BITS_ENABLE;           /*Minute*/
    // PCF8563_Read_buf[V_THREE] &= LOW_SIX_BITS_ENABLE;           /*hour*/
    // PCF8563_Read_buf[V_FOUR] &= LOW_SIX_BITS_ENABLE;           /*day*/
    // PCF8563_Read_buf[V_FIVE] &= LOW_THREE_BITS_ENABLE;           /*week*/
    // PCF8563_Read_buf[V_SIX] &= LOW_FIVE_BITS_ENABLE;           /*month*/

    count_time->millis = bcd_to_bin(PCF8563_Read_buf[V_ZERO]) * TIMES_TEN;
    count_time->second = bcd_to_bin(PCF8563_Read_buf[V_ONE]);
    count_time->minute = bcd_to_bin(PCF8563_Read_buf[V_TWO]);
    count_time->ten = bcd_to_bin(PCF8563_Read_buf[V_THREE]);
    count_time->kilo = bcd_to_bin(PCF8563_Read_buf[V_FOUR]);
    count_time->hundredkilo = bcd_to_bin(PCF8563_Read_buf[V_FIVE]);

    printf("rtc time:%d %d %d:%d:%d.%d\n", count_time->hundredkilo,count_time->kilo,count_time->ten,count_time->minute,count_time->second,count_time->millis);
    if (count_time->second > SECOND_MAX)
    {
        printf("rtc second invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }    
    else if (count_time->minute > MINUTE_MAX)
    {
        printf("rtc minute invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }    
    else if (count_time->ten > TEN_MAX)
    {    
        printf("rtc hour ten invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    } 
    else if (count_time->kilo > KILO_MAX)
    {    
        printf("rtc hour kilo invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }    
    else if (count_time->hundredkilo > H_KILO_MAX)
    {    
        printf("rtc hour hundredkilo invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }    

    if(ret == RTC_OP_INVALID_DATE_TIME)
    {
        printf("ERR:pcf85263_query_time:RTC_OP_INVALID_DATE_TIME\n");
        return ret;
    } 

    if (ret == V_ZERO)
    {
        if (ram_flag != RAM_FLAG_VALUE)                                            /*Here protocol RTC RAM value, judge whether the value is bit*/
        {
            return RTC_OP_INVALID_DATE_TIME;                            /*Time is not accurate*/
        }
        else
        {
            return RTC_OP_SUCCESS;                                      /*Correct read to time*/
        }
    }
    else
    {
        return RTC_OP_COMMUNICATION_FAILURE;                             /*read failure communication error*/
    }
}

/***************************************************************
* @brief  This is the underlying driver interface to configure the Date and Time register 
*        for the external rtc chip 
* @param[in]
*          pcf8563_set_timer
* @return
*          RTC_OP_SUCCESS                
*          RTC_OP_COMMUNICATION_FAILURE  
**************************************************************/
rtc_op_return_type_t pcf85263_set_time(RTC_COUNT_TIME_T count_time)
{
    uint8_t ret = RTC_OP_SUCCESS;
    uint8_t m_100th_seconds_dis = I_ZERO;
    uint8_t m_stop_clock = V_ONE;
    uint8_t m_clear_prescaler = 0xA4;
    printf("pcf85263 set time\n");
    if (count_time.second > SECOND_MAX)
    {
        printf("rtc second invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }    
    else if (count_time.minute > MINUTE_MAX)
    {
        printf("rtc minute invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }    
    else if (count_time.ten > TEN_MAX)
    {    
        printf("rtc hour ten invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    } 
    else if (count_time.kilo > KILO_MAX)
    {    
        printf("rtc hour kilo invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }    
    else if (count_time.hundredkilo > H_KILO_MAX)
    {    
        printf("rtc hour hundredkilo invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }  

    if(ret == RTC_OP_INVALID_DATE_TIME)
    {
        printf("ERR:pcf85263_query_time:RTC_OP_INVALID_DATE_TIME\n");
        return ret;
    }    
        
    uint8_t pcf8563_flag = RAM_FLAG_VALUE;
    uint8_t PCF8563_Set_buf[V_SIX] = {I_ZERO, I_ZERO, I_ZERO, I_ZERO,\
                                            I_ZERO, I_ZERO};

    PCF8563_Set_buf[V_ZERO] = I_ZERO;
    PCF8563_Set_buf[V_ONE] = count_time.second;
    PCF8563_Set_buf[V_TWO] = count_time.minute;
    PCF8563_Set_buf[V_THREE] = count_time.ten;
    PCF8563_Set_buf[V_FOUR] = count_time.kilo;
    PCF8563_Set_buf[V_FIVE] = count_time.hundredkilo;                                                 

    PCF8563_Set_buf[V_ZERO] &= LOW_SEVEN_BITS_ENABLE;           /*millisecond*/
    PCF8563_Set_buf[V_ONE] &= LOW_SEVEN_BITS_ENABLE;           /*second*/
    PCF8563_Set_buf[V_TWO] &= LOW_SEVEN_BITS_ENABLE;           /*Minute*/
    
    printf("set zero to five:");    
    for(uint8_t i = 0; i < 6; i++)
    {
        printf("%d ", PCF8563_Set_buf[i]);
    }
    printf("\n");

    for(uint8_t i = V_ZERO; i < V_SIX; i++)                                          /*bin change to bcd*/
    {
        PCF8563_Set_buf[i] = bin_to_bcd(PCF8563_Set_buf[i]);
    }

    ret = pcf85263_write_data(PCF8563_ADDR_RAM_BYTE, &pcf8563_flag, L_ONE);    /*Writing flag to PCF8563 ram indicates that the configuration time is too long*/
    if (ret != V_ZERO)
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }
    ret = pcf85263_write_data(PCF8563_ADDR_STOP_EN, &m_stop_clock, L_ONE);    /*STOP CLOCK*/
    if (ret != V_ZERO)
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }

    ret = pcf85263_write_data(PCF8563_ADDR_RESET_REG, &m_clear_prescaler, L_ONE);    /*clear prescaler*/
    if (ret != V_ZERO)
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }

    ret = pcf85263_write_data(PCF8563_ADDR_FUN, &m_100th_seconds_dis, L_ONE);    /*100TH seconds disable*/
    if (ret != V_ZERO)
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }

    ret = pcf85263_write_data(PCF8563_ADDR_MS, PCF8563_Set_buf, V_SIX);    /*write time to pcf85263*/
    m_stop_clock = V_ZERO;
    ret = pcf85263_write_data(PCF8563_ADDR_STOP_EN, &m_stop_clock, L_ONE);    /*STOP CLOCK*/
    if (ret == V_ZERO)
    {
        return RTC_OP_SUCCESS;
    }
    else
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }
}


/***************************************************************
* @brief   This api is the underlying driver interface  to  enable alarm1 or alarm2 interrupt function
* @param[in]  
*           alarm_id
* @return
*           RTC_OP_SUCCESS                    
*           RTC_OP_COMMUNICATION_FAILURE     
*           RTC_OP_INVALID_DATE_TIME        
**************************************************************/
rtc_op_return_type_t  pcf85263_config_inta(uint32_t alarm_id)
{
    uint8_t ret = I_ZERO;
    uint8_t read_data = I_ZERO;
    uint8_t write_data = I_ZERO;

    write_data = PCF_CLK_REGISTER_ENABLE;                                  /*config  clk pin  is intA    Enable CLK pin for inta function, default high level*/
    ret = pcf85263_write_data(PCF8563_ADDR_PIN_CON, &write_data, L_ONE);          
    if (ret != V_ZERO)
    {
        return RTC_OP_COMMUNICATION_FAILURE;

    }

    write_data  = V_ZERO;
    ret = pcf85263_read_data(PCF8563_ADDR_INTA_CON, &read_data, L_ONE);          /* read inta config*/
	
    if (ret != V_ZERO)
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }

    if (alarm_id == ALARM_ID_ONE)
    {
        write_data = read_data | PCF_ALARM_ONE_REGISTER_ENABLE; /* 0b1001 0000*/                                     

    }
    else if (alarm_id == ALARM_ID_TWO)
    {
        write_data = read_data | PCF_ALARM_TWO_REGISTER_ENABLE; /* 0b1000 1000*/
    }
    else
    {
        return RTC_OP_INVALID_DATE_TIME;
    }

    ret = pcf85263_write_data(PCF8563_ADDR_INTA_CON, &write_data, L_ONE);          /*get alarm1 int pin is intA*/
	
    if (ret == V_ZERO)
    {
        return RTC_OP_SUCCESS;                                               /*success*/
    }
    else
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }
}

/***************************************************************
*@brief   This api  is the underlying driver interface  to configure INTA pin  as the interrupt output pin
*@param   void
*@return
*         RTC_OP_SUCCESS                    
*         RTC_OP_COMMUNICATION_FAILURE    
**************************************************************/

rtc_op_return_type_t pcf85263_check_inta(void)
{
    uint8_t ret = I_ZERO;
    uint8_t write_data[V_TWO];
    uint8_t read_data[V_TWO];

    ret = pcf85263_read_data(PCF8563_ADDR_PIN_CON, read_data, L_ONE);          /*Configure alarm1 interrupt output on the INTA*/
    if (ret != V_ZERO)
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }

    if (read_data[V_ZERO] == PCF_CLK_REGISTER_ENABLE)
    {
        return RTC_OP_SUCCESS;
    }

    write_data[V_ZERO] = PCF_CLK_REGISTER_ENABLE;                                        /*Enable CLK pin for inta function, default high level*/
    ret = pcf85263_write_data(PCF8563_ADDR_PIN_CON, write_data, L_ONE);    

    if (ret != V_ZERO)
    {
        return RTC_OP_COMMUNICATION_FAILURE; 
    }
    return RTC_OP_SUCCESS;
}


/***************************************************************
* @brief  This is the underlying driver interface to set the alarm1 or alarm2 register value and enable the  alram interrupt.
*         Month, day, Hour, minute and second can be configured  by the Alarm1.
*         only  weekday, hours and minutes can be configured by the Alarm2
* @param[in]
*           alarm_id
*           date_time
* @return
*           RTC_OP_SUCCESS                
*           RTC_OP_COMMUNICATION_FAILURE  
**************************************************************/
rtc_op_return_type_t pcf85263_set_alarm(uint32_t alarm_id , RTC_DATE_TIME_T date_time)
{
    uint8_t ret = RTC_OP_SUCCESS;

    if (date_time.second > SECOND_MAX)
    {
        printf("rtc second invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }
    else if (date_time.minute > MINUTE_MAX)
    {
        printf("rtc minute invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }    
    else if (date_time.hour > HOUR_MAX)
    {    
        printf("rtc hour invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    } 
    else if ((date_time.day_of_month < DAY_MIN) || (date_time.day_of_month > DAY_MAX))   
    {
        printf("rtc day invalid\r\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }    
    else if ((date_time.month < MONTH_MIN) || (date_time.month > MONTH_MAX))
    {
        printf("rtc month invalid\r\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }     
    if(ret == RTC_OP_INVALID_DATE_TIME)
    {
        printf("ERR:pcf85263_set_alarm:RTC_OP_INVALID_DATE_TIME\r\n");
        return ret;
    }   
    rtc_op_return_type_t err_code;

    uint8_t alarm_status = I_ZERO;             /*Configure alarm interrupt*/
    uint8_t set_alarm[V_FIVE];

    memset(set_alarm, V_ZERO, V_FIVE);
    set_alarm[V_ZERO] = date_time.second;
    set_alarm[V_ONE] = date_time.minute;
    set_alarm[V_TWO] = date_time.hour;
    set_alarm[V_THREE] = date_time.day_of_month;
    set_alarm[V_FOUR] = date_time.month;

    set_alarm[V_ZERO] &= LOW_SEVEN_BITS_ENABLE;           
    set_alarm[V_ONE] &= LOW_SEVEN_BITS_ENABLE;           
    set_alarm[V_TWO] &= LOW_SIX_BITS_ENABLE;           
    set_alarm[V_THREE] &= LOW_SIX_BITS_ENABLE;           
    set_alarm[V_FOUR] &= LOW_SIX_BITS_ENABLE;           

    for(uint8_t i = V_ZERO; i < V_FIVE; i++) /*change to bcd code*/
    {
        set_alarm[i] = bin_to_bcd(set_alarm[i]);
    }

    err_code = pcf85263_config_inta(alarm_id);                                       /*Configure interrupt inta function*/
    if (err_code != RTC_OP_SUCCESS)
    {
        return err_code;
    }

    ret = pcf85263_read_data(PCF8563_ALARM_ENABLES, &alarm_status, L_ONE);        /*read alarm config*/
    if (ret != V_ZERO)
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }

    if (alarm_id == ALARM_ID_ONE)
    {
        //alarm_status |= LOW_FIVE_BITS_ENABLE;        /*0x 0001 1111*/
        alarm_status |= 0x01;
        ret = pcf85263_write_data(PCF8563_ALARM_ENABLES, &alarm_status, L_ONE);     /*Start alarm function*/
        if (ret != V_ZERO)
        {
            return RTC_OP_COMMUNICATION_FAILURE;
        }
        //ret = pcf85263_write_data(PCF8563_ALARM_SECONDS, set_alarm, L_FIVE);        /*Write time value to alarm register*/
        ret = pcf85263_write_data(PCF8563_ALARM_SECONDS, set_alarm, L_ONE);
        if (ret == V_ZERO)
        {
            //return RTC_OP_SUCCESS;
        }
        else
        {
            return RTC_OP_COMMUNICATION_FAILURE;
        }
    }
    else if (alarm_id == ALARM_ID_TWO)
    {
        alarm_status |= ALARM_VALUES;    /*0x0110 0000*/
        //alarm_status |= 0xE0;
        ret = pcf85263_write_data(PCF8563_ALARM_ENABLES, &alarm_status, L_ONE);   /*Start alarm function*/
        if (ret != V_ZERO)
        {
            return RTC_OP_COMMUNICATION_FAILURE;
        }

        ret = pcf85263_write_data(PCF8563_ALARM2_MINUTES, &set_alarm[V_ONE], L_TWO);        /*Write time value to alarm register*/
        //ret = pcf85263_write_data(PCF8563_ALARM_MINUTES, &set_alarm[V_ONE], L_THREE);
        if (ret == V_ZERO)
        {
            //return RTC_OP_SUCCESS;
        }
        else
        {
            return RTC_OP_COMMUNICATION_FAILURE;
        }
    }
    return RTC_OP_SUCCESS;
}


rtc_op_return_type_t pcf85263_clear_alarm_flag(void)
{
    uint8_t rtc_read = V_ZERO;
    uint8_t ret = RTC_OP_SUCCESS;
    printf("alarm flag clear\n");
    ret = pcf85263_write_data(PCF8563_ADDR_FLAGS, &rtc_read, L_ONE);
    HAL_Delay(2);
    ret = pcf85263_read_data(PCF8563_ADDR_FLAGS, &rtc_read, L_ONE);
    printf("alarm flag = %d\n", rtc_read);
    if(ret != V_ZERO)
    {
        ret = RTC_OP_COMMUNICATION_FAILURE;
    }

    return ret;
}

/***************************************************************
*@brief  This api is the underlying driver interface to get Alarm1 or Alarm2 register value
*        related to the alarm time
*@param 
*      [in]    alarm_id
*      [out]   date_time
*@return
*       RTC_OP_SUCCESS                 
*       RTC_OP_COMMUNICATION_FAILURE  
**************************************************************/
rtc_op_return_type_t pcf85263_read_alarm(uint32_t alarm_id)
{
    uint8_t ret = I_ZERO;
    uint8_t read_alarm[V_EIGHT] = {I_ZERO,I_ZERO,I_ZERO,I_ZERO,I_ZERO,I_ZERO,I_ZERO,I_ZERO};

    if (alarm_id == ALARM_ID_ONE)
    {
        printf("read alarm 1\n");
        ret = pcf85263_read_data(PCF8563_ALARM_SECONDS, &read_alarm[V_ZERO], L_EIGHT);
        //ret = pcf85263_read_data(PCF8563_ALARM_ENABLES, &alarm_status, L_ONE);
        if (ret != V_ZERO)
        {
            return RTC_OP_COMMUNICATION_FAILURE;
        }
    }
    else if (alarm_id == ALARM_ID_TWO)
    {
        printf("read alarm 2\n");
        ret = pcf85263_read_data(PCF8563_ALARM2_MINUTES, &read_alarm[V_ONE], L_TWO);
        if (ret != V_ZERO)
        {
            return RTC_OP_COMMUNICATION_FAILURE;
        }
    }
    else
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }

    read_alarm[V_ZERO] &= LOW_SEVEN_BITS_ENABLE;           /*second*/
    read_alarm[V_ONE] &= LOW_SEVEN_BITS_ENABLE;           /*Minute*/
    // read_alarm[V_TWO] &= LOW_SIX_BITS_ENABLE;           /*hour*/
    // read_alarm[V_THREE] &= LOW_SIX_BITS_ENABLE;           /*day*/
    // read_alarm[V_FOUR] &= LOW_SIX_BITS_ENABLE;           /*month*/

    read_alarm[V_ZERO] = bcd_to_bin(read_alarm[V_ZERO]);
    read_alarm[V_ONE] = bcd_to_bin(read_alarm[V_ONE]);
    read_alarm[V_TWO] = bcd_to_bin(read_alarm[V_TWO]);
    read_alarm[V_THREE] = bcd_to_bin(read_alarm[V_THREE]);
    read_alarm[V_FOUR] = bcd_to_bin(read_alarm[V_FOUR]);
    read_alarm[V_FIVE] = bcd_to_bin(read_alarm[V_FIVE]);
    read_alarm[V_SIX] = bcd_to_bin(read_alarm[V_SIX]);
    read_alarm[V_SEVEN] = bcd_to_bin(read_alarm[V_SEVEN]);    
    //date_time->year  = V_ZERO;
    printf("alarm time : ");
    for(uint8_t i = 0; i < 8; i++)
    {
        printf("%d ", read_alarm[i]);
    }
    printf("\n ");    
    if (ret == V_ZERO)
    {
        return RTC_OP_SUCCESS;
    }
    else
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }
}
/***************************************************************
* @brief  
*           In stop-watch mode, Alarm 1 can be configured from seconds to 999999 hours. Alarm 2 
*           operates on minutes up to 9999 hours.
* @param[in]
*           alarm_id
*           count_time
* @return
*           RTC_OP_SUCCESS                
*           RTC_OP_COMMUNICATION_FAILURE  
*           RTC_OP_INVALID_DATE_TIME
**************************************************************/
rtc_op_return_type_t pcf85263_set_sw_mode_alarm(uint32_t alarm_id , RTC_COUNT_TIME_T count_time)
{
    uint8_t ret = RTC_OP_SUCCESS;
    uint8_t m_rtc_mode_bit = SW_MODE;   
    uint8_t m_rtc_mode_read = 0;    
    ret = pcf85263_write_data(PCF8563_ADDR_FUN, &m_rtc_mode_bit, L_ONE);    /*stop watch mode enable*/
    printf("stop watch mode enable\n");
    if (ret != V_ZERO)
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }

    if (count_time.second > SECOND_MAX)
    {
        printf("rtc second invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }
    else if (count_time.minute > MINUTE_MAX)
    {
        printf("rtc minute invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }    
    else if (count_time.ten > TEN_MAX)
    {    
        printf("rtc hour ten invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    } 
    else if (count_time.kilo > KILO_MAX)
    {    
        printf("rtc hour kilo invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }    
    else if (count_time.hundredkilo > H_KILO_MAX)
    {    
        printf("rtc hour hundredkilo invalid\n");
        ret = RTC_OP_INVALID_DATE_TIME;
    }   
    if(ret == RTC_OP_INVALID_DATE_TIME)
    {
        printf("ERR:pcf85263_set_alarm:RTC_OP_INVALID_DATE_TIME\r\n");
        return ret;
    }   
    rtc_op_return_type_t err_code;

    uint8_t alarm_status = I_ZERO;             /*Configure alarm interrupt*/
    uint8_t set_alarm[V_FIVE];

    memset(set_alarm, V_ZERO, V_FIVE);
    set_alarm[V_ZERO] = count_time.second;
    set_alarm[V_ONE] = count_time.minute;
    set_alarm[V_TWO] = count_time.ten;
    set_alarm[V_THREE] = count_time.kilo;
    set_alarm[V_FOUR] = count_time.hundredkilo;

    set_alarm[V_ZERO] &= LOW_SEVEN_BITS_ENABLE;           
    set_alarm[V_ONE] &= LOW_SEVEN_BITS_ENABLE;                  
    printf("%d %d %d:%d:%d\n",count_time.hundredkilo,count_time.kilo,count_time.ten,count_time.minute,count_time.second);
    for(uint8_t i = V_ZERO; i < V_FIVE; i++) /*change to bcd code*/
    {
        set_alarm[i] = bin_to_bcd(set_alarm[i]);
    }

    err_code = pcf85263_config_inta(alarm_id);                                       /*Configure interrupt intA function*/
    if (err_code != RTC_OP_SUCCESS)
    {
        return err_code;
    }

    ret = pcf85263_read_data(PCF8563_ALARM_ENABLES, &alarm_status, L_ONE);        /*read alarm config*/
    if (ret != V_ZERO)
    {
        return RTC_OP_COMMUNICATION_FAILURE;
    }

    if (alarm_id == ALARM_ID_ONE)
    {
        printf("alarm 1 started\n");
        //alarm_status |= LOW_FIVE_BITS_ENABLE;        /*0x 0001 1111*/
        alarm_status |= LOW_TWO_BITS_ENABLE;         /*0x 0000 0111*/
        ret = pcf85263_write_data(PCF8563_ALARM_ENABLES, &alarm_status, L_ONE);     /*Start alarm function*/
        if (ret != V_ZERO)
        {
            return RTC_OP_COMMUNICATION_FAILURE;
        }
        //ret = pcf85263_write_data(PCF8563_ALARM_SECONDS, set_alarm, L_FIVE);        /*Write time value to alarm register*/
        ret = pcf85263_write_data(PCF8563_SECOND_ALM1, &set_alarm[V_ZERO], L_TWO);
        if (ret == V_ZERO)
        {
            //return RTC_OP_SUCCESS;
        }
        else
        {
            return RTC_OP_COMMUNICATION_FAILURE;
        }
    }
    else if (alarm_id == ALARM_ID_TWO)
    {
        printf("alarm 2 started\n");
    //    alarm_status |= ALARM_VALUES;    /*0x0110 0000*/
        alarm_status |= 0x20;
        ret = pcf85263_write_data(PCF8563_ALARM_ENABLES, &alarm_status, L_ONE);   /*Start alarm function*/
        if (ret != V_ZERO)
        {
            return RTC_OP_COMMUNICATION_FAILURE;
        }

        ret = pcf85263_write_data(PCF8563_MINUTE_ALM2, &set_alarm[V_ONE], L_ONE);        /*Write time value to alarm register*/
        //ret = pcf85263_write_data(PCF8563_ALARM_MINUTES, &set_alarm[V_ONE], L_THREE);
        if (ret == V_ZERO)
        {
            //return RTC_OP_SUCCESS;
        }
        else
        {
            return RTC_OP_COMMUNICATION_FAILURE;
        }
    }

    ret = pcf85263_read_data(PCF8563_ADDR_FUN, &m_rtc_mode_read, L_ONE);    /*read rtc mode */
    printf("m_rtc_mode_read = %d\n", m_rtc_mode_read);
    pcf85263_read_alarm(L_ONE);

    return RTC_OP_SUCCESS;    
}
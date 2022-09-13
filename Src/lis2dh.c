/*
    Copyright 2019 SHENZHEN ZHILAI SCI AND TECH CO., LTD,
    15th floor, C3 building, Nanshan ipark,Xueyuan road 1001,
	Nanshan District, Shenzhen 518052, China.
    All rights are reserved. Reproduction in whole or in part is prohibited
    without the prior written consent of the copyright owner.

    COMPANY CONFIDENTIAL

Filename  :  lis2dh.c


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

#include <float.h>
#include "i2c.h"
#include "lis2dh.h"
#include "global.h"

extern I2C_HandleTypeDef hi2c3;

#define LIS2DH12_FROM_FS_2g_HR_TO_mg(lsb)  (float)((int16_t)lsb>>4)* 1.0f
#define LIS2DH12_FROM_FS_4g_HR_TO_mg(lsb)  (float)((int16_t)lsb>>4)* 2.0f
#define LIS2DH12_FROM_FS_8g_HR_TO_mg(lsb)  (float)((int16_t)lsb>>4)* 4.0f
#define LIS2DH12_FROM_FS_16g_HR_TO_mg(lsb) (float)((int16_t)lsb>>4)*12.0f
#define LIS2DH12_FROM_LSB_TO_degC_HR(lsb) (float)((int16_t)lsb>>6)/4.0f+25.0f

#define LIS2DH12_FROM_FS_2g_NM_TO_mg(lsb)  (float)((int16_t)lsb>>6)*  4.0f
#define LIS2DH12_FROM_FS_4g_NM_TO_mg(lsb)  (float)((int16_t)lsb>>6)*  8.0f
#define LIS2DH12_FROM_FS_8g_NM_TO_mg(lsb)  (float)((int16_t)lsb>>6)* 16.0f
#define LIS2DH12_FROM_FS_16g_NM_TO_mg(lsb) (float)((int16_t)lsb>>6)* 48.0f
#define LIS2DH12_FROM_LSB_TO_degC_NM(lsb) (float)((int16_t)lsb>>6)/4.0f+25.0f

#define LIS2DH12_FROM_FS_2g_LP_TO_mg(lsb)  (float)((int16_t)lsb>>8)*16.0f
#define LIS2DH12_FROM_FS_4g_LP_TO_mg(lsb)  (float)((int16_t)lsb>>8)*32.0f
#define LIS2DH12_FROM_FS_8g_LP_TO_mg(lsb)  (float)((int16_t)lsb>>8)*64.0f
#define LIS2DH12_FROM_FS_16g_LP_TO_mg(lsb) (float)((int16_t)lsb>>8)*192.0f
#define LIS2DH12_FROM_LSB_TO_degC_LP(lsb) (float)((int16_t)lsb>>8)*1.0f + 25.0f

axis_info_t acc_sample;
filter_avg_t acc_data;
void old_angle_calculate(void);


/***************************************************************
* @brief  To specify length data to PCF8563
* @param
*       add     Write address
*       write_buf Write data
*       len     data lenth
* @return ：0-> success          1-> fail
**************************************************************/

uint8_t lis2dh_write_data(uint8_t addr, uint8_t *write_buf, uint8_t len)
{
    uint8_t ret = I_ZERO;
    //MX_I2C3_Init();
    //i2c3_gpio_init();
    //HAL_Delay(5);
	if(HAL_OK == HAL_I2C_Mem_Write(&hi2c3, LIS2DH_WRITE_ADDR, addr, I2C_MEMADD_SIZE_8BIT, write_buf, len, 1000))
    {
        ret = V_ZERO;
        printf("lis2dh write data success!\n");
    }
    else
    {
        ret = V_ONE;
        printf("lis2dh write data failed!\n");
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
uint8_t lis2dh_read_data(uint8_t addr, uint8_t *read_buf, uint8_t len)
{
    uint8_t ret = I_ZERO;
    //MX_I2C3_Init();
    //i2c3_gpio_init();
    //HAL_Delay(5);
	if(HAL_OK == HAL_I2C_Mem_Read(&hi2c3, LIS2DH_READ_ADDR, addr, I2C_MEMADD_SIZE_8BIT, read_buf, len, 1000))
    {
        //printf("lis2dh read data success!\n");
        ret = V_ZERO;
    }
    else
    {
        ret = V_ONE;
        printf("lis2dh read data failed!\n");
    }
    //HAL_Delay(5);
    //HAL_I2C_MspDeInit(I2C3);
    //set_gpio_mode_input_up(GPIOC, GPIO_PIN_0);
    //set_gpio_mode_input_up(GPIOC, GPIO_PIN_1);
    return ret;
}

/***************************************************************
* @brief  This api can be called to  initialize the lis2dh device
* @param  void
* @return lis2dh_status_code_t The status of the lis2dh working result
**************************************************************/
lis2dh_status_code_t lis2dh_init(void)
{
    uint8_t ret = I_ZERO;
    uint8_t CTRL_REG1 = LIS2DH_CTRL_REG1;       /*50Hz Data Rate,lowpower mode ,x/y/z can use*/
    uint8_t CTRL_REG2 = LIS2DH_CTRL_REG2;       /*INT1 use High-pass*/
    uint8_t CTRL_REG3 = LIS2DH_CTRL_REG3;       /*INT1 interrupt*/
    uint8_t CTRL_REG4 = LIS2DH_CTRL_REG4;
    uint8_t INT1_CFG = LIS2DH_INT1_CFG;       /*enable ,x/y/z cover interrupt*/
    uint8_t INT1_THS = LIS2DH_DEFAULT_THRESHOLD;       /*Interruption threshold*/
    uint8_t INT1_DURATION = LIS2DH_INT1_DURATION;   /*Interrupt immediately when exceeding*/
    uint8_t id;

    id = lis2dh_rd_id();
    if (LIS2DH_WHO_AM_I != id)
    {
        printf("lis2dh id:0x%x wrong\r\n",id);
        return LIS2DH_ERROR;
    }
    
    ret = lis2dh_write_data(GRTL_REG1_ADDR, &CTRL_REG1, L_ONE);
    if (ret != LIS2DH_SUCCREE)
    {
        return LIS2DH_ERROR;
    }

    ret = lis2dh_write_data(GRTL_REG2_ADDR, &CTRL_REG2, L_ONE);

    if (ret != LIS2DH_SUCCREE)
    {
        return LIS2DH_ERROR;
    }

    ret = lis2dh_write_data(GRTL_REG3_ADDR, &CTRL_REG3, L_ONE);
    if (ret != LIS2DH_SUCCREE)
    {
        return LIS2DH_ERROR;
    }
	
    ret = lis2dh_write_data(GRTL_REG4_ADDR, &CTRL_REG4, L_ONE);
    if (ret != LIS2DH_SUCCREE)
    {
        return LIS2DH_ERROR;
    }

    ret = lis2dh_write_data(INT1_CFG_ADDR, &INT1_CFG, L_ONE);
    if (ret != LIS2DH_SUCCREE)
    {
        return LIS2DH_ERROR;
    }

    ret = lis2dh_write_data(INT1_THS_REG, &INT1_THS, L_ONE);
    if (ret != LIS2DH_SUCCREE)
    {
        return LIS2DH_ERROR;
    }

    ret = lis2dh_write_data(INT1_DURATION_REG, &INT1_DURATION, L_ONE);
    if (ret != LIS2DH_SUCCREE)
    {
        return LIS2DH_ERROR;
    }
      
  	filter_acc();
    old_angle_calculate();
    return LIS2DH_SUCCESS;
}

/***************************************************************
* @brief  This api  can  be called to  set  the  interrupt threshold
* @param  level
* @return 1 means err,0 means ok.
**************************************************************/
uint8_t lis2dh_set_threshold(uint8_t level)
{
    uint8_t ret = LIS2DH_SUCCESS;
    uint8_t read_data;

    if (level > LIS2DH_INT1_THS_MAX)
    {
        level = LIS2DH_INT1_THS_MAX;
    }
    
    if (level < LIS2DH_INT1_THS_MIN)
    {
        level = LIS2DH_INT1_THS_MIN;
    }

    ret = lis2dh_write_data(INT1_THS_REG, &level, L_ONE);
    if (ret != LIS2DH_SUCCREE)
    {
        printf("lis2dh_set_threshold fail\r\n");
        return LIS2DH_ERROR;

    }
    
    ret = lis2dh_read_data(INT1_THS_REG, &read_data, L_ONE);
    if(ret != LIS2DH_SUCCREE)
    {
        return LIS2DH_ERROR;
    }
    else
    {
        if(read_data != level)
        {
            printf("lis2dh_set_threshold fail,set:%d,read:%d\r\n",level,read_data);
            return LIS2DH_ERROR;
        }
    }    

    return ret;
}

/***************************************************************
* @brief  This api  can  be called to  read  the  interrupt threshold
* @param  level
* @return 1 means err,0 means ok.
**************************************************************/
uint8_t lis2dh_get_threshold(uint8_t * threshold)
{
    uint8_t ret;
    //uint8_t read_data;
    
    ret = lis2dh_read_data(INT1_THS_REG, threshold, L_ONE);
    if(ret)
    {
        printf("lis2dh_get_threshold error:%d\r\n",ret);
    }
    
    return ret;
}

/***************************************************************
* @brief  This api  can be  called to read the device ID
* @param  void
* @return 0 means err,lis2dh_id = 0x33 means ok.
**************************************************************/
uint8_t lis2dh_rd_id(void)
{
    uint8_t ret = I_ZERO;
    uint8_t lis2dh_id = I_ZERO;	

    ret = lis2dh_read_data(WHO_AM_I_REG, &lis2dh_id, L_ONE);
    
    if (ret != LIS2DH_SUCCREE)
    {
         printf("lis2dh_rd_id err");
        return LIS2DH_ERROR;
    }
    else
    {
        return lis2dh_id;
    }
}


/***************************************************************
  * @brief  This api can be  called to  read axis value
  * @param  
  * @return 
**************************************************************/
void get_acc_value(void)
{
    uint8_t data[XYZ_VALUE];
    int16_t x,y,z;
    
    for (uint8_t i = I_ZERO; i < XYZ_VALUE; i++)
    {
        lis2dh_read_data(OUT_X_L_ADDR + i, &data[i], L_ONE);    /*read xyz info*/
    }
    x = *(int16_t*)data;
    y = *(int16_t*)(data + 2);
    z = *(int16_t*)(data + 4);
    
    acc_sample.x = LIS2DH12_FROM_FS_2g_HR_TO_mg(x); 
    acc_sample.y = LIS2DH12_FROM_FS_2g_HR_TO_mg(y);
    acc_sample.z = LIS2DH12_FROM_FS_2g_HR_TO_mg(z);
    /*
    NRF_LOG_INFO("raw_data:x:0x%x,y:0x%x,z:0x%x\r\n",x,y,z);
    NRF_LOG_FLUSH();
    */
}

uint8_t is_lis2dh_data_valid(void)
{
    uint8_t ret;
    uint8_t valid = I_ZERO;
    uint8_t status_reg_value;
    ret = lis2dh_read_data(STATUS_REG_ADDR, &status_reg_value, L_ONE);
    if (ret)
    {
        printf("STATUS_REG_ADDR rd error:%d\r\n",ret);
        valid = V_ZERO;
    }
    else
    {
        if (status_reg_value & STATUS_REG_ZYXDA_VALID_MASK)
        {
            valid = V_ONE;
        }
        else
        {
            valid = V_ZERO;
        }
    }
    return valid;
}  

uint8_t lis2dh_raw_data_read(axis_raw_data_t * axis_raw_data)
{
    uint8_t data[XYZ_VALUE];
    uint8_t ret = I_ZERO;

    for (uint8_t i = I_ZERO; i < XYZ_VALUE; i++)
    {
        ret = lis2dh_read_data(OUT_X_L_ADDR + i, &data[i], L_ONE);  /*read xyz info*/
        if (ret)
        {           
            printf("lis2dh_raw_data_read,reg addr:%d error\r\n",OUT_X_L_ADDR + i);
            break;
        }    
    }
    if (ret == V_ZERO)
    {
        axis_raw_data->x = *(int16_t*)data; 
        axis_raw_data->y = *(int16_t*)(data + 2);
        axis_raw_data->z = *(int16_t*)(data + 4);
    }

    return ret;
}    
 
/***************************************************************
  * @brief  This api  read  axis value multiple  times  and calculate   the average val
  * @param  void
  * @return void
**************************************************************/
void filter_acc(void)
{
    int16_t x_sum = I_ZERO, y_sum = I_ZERO, z_sum = I_ZERO;	
    
    for (uint8_t i = I_ZERO; i < FILTER; i++) 
    {
        get_acc_value();

        acc_data.info[i].x = acc_sample.x;
        acc_data.info[i].y = acc_sample.y;
        acc_data.info[i].z = acc_sample.z;

        x_sum += acc_data.info[i].x;
        y_sum += acc_data.info[i].y;
        z_sum += acc_data.info[i].z;
    }
    acc_sample.x = x_sum / FILTER;
    acc_sample.y = y_sum / FILTER;
    acc_sample.z = z_sum / FILTER;	
}
 
 
/***************************************************************
  * @brief  new_angle_calculate
  * @param  
  * @return 
**************************************************************/
void new_angle_calculate(void)
{
    
    acc_sample.new_angle_x = atan((short)acc_sample.x/(short)sqrt(pow(acc_sample.y, 2)+pow(acc_sample.z, 2))) * DEGREE_CAL;
    acc_sample.new_angle_y = atan((short)acc_sample.y/(short)sqrt(pow(acc_sample.x, 2)+pow(acc_sample.z, 2))) * DEGREE_CAL;
    acc_sample.new_angle_z = atan((short)acc_sample.z/(short)sqrt(pow(acc_sample.x, 2)+pow(acc_sample.y, 2))) * DEGREE_CAL;
    if (acc_sample.new_angle_z < I_ZERO)
    {
        acc_sample.new_angle_x = HALF_ANGLE - acc_sample.new_angle_x;
        acc_sample.new_angle_y = HALF_ANGLE - acc_sample.new_angle_y;
    }
}
 
/***************************************************************
  * @brief  old_angle_calculate
  * @param  
  * @return 
**************************************************************/
void old_angle_calculate(void)
{
    acc_sample.old_angle_x = atan((short)acc_sample.x/(short)sqrt(pow(acc_sample.y, 2)+pow(acc_sample.z, 2))) * DEGREE_CAL;
    acc_sample.old_angle_y = atan((short)acc_sample.y/(short)sqrt(pow(acc_sample.x, 2)+pow(acc_sample.z, 2))) * DEGREE_CAL;
    acc_sample.old_angle_z = atan((short)acc_sample.z/(short)sqrt(pow(acc_sample.x, 2)+pow(acc_sample.y, 2))) * DEGREE_CAL;
    if (acc_sample.old_angle_z < I_ZERO)
    {
        acc_sample.old_angle_x = HALF_ANGLE - acc_sample.old_angle_x;
        acc_sample.old_angle_y = HALF_ANGLE - acc_sample.old_angle_y;
    }
}

uint8_t lis2dh_self_test(uint8_t debug)
{
    uint8_t ret = I_ZERO;
    uint8_t flow = I_ZERO;
    axis_raw_data_t axis_raw_data;
    uint8_t sample_count;
    int16_t outx_nost,outy_nost,outz_nost;
    int16_t outx_st,outy_st,outz_st;
    uint8_t self_test_pass = I_ZERO;
    uint8_t retry_num = 100;
    uint8_t retry_count = I_ZERO;
    uint8_t average_num = V_FIVE;
    uint8_t m_write_data = I_ZERO;
    while(1)
    {
        switch(flow)
        {
            case 0:
                m_write_data = 0x47;
                ret = lis2dh_write_data(GRTL_REG1_ADDR, &m_write_data, L_ONE);
                if (ret)
                {
                    printf("flow:%d,GRTL_REG1_ADDR wr error:%d\r\n",flow,ret);
                    flow = V_ZERO;
                    goto END;
                }
                m_write_data = 0x00;
                ret = lis2dh_write_data(GRTL_REG2_ADDR, &m_write_data, L_ONE);
                if (ret)
                {
                    printf("flow:%d,GRTL_REG2_ADDR wr error:%d\r\n",flow,ret);
                    flow = V_ZERO;
                    goto END;
                }

                ret = lis2dh_write_data(GRTL_REG3_ADDR, &m_write_data, L_ONE);
                if (ret)
                {
                    printf("flow:%d,GRTL_REG3_ADDR wr error:%d\r\n",flow,ret); 
                    flow = V_ZERO;
                    goto END;
                }
                m_write_data = 0x80;
                ret = lis2dh_write_data(GRTL_REG4_ADDR, &m_write_data, L_ONE);
                if (ret)
                {
                    printf("flow:%d,GRTL_REG4_ADDR wr error:%d\r\n",flow,ret);
                    flow = V_ZERO;
                    goto END;
                }
                m_write_data = LIS2DH_INT1_THS_MAX;
                ret = lis2dh_write_data(INT1_THS_REG, &m_write_data, L_ONE); //Prevent the chip from interrupting after the completion of the self-test
                if (ret != LIS2DH_SUCCREE)
                {
                    printf("flow:%d,INT1_THS_REG wr error:%d\r\n",flow,ret);
                    flow = V_ZERO;
                    goto END;;

                }
                flow++;
                break;

            case 1:
                check_scheduler_and_delay(100);
                flow++;
                retry_count = V_ZERO;
                break;

            case 2:
                if (is_lis2dh_data_valid())
                {                   
                    ret = lis2dh_raw_data_read(&axis_raw_data);
                    if (ret == V_ZERO)
                    {
                        flow++;
                    }  
                    else
                    {
                        goto END;
                    }    
                }    
                else
                {
                    retry_count++;
                    if (retry_count > retry_num)
                    {
                        printf("wait lis2dh data valid,retry_count timeout\r\n");
                        goto END;
                    }    
                    check_scheduler_and_delay(10);
                } 
                break;

            case 3:                 
                sample_count = V_ZERO;
                outx_nost = V_ZERO;
                outy_nost = V_ZERO;
                outz_nost = V_ZERO;
                outx_st = V_ZERO;
                outy_st = V_ZERO;
                outz_st = V_ZERO;
                retry_count = V_ZERO;
                flow++;
                break;

            case 4:
                if (is_lis2dh_data_valid())
                {
                    ret = lis2dh_raw_data_read(&axis_raw_data);
                    if (ret == V_ZERO)
                    {
                        outx_nost += (axis_raw_data.x>>6);  //The value is expressed as twos complement left justified
                        outy_nost += (axis_raw_data.y>>6);
                        outz_nost += (axis_raw_data.z>>6);
                        sample_count++;            
                        if (sample_count == average_num)
                        {
                            outx_nost /= sample_count;
                            outy_nost /= sample_count;
                            outz_nost /= sample_count;
                            sample_count = V_ZERO;
                            flow++;
                        }    
                    }
                    else
                    {
                        goto END;
                    }    
                }    
                else
                {
                    retry_count++;
                    if (retry_count > retry_num)
                    {
                        printf("wait lis2dh data valid,retry_count timeout\r\n");
                        goto END;
                    } 
                    check_scheduler_and_delay(10);
                }    
                break;

            case 5:
                m_write_data = 0x82;
                ret = lis2dh_write_data(GRTL_REG4_ADDR, &m_write_data, L_ONE);
                if (ret != LIS2DH_SUCCREE)
                {
                    printf("flow:%d,GRTL_REG4_ADDR wr error:%d\r\n",flow,ret);
                    goto END;
                }
                else
                {
                    flow++;
                    retry_count = V_ZERO;
                    check_scheduler_and_delay(100);
                }    
                break;

            case 6:
                if (is_lis2dh_data_valid())
                {
                    ret = lis2dh_raw_data_read(&axis_raw_data);
                    if (ret == V_ZERO)
                    {
                        sample_count = V_ZERO;
                        outx_st = V_ZERO;
                        outy_st = V_ZERO;
                        outz_st = V_ZERO;
                        retry_count = V_ZERO;
                        flow++;
                    }
                    else
                    {
                        goto END;
                    }    
                }    
                else
                {
                    retry_count++;
                    if (retry_count > retry_num)
                    {
                        printf("wait lis2dh data valid,retry_count timeout\r\n");
                        goto END;
                    } 
                    check_scheduler_and_delay(10);
                }    
                break;

            case 7:
                if (is_lis2dh_data_valid())
                {
                    ret = lis2dh_raw_data_read(&axis_raw_data);
                    if (ret == 0)
                    {
                        outx_st += (axis_raw_data.x>>6);
                        outy_st += (axis_raw_data.y>>6);
                        outz_st += (axis_raw_data.z>>6);
                        sample_count++;            
                        if (sample_count == average_num)
                        {
                            outx_st /= sample_count;
                            outy_st /= sample_count;
                            outz_st /= sample_count;
                            sample_count = 0;
                            flow++;
                        }    
                    }
                    else
                    {
                        goto END;
                    }    
                }    
                else
                {
                    retry_count++;
                    if (retry_count > retry_num)
                    {
                        printf("wait lis2dh data valid,retry_count timeout\r\n");
                        goto END;
                    } 
                    check_scheduler_and_delay(10);
                }    
                break;

            case 8:
                if (debug)
                {
                    NRF_LOG_INFO("outx_st:%d,outx_nost:%d,outx_st - outx_nost:%d\r\n",outx_st,outx_nost,outx_st - outx_nost);
                    NRF_LOG_INFO("outy_st:%d,outy_nost:%d,outy_st - outy_nost:%d\r\n",outy_st,outy_nost,outy_st - outy_nost);
                    NRF_LOG_INFO("outz_st:%d,outz_nost:%d,outz_st - outz_nost:%d\r\n",outz_st,outz_nost,outz_st - outz_nost);
                    NRF_LOG_FLUSH();
                }    
                
                if ((abs(outx_st - outx_nost) >= abs(LIS2DH_MIN_ST_X)) && (abs(outx_st - outx_nost) <= abs(LIS2DH_MAX_ST_X))\
                    && (abs(outy_st - outy_nost) >= abs(LIS2DH_MIN_ST_Y)) && (abs(outy_st - outy_nost) <= abs(LIS2DH_MAX_ST_Y))\
                    && (abs(outz_st - outz_nost) >= abs(LIS2DH_MIN_ST_Z)) && (abs(outz_st - outz_nost) <= abs(LIS2DH_MAX_ST_Z)))
                {
                    self_test_pass = 1;
                    NRF_LOG_INFO("************lis2dh self_test pass************\r\n");
 
                }
                else
                {
                    self_test_pass = 0;
                    printf("************lis2dh self_test fail************\r\n");
                }
                
                flow++;
                break;

            case 9:
                m_write_data = 0x00;
                ret = lis2dh_write_data(GRTL_REG1_ADDR, &m_write_data, L_ONE);
                if (ret != 0)
                {
                    printf("flow:%d,GRTL_REG1_ADDR wr error:%d\r\n",flow,ret);
                    goto END;
                }

                ret = lis2dh_write_data(GRTL_REG4_ADDR, &m_write_data, L_ONE);
                if (ret != LIS2DH_SUCCREE)
                {
                    printf("flow:%d,GRTL_REG4_ADDR wr error:%d\r\n",flow,ret);
                    goto END;
                }
                flow++;
                retry_count = V_ZERO;
                goto END;
                break;   
    
            default:
                printf("default lis2dh self_test flow %d\r\n",flow);
                goto END;
        }

    }   
END:    
        if (ret)
        {
            check_scheduler_and_delay(1000);   //if lis2dh self_test fail,lis2dh_set_threshold will generate interrupt,delay for show status
            lis2dh_init();          
            lis2dh_set_threshold(LIS2DH_INT1_THS_MAX);  
            goto END;
        }   
        else
        {
        	ret = lis2dh_init();          
            ret = lis2dh_set_threshold(lis2dh_config_load());    
        } 
        NRF_LOG_FLUSH();
        return self_test_pass?0:1;   
}        

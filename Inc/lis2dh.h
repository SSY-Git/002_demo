/*
Copyright (c) 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved

Filename  : lis2dh.h    

				
Rev     Date    Author  Comments
-------------------------------------------------------------------------------
001              Gino                 
002         
003            
004            
-------------------------------------------------------------------------------

@Begin
@module
        Module name   lis2dh
        Module description (Vibration sensor configuration file.
        The triaxial sensor-related code contains a comparison of the current and last angles, as well as an alarm that exceeds the threshold)
@end
*/

#ifndef __LIS2DH_H
#define __LIS2DH_H


#include <stdint.h>
#include "zl_math.h"

#define LIS2DH_DEBUG      0U
#define LIS2DH_READ_ADDR  0x33
#define LIS2DH_WRITE_ADDR  0x32
#define STATUS_REG_AUX   0X07      /* R Notes on temperature control are */
#define INT_COUNTER_REG  0x0E      /* R Interrupt configuration register */
#define WHO_AM_I_REG     0X0F      /* Device id */

#define GRTL_REG1_ADDR   0X20     /* RW Data rate XYZ channel enable */
#define GRTL_REG2_ADDR   0X21     /*RW High pass filter selection configuration */
#define GRTL_REG3_ADDR   0X22     /*RW INT1 pin Pin interrupt configuration */
#define GRTL_REG4_ADDR   0X23     /*RW Data block update and self test mode configuration */
#define GRTL_REG5_ADDR   0X24     /*RW */
#define GRTL_REG6_ADDR   0X25     /*RW INT2 pin Pin interrupt configuration */

#define STATUS_REG_ADDR  0X27     /* R Status register xyz Overflow state */

#define STATUS_REG_ZYXDA_VALID_MASK     0x40

#define LIS2DH_SUCCREE     0x00

#define OUT_X_L_ADDR     0X28    /* R */
#define OUT_X_H_ADDR     0X29    /* R */
#define OUT_Y_L_ADDR     0X2A    /* R */
#define OUT_Y_H_ADDR     0X2B    /* R */
#define OUT_Z_L_ADDR     0X2C    /* R */
#define OUT_Z_H_ADDR     0X2D    /* R */

#define INT1_CFG_ADDR     0X30   /* RW  Interrupt configuration */
#define INT1_SRC_REG      0X31   /* R */
#define INT1_THS_REG      0X32   /* RW */
#define INT1_DURATION_REG 0X33   /* RW */
 
#define INT2_CFG_ADDR     0X34   /*RW*/
#define INT2_SRC_REG      0X35   /*R */
#define INT2_THS_REG      0X36   /*RW*/
#define INT2_DURATION_REG 0X37   /*RW*/

#define CLICK_CFG_REG     0X38   /*RW*/
#define CLICK_SRC_REG     0X39   /*R*/
#define CLICK_THS_REG     0X3A   /*RW*/

#define TIME_LIMIT_REG    0X3B   /*RW*/
#define TIME_LATENCY_REG  0X3C   /*RW*/
#define TIME_WINDOW_REG   0X3D   /*RW*/

#define Act_THS_REG       0X3E   /*RW*/
#define Act_DUR_REG       0X3F   /*RW*/

#define REG4_FS_2G  (0x00<<4)
#define REG4_FS_4G  (0x01<<4)
#define REG4_FS_8G  (0x02<<4)
#define REG4_FS_16G (0x03<<4)

#define REG4_ST_NORMAL  (0x00<<1)
#define REG4_ST_MODE0   (0x01<<1)
#define REG4_ST_MODE1   (0x02<<1)

#define LIS2DH_MIN_ST_X  17
#define LIS2DH_MAX_ST_X  360
#define LIS2DH_MIN_ST_Y  17
#define LIS2DH_MAX_ST_Y  360
#define LIS2DH_MIN_ST_Z  17
#define LIS2DH_MAX_ST_Z  360

#define LIS2DH_CTRL_REG1       0x4fU
#define LIS2DH_CTRL_REG2       0x09U
#define LIS2DH_CTRL_REG3       0x40U
#define LIS2DH_CTRL_REG4       0x0U
#define LIS2DH_CTRL_REG5       0x08U
#define LIS2DH_INT1_CFG        0x2aU
#define LIS2DH_INT1_THS_MIN        0x1FU
#define LIS2DH_INT1_DURATION   0x02U
#define LIS2DH_WHO_AM_I        0x33U
#define LIS2DH_CLOSE_SENSOR    0x00U
#define DEGREE_CAL             180.0/3.1416
#define FILTER                 4U
#define XYZ_VALUE              6U
#define HALF_ANGLE             180U
#define LIS2DH_INT1_THS_MAX          127U
#define LIS2DH_DEFAULT_THRESHOLD    50
#define MAX_THRESHOLD          127U

typedef struct {
	int16_t x;
	int16_t y;
	int16_t z;
	int16_t new_angle_x;
	int16_t new_angle_y;
	int16_t new_angle_z;
	int16_t old_angle_x;
	int16_t old_angle_y;
	int16_t old_angle_z;
}axis_info_t;

typedef struct{
    int16_t x;
    int16_t y;
    int16_t z;
}axis_raw_data_t;    


typedef struct filter_avg{
	axis_info_t info[4];
	uint8_t count;
}filter_avg_t;

/** 
* @brief status code enum for lis2dh operation.
*/
typedef enum {
        LIS2DH_SUCCESS,
        LIS2DH_ERROR,
        LIS2DH_ERROR_UNKNOWN
} lis2dh_status_code_t;


/***************************************************************
* @brief  This api  can be  called to read the device ID
* @param  void
* @return 0 means err,lis2dh_id = 0x33 means ok.
**************************************************************/
uint8_t lis2dh_rd_id(void);



/***************************************************************
* @brief  This api can be called to  initialize the lis2dh device
* @param  void
* @return lis2dh_status_code_t The status of the lis2dh working result
**************************************************************/
lis2dh_status_code_t lis2dh_init(void);


/***************************************************************
* @brief  This api can be called to  get the angle.
* @param  void
* @return void
**************************************************************/
void new_angle_calculate(void);



/***************************************************************
* @brief  This api  read  axis value multiple  times  and calculate   the average val
* @param  void
* @return void
**************************************************************/
void filter_acc(void);

/***************************************************************
* @brief  This api  can  be called to  set  the  interrupt threshold
* @param  level
* @return 1 means err,0 means ok.
**************************************************************/
uint8_t lis2dh_set_threshold(uint8_t level);

/***************************************************************
  * @brief  get_acc_sample
  * @param  void
  * @return return the acc sample result
**************************************************************/
axis_info_t get_acc_sample(void);

/***************************************************************
* @brief  This api  can  be called to  read  the  interrupt threshold
* @param  level
* @return 1 means err,0 means ok.
**************************************************************/
uint8_t lis2dh_get_threshold(uint8_t * threshold);
uint8_t lis2dh_raw_data_read(axis_raw_data_t * axis_raw_data);
uint8_t is_lis2dh_data_valid(void);
void get_acc_value(void);
uint8_t lis2dh_self_test(uint8_t debug);

#endif

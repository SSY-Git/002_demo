#ifndef _PCF85263_H_
#define _PCF85263_H_
#include "i2c.h"

#define PCF8563_ADDR                             (unsigned char)0x51
#define PCF85263_WRITE_ADDR                      0xA2
#define PCF85263_READ_ADDR                       0xA3
#define PCF8563_ADDR_CLKOUT                      (unsigned char)0x0d
#define PCF8563_ADDR_TIMER                       (unsigned char)0x0e
#define PCF8563_ADDR_TIMER_VAL                   (unsigned char)0x0f

#define PCF8563_ADDR_YEARS                       (unsigned char)0x07
#define PCF8563_ADDR_MONTHS                      (unsigned char)0x06
#define PCF8563_ADDR_WEEKDAYS                    (unsigned char)0x05
#define PCF8563_ADDR_DAYS                        (unsigned char)0x04
#define PCF8563_ADDR_HOURS                       (unsigned char)0x03
#define PCF8563_ADDR_MINUTES                     (unsigned char)0x02
#define PCF8563_ADDR_SECONDS                     (unsigned char)0x01
#define PCF8563_ADDR_MS                          (unsigned char)0x00


#define PCF8563_ALARM_SECONDS                    (unsigned char)0x08
#define PCF8563_ALARM_MINUTES                    (unsigned char)0x09
#define PCF8563_ALARM_HOURS                      (unsigned char)0x0A
#define PCF8563_ALARM_DAYS                       (unsigned char)0x0B
#define PCF8563_ALARM_MONTHS                     (unsigned char)0x0C
	
#define PCF8563_ALARM2_MINUTES                   (unsigned char)0x0D
#define PCF8563_ALARM2_HOURS                     (unsigned char)0x0E
#define PCF8563_ALARM2_WEEKDAYS                  (unsigned char)0x0F

#define PCF8563_ALARM_ENABLES                    (unsigned char)0x10

#define PCF8563_ADDR_PIN_CON                     (unsigned char)0x27

#define PCF8563_ADDR_FUN                         (unsigned char)0x28
#define PCF8563_ADDR_INTA_CON                    (unsigned char)0x29

#define PCF8563_ADDR_RAM_BYTE                    (unsigned char)0x2C

#define PCF8563_ADDR_FLAGS                       (unsigned char)0x2B

#define PCF8563_ADDR_STOP_EN                     (unsigned char)0x2E
#define PCF8563_ADDR_RESET_REG                   (unsigned char)0x2F

#define ALARM_ID_ZERO            0x00U
#define ALARM_ID_ONE             0x01U
#define ALARM_ID_TWO             0x02U
#define ALARM_VALUES             0x60U

#define FUNCTION_DATA_VALUE      0x80U
#define LOW_ONE_BITS_ENABLE      0x01U
#define LOW_TWO_BITS_ENABLE      0x03U
#define LOW_THREE_BITS_ENABLE    0x07U
#define LOW_FOUR_BITS_ENABLE     0x0fU
#define LOW_FIVE_BITS_ENABLE     0x1fU
#define LOW_SIX_BITS_ENABLE      0x3fU
#define LOW_SEVEN_BITS_ENABLE    0x7fU
#define HIGH_EIGHT_BITS_ENABLE   0xff00U
#define HIGH_FOUR_BITS_ENABLE    0xf0U
#define HIGH_THREE_BITS_ENABLE   0xe0U

#define PCF_ALA1_REGISTER_ENABLE 0x20U
#define PCF_ALA2_REGISTER_ENABLE 0x40U
#define PCF_CLK_REGISTER_ENABLE  0x82U
#define PCF_ALARM_ONE_REGISTER_ENABLE  0x90U 
#define PCF_ALARM_TWO_REGISTER_ENABLE  0x88U

#define SECOND_MAX  59
#define MINUTE_MAX  59
#define HOUR_MAX    23
#define DAY_MAX     31
#define DAY_MIN     1
#define MONTH_MAX   12
#define MONTH_MIN   1
#define WEEKDAY_MAX 6
#define YEAR_MAX    2099
#define YEAR_MIN    2000
#define YEAR_BASE   2000
#define TEN_MAX     99
#define KILO_MAX    99
#define H_KILO_MAX 99

#define TIMES_TEN   0x0aU

#define RTC_YEARS_MAX_NUMBER    99U
#define RAM_FLAG_VALUE          0xa5U


#define PCF8563_HOURS_00_xx_xx                   (unsigned char)0x05
#define PCF8563_HOURS_xx_00_xx                   (unsigned char)0x04
#define PCF8563_HOURS_xx_xx_00                   (unsigned char)0x03

#define PCF8563_SECOND_ALM1                      (unsigned char)0x08
#define PCF8563_MINUTE_ALM1                      (unsigned char)0x09
#define PCF8563_HR_xx_xx_00_ALM1                 (unsigned char)0x0A
#define PCF8563_HR_xx_00_xx_ALM1                 (unsigned char)0x0B
#define PCF8563_HR_00_xx_xx_ALM1                 (unsigned char)0x0C
#define PCF8563_MINUTE_ALM2                      (unsigned char)0x0D
#define PCF8563_HR_xx_xx_00_ALM2                 (unsigned char)0x0E
#define PCF8563_HR_xx_00_xx_ALM2                 (unsigned char)0x0F
#define SW_MODE                                  (unsigned char)0x10    
#define RTC_MODE                                 (unsigned char)0x00    

/**@brief RTC operation return status type
*/
typedef enum {
	RTC_OP_SUCCESS,
	RTC_OP_COMMUNICATION_FAILURE,
	RTC_OP_INVALID_DATE_TIME
} rtc_op_return_type_t;

typedef struct {
	uint16_t millis;
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t day_of_month;
	uint8_t month;
	uint16_t year;
} RTC_DATE_TIME_T;

typedef struct {
	uint16_t millis;
	uint8_t second;
	uint8_t minute;
	uint8_t ten;
	uint8_t kilo;
	uint8_t hundredkilo;
} RTC_COUNT_TIME_T;

extern uint8_t pcf85263_write_data(uint8_t addr, uint8_t *write_buf, uint8_t len);
extern rtc_op_return_type_t pcf85263_query_time(RTC_COUNT_TIME_T *count_time);
extern rtc_op_return_type_t pcf85263_set_time(RTC_COUNT_TIME_T count_time);
extern rtc_op_return_type_t  pcf85263_check_inta(void);
extern rtc_op_return_type_t pcf85263_clear_alarm_flag(void);
extern rtc_op_return_type_t pcf85263_set_sw_mode_alarm(uint32_t alarm_id , RTC_COUNT_TIME_T count_time);
#endif 


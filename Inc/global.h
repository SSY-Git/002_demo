
#ifndef __global_H
#define __global_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
#define I_ZERO      0
#define V_ZERO      0
#define V_ONE      0x01
#define V_TWO      0x02
#define V_THREE      0x03
#define V_FOUR      0x04
#define V_FIVE      0x05
#define V_SEVEN     0x07
#define V_EIGHT     0x08
#define V_NINE     0x09
#define V_TEN     0x0a
#define V_SIX      0x06
#define V_ELEVEN     0x0b
#define V_TWELVE     0x0c
#define V_SIXTEEN     0x10
#define V_SEVENTEEN     0x11
#define V_EIGHTTEEN     0x12
#define V_TWENTY     0x14
#define V_FIFTEEN     0x0F
#define V_TWENTY_THREE     0x17
#define V_NINETEEN     0x13
#define L_ZERO      0
#define L_ONE     0x01
#define L_TWO     0x02
#define L_THREE     0x03
#define L_FOUR     0x04
#define L_FIVE     0x05
#define L_SIX     0x06
#define L_SEVEN     0x07
#define L_EIGHT     0x08
#define L_NINE     0x09
#define L_TEN     0x0a
#define L_ELEVEN     0x0b
#define L_TWELVE     0x0c
#define L_THIRTEEN     0x0d
#define L_FOURTEEN     0x0e
#define L_FIVETEEN     0x0f
#define L_SIXTEEN     0x10
#define L_SEVENTEEN     0x11
#define L_EIGHTEEN     18
#define L_NINETEEN     19
#define L_TWENTY     20
#define L_TWENTY_ONE     21
#define L_TWENTY_TWO     22
#define L_TWENTY_FOUR     24
#define L_FORTY_ONE    41
#define L_FORTY_THREE     43
#define MAX_DOOR_NUM     0x18
#define START_OPEN_DOOR_FLAG     0x01
#define FINISH_OPEN_DOOR_FLAG     0x00
#define EXTEND_IC_CTRL_MAX_LOCK_NUM     0x06
#define MAX_RECEIVE_DATA     255
//#define UINT8_MAX     0xff

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
typedef enum {
	LOCK_CTRL_OK,
	LOCK_CTRL_FIALED,
    LOCK_NUM_ERR
} lock_ctrl_return_type_t;

typedef enum{
    USART_PROTOCOL_OK = 1,
    USART_PROTOCOL_HEAD_ERR,
    USART_PROTOCOL_LEN_ERR,
    USART_PROTOCOL_DATA_NULL
}usart_protocol;
/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/


/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

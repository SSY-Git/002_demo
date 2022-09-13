/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __W25Q64_H
#define __W25Q64_H
#include "spi.h"
#include "stdint.h"


#define SPI1_CS_EN   HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_RESET);
#define SPI1_CS_DIS  HAL_GPIO_WritePin(SPI_CS_GPIO_Port, SPI_CS_Pin, GPIO_PIN_SET);

void MX_SPI2_Init(void);

#define W25Q_TIMEOUT 1000


#define  APP_STORYFLAG             0x5555
#define  APP_STORY_ADDRESS         0x100
#define  CARBINET_STORYFLAG    		 0xBD
#define  CARBINET_STORY_ADDRESS    0x21000

#define ERROR_FLAG_ADDRESS         0x22000

#define HighSpeedReadData             0x0B
#define ReadData                      0x03
#define SectorErace_4KB               0x20
#define Page_Program                  0x02
#define ReadStatusRegister            0x05
#define EnableWriteStatusRegister     0x50
#define WriteStatusRegister           0x01
#define WriteEnable                   0x06
#define WriteDisable                  0x04
#define Dummy_Byte                    0xA5


void W25Q16_HighSpeedBufferRead(uint32_t ReadAddr, uint8_t *pBuffer,  uint16_t Count);
void W25Q16_SectorErase_4KByte(uint32_t Addr);
void W25Q16_PageWrite(uint32_t WriteAddr, uint8_t *WritePageBuf, uint16_t Count );
uint16_t W25Q16_ReadID(void);
uint8_t Spi_Flash_TransmitReveive(uint8_t wr_dat);
uint8_t SPI_Flash_SendByte(uint8_t Data);
void GetFlashIDTask(void const * argument);
#endif /*__ adc_H */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

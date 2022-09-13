/* Includes ------------------------------------------------------------------*/
#include "w25q64.h"
#include "global.h"

SPI_HandleTypeDef hspi1;


uint8_t Spi_Flash_TransmitReveive(uint8_t wr_dat)
{
	uint8_t wr_buf[2] = { 0 };
	uint8_t rd_buf[2] = { 0 };
	
	wr_buf[0] =  wr_dat;
	HAL_SPI_TransmitReceive(&hspi1, wr_buf, rd_buf, 1, W25Q_TIMEOUT);
  return rd_buf[0];
}




uint8_t SPI_Flash_SendByte(uint8_t Data)
{ 
	  uint8_t ret = 0;;
		ret = Spi_Flash_TransmitReveive(Data);
    return ret; 
} 
uint8_t SPI_Flash_ReceiveByte(void)
{
	uint8_t ret = 0;
	ret = Spi_Flash_TransmitReveive(0xFF);
	return ret;
}

uint8_t W25Q16_ReadStatusRegister(void)
{
  uint8_t StatusRegister = 0;
  SPI1_CS_EN;
  Spi_Flash_TransmitReveive(ReadStatusRegister);
  StatusRegister = Spi_Flash_TransmitReveive(0xFF);
  SPI1_CS_DIS;
  return StatusRegister;
}


void W25Q16_WriteEnable(void)
{
  SPI1_CS_EN;
  SPI_Flash_SendByte(WriteEnable);
  SPI1_CS_DIS;
}
void W25Q16_WriteDisable(void)
{
  SPI1_CS_EN;
  SPI_Flash_SendByte(WriteDisable);
  SPI1_CS_DIS;
}
void SPI1_CS_ENableWriteStatusRegister(void)
{
  SPI1_CS_EN;
  SPI_Flash_SendByte(EnableWriteStatusRegister);
  SPI1_CS_DIS;
}
void W25Q16_WriteStatusRegister(uint8_t Byte)
{
  SPI1_CS_EN;
  SPI_Flash_SendByte(WriteStatusRegister);
  SPI_Flash_SendByte(Byte);
  SPI1_CS_DIS;
}
void W25Q16_WaitForWriteEnd(void)
{
  uint8_t FLASH_Status = 0;
	//FLASH_Status = W25Q16_ReadStatusRegister();
  do
  {
     FLASH_Status = W25Q16_ReadStatusRegister();
  }while(FLASH_Status & 0x01);
  // while(FLASH_Status & WriteStatusRegister);
}

void W25Q16_HighSpeedBufferRead(uint32_t ReadAddr, uint8_t *pBuffer,  uint16_t Count)
{
	
  SPI1_CS_EN;
  SPI_Flash_SendByte(HighSpeedReadData);
  SPI_Flash_SendByte((ReadAddr & 0xFF0000) >> 16);
  SPI_Flash_SendByte((ReadAddr& 0xFF00) >> 8);
  SPI_Flash_SendByte(ReadAddr & 0xFF);
  SPI_Flash_SendByte(Dummy_Byte);
  while(Count--)
  {
    *pBuffer = SPI_Flash_ReceiveByte();
     pBuffer++;
  }
  SPI1_CS_DIS;
}


uint8_t W25Q16_HighSpeedRead(uint32_t ReadAddr)
{
  uint32_t Temp = 0;
  SPI1_CS_EN;
  SPI_Flash_SendByte(HighSpeedReadData);
  SPI_Flash_SendByte((ReadAddr & 0xFF0000) >> 16);
  SPI_Flash_SendByte((ReadAddr& 0xFF00) >> 8);
  SPI_Flash_SendByte(ReadAddr & 0xFF);
  SPI_Flash_SendByte(Dummy_Byte);
  Temp = SPI_Flash_ReceiveByte();
  SPI1_CS_DIS;
  return Temp;
}

void W25Q16_PageWrite(uint32_t WriteAddr, uint8_t *WritePageBuf, uint16_t  Count )
{
	uint8_t i;
	W25Q16_WriteEnable();
	SPI1_CS_EN;
  SPI_Flash_SendByte(Page_Program);
  //SPI_Flash_SendByte((WriteAddr & 0xFF0000) >> 16);
	SPI_Flash_SendByte((WriteAddr) >> 16);
  //SPI_Flash_SendByte((WriteAddr & 0xFF00) >> 8);
	SPI_Flash_SendByte((WriteAddr) >> 8);
  SPI_Flash_SendByte(WriteAddr);  
  for(i = 0; i < Count; i++)
  {
     Spi_Flash_TransmitReveive(WritePageBuf[i]);

  }
  SPI1_CS_DIS;
  W25Q16_WaitForWriteEnd();	
}

/*********************锟斤拷取ID*********************/
//ID:EF14
uint16_t W25Q16_ReadID(void)
{
  uint16_t ID = 0;
	SPI1_CS_EN;
	Spi_Flash_TransmitReveive(0x90);//锟斤拷取ID锟斤拷锟斤拷
	Spi_Flash_TransmitReveive(0xFF);
	Spi_Flash_TransmitReveive(0xFF);
	Spi_Flash_TransmitReveive(0xFF);
	ID = Spi_Flash_TransmitReveive(0xFF) << 8;
	ID += Spi_Flash_TransmitReveive(0xFF);
  SPI1_CS_DIS;
	return ID;
}


void W25Q16_SectorErase_4KByte(uint32_t Addr)
{
  W25Q16_WriteEnable();
  SPI1_CS_EN;
  SPI_Flash_SendByte(SectorErace_4KB);
  SPI_Flash_SendByte((Addr & 0xFF0000) >> 16);
  SPI_Flash_SendByte((Addr & 0xFF00) >> 8);
  SPI_Flash_SendByte(Addr & 0xFF);
  SPI1_CS_DIS;
  W25Q16_WaitForWriteEnd();
}

void W25QXX_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)   
{ 
 	uint16_t i;   										    
	SPI1_CS_EN;                            	//使锟斤拷锟斤拷锟斤拷   
	SPI_Flash_SendByte(ReadData);         	//锟斤拷锟酵讹拷取锟斤拷锟斤拷   
	SPI_Flash_SendByte((uint8_t)((ReadAddr)>>16));  	//锟斤拷锟斤拷24bit锟斤拷址    
	SPI_Flash_SendByte((uint8_t)((ReadAddr)>>8));   
	SPI_Flash_SendByte((uint8_t)ReadAddr);   
	for(i=0;i<NumByteToRead;i++)
	{ 
			pBuffer[i]=SPI_Flash_SendByte(0XFF);   	//循锟斤拷锟斤拷锟斤拷  
	}
	SPI1_CS_DIS;  				    	      
}
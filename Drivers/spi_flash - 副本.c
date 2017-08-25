#include "config.h"
#include "spi_flash.h"


uint8_t TempData[6] = {0};

void SPI_FLASH_WaitBusy(void);
void SPI_FLASH_EraseSector(u32 Dst_Addr);

/* for test */
void SPI_FLASH_Test(void)
{
	memset(TempData, 0xAA, 6);
//	SPI_FLASH_Write(TempData, 0x00, 6);
//	memset(TempData, 0x00, 6);
//	SPI_FLASH_Read(TempData, 0x00, 6);
}

/* SPI FLASH初始化 */
void SPI_FLASH_Init(void)
{
	CLK_PeripheralClockConfig(CLK_Peripheral_SPI2, ENABLE);
	
	/* SPI_CLOCK:PG5, SPI_MOSI: PG6, SPI_MISO: PG7 */
	GPIO_Init(GPIOG, GPIO_Pin_5, GPIO_Mode_Out_PP_High_Fast);
	GPIO_Init(GPIOG, GPIO_Pin_6, GPIO_Mode_Out_PP_High_Fast);
	
	/* 主机模式，配置为输入 */
	GPIO_Init(GPIOG, GPIO_Pin_7, GPIO_Mode_In_PU_No_IT);
	
	/* 初始化SPI */
	SPI_Init(SPI2, SPI_FirstBit_MSB, SPI_BaudRatePrescaler_4, SPI_Mode_Master,\
			SPI_CPOL_High, SPI_CPHA_2Edge, \
			SPI_Direction_2Lines_FullDuplex, SPI_NSS_Soft, 0x07);
    
	SPI_Cmd(SPI2, ENABLE);
	
	/* 配置CS管脚 */
	GPIO_Init(SPI_CS_GPIO, SPI_CS_PIN, GPIO_Mode_Out_PP_High_Fast);
	SPI_CS_OFF();
	
	SPI_FLASH_Test();
}

/* SPI FLASH 读写一个字节 */
uint8_t SPI_ReadWriteByte(uint8_t byte)
{
	/* Loop while DR register in not emplty */
	while (SPI_GetFlagStatus(SPI2, SPI_FLAG_TXE) == RESET);
	/* Send byte through the SPI1 peripheral */
	SPI_SendData(SPI2, byte);
	/* Wait to receive a byte */
	while (SPI_GetFlagStatus(SPI2, SPI_FLAG_RXNE) == RESET);
	/* Return the byte read from the SPI bus */
	return SPI_ReceiveData(SPI2);
 }

/* 读SPI FLASH状态寄存器 */
uint8_t SPI_FLASH_ReadSR(void)
{  
	uint8_t value = 0;
	
	SPI_CS_ON();
	SPI_ReadWriteByte(W25X_ReadStatusReg);
	value = SPI_ReadWriteByte(0Xff);
	SPI_CS_OFF();
	
	return value;
}

/* 写SPI状态寄存器 */
void SPI_FLASH_WriteSR(uint8_t SRValue)
{   
	SPI_CS_ON();
	SPI_ReadWriteByte(W25X_WriteStatusReg);    
	SPI_ReadWriteByte(SRValue); 
	SPI_CS_OFF();
}

/* SPI_FLASHD写使能 */
void SPI_FLASH_WriteEnable(void)   
{
	SPI_CS_ON();
    SPI_ReadWriteByte(W25X_WriteEnable);
	SPI_CS_OFF();
}

/* SPI_FLASHD写禁止 */
void SPI_FLASH_WriteDisable(void)   
{  
	SPI_CS_ON();
    SPI_ReadWriteByte(W25X_WriteDisable);
	SPI_CS_OFF();
} 

/* 读芯片ID */
uint16_t SPI_FLASH_ReadID(void)
{
	uint16_t DeviceID = 0;
	
    SPI_CS_ON();
	
    /* Send read id instruction */  
    SPI_ReadWriteByte(0x90);
    SPI_ReadWriteByte(0X00);
    SPI_ReadWriteByte(0X00);
    SPI_ReadWriteByte(0X00);
	
    /* Read a byte from the FLASH */
    DeviceID = (SPI_ReadWriteByte(0xFF)<<8);  
    DeviceID |= SPI_ReadWriteByte(0xFF);
	
    SPI_CS_OFF();
    
	return DeviceID; 
}

// 从SPI FLASH指定地址读取指定长度的数据
// pBuffer - 数据存储区
// ReadAddr - 读数据起始地址
// NumByteToRead - 要读取的数据长度
void SPI_FLASH_Read(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)   
{ 
 	uint16_t i;
	
	SPI_CS_ON();
	
    SPI_ReadWriteByte(W25X_ReadData);
    SPI_ReadWriteByte((uint8_t)((ReadAddr)>>16));
    SPI_ReadWriteByte((uint8_t)((ReadAddr)>>8));
    SPI_ReadWriteByte((uint8_t)ReadAddr);
    for(i = 0; i< NumByteToRead; i++)
	{
    	pBuffer[i] = SPI_ReadWriteByte(0XFF);
    }
	
	SPI_CS_OFF();
}

/* SPI FLASH在一页（0~65535）内写入少于256个字节的数据 */
void SPI_FLASH_WritePage(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
 	u16 i;
	
    SPI_FLASH_WriteEnable();
	
	SPI_CS_ON();
	
    SPI_ReadWriteByte(W25X_PageProgram);
    SPI_ReadWriteByte((u8)((WriteAddr)>>16));
    SPI_ReadWriteByte((u8)((WriteAddr)>>8));
    SPI_ReadWriteByte((u8)WriteAddr);
	
    for(i = 0; i < NumByteToWrite; i++)
	{
		SPI_ReadWriteByte(pBuffer[i]);
	}
	
	SPI_CS_OFF();
	
	SPI_FLASH_WaitBusy();
}

/* 无效验写SPI（必须保证所写的地址数据为0xFF） */
void SPI_Flash_WriteNoCheck(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)   
{
	u16 pageremain;
	
	pageremain = 256 - WriteAddr%256;
	
	if (NumByteToWrite <= pageremain)
	{
		pageremain = NumByteToWrite;
	}
	
	while(1)
	{	   
		SPI_FLASH_WritePage(pBuffer, WriteAddr, pageremain);
		
		if (NumByteToWrite == pageremain)
		{
			break;
		}
	 	else  /* NumByteToWrite>pageremain */
		{
			pBuffer   += pageremain;
			WriteAddr += pageremain;
			NumByteToWrite -= pageremain;
			
			if (NumByteToWrite > 256)
			{
				pageremain = 256;
			}
			else
			{
				pageremain = NumByteToWrite;
			}
		}
	}  
}

/* SPI FLASH 在指定地方写入指定长度的数据 */ 		   
u8 SPI_FLASH_BUF[4096];
void SPI_FLASH_Write(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite)
{
	u16 i;
	u32 secpos;
	u16 secoff;
	u16 secremain;
 	
	secpos = WriteAddr/4096;
	secoff = WriteAddr%4096;
	secremain = 4096 - secoff;
	
	if (NumByteToWrite <= secremain)
	{
		secremain = NumByteToWrite;
	}
	
	while(1)
	{	
		SPI_FLASH_Read(SPI_FLASH_BUF, secpos*4096, 4096);
		for (i = 0; i < secremain; i++)
		{
			if (SPI_FLASH_BUF[secoff+i] != 0xFF)
			{
				break;
			}
		}
		
		if (i < secremain)
		{
			SPI_FLASH_EraseSector(secpos);
			
			for (i = 0; i < secremain; i++)
			{
				SPI_FLASH_BUF[i+secoff] = pBuffer[i];
			}
			
			SPI_Flash_WriteNoCheck(SPI_FLASH_BUF, secpos*4096, 4096); 
		}
		else
		{
			SPI_Flash_WriteNoCheck(pBuffer, WriteAddr, secremain);
		}
		
		if(NumByteToWrite == secremain)
		{
			break;
		}
		else
		{
			secpos++;
			secoff = 0;
		   	pBuffer += secremain;
			WriteAddr += secremain;
		   	NumByteToWrite -= secremain;
			if (NumByteToWrite > 4096)
			{
				secremain = 4096;
			}
			else
			{
				secremain = NumByteToWrite;
			}
		}
	}	 
}

/* 擦除整个芯片 */
void SPI_Flash_Erase_Chip(void)
{
    SPI_FLASH_WriteEnable();
    SPI_FLASH_WaitBusy();
  	SPI_CS_ON();
    SPI_ReadWriteByte(W25X_ChipErase);
	SPI_CS_OFF();      
	SPI_FLASH_WaitBusy();
}

/* 擦除一个扇区 */
void SPI_FLASH_EraseSector(u32 Dst_Addr)
{
	Dst_Addr *= 4096;
	
    SPI_FLASH_WriteEnable();
    SPI_FLASH_WaitBusy();
  	SPI_CS_ON();
    SPI_ReadWriteByte(W25X_SectorErase);
    SPI_ReadWriteByte((u8)((Dst_Addr)>>16));
    SPI_ReadWriteByte((u8)((Dst_Addr)>>8)); 
    SPI_ReadWriteByte((u8)Dst_Addr);
	SPI_CS_OFF();	      
    SPI_FLASH_WaitBusy();
}

/* 等待空闲 */
void SPI_FLASH_WaitBusy(void)
{   
	while((SPI_FLASH_ReadSR()&0x01) == 0x01);
}

/* 进入掉电模式 */
void SPI_FLASH_PowerDown(void)   
{ 
  	SPI_CS_ON();
    SPI_ReadWriteByte(W25X_PowerDown);
	SPI_CS_OFF();  	      
    Soft_delay_us(3);
}

/* 唤醒 */
void SPI_FLASH_WAKEUP(void)
{  
  	SPI_CS_ON();
    SPI_ReadWriteByte(W25X_ReleasePowerDown);
	SPI_CS_OFF();	      
    Soft_delay_us(3);
}

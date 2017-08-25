#ifndef __SPI_FLASH_H
#define	__SPI_FLASH_H


#define	SPI_CS_PIN			GPIO_Pin_4
#define	SPI_CS_GPIO			GPIOG
#define SPI_CS_ON()			GPIO_ResetBits(SPI_CS_GPIO, SPI_CS_PIN)
#define SPI_CS_OFF()		GPIO_SetBits(SPI_CS_GPIO, SPI_CS_PIN)

// W25Q32
#define FLASH_ID				0XEF15
// ÷∏¡Ó±Ì
#define W25X_WriteEnable		0x06
#define W25X_WriteDisable		0x04
#define W25X_ReadStatusReg		0x05
#define W25X_WriteStatusReg		0x01
#define W25X_ReadData			0x03
#define W25X_FastReadData		0x0B
#define W25X_FastReadDual		0x3B
#define W25X_PageProgram		0x02
#define W25X_BlockErase			0xD8
#define W25X_SectorErase		0x20
#define W25X_ChipErase			0xC7
#define W25X_PowerDown			0xB9
#define W25X_ReleasePowerDown	0xAB
#define W25X_DeviceID			0xAB
#define W25X_ManufactDeviceID	0x90
#define W25X_JedecDeviceID		0x9F

void SPI_FLASH_Init(void);
uint16_t SPI_FLASH_ReadID(void);
void SPI_FLASH_Write(u8* pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_Read(uint8_t *pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);

#endif

#ifndef __SPI_FLASH_H
#define	__SPI_FLASH_H


#define	METER_DAY_ADDR		(0x00)
#define	METER_MONTH_ADDR	(SAVE_DAY * sizeof(T_MeterData))

#define	SPI_CS_PIN			GPIO_Pin_4
#define	SPI_CS_GPIO			GPIOG
#define SPI_CS_ON()			GPIO_ResetBits(SPI_CS_GPIO, SPI_CS_PIN)
#define SPI_CS_OFF()		GPIO_SetBits(SPI_CS_GPIO, SPI_CS_PIN)


/* MB85RS256A¼Ä´æÆ÷¶¨Òå */
#define MB85RS256A_WREN_INST	0x06	/* ÉèÖÃÐ´Ê¹ÄÜËø´æÆ÷ */
#define MB85RS256A_WRDI_INST	0x04	/* Ð´½ûÖ¹ */
#define MB85RS256A_RDSR_INST	0x05	/* ¶Á×´Ì¬¼Ä´æÆ÷ */
#define MB85RS256A_WRSR_INST	0x01	/* Ð´×´Ì¬¼Ä´æÆ÷ */
#define MB85RS256A_READ_INST	0x03	/* ¶Á´æ´¢Æ÷Êý¾Ý */
#define MB85RS256A_WRITE_INST	0x02	/* Ð´´æ´¢Æ÷Êý¾Ý */
#define MB85RS256A_STATUS_REG	0x00
#define MB85RS256A_INIT_STATE	0x09
#define MB85RS256A_RDID_INST	0x9F	/* ¶ÁÆ÷¼þID */
#define MB85RS256A_SLEEP_INST	0xB9	/* Ë¯ÃßÄ£Ê½ */


void SPI_FLASH_Init(void);
void SPI_FLASH_Reset(void);
void SPI_FLASH_WriteByte(u8 data, u32 WriteAddr);
void SPI_FLASH_ReadByte(u8 *data, uint32_t ReadAddr);
void SPI_FLASH_WriteBuffer(u8 *pBuffer, u32 WriteAddr, u16 NumByteToWrite);
void SPI_FLASH_ReadBuffer(u8 *pBuffer, u32 ReadAddr, u16 NumByteToRead);

#endif

#ifndef __EEPROM_H
#define	__EEPROM_H


#define	EEPROM_BASE_ADDR		(0x1000)

void EEPROMInit(void);
void EEPROMReset(void);
uint8_t EEPROMReadByte(uint32_t addr);
void EEPROMWriteByte(uint32_t addr, uint8_t data);
void EEPROMReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint8_t NumByteToRead);
void EEPROMWriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint8_t NumByteToWrite);

#endif

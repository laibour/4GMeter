#include "config.h"
#include "eeprom.h"


void EEPROMInit(void)
{
	FLASH_DeInit();
	
#if 0
	memset(EEBuffer, 0xAA, sizeof(EEBuffer));
	
	EEPROMWriteBuffer(EEBuffer, 0x00, (uint8_t)sizeof(EEBuffer));
	
	memset(EEBuffer, 0x00, sizeof(EEBuffer));
	
	EEPROMReadBuffer(EEBuffer, 0x00, (uint8_t)sizeof(EEBuffer));
#endif
}


// set all eeprom memory zero
void EEPROMReset(void)
{
	uint8_t i;
	uint32_t WriteAddr = EEPROM_BASE_ADDR;
	
	FLASH_Unlock(FLASH_MemType_Data);
	
	for (i = 0; i < 255; i++)
	{
		FLASH_ProgramByte(WriteAddr+i, 0);
		FLASH_WaitForLastOperation(FLASH_MemType_Data);
	}
	
	FLASH_Lock(FLASH_MemType_Data);
}


void EEPROMWriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint8_t NumByteToWrite)
{
	uint8_t i;
	
	WriteAddr += EEPROM_BASE_ADDR;
	FLASH_Unlock(FLASH_MemType_Data);
	
	for (i = 0; i < NumByteToWrite; i++)
	{
		FLASH_ProgramByte(WriteAddr+i, *(pBuffer+i));
		FLASH_WaitForLastOperation(FLASH_MemType_Data);
	}
	
	FLASH_Lock(FLASH_MemType_Data);
}


void EEPROMReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint8_t NumByteToRead)
{
	uint8_t i;
	
	ReadAddr += EEPROM_BASE_ADDR;
	
	for (i = 0; i < NumByteToRead; i++)
	{
		*(pBuffer+i) = FLASH_ReadByte(ReadAddr+i);
		FLASH_WaitForLastOperation(FLASH_MemType_Data);
	}
}


void EEPROMWriteByte(uint32_t addr, uint8_t data)
{
	addr += EEPROM_BASE_ADDR;
	
	FLASH_Unlock(FLASH_MemType_Data);
	
	FLASH_ProgramByte(addr, data);
	FLASH_WaitForLastOperation(FLASH_MemType_Data);
	
	FLASH_Lock(FLASH_MemType_Data);
}


uint8_t EEPROMReadByte(uint32_t addr)
{
	uint8_t  data;
	
	addr += EEPROM_BASE_ADDR;
	data = FLASH_ReadByte(addr);
	
	return data;
}

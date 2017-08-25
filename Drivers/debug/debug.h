#ifndef __DEBUG_H
#define __DEBUG_H


#define DBG_PRINTF	printf			/* ¿ªÆô´òÓ¡ºê */
//#define DBG_PRINTF(...)			/* ¹Ø±Õ´òÓ¡ºê */

#define DATA_PRINTF	DataPrint		/* ¿ªÆô´òÓ¡ºê */
//#define INFO_PRINTF(...)			/* ¹Ø±Õ´òÓ¡ºê */

void DebugInit(void);
void DataPrint(uint8_t *data, uint8_t len);
void DebugPrintData(uint8_t *chr, uint8_t num);

#endif
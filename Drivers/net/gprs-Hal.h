#ifndef __GPRS_HAL_H
#define __GPRS_HAL_H


void GPRSIOInit(void);
void USART3_Init(void);
void USART3_SendString(uint8_t *ch);
void USART3_SendData(uint8_t *chr, uint8_t num);

#endif
#ifndef __LCD_H
#define __LCD_H


void LCDTest(void);
void LCDInit(void);
void LCDClear(void);
void FlushDataToLCD(PT_DispData ptDispData);
void DispNumber(uint8_t position, uint8_t num);

#endif

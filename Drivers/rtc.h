#ifndef __RTC_H
#define __RTC_H


#define isPrime(year) ((year%4==0 && year%100!=0) || (year%400==0))

void RTCInit(void);
void RTCReset(void);
void RTCTaskProcess(void);
void RTCRead(PT_SystemTime ptSystemTime);
void RTCWrite(PT_SystemTime ptSystemTime);

#endif
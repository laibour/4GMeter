#ifndef __TIMER_H
#define __TIMER_H


#define	TIMER_MAX_COUNT		(2000000000)

extern volatile uint32_t TimeCnt;

void TimerInit(void);
uint32_t GetTimer(uint32_t LastTime);

#endif
#ifndef __COUNTER_H
#define __COUNTER_H


#define	IN_SAMP_SUM		(30)
#define	SAMP_GPIO		GPIOA
#define	EX_SAMP_PIN		GPIO_Pin_2
#define	IN_SAMP_PIN		GPIO_Pin_7
#define	MAGNET_PIN		GPIO_Pin_3

void FlowProcess(void);
void CounterInit(void);
bool GetMagnetPort(void);
void CounterProcess(void);
uint8_t GetFaultNumber(void);
void SetFaultNumber(uint8_t data);

#endif

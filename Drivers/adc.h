#ifndef __ADC_H
#define __ADC_H


#define VREF 				(3.3L)
#define	VOL_STAND			(1.1L)  // 3.6V
#define ADC_CONV 			(4096)

#define	ADSWITCH_PIN		GPIO_Pin_6
#define	ADSWITCH_GPIO		GPIOE
#define ADSWITCH_ON()		GPIO_SetBits(ADSWITCH_GPIO, ADSWITCH_PIN)
#define ADSWITCH_OFF()		GPIO_ResetBits(ADSWITCH_GPIO, ADSWITCH_PIN)


void ADCInit(void);
bool ADCAtOnce(void);
void ADCConvert(void);
void ADCProcess(void);
uint8_t GetBatStatus(void);

#endif

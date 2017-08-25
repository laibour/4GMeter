#ifndef __BEEP_H
#define __BEEP_H


// BEEP PORT
#define	BEEP_PIN		GPIO_Pin_3
#define	BEEP_GPIO		GPIOG
#define BEEP_ON()		GPIO_SetBits(BEEP_GPIO, BEEP_PIN)
#define BEEP_OFF()		GPIO_ResetBits(BEEP_GPIO, BEEP_PIN)
#define	BEEP_TOGGLE()	GPIO_ToggleBits(BEEP_GPIO, BEEP_PIN)


extern bool BeepStart;
extern uint16_t BeepTime;


void BeepInit(void);
void BeepSpeak(void);

#endif

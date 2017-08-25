#ifndef __KEY_H
#define __KEY_H


#define	KEY_NUM			3		/* 最多识别按键连击次数 */

#define	KEY_PIN			GPIO_Pin_1
#define	KEY_GPIO		GPIOC


typedef enum
{
	KEY_IDLE,
    KEY_BUSY,
    KEY_DONE,
}tKeyReturnCodes;


typedef enum
{
	KEY_STATE_IDLE,
	KEY_STATE_INIT,
	KEY_STATE_READ,
	KEY_STATE_300MS,
	KEY_STATE_FULL,
	KEY_STATE_500MS,
}tKeyStates;


void KeyInit(void);
void KeyProcess(void);

#endif

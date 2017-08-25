#ifndef __BATTERY_H
#define	__BATTERY_H


#define	BAT_PIN			GPIO_Pin_0
#define	BAT_GPIO		GPIOC

void BatInit(void);
void BatCheckProcess(void);

#endif

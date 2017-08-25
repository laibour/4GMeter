#include "config.h"
#include "beep.h"


bool BeepStart = TRUE;
uint16_t BeepTime = 80;

void BeepSpeak(void)
{
	static uint32_t BeepTimer;
	
	if (BeepTime != 0)
	{
		if (BeepStart == TRUE)
		{
			BEEP_ON();
			BeepStart = FALSE;
			BeepTimer = TimeCnt;
		}
		else if (GetTimer(BeepTimer) > BeepTime)
		{
			BEEP_OFF();
			BeepTime  = 0;
			BeepStart = TRUE;
		}
	}
	else
	{
		BEEP_OFF();
		BeepStart = TRUE;
	}
}


void BeepInit(void)
{
    GPIO_Init(BEEP_GPIO, BEEP_PIN, GPIO_Mode_Out_PP_High_Slow);
	
    BEEP_OFF();
}

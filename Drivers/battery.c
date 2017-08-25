#include "config.h"
#include "battery.h"


static uint32_t BatTimer;

void BatInit(void)
{
	EXTI_ClearITPendingBit(EXTI_IT_Pin0);
	GPIO_Init(BAT_GPIO, BAT_PIN, GPIO_Mode_In_FL_IT);
	EXTI_SetPinSensitivity(EXTI_Pin_0, EXTI_Trigger_Falling);
}

void BatCheckProcess(void)
{
	if (WakeupState.BatStatus==TRIG_ACTIVE && GetTimer(BatTimer)>=1)
	{
		if (!GPIO_ReadInputDataBit(BAT_GPIO, BAT_PIN))
		{
			CloseMotor(0);
			DBG_PRINTF("The battery power down! \n");
			WakeupState.BatStatus = TRIG_SLEEP;
		}
	}
}

INTERRUPT_HANDLER(EXTI0_IRQHandler, 8)
{
	BatTimer = TimeCnt;
	WakeupState.BatStatus = TRIG_ACTIVE;
	EXTI_ClearITPendingBit(EXTI_IT_Pin0);
}

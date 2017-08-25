#include "config.h"
#include "timer.h"


volatile uint32_t TimeCnt = 0;

// timer4
void TimerInit(void)
{
//	TIM4_SetCounter(0);
	
    CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);
    
    TIM4_DeInit();
    
    TIM4_TimeBaseInit(TIM4_Prescaler_2048, 78);	// 定时器计算1/16m*2048*78 == 10ms
	
	TIM4_ClearITPendingBit(TIM4_IT_Update);		// 清除中断标志
    
    TIM4_ITConfig(TIM4_IT_Update, ENABLE);      // 使能溢出中断
    
    TIM4_Cmd(ENABLE);							// 开启定时器
}


uint32_t GetTimer(uint32_t LastTime)
{
    uint32_t ret;

	ret = TimeCnt >= LastTime? TimeCnt - LastTime : TimeCnt + TIMER_MAX_COUNT - LastTime;

	return ret;
}


INTERRUPT_HANDLER(TIM4_UPD_OVF_TRG_IRQHandler, 25)
{
	TIM4_ClearITPendingBit(TIM4_IT_Update);

	TimeCnt++;
	if (TimeCnt >= TIMER_MAX_COUNT)
	{
		TimeCnt = 0;
	}
	
//	GPIO_ToggleBits(GPIOA, GPIO_Pin_2);
}

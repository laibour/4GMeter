#include "config.h"
#include "stm8l15x.h"
#include "low_power.h"


static void RTCTimeRead(void);

void main(void)
{
	BoardInit();
	
	enableInterrupts();
	
	DBG_PRINTF("The program is running... \n");
	
    while (1)
    {
		RTCTimeRead();
		
		MotorProcess();
		
		KeyProcess();
		
		CounterProcess();
		
		RTCTaskProcess();
		
		NetTaskProcess();
		
		BatCheckProcess();
		
		BeepSpeak();
		
		LowPower();
    }
}


static uint32_t RTCTimer = 0;

void RTCTimeRead(void)
{
	// read system time per 200ms
	if (GetTimer(RTCTimer) >= 20)
	{
		RTCTimer = TimeCnt;
		RTCRead(&SystemTime);
	}
}

// ÷ÿ∂®Œªprintf
int putchar(int ch)
{
  	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET);
	USART_SendData8(USART1, (uint8_t)ch);
	
	return ch;
}


#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/********************************END OF FILE***********************************/

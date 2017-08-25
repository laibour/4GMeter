#include "config.h"
#include "debug.h"


void DataPrint(uint8_t *data, uint8_t len)
{
  	uint8_t i;
	
	for (i = 0; i< len; i++)
	{
		printf("%02X ", *(data+i));
	}
	printf("\n");
}

/* 打印串口初始化 */
void DebugInit(void)
{
    USART_DeInit(USART1);	/* 将寄存器的值复位 */ 
    
	GPIO_Init(GPIOC, GPIO_Pin_2, GPIO_Mode_In_PU_No_IT);        // 串口接收
	GPIO_Init(GPIOC, GPIO_Pin_3, GPIO_Mode_Out_PP_High_Slow);   // 串口发送

	GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_2, ENABLE);       // 使能软件上拉
	GPIO_ExternalPullUpConfig(GPIOC, GPIO_Pin_3, ENABLE);       // 使能软件上拉
    
    CLK_PeripheralClockConfig(CLK_Peripheral_USART1, ENABLE);   // 配置串口时钟

	USART_Init(USART1, (u32)115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No, \
			  (USART_Mode_TypeDef)(USART_Mode_Tx | USART_Mode_Rx)); 

    USART_ClearITPendingBit(USART1, USART_IT_RXNE);

//  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); 

	USART_Cmd(USART1, ENABLE);
}

#if 0
INTERRUPT_HANDLER(USART1_RX_TIM5_CC_IRQHandler, 28)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
    {
        TempBuff[TempPoint++] = USART_ReceiveData8(USART1);
		if (TempPoint == 50)
		{
			TempPoint = 0;
		}

        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }

	if (USART_GetFlagStatus(USART1, USART_FLAG_OR) == SET)
	{
		USART_ClearFlag(USART1, USART_FLAG_OR);  // 清除OR
//		USART_ReceiveData8(USART1); 			 // 读DR
	}
}
#endif

#include "config.h"
#include "gprs.h"
#include "gprs-Hal.h"


void USART3_Init(void)
{
    SYSCFG_REMAPDeInit();
    SYSCFG_REMAPPinConfig(REMAP_Pin_USART3TxRxPortF, ENABLE);   // USART3映射到 PF0 PF1
    
	GPIO_Init(GPIOF, GPIO_Pin_1, GPIO_Mode_In_PU_No_IT);        // 串口接收
	GPIO_Init(GPIOF, GPIO_Pin_0, GPIO_Mode_Out_PP_High_Slow);   // 串口发送

	GPIO_ExternalPullUpConfig(GPIOF, GPIO_Pin_0, ENABLE);       // 使能软件上拉
	GPIO_ExternalPullUpConfig(GPIOF, GPIO_Pin_1, ENABLE);       // 使能软件上拉
    
    USART_DeInit(USART3);	/* 将寄存器的值复位 */

    CLK_PeripheralClockConfig(CLK_Peripheral_USART3, ENABLE);   // 配置串口时钟 

	USART_Init(USART3, (u32)115200, USART_WordLength_8b, USART_StopBits_1, USART_Parity_No, \
			  (USART_Mode_TypeDef)(USART_Mode_Tx | USART_Mode_Rx)); 

    USART_ClearITPendingBit(USART3, USART_IT_IDLE);

    USART_ITConfig(USART3, USART_IT_IDLE, ENABLE);

	USART_Cmd(USART3, ENABLE);
	
	/* Enable DMA1 clock */
	CLK_PeripheralClockConfig(CLK_Peripheral_DMA1, ENABLE);

	DMA_Init(DMA1_Channel2,(uint32_t)&GPRSRecvBuffer,
			(uint16_t)(&USART3->DR),
		    GPRS_BUFFER_SIZE,
		    DMA_DIR_PeripheralToMemory,
		    DMA_Mode_Normal,
		    DMA_MemoryIncMode_Inc,
		    DMA_Priority_High,
		    DMA_MemoryDataSize_Byte);

	/* DMA1 Channel2 enable */
	DMA_Cmd(DMA1_Channel2, ENABLE);

	/* Enable DMA1 channel2 Transfer complete interrupt */
//	DMA_ITConfig(DMA1_Channel2, DMA_ITx_TC, ENABLE);
	
	USART_DMACmd(USART3, USART_DMAReq_RX, ENABLE); 

	/* DMA enable */
	DMA_GlobalCmd(ENABLE);
}


void USART3_SendString(uint8_t *chr)
{
	while(*chr != 0)
	{
	  	while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
        USART_SendData8(USART3, *chr);
		chr++;
	}
}


void USART3_SendData(uint8_t *chr, uint8_t num)
{
	while(num != 0)
	{
		while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET);
		USART_SendData8(USART3, *chr);
		chr++;
		num--;
	}
}


void GPRSIOInit(void)
{
    GPIO_DeInit(GPRS_POWER_GPIO);
    GPIO_DeInit(GPRS_PWRKEY_GPIO);
    
    GPIO_Init(GPRS_POWER_GPIO, GPRS_POWER_PIN, GPIO_Mode_Out_PP_High_Slow);
	
    GPIO_Init(GPRS_PWRKEY_GPIO, GPRS_PWRKEY_PIN, GPIO_Mode_Out_PP_High_Slow);
	
	GPRS_POWER_OFF();
	GPRS_PWRKEY_OFF();
}

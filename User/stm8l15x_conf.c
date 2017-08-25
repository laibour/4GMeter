#include "config.h"
#include "common.h"

void SystemClockConfig(void)
{
	CLK_DeInit();

	CLK_HSICmd(ENABLE);

	CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);

	CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_1);
    
    while(CLK_GetFlagStatus(CLK_FLAG_HSIRDY)== RESET);

	CLK_ClockSecuritySystemEnable();
	
	Soft_delay_ms(1000);
}

static void GPIODeIint(void)
{
	EXTI_DeInit();
	
	GPIO_DeInit(GPIOA);
	
	GPIO_DeInit(GPIOC);
	
	GPIO_DeInit(GPIOE);
	
	GPIO_DeInit(GPIOG);
}

void BoardInit(void)
{
	SystemClockConfig();
	
	GPIODeIint();
	
	DebugInit();
	
	TimerInit();
	
	RTCInit();
	
	KeyInit();
    
	NetInit();
	
	LCDInit();
	
	ADCInit();
	
	EEPROMInit();
	
	SPI_FLASH_Init();
	
	SetMeterParm();
	
	MotorInit();
	
	CounterInit();
	
	BeepInit();
	
	BatInit();
}

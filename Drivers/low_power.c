#include "config.h"
#include "low_power.h"
#include "counter.h"


static void HaltOffDevice(void);
static void BAT_WakeupConfig(void);
static void KEY_WakeupConfig(void);
static void Count_WakeupConfig(void);
static void RTC_WakeupConfig(uint16_t time);

void LowPower(void)
{
	if (WakeupState.CountStatus == TRIG_ACTIVE)
	{
		if (CountState.InSampStatus == TRIG_SLEEP && \
			CountState.ExSampStatus == TRIG_SLEEP && \
			CountState.MagnetStatus == TRIG_SLEEP)
		{
			WakeupState.CountStatus = TRIG_SLEEP;
		}
	}
	
	if (WakeupState.KeyStatus   == TRIG_SLEEP && \
		WakeupState.RTCStatus   == TRIG_SLEEP && \
		WakeupState.GPRSStatus  == TRIG_SLEEP && \
		WakeupState.CountStatus == TRIG_SLEEP && \
		WakeupState.BatStatus   == TRIG_SLEEP && \
		WakeupState.MotorStatus	== TRIG_SLEEP && \
		BeepTime == 0)
	{
		WakeupState.LowPowerStatus = TRUE;
	}
	
	if (WakeupState.LowPowerStatus == TRUE)
	{
		Soft_delay_ms(1);
		
		HaltOffDevice();
		
		BAT_WakeupConfig();
		
		KEY_WakeupConfig();
		
		Count_WakeupConfig();
		
		RTC_WakeupConfig(0);   /* wake up once every second */
		
		enableInterrupts();
		
		PWR_FastWakeUpCmd(DISABLE);
		PWR_UltraLowPowerCmd(ENABLE);
		halt();	 // 进入停机模式
		
		SPI_FLASH_Init();
		WakeupState.LowPowerStatus = FALSE;
	}
}

void Count_WakeupConfig(void)
{
	GPIO_Init(SAMP_GPIO, EX_SAMP_PIN, GPIO_Mode_In_FL_IT);
	EXTI_SetPinSensitivity(EXTI_Pin_2, EXTI_Trigger_Rising_Falling);
	EXTI_ClearITPendingBit(EXTI_IT_Pin2);
	ITC_SetSoftwarePriority(EXTI2_IRQn, ITC_PriorityLevel_1);

	GPIO_Init(SAMP_GPIO, IN_SAMP_PIN, GPIO_Mode_In_FL_IT);
	EXTI_SetPinSensitivity(EXTI_Pin_7, EXTI_Trigger_Falling);
	EXTI_ClearITPendingBit(EXTI_IT_Pin7);
	ITC_SetSoftwarePriority(EXTI7_IRQn, ITC_PriorityLevel_1);
	
	GPIO_Init(SAMP_GPIO, MAGNET_PIN, GPIO_Mode_In_FL_IT);
	EXTI_ClearITPendingBit(EXTI_IT_Pin3);
	EXTI_SetPinSensitivity(EXTI_Pin_3, EXTI_Trigger_Falling);
	ITC_SetSoftwarePriority(EXTI3_IRQn, ITC_PriorityLevel_1);
}

void RTC_WakeupConfig(uint16_t time)
{
	RTC_WakeUpCmd(DISABLE);
	RTC_SetWakeUpCounter(time);
	RTC_WakeUpCmd(ENABLE);
}

void KEY_WakeupConfig(void)
{
	EXTI_ClearITPendingBit(EXTI_IT_Pin1);
	GPIO_Init(KEY_GPIO, KEY_PIN, GPIO_Mode_In_FL_IT);  // key for wakeup
	EXTI_SetPinSensitivity(EXTI_Pin_1, EXTI_Trigger_Falling);
}

void BAT_WakeupConfig(void)
{
	EXTI_ClearITPendingBit(EXTI_IT_Pin0);
	GPIO_Init(BAT_GPIO, BAT_PIN, GPIO_Mode_In_FL_IT);  // BAT power down for wakeup
	EXTI_SetPinSensitivity(EXTI_Pin_0, EXTI_Trigger_Falling_Low);
}

void HaltOffDevice(void)
{
	ADC_Cmd(ADC1, DISABLE);
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, DISABLE);
	ADC_DeInit(ADC1);
	
	GPIO_Init(GPIOA, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 \
			        |GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);
	
	GPIO_Init(GPIOB, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 \
			        |GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);
	
	GPIO_Init(GPIOC, GPIO_Pin_2 | GPIO_Pin_3 |GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);
	
	GPIO_Init(GPIOD, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 \
			        |GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);
	
	GPIO_Init(GPIOE, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 \
			        |GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);
	
	GPIO_Init(GPIOF, GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 \
					|GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);
	
	GPIO_Init(GPIOG, GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 \
			        |GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7, GPIO_Mode_Out_PP_Low_Slow);
	
	GPIO_Write(GPIOA, 0x00);
	GPIO_Write(GPIOB, 0x00);
	GPIO_Write(GPIOC, 0x00);
	GPIO_Write(GPIOD, 0x00);
	GPIO_Write(GPIOE, 0x00);
	GPIO_Write(GPIOF, 0x00);
	GPIO_Write(GPIOG, 0x00);
	
	GPIO_Init(GPIOA, GPIO_Pin_1, GPIO_Mode_Out_PP_High_Slow);	// 2  SWD
	GPIO_Init(GPIOA, GPIO_Pin_2, GPIO_Mode_Out_PP_High_Slow);	// 15 外采样
	GPIO_Init(GPIOA, GPIO_Pin_3, GPIO_Mode_Out_PP_High_Slow);	// 16 防磁
	GPIO_Init(GPIOA, GPIO_Pin_7, GPIO_Mode_Out_PP_High_Slow);	// 8  内采样
	GPIO_Init(GPIOC, GPIO_Pin_0, GPIO_Mode_Out_PP_High_Slow);	// 53 电池检测
	GPIO_Init(GPIOC, GPIO_Pin_1, GPIO_Mode_Out_PP_High_Slow);	// 54 按键
	GPIO_Init(GPIOG, GPIO_Pin_1, GPIO_Mode_In_FL_No_IT);		// MOTOR_SW
	GPIO_Init(GPIOG, GPIO_Pin_4, GPIO_Mode_Out_PP_High_Slow);	// 49 SPI_NSS
}

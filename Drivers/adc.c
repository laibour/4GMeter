#include "config.h"
#include "adc.h"


static uint8_t BatStatus;

/* ADC使能端口初始化 */
static void ADCIOInit(void)
{
    GPIO_Init(ADSWITCH_GPIO, ADSWITCH_PIN, GPIO_Mode_Out_PP_High_Slow);

	ADSWITCH_OFF();
}

/* 测量8次取平均 */
static uint16_t GetADCValue(void)
{
	uint8_t i;
	uint16_t res = 0;
	
	ADC_ChannelCmd(ADC1, ADC_Channel_4, ENABLE);
	ADSWITCH_ON();
	for (i = 0; i < 8; i++)
	{
		ADC_SoftwareStartConv(ADC1);  // start
		while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET);
		res += ADC_GetConversionValue(ADC1);
	}
	ADSWITCH_OFF();
	ADC_ChannelCmd(ADC1, ADC_Channel_4, DISABLE);
	
	return (res>>3);
}

/* ADC初始化 */
void ADCInit(void)
{
	uint16_t ADCValue;
	
	ADCIOInit();
	
	CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE);
	
	ADC_DeInit(ADC1);
	
	ADC_Init(ADC1, ADC_ConversionMode_Single, ADC_Resolution_12Bit, ADC_Prescaler_2);
	
	ADC_SamplingTimeConfig(ADC1, ADC_Group_SlowChannels, ADC_SamplingTime_384Cycles);
	
	ADC_Cmd(ADC1, ENABLE);
	
	ADCValue = GetADCValue();
	Vapp = (VREF * ADCValue) / ADC_CONV;
}

/* 电池检测处理函数 */
void ADCProcess(void)
{
	uint8_t temp = 0;
	static uint8_t ADCError = 0;
	static uint8_t ADCCount = 0;
	static uint32_t ADCTimer = 0;
	
	if (WakeupState.RTCStatus == TRIG_ACTIVE)
	{
		ADCCount++;
		if (ADCCount >= 10)
		{
			temp = 1;
			ADCCount = 0;
			ADCTimer = TimeCnt;
		}
	}
	else if (GetTimer(ADCTimer) > 1000)
	{
		temp = 1;
		ADCTimer = TimeCnt;
	}
	
	/* 检测 */
	if (temp == 1)
	{
		ADCInit();
		BatStatus = (uint8_t)(30*Vapp);
		if (Vapp < VOL_STAND)
		{
			ADCError++;
			if (ADCError >= 3)
			{
				ADCError = 0;
				CloseMotor(0);
				DBG_PRINTF("The battery is low power! \n");
			}
			
			if (Vapp < 0.5)
			{
				BatStatus |= 0x80;  /* 掉电 */
			}
			else
			{
				BatStatus |= 0x40;  /* 欠压 */
			}
		}
		else
		{
			ADCError = 0;
		}
	}
}


uint8_t GetBatStatus(void)
{
	return (BatStatus + 6);
}

/* 即时检测电池电压 */
bool ADCAtOnce(void)
{
	ADCInit();
	if (Vapp >= VOL_STAND)
	{
		return TRUE;
	}
	
	return FALSE;
}

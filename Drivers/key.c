#include "config.h"
#include "key.h"


#define	KEY_DISP_SUM		(9)
#define	KEY_DISP_INTERVAL	(300)  /* 按键显示间隔时间3s */


uint8_t KeyVal = 0;
static tKeyStates KeyState = KEY_STATE_INIT;

static uint8_t KeyUp(void);
static uint8_t KeyDown(void);
static uint8_t ReadKey(void);

static uint32_t power(uint8_t val)
{
    uint32_t ret;
	
    switch(val)
    {
        case 0: ret = 1; break;
        case 1: ret = 10; break;
        case 2: ret = 100; break;
        case 3: ret = 1000; break;
        case 4: ret = 10000; break;
        case 5: ret = 100000; break;
        case 6: ret = 1000000; break;
        case 7: ret = 10000000; break;
        case 8: ret = 100000000; break;
        default: break;
    }
	
	return ret;
}

void DispDigit(uint32_t num)
{
	uint8_t i, j;
	uint32_t dump = 10000000;
	
	for (i = 0; i < 8; i++)
	{
		if (num >= dump)
		{
			for (j = 0; j < 8 - i; j++)
			{
				DispData.digit[i+j] = num%power(8-i-j)/power(7-i-j);
			}
			break;
		}
		
		dump /= 10;
	}
	
	if (i == 8)
	{
		DispData.digit[5] = 0;
		DispData.digit[6] = 0;
		DispData.digit[7] = 0;
	}
	
	for (i = 0; i< 2; i++)
	{
		if (DispData.digit[5+i] > 9)
		{
			DispData.digit[5+i] = 0;
		}
	}
}

/* 余量 */
void DispRemainMoney(void)
{
	LCDClear();
	DispDigit((MeterParm.RemainMoney+50)/100);
	DispData.remain   = 1;
	DispData.PosPoint = 1;
	DispData.Yuan     = 1;
	FlushDataToLCD(&DispData);
}

/* 月用量 */
void DispMonthUse(void)
{
	LCDClear();
	DispDigit(MeterParm.TotalGas - MeterParm.MonthBaseGas);
	DispData.month       = 1;
	DispData.consumption = 1;
	DispData.PosPoint    = 1;
	DispData.cube        = 1;
	FlushDataToLCD(&DispData);
}

/* 总用量 */
void DispTotalUse(void)
{
	LCDClear();
	DispDigit(MeterParm.TotalGas);
	DispData.total       = 1;
	DispData.consumption = 1;
	DispData.PosPoint    = 1;
	DispData.cube        = 1;
	FlushDataToLCD(&DispData);
}

/* 单价 */
void DispUnitPrice(void)
{
	LCDClear();
	DispDigit(MeterParm.CurUnitPrice);  // 单价单位为分
	DispData.price    = 1;
	DispData.PosPoint = 1;
	DispData.Yuan     = 1;
	DispData.divide   = 1;
	DispData.cube     = 1;
	FlushDataToLCD(&DispData);
}

/* 电池电压 */
void DispVoltage(void)
{
	LCDClear();
	DispData.digit[1] = 18;
	DispData.digit[2] = 21;
	DispData.digit[4] = (uint8_t)(Vapp*30 + 2) / 10;
	DispData.digit[5] = (uint8_t)(Vapp*30 + 2) % 10;
	DispData.PrePoint = 1;
	DispData.voltage  = 1;
	if (Vapp < VOL_STAND)  // 请换电池
	{
		DispData.changebat = 1;
	}
	FlushDataToLCD(&DispData);
}

/* 年月日 */
void DispCurDate(void)
{
	LCDClear();
	DispData.digit[0] = (SystemTime.yearL >> 4);
	DispData.digit[1] = SystemTime.yearL & 0x0F;
	DispData.digit[2] = 19;
	DispData.digit[3] = (SystemTime.month >> 4);
	DispData.digit[4] = SystemTime.month & 0x0F;
	DispData.digit[5] = 19;
	DispData.digit[6] = (SystemTime.day >> 4);
	DispData.digit[7] = SystemTime.day & 0x0F;
	FlushDataToLCD(&DispData);
}

/* 时分秒 */
void DispCurTime(void)
{
	LCDClear();
	DispData.digit[0] = (SystemTime.hour >> 4);
	DispData.digit[1] = SystemTime.hour & 0x0F;
	DispData.digit[2] = 20;
	DispData.digit[3] = (SystemTime.minute >> 4);
	DispData.digit[4] = SystemTime.minute & 0x0F;
	DispData.digit[5] = 20;
	DispData.digit[6] = (SystemTime.second >> 4);
	DispData.digit[7] = SystemTime.second & 0x0F;
	FlushDataToLCD(&DispData);
}

/* 显示从站地址中的公司代码和年月 */
void DispCompany(void)
{
	LCDClear();
	DispData.digit[0] = (MeterParm.SlaveAddr.MID[0] >> 4);
	DispData.digit[1] = MeterParm.SlaveAddr.MID[0] & 0x0F;
	DispData.digit[2] = (MeterParm.SlaveAddr.MID[1] >> 4);
	DispData.digit[3] = MeterParm.SlaveAddr.MID[1] & 0x0F;
	DispData.digit[4] = (MeterParm.SlaveAddr.ConAddr[0] >> 4);
	DispData.digit[5] = MeterParm.SlaveAddr.ConAddr[0] & 0x0F;
	DispData.digit[6] = (MeterParm.SlaveAddr.ConAddr[1] >> 4);
	DispData.digit[7] = MeterParm.SlaveAddr.ConAddr[1] & 0x0F;
	FlushDataToLCD(&DispData);
}

/* 显示从站地址中的表号 */
void DispMeterID(void)
{
	LCDClear();
	DispData.digit[0] = (MeterParm.SlaveAddr.ConAddr[1] >> 4);
	DispData.digit[1] = MeterParm.SlaveAddr.ConAddr[1] & 0x0F;
	DispData.digit[2] = (MeterParm.SlaveAddr.ConAddr[2] >> 4);
	DispData.digit[3] = MeterParm.SlaveAddr.ConAddr[2] & 0x0F;
	DispData.digit[4] = (MeterParm.SlaveAddr.ConAddr[3] >> 4);
	DispData.digit[5] = MeterParm.SlaveAddr.ConAddr[3] & 0x0F;
	DispData.digit[6] = (MeterParm.SlaveAddr.ConAddr[4] >> 4);
	DispData.digit[7] = MeterParm.SlaveAddr.ConAddr[4] & 0x0F;
	FlushDataToLCD(&DispData);
}

/* 清屏 */
void DispNull(void)
{
	LCDClear();
}

void (*DispArray[])(void) = 
{
	DispRemainMoney,
	DispMonthUse,
	DispTotalUse,
	DispUnitPrice,
	DispVoltage,
	DispCurDate,
	DispCurTime,
//	DispCompany,
	DispMeterID,
	DispNull,
};

// 按键处理函数
void KeyProcess(void)
{
	uint8_t i;
	uint8_t PressTime = 0;
	static uint8_t KeyDispNum = 0;
	static uint32_t KeyPressTimer;
	static tKeyDispStates KeyDispState = KEY_DISP_INIT;
	
	if (ReadKey() == KEY_DONE)
	{
		for (i = 0; i < 8; i++)
		{
			if ( (KeyVal<<i) & 0x80 )
			{
				break;
			}
		}
		
		if (i >= 7)	 /* 错误 */
		{
			return;
		}
		
		PressTime = 7 - i;
		KeyDispState = KEY_DISP_INIT;
	}
	
	switch (KeyDispState)
	{
		case KEY_DISP_INIT:
			if (PressTime != 0)
			{
				/* 判断是否需要开阀 */
				if ( MotorOpenOption() )
				{
					OpenMotor();
				}
				KeyDispNum   = (KeyDispNum + PressTime - 1) % KEY_DISP_SUM;
				KeyDispState = KEY_DISP_RUN;
			}
			break;
		
		case KEY_DISP_RUN:
			DispArray[KeyDispNum]();
			KeyDispNum++;
			if (KeyDispNum == KEY_DISP_SUM)
			{
				KeyDispNum   = 0;
				KeyDispState = KEY_DISP_INIT;
				WakeupState.KeyStatus = TRIG_SLEEP;
			}
			else
			{
				KeyPressTimer = TimeCnt;
				KeyDispState  = KEY_DISP_WAIT;
			}
			break;
			
		case KEY_DISP_WAIT:
			if ( GetTimer(KeyPressTimer) >= KEY_DISP_INTERVAL )
			{
				KeyDispState = KEY_DISP_RUN;
			}
			break;
			
		default:
			break;
	}
}

uint8_t ReadKey(void)
{
	uint8_t result = KEY_BUSY;
	static uint8_t KeyNum = 0;
	static uint32_t ReadKeyTimer;
	
	switch(KeyState)
	{
		case KEY_STATE_INIT:
			KeyVal = 0;
			KeyNum = 0;
			KeyState = KEY_STATE_READ;
			break;
			
		case KEY_STATE_READ:
			if (KeyDown() == KEY_DONE)
			{
				KeyVal |= 0x01;
				// BeepTime = 20;  /* 按键音 */
				ReadKeyTimer = TimeCnt;
				KeyState = KEY_STATE_300MS;
			}
			break;
		
		case KEY_STATE_300MS:
			if ( GetTimer(ReadKeyTimer) < 30 )	/* 300ms */
			{
				if (KeyUp() == KEY_DONE)
				{
					KeyVal <<= 1;
					KeyState = KEY_STATE_FULL;
				}
			}
			else
			{
				if (GetTimer(ReadKeyTimer) > 200)	/* >2.0s */
				{
					BeepTime = 200;
					if (GetTimer(ReadKeyTimer) > 280)	/* >2.8s */
					{
						BeepTime  = 0;
						if (KeyUp() == KEY_DONE)
						{
							AddNetTask(C3041, 0);	/* 长按按键，主动上报 */
							KeyVal <<= 1;
							KeyVal |= 0x01;
							result = KEY_DONE;
							KeyState = KEY_STATE_INIT;
						}
					}
				}
				else if (KeyUp() == KEY_DONE)
				{
					KeyVal <<= 1;
					KeyVal |= 0x01;
					KeyState = KEY_STATE_FULL;
				}
			}
			break;
		
		case KEY_STATE_FULL:
			KeyNum++;
			if (KeyNum == KEY_NUM)
			{
				result = KEY_DONE;
				KeyState = KEY_STATE_INIT;
			}
			else
			{
				ReadKeyTimer = TimeCnt;
				KeyState = KEY_STATE_500MS;
			}
			break;
		
		case KEY_STATE_500MS:
			if ( GetTimer(ReadKeyTimer) <= 50 )	 /* 500ms */
			{
				if ( KeyDown() == KEY_DONE )
				{
					//BeepTime = 20;  /* 按键音 */
					ReadKeyTimer = TimeCnt;
					KeyState = KEY_STATE_300MS;
				}
			}
			else
			{
				result = KEY_DONE;
				KeyState = KEY_STATE_INIT;
			}
			break;
		
		default:
			break;
	}
	
	return result;
}

uint8_t KeyDown(void)
{
	uint8_t result = KEY_BUSY;
	static uint8_t start = 1;
	static uint32_t KeyDownTimer;
	
	if ((start==1) && (!GPIO_ReadInputDataBit(KEY_GPIO, KEY_PIN)))
	{
		start = 0;
		KeyDownTimer = TimeCnt;
	}
	else if ((start==0) && (GetTimer(KeyDownTimer) >= 2))
	{
		start = 1;
		if ( !GPIO_ReadInputDataBit(KEY_GPIO, KEY_PIN) )
		{
			result = KEY_DONE;
		}
	}
	
	return result;
}

uint8_t KeyUp(void)
{
	uint8_t result = KEY_BUSY;
	static uint8_t start = 1;
	static uint32_t KeyUpTimer;
	
	if ((start==1) && (GPIO_ReadInputDataBit(KEY_GPIO, KEY_PIN)))
	{
		start = 0;
		KeyUpTimer = TimeCnt;
	}
	else if ((start==0) && (GetTimer(KeyUpTimer) >= 2))
	{
		start = 1;
		if ( GPIO_ReadInputDataBit(KEY_GPIO, KEY_PIN) )
		{
			result = KEY_DONE;
		}
	}
	
	return result;
}

/* 按键初始化 */
void KeyInit(void)
{
    EXTI_ClearITPendingBit(EXTI_IT_Pin1);
    GPIO_Init(KEY_GPIO, KEY_PIN, GPIO_Mode_In_FL_No_IT);
}

/* 按键中断函数 */
INTERRUPT_HANDLER(EXTI1_IRQHandler, 9)
{
	BeepStart = TRUE;
	WakeupState.KeyStatus = TRIG_ACTIVE;
	EXTI_ClearITPendingBit(EXTI_IT_Pin1);
	GPIO_Init(KEY_GPIO, KEY_PIN, GPIO_Mode_In_FL_No_IT);
}

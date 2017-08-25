#include "config.h"
#include "rtc.h"

uint8_t DayOfMonth[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

static void GasRecord(void)
{
	uint8_t day;
	uint8_t month;
	uint16_t year;
	T_MeterData tMeterDayData;
	T_MeterData tMeterMonthData;
	static uint8_t LastDay = 0xFF;
	static uint8_t LastMonth = 0xFF;
	
	if (SystemTime.day != LastDay || ((SystemTime.day == LastDay) && (SystemTime.month != LastMonth)))
	{
		if (SystemTime.hour==0x23 && SystemTime.minute==0x59 && SystemTime.second==0x59)
		{
			/* 计算日用量 */
			tMeterDayData.Time   = SystemTime;
			tMeterDayData.GasPot = (MeterParm.TotalGas % 100) * 10;
			tMeterDayData.GasInt = MeterParm.TotalGas / 100;
			MeterParm.DayPoint   = (MeterParm.DayPoint + 1) % SAVE_DAY;
			
			/* 保存日用量指数 */
			SPI_FLASH_WriteBuffer((uint8_t *)&tMeterDayData, METER_DAY_ADDR+MeterParm.DayPoint*sizeof(T_MeterData), sizeof(T_MeterData));
						
			/* 记录月用量 */
			day   = (SystemTime.day>>4)*10 + (SystemTime.day&0x0F);
			month = (SystemTime.month>>4)*10 + (SystemTime.month&0x0F);
			year  = 2000 + (SystemTime.yearL>>4)*10 + (SystemTime.yearL&0x0F);
			if (SystemTime.month == 2 && isPrime(year))
			{
				DayOfMonth[1] = 29;
			}
			else
			{
				DayOfMonth[1] = 28;
			}
			
			if (day == DayOfMonth[month])
			{
				/* 计算日用量 */
				tMeterMonthData.Time   = SystemTime;
				tMeterMonthData.GasPot = (MeterParm.TotalGas % 100) * 10;
				tMeterMonthData.GasInt = MeterParm.TotalGas / 100;
				MeterParm.MonthPoint   = (MeterParm.MonthPoint + 1) % SAVE_MONTH;
				
				/* 保存日用量指数 */
				SPI_FLASH_WriteBuffer((uint8_t *)&tMeterMonthData, METER_MONTH_ADDR+MeterParm.MonthPoint*sizeof(T_MeterData), sizeof(T_MeterData));
			}
			
			LastDay = SystemTime.day;
			LastMonth = SystemTime.month;
		}
	}

#if 0
	if (SystemTime.day != LastDay || ((SystemTime.day == LastDay) && (SystemTime.month != LastMonth)))
	{
		if (SystemTime.hour==0x00 && SystemTime.minute==0x00 && SystemTime.second==0x01)
		{
			/* 记录日用气量 */
			MeterData.DayGas[MeterData.DayPoint] = (uint16_t)((MeterParm.TotalGas - MeterParm.DayBaseGas + 50) / 100);
			MeterParm.DayBaseGas = MeterParm.TotalGas;
			
			/* 保存日用气量参数 */
			ofet = offset(T_MeterData, DayGas[MeterData.DayPoint]);
			EEPROMWriteBuffer((uint8_t *)&MeterData.DayGas[MeterData.DayPoint], METER_DATA_ADDR+ofet, 2);
			
			/* 保存日用气量基数 */
			ofet = offset(T_MeterParm, DayBaseGas);
			EEPROMWriteBuffer((uint8_t *)&MeterParm.DayBaseGas, METER_PAM_ADDR+ofet, 4);
			
			MeterData.DayPoint = (MeterData.DayPoint + 1) % SAVE_DAY;
			EEPROMWriteBuffer((uint8_t *)&MeterData.DayPoint, METER_PAM_ADDR, 1);
			
			/* 记录月用气量 */
			day   = (SystemTime.day>>4)*10 + (SystemTime.day&0x0F);
			month = (SystemTime.month>>4)*10 + (SystemTime.month&0x0F);
			year  = 2000 + (SystemTime.yearL>>4)*10 + (SystemTime.yearL&0x0F);
			if (SystemTime.month == 2 && isPrime(year))
			{
				DayOfMonth[1] = 29;
			}
			else
			{
				DayOfMonth[1] = 28;
			}
			
			/* 清0??? */
			if (day == DayOfMonth[month])
			{
				/* 记录月用气量 */
				MeterData.MonthGas[month] = (uint16_t)((MeterParm.TotalGas - MeterParm.MonthBaseGas + 50) / 100);
				MeterParm.MonthBaseGas = MeterParm.TotalGas;
				
				/* 保存月用气量参数 */
				ofet = offset(T_MeterData, MonthGas[month]);
				EEPROMWriteBuffer((uint8_t *)&MeterData.MonthGas[month], METER_DATA_ADDR+ofet, 2);
				
				/* 保存月用气量基数 */
				ofet = offset(T_MeterParm, MonthBaseGas);
				EEPROMWriteBuffer((uint8_t *)&MeterParm.MonthBaseGas, METER_PAM_ADDR+ofet, 4);
			}
			
			LastDay = SystemTime.day;
			LastMonth = SystemTime.month;
		}
	}
#endif
}

/* 定时上报 */
static void TimedReport(void)
{
	uint16_t Offet;
	T_Time tPreWakeTime;
	T_Time tPostWakeTime;
	static uint8_t LastDay = 0xFF;
	
	Offet = (((MeterParm.SlaveAddr.ConAddr[4]>>4) * 10) + (MeterParm.SlaveAddr.ConAddr[4] & 0x0F)) * 10;
	
	tPreWakeTime = MeterParm.Wakeup.Time;
	GetDecTime(&tPreWakeTime);  /* 将设置的上报时间转换为十进制 */
	if ((MeterParm.Wakeup.Time.minute + Offet%60) < 60)
	{
		tPostWakeTime.second = tPreWakeTime.second;
		tPostWakeTime.minute = tPreWakeTime.minute + Offet%60;
		tPostWakeTime.hour   = (tPreWakeTime.hour + Offet/60)%24;
	}
	else
	{
		tPostWakeTime.second = tPreWakeTime.second;
		tPostWakeTime.minute = (tPreWakeTime.minute + Offet%60)%60;
		tPostWakeTime.hour   = (tPreWakeTime.hour + Offet/60 + 1)%24;
	}
	SetBCDTime(&tPostWakeTime);  /* 将实际上报时间转换为BCD码 */
	
	if (SystemTime.day != LastDay)
	{
		if ((MeterParm.UploadCycle == 0x0101) || \
		    (MeterParm.UploadCycle == 0x0201 && (SystemTime.day==0x01 || SystemTime.day==0x10 || SystemTime.day==0x20)) || \
		    (MeterParm.UploadCycle == 0x0301 && SystemTime.day==0x01))
		{
			LastDay = SystemTime.day;
			if (CompareData(&SystemTime.hour, (uint8_t *)&tPostWakeTime, 3) == TRUE)
			{
				AddNetTask(C3041, 0);	/* 定时上报 */
			}
		}
	}
}

/* 表具、计费周期管理 */
static void PeriodManage(void)
{
	uint8_t year, month, sum;
	static uint8_t LastYear = 0;
	static uint8_t LastMonth = 0;
	static uint8_t OverPeriod = 0;
	
	if (MeterParm.isUse == 1)
	{
		year  = BCDToDec(&SystemTime.yearL, 1);
		month = BCDToDec(&SystemTime.month, 1);
		
		/* 没有校时，返回 */
		if ((year == 16) && (month == 1))
		{
			return;
		}
		sum = (year - 16) * 12 + month;  /* 从2016年1月到当前的月数 */
		
		/* 预调价判断 */
		if ((MeterParm.NewPrice.ActMonth != 0) && (sum >= MeterParm.NewPrice.ActMonth))
		{
			MeterParm.CurPrice = MeterParm.NewPrice;
			memset(&MeterParm.NewPrice, 0, sizeof(T_Price));
			MeterParm.CycleBaseGas = MeterParm.TotalGas;
			MeterParm.CurUnitPrice = MeterParm.CurPrice.PriceDevide[0];
			
			/* 保存表具参数 */
			EEPROMWriteBuffer((uint8_t *)&MeterParm, METER_PAM_ADDR, (uint8_t)sizeof(T_MeterParm));
		}
		
		/* 价格有效期判断 */
		if ((sum >= MeterParm.CurPrice.ActPeriod) && OverPeriod == 0)
		{
			CloseMotor(1);  /* 超过价格有效期，强制关阀 */
			OverPeriod = 1;
		}
		else if ((sum < MeterParm.CurPrice.ActPeriod) && OverPeriod == 1)
		{
			OverPeriod = 0;
			MeterParm.MeterSta.bits.force = 0;  /* 价格有效期内，清强关标志 */
		}
		
		/* 表具有效期判断 */	
		if ( CompareData((uint8_t *)&SystemTime, (uint8_t *)&MeterParm.MeterOutDate, 4) )
		{
			CloseMotor(1);  /* 超过表具有效期，强制关阀 */
		}
		
		/* 阶梯(价格)周期判断 */
		switch(MeterParm.CurPrice.StepCycle)
		{
			case CYCLE_YEAR:
				if ((month == MeterParm.CurPrice.ActMonth) && (year != LastYear))
				{
					LastYear = year;
					MeterParm.CycleBaseGas = MeterParm.TotalGas;
				}
				break;
				
			case CYCLE_HALF_YEAR:
				if (month%6 == 1)
				{
					if ((month != LastMonth) || ((month == LastMonth) && (year != LastYear)))
					{
						LastYear  = year;
						LastMonth = month;
						MeterParm.CycleBaseGas = MeterParm.TotalGas;
					}
				}
				break;
				
			case CYCLE_QUARTER:
				if (month%3 == 2)
				{
					if ((month != LastMonth) || ((month == LastMonth) && (year != LastYear)))
					{
						LastYear  = year;
						LastMonth = month;
						MeterParm.CycleBaseGas = MeterParm.TotalGas;
					}
				}
				break;
				
			case CYCLE_MONTH:
				if ((month != LastMonth) || ((month == LastMonth) && (year != LastYear)))
				{
					LastYear  = year;
					LastMonth = month;
					MeterParm.CycleBaseGas = MeterParm.TotalGas;
				}
				break;
				
			default:
				break;
		}
	}
}

/* 长时不用保护 */
void LongTimeUnused(void)
{
	static uint8_t UnusedDays = 0;
	static uint32_t LastTotalGas = 0;
	
	if (MeterParm.isUse == 1)
	{
		if (SystemTime.hour==0x23 && SystemTime.minute==0x59 && SystemTime.second==0x50)
		{
			if ((LastTotalGas == MeterParm.TotalGas) && (MeterParm.TotalGas != 0))
			{
				UnusedDays++;
				if (UnusedDays >= MeterParm.LongtimeProtect)
				{
					UnusedDays = 0;
					CloseMotor(0);
				}
			}
			else
			{
				UnusedDays = 0;
			}
			
			LastTotalGas = MeterParm.TotalGas;
		}
	}
}

// RTC唤醒处理(表具、计费周期管理,主动上报和AD采样)
void RTCTaskProcess(void)
{
	static uint8_t LastSecond = 0xFF;
	
	if (WakeupState.RTCStatus == TRIG_ACTIVE)
	{
		RTCRead(&SystemTime);  /* reading twice can get value */
	 // DATA_PRINTF(&SystemTime.second, 1);  /* display time second */
	}
	
	if (LastSecond != SystemTime.second)
	{
		LastSecond = SystemTime.second;
		
		GasRecord();
		
		ADCProcess();
		
		FlowProcess();
		
		TimedReport();
		
		PeriodManage();
		
		LongTimeUnused();
		
		WakeupState.RTCStatus = TRIG_SLEEP;
	}
}

/* RTC恢复初始时间 */
void RTCReset(void)
{
	RTCWrite(&SystemTime);
}

/* 读RTC时间 */
void RTCRead(PT_SystemTime ptSystemTime)
{
	RTC_TimeTypeDef RTC_TimeStr;
    RTC_DateTypeDef RTC_DateStr;
	
	/* 时分秒 */
//	while (RTC_WaitForSynchro() != SUCCESS);
    RTC_GetTime(RTC_Format_BCD, &RTC_TimeStr);
	
	ptSystemTime->hour   = RTC_TimeStr.RTC_Hours;
    ptSystemTime->minute = RTC_TimeStr.RTC_Minutes;
    ptSystemTime->second = RTC_TimeStr.RTC_Seconds;
	
	/* 年月日 */
//	while (RTC_WaitForSynchro() != SUCCESS);
    RTC_GetDate(RTC_Format_BCD, &RTC_DateStr);
	
	ptSystemTime->yearL = RTC_DateStr.RTC_Year;
	ptSystemTime->month  = RTC_DateStr.RTC_Month;
    ptSystemTime->day = RTC_DateStr.RTC_Date;
}

/* 写RTC时间 */
void RTCWrite(PT_SystemTime ptSystemTime)
{
	RTC_TimeTypeDef RTC_TimeStr;
    RTC_DateTypeDef RTC_DateStr;
	
	RTC_TimeStructInit(&RTC_TimeStr);
    RTC_TimeStr.RTC_Hours = ptSystemTime->hour;
    RTC_TimeStr.RTC_Minutes = ptSystemTime->minute;
    RTC_TimeStr.RTC_Seconds = ptSystemTime->second;
    RTC_SetTime(RTC_Format_BCD, &RTC_TimeStr);
	
	RTC_DateStructInit(&RTC_DateStr);
    RTC_DateStr.RTC_WeekDay = RTC_Weekday_Friday;
    RTC_DateStr.RTC_Date = ptSystemTime->day;
    RTC_DateStr.RTC_Month = (RTC_Month_TypeDef)ptSystemTime->month;
    RTC_DateStr.RTC_Year = ptSystemTime->yearL;
    RTC_SetDate(RTC_Format_BCD, &RTC_DateStr);
}

/* RTC初始化 */
void RTCInit(void)
{
	RTC_InitTypeDef RTC_InitStr;

	/* use LSE */
	CLK_LSEConfig(CLK_LSE_ON);
	while (CLK_GetFlagStatus(CLK_FLAG_LSERDY) == RESET);
	CLK_RTCClockConfig(CLK_RTCCLKSource_LSE, CLK_RTCCLKDiv_1);  // 32.768K
	CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);		// 使能RTC时钟
	
	RTC_WakeUpClockConfig(RTC_WakeUpClock_CK_SPRE_16bits);	// RTCCLK = LSE/16 = 488.28125 us

	RTC_ClearITPendingBit(RTC_IT_WUT);
	RTC_ITConfig(RTC_IT_WUT, ENABLE);  /* 使能唤醒中断 */
	RTC_WakeUpCmd(DISABLE);

	RTC_InitStr.RTC_HourFormat   = RTC_HourFormat_24;
	RTC_InitStr.RTC_AsynchPrediv = 0x7F;  /* 1s */
	RTC_InitStr.RTC_SynchPrediv  = 0xFF;
	RTC_Init(&RTC_InitStr);
}

/* RTC中断 */
INTERRUPT_HANDLER(RTC_CSSLSE_IRQHandler, 4)
{
    RTC_WakeUpCmd(DISABLE);  /* 禁止唤醒 */
	RTC_ClearITPendingBit(RTC_IT_WUT);
	WakeupState.RTCStatus = TRIG_ACTIVE;
}

#include "config.h"
#include "common.h"


float Vapp = 0;
uint8_t GPRSFlag = 0;
uint8_t LongPress = 0;
T_MeterData MeterData;
T_HeartBeat HeartBeat;  /* GPRS心跳包 */
T_MeterParm MeterParm;
T_CountState CountState;
T_MultiPacket MultiPacket;
T_DispData DispData = {0};
T_MotorSta MotorSta = {MOTOR_OPEN, 0};
T_SystemTime SystemTime = {0x20, 0x17, 0x01, 0x01, 0x00, 0x09, 0x55};
uint8_t GPRSOpenData[] = "AT+CIPOPEN=0,\"TCP\",\"101.204.248.138\",08168\r";
T_WakeupState WakeupState = {FALSE, TRIG_SLEEP, TRIG_SLEEP, TRIG_SLEEP, TRIG_SLEEP, TRIG_SLEEP};

/* 上电时读取表具数据 */
void SetMeterParm(void)
{
	uint8_t i;
	T_SlaveAddr SlaveAddr = {0x23, 0x76, 0x16, 0x01, 0x12, 0x34, 0x08};
	T_Socket DefaultSocket = {'1', '2', '0', '.', '0', '2', '6', '.', '2', '4', '0', '.', '1', '9', '0', '0', '0', '8', '1', '7', '0'};
	
	RTCReset();	 /* 上电时，RTC时间设置为默认值 */
	EEPROMReadBuffer((uint8_t *)&MeterParm, METER_PAM_ADDR, (uint8_t)sizeof(T_MeterParm));
//	MeterParm.MeterSta.bits.force = 0;
//	MeterParm.MeterSta.bits.magnet = 0;
//	MeterParm.isUse = 0;
//	MeterParm.MeterSta.Val &= 0xE0;
//	MeterParm.SlaveAddr = SlaveAddr;   /* 从站编号 */
//	EEPROMWriteBuffer((uint8_t *)&MeterParm, METER_PAM_ADDR, (uint8_t)sizeof(T_MeterParm));
	if (MeterParm.isUse != 1)
	{
		SPI_FLASH_Reset();
		EEPROMReset();
		EEPROMReadBuffer((uint8_t *)&MeterParm, METER_PAM_ADDR, (uint8_t)sizeof(T_MeterParm));
//		MeterParm.isUse = 1;
		MeterParm.MeterType = POST_PAY;
		MeterParm.FlowMax   = 3;
		MeterParm.CountMax  = 3;
		MeterParm.MagnetMax = 1;
		MeterParm.FlowLimit = 10;
		
		MeterParm.UploadCycle = 0x0101;
		MeterParm.Wakeup.Duration = 10;
		MeterParm.Wakeup.Time.hour = 0x00;
		MeterParm.Wakeup.Time.minute = 0x00;
		MeterParm.Wakeup.Time.second = 0x00;
		MeterParm.GPRSHeartTime = 3000;
		MeterParm.TotalGas = 0;
		MeterParm.TotalMoney = 100000;
		MeterParm.LongtimeProtect = 30;
//		
//		MeterParm.CurPrice.PriceType = 1;
//		MeterParm.CurPrice.PriceVer  = 0;
//		MeterParm.CurPrice.ActMonth  = 12;			
		MeterParm.CurPrice.ActPeriod = 200;        	 /* 有效期 */
//		MeterParm.CurPrice.StepCycle = CYCLE_MONTH;  /* 阶梯周期 */
//		MeterParm.CurPrice.PriceDevide[0] = 100;
//		MeterParm.CurPrice.GasDivide[0] = 2;
//		MeterParm.CurPrice.PriceDevide[1] = 200;
//		MeterParm.CurPrice.GasDivide[1] = 3;
//		MeterParm.CurPrice.PriceDevide[2] = 300;
//		MeterParm.CurUnitPrice = MeterParm.CurPrice.PriceDevide[0];
//		
		MeterParm.UsedMoney = 0;
		MeterParm.RemainMoney = MeterParm.TotalMoney;
		
		MeterParm.Socket = DefaultSocket;
		MeterParm.SlaveAddr = SlaveAddr;   /* 从站编号 */
		EEPROMWriteBuffer((uint8_t *)&MeterParm, METER_PAM_ADDR, (uint8_t)sizeof(T_MeterParm));
	}
	
	MeterParm.MeterSta.bits.newmeter = MeterParm.isUse;
	
	/* 主站IP参数 */
	for (i = 0; i < 15; i++)  /* IP */
	{
		GPRSOpenData[20+i] = MeterParm.Socket.IP[i];
	}
	
	for (i = 0; i < 5; i++)  /* PORT */
	{
		GPRSOpenData[37+i] = MeterParm.Socket.Port[1+i];
	}
	
	/* 生成GPRS心跳包 */
	HeartBeat.Head.Preamble     = 0x68;
	HeartBeat.Head.PacketLen    = 0x2100;
	HeartBeat.Head.FuncCode     = 0x3003;
	HeartBeat.Head.TranDirect   = 0x01;
	HeartBeat.Head.ResponseFlag = 0x00;
	HeartBeat.Head.SlaveAddr = MeterParm.SlaveAddr;
	HeartBeat.Head.PacketID.yearL = SystemTime.yearL;
	HeartBeat.Head.PacketID.month = SystemTime.month;
	HeartBeat.Head.PacketID.day   = SystemTime.day;
	HeartBeat.Head.PacketID.Order = 0x01;
	HeartBeat.Head.DataLen = 0x0700;
	memcpy(HeartBeat.MeterAddr, (uint8_t *)&MeterParm.SlaveAddr, 7);
	HeartBeat.Crc = 0x00;
	HeartBeat.End = 0x16;
	
#if 0
	/* 构造测试数据 */
	T_MeterData tMeterDayData;
	T_MeterData tMeterMonthData;
	MeterParm.TotalGas = 1000;
	
	for (i = 0; i < 10; i++)
	{
		MeterParm.TotalGas += 100;
		/* 计算日用量 */
		tMeterDayData.Time   = SystemTime;
		tMeterDayData.GasPot = (MeterParm.TotalGas % 100) * 10;
		tMeterDayData.GasInt = MeterParm.TotalGas / 100;
		MeterParm.DayPoint   = (MeterParm.DayPoint + 1) % SAVE_DAY;
		
		/* 保存日用量指数 */
		SPI_FLASH_WriteBuffer((uint8_t *)&tMeterDayData, METER_DAY_ADDR+MeterParm.DayPoint*sizeof(T_MeterData), sizeof(T_MeterData));
		
		/* 计算月用量 */
		tMeterMonthData.Time   = SystemTime;
		tMeterMonthData.GasPot = (MeterParm.TotalGas % 100) * 10;
		tMeterMonthData.GasInt = MeterParm.TotalGas / 100;
		MeterParm.MonthPoint   = (MeterParm.MonthPoint + 1) % SAVE_MONTH;

		/* 保存月用量指数 */
		SPI_FLASH_WriteBuffer((uint8_t *)&tMeterMonthData, METER_MONTH_ADDR+MeterParm.MonthPoint*sizeof(T_MeterData), sizeof(T_MeterData));
	}
#endif
}

/* 生成表具状态字,bit[0-7]:阀门状态、强制关阀、无余量、防护报警/采样保护/防磁保护/过流保护/新表 */
uint8_t GetMeterSta(void)
{
	BYTE_VAL MeterSta;
	
	/* 0.阀门状态 */
	if (MotorSta.SW == MOTOR_OPEN)
	{
		MeterSta.bits.b0 = 1;
	}
	else
	{
		MeterSta.bits.b0 = 0;
	}
	
	/* 1.强制关阀 */
	MeterSta.bits.b1 = MeterParm.MeterSta.bits.force;
	
	/* 2.无余量 */
	if (MeterParm.RemainMoney == 0)
	{
		MeterSta.bits.b2 = 1;
	}
	else
	{
		MeterSta.bits.b2 = 0;
	}
	
	/* 3.防护报警(未用) */
	MeterSta.bits.b3 = MeterParm.MeterSta.bits.defend;
	
	MeterSta.bits.b4 = MeterParm.MeterSta.bits.samp;
	MeterSta.bits.b5 = MeterParm.MeterSta.bits.magnet;
	MeterSta.bits.b6 = MeterParm.MeterSta.bits.flow;
	/* 7.新表 */
	MeterSta.bits.b7 = MeterParm.MeterSta.bits.newmeter;
	
	return MeterSta.Val;
}

/* 将BCD时间格式转换为十进制 */
void GetDecTime(PT_Time ptReadTime)
{
	ptReadTime->second = ((ptReadTime->second>>4)&0x0F)*10 + (ptReadTime->second&0x0F);
	ptReadTime->minute = ((ptReadTime->minute>>4)&0x0F)*10 + (ptReadTime->minute&0x0F);
	ptReadTime->hour   = ((ptReadTime->hour>>4)&0x0F)*10 + (ptReadTime->hour&0x0F);
}

/* 将十进制时间格式转换为BCD格式 */
void SetBCDTime(PT_Time ptReadTime)
{
	ptReadTime->second = ((ptReadTime->second/10)<<4) + ((ptReadTime->second%10)&0x0F);
	ptReadTime->minute = ((ptReadTime->minute/10)<<4) + ((ptReadTime->minute%10)&0x0F);
	ptReadTime->hour   = ((ptReadTime->hour/10)<<4) + ((ptReadTime->hour%10)&0x0F);
}

/* 求权 */
static uint16_t power(int base, int times)
{
	uint8_t  i;
	uint16_t result = 1;
	
	for (i = 0; i < times; i++)
	{
		result *= base;
	}

	return result;
}

/* BCD转换为十进制 */
uint16_t BCDToDec(uint8_t *bcd, uint8_t len)
{
	uint8_t  i, temp;
	uint16_t dec = 0;
	
	for (i = 0; i < len; i++)
	{
		temp = ((bcd[i]>>4)&0x0F)*10 + (bcd[i]&0x0F);
		dec += temp * power(100, i);
	}
	
	return dec;
}

/* 十进制转换为BCD */
void DecToBCD(uint16_t Dec, uint8_t *BCD, uint8_t len)
{
	uint8_t i, temp;

	for (i = 0; i < len; i++)
	{
		temp = Dec%100;
		BCD[i] = ((temp/10)<<4) + ((temp%10)&0x0F);
		Dec /= 100;
	}
}

/* 比较是否相等 */
bool CompareData(uint8_t *a, uint8_t *b, uint16_t n)
{
	uint16_t i;
	
	for (i = 0; i < n; i++)
	{
		if (*(a+i) != *(b+i))
		{
			return FALSE;
		}
	}
	
	return TRUE;
}

/* 查找字符串 */
uint8_t LookForStr(uint8_t *s, uint8_t *t)
{
	uint8_t i = 0;
	uint8_t *s_temp;
	uint8_t *m_temp;
	uint8_t *t_temp;
	
	if (s==0 || t==0) return 0;
	
	for (s_temp=s; *s_temp!='\0'; s_temp++,i++)
	{
		for (m_temp=s_temp, t_temp=t; *t_temp!='\0' && *t_temp==*m_temp; t_temp++, m_temp++);
		if (*t_temp == '\0')
		{
			return i;
		}
	}
	
	return FULL;
}

void ConvertEndian(uint8_t *buffer, uint8_t num)
{
	uint8_t i;
	uint8_t temp;
	
	for (i = 0; i< num/2+1; i++)
	{
		temp = *(buffer+i);
		*(buffer+i) = *(buffer+num-1-i);
		*(buffer+num-1-i) = temp;
	}
}

/* CRC校验 */
uint32_t NetComputeCRC(uint8_t *bufData, uint16_t buflen, uint8_t *pcrc)
{
	uint16_t i, j;
	uint32_t ret = 0;
	uint32_t CRCa = 0xffff;
	uint32_t POLYNOMIAL = 0x0000a001;
	
	if (buflen == 0)
	{
		return ret;
	}
	
	for (i = 0; i < buflen; i++)
	{
		CRCa ^= ((uint32_t) bufData[i] & 0x000000ff);
		for (j = 0; j < 8; j++)
		{
			if ((CRCa & 0x00000001) != 0)
			{
				CRCa >>= 1;
				CRCa ^= POLYNOMIAL;
			}
			else 
			{
				CRCa >>= 1;
			}
		}
	}
	pcrc[0] = (uint8_t) (CRCa & 0x00ff);
	pcrc[1] = (uint8_t) (CRCa >> 8);
	
	return ret;
}

/* 软件延时函数, ms级别 */
void Soft_delay_ms(uint16_t time)
{
   uint16_t i = 0;
   
   while(time--)
   {
      i = 666;
      while(i--);
   }
}

/* 软件延时函数, us级别 */
void Soft_delay_us(uint16_t time)
{
   while(time--)
   {
      asm("nop");
	  asm("nop");
	  asm("nop");
	  asm("nop");
	  asm("nop");
	  asm("nop");
	  asm("nop");
	  asm("nop");
	  asm("nop");
	  asm("nop");
   }
}
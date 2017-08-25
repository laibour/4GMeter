#include "config.h"
#include "gprs.h"
#include "gprs-Hal.h"
#include "gprs-Misc.h"


uint8_t Deact[] = "DEACT";   /* PDP Context Deactivate */
uint8_t Close[] = "CLOSED";  /* TCP 线程被关闭后返回字符 */

uint8_t ResetFlag = 0;
uint8_t GPRSOnline = 0;
uint8_t GPRSSendFlag = 0;
uint8_t GPRSSendSize = 0;
uint8_t GPRSSendBuffer[GPRS_BUFFER_SIZE];
uint8_t GPRSRecvFlag = 0;
uint8_t GPRSRecvSize = 0;
uint8_t GPRSRecvBuffer[GPRS_BUFFER_SIZE];

static uint8_t NetSendSize = 0;
static uint8_t NetSendBuffer[GPRS_BUFFER_SIZE];
static tGPRSStates GPRSState = GPRS_STATE_IDLE;

static void GPRSReplyHandle(uint8_t *buffer, uint8_t size);

void GPRSProcess(void)
{
	uint8_t ret;
	static T_HeartBeat Register;
	static uint8_t RegisterNum = 0;
	static uint32_t TxTimeoutTimer;
	static uint32_t HeartTimeoutTimer;
	static uint32_t RegisterTimeoutTimer;
	
	switch(GPRSState)
	{
		case GPRS_STATE_IDLE:
			if (GPRSFlag == 1)
			{
				GPRSState = GPRS_STATE_POWERON;
				WakeupState.GPRSStatus = TRIG_ACTIVE;
			}
			else
			{
				GPRS_POWER_OFF();
				GPRS_PWRKEY_OFF();
				WakeupState.GPRSStatus = TRIG_INIT;
			}
			break;
		
		case GPRS_STATE_POWERON:
			if ( GPRSPowerOn() )
			{
				GPRSState = GPRS_STATE_CFG;
			}
			break;
		
		case GPRS_STATE_CFG:
			ret = GPRSCfg();
			if (ret == GPRS_DONE)
			{
				GPRSState = GRPS_STATE_OPEN;
			}
			else if (ret == GPRS_ERROR)
			{
				GPRSState = GPRS_STATE_POWERON;
			}
			break;
		
		case GRPS_STATE_OPEN:
			ret = GPRSOpen();
			if (ret == GPRS_DONE)
			{
				Soft_delay_ms(500);
				Register = HeartBeat;
				Register.Head.FuncCode = 0x3004;
				HeartBeat.Head.PacketID.Order++;
				memcpy(&Register.Head.PacketID.yearL, &SystemTime.yearL, 3);
				NetComputeCRC((uint8_t *)&Register, 30, (uint8_t *)&Register.Crc);
				USART3_SendData((uint8_t *)&Register, sizeof(T_HeartBeat));
				DATA_PRINTF((uint8_t *)&Register, sizeof(T_HeartBeat));
				RegisterTimeoutTimer = TimeCnt;
				GPRSState = GPRS_STATE_REGISTER;
			}
			else if (ret == GPRS_ERROR)
			{
				GPRSState = GPRS_STATE_EXIT;
			}
			break;
			
		case GPRS_STATE_REGISTER:
			if (GetTimer(RegisterTimeoutTimer) < 1000)  /* 注册 */
			{
				if (GPRSRecvFlag == 1)  /* 接收到GPRS数据 */
				{
					GPRSRecvFlag = 0;
					DATA_PRINTF(GPRSRecvBuffer, GPRSRecvSize);
					if (GPRSRecvBuffer[3]==0x30 && GPRSRecvBuffer[4]==0x04)
					{
						RegisterNum = 0;
						Soft_delay_ms(5);
						RTCWrite((PT_SystemTime)(GPRSRecvBuffer+32));
						TxTimeoutTimer    = TimeCnt;
						HeartTimeoutTimer = TimeCnt;
						GPRSState = GPRS_STATE_RUNNING;
					}
				}
			}
			else
			{
				RegisterNum++;
				if (RegisterNum < 3)
				{
					USART3_SendData((uint8_t *)&Register, sizeof(Register));
					DATA_PRINTF((uint8_t *)&Register, sizeof(Register));
					RegisterTimeoutTimer = TimeCnt;
				}
				else
				{
					RegisterNum = 0;
					GPRSState = GPRS_STATE_EXIT;
				}
			}
			break;
		
		case GPRS_STATE_RUNNING:
			if (GetTimer(TxTimeoutTimer) < MeterParm.GPRSHeartTime)
			{
				if (GPRSRecvFlag == 1)  /* 接收到GPRS数据 */
				{
					GPRSRecvFlag = 0;
					DATA_PRINTF(GPRSRecvBuffer, GPRSRecvSize);
					if (GPRSRecvBuffer[3]==0x30 && GPRSRecvBuffer[4]==0x03)  /* recieved heartbeat */
					{
						HeartTimeoutTimer = TimeCnt;
					}
					else if (GPRSRecvBuffer[0]==0x68 && GPRSRecvBuffer[GPRSRecvSize-1]==0x16)  /* recieved net data */
					{
						TxTimeoutTimer    = TimeCnt;
						HeartTimeoutTimer = TimeCnt;
						GPRSReplyHandle(GPRSRecvBuffer, GPRSRecvSize);
					}
					else if (LookForStr(GPRSRecvBuffer, Close)!=FULL || LookForStr(GPRSRecvBuffer, Deact)!=FULL)  /* off line data */
					{
						GPRSState = GRPS_STATE_OPEN;
					}
					else
					{
						memset(GPRSRecvBuffer, 0, GPRS_BUFFER_SIZE);
					}
				}
				
				if (GPRSSendFlag == 1)	/* 发送GPRS数据 */
				{
					GPRSSendFlag = 0;
					TxTimeoutTimer = TimeCnt;
					NetComputeCRC((uint8_t *)&GPRSSendBuffer, GPRSSendSize-3, GPRSSendBuffer+GPRSSendSize-3);
					USART3_SendData(GPRSSendBuffer, GPRSSendSize);
					DATA_PRINTF(GPRSSendBuffer, GPRSSendSize);
					if (ResetFlag == 1)
					{
						Soft_delay_ms(2500);
						WWDG_SWReset();
					}
				}
			}
			else
			{
				HeartBeat.Head.PacketID.Order++;
				memcpy(&HeartBeat.Head.PacketID.yearL, &SystemTime.yearL, 3);
				NetComputeCRC((uint8_t *)&HeartBeat, 30, (uint8_t *)&HeartBeat.Crc);
				USART3_SendData((uint8_t *)&HeartBeat, sizeof(T_HeartBeat));
				DATA_PRINTF((uint8_t *)&HeartBeat, sizeof(T_HeartBeat));
				TxTimeoutTimer = TimeCnt;
			}
			
			if (GetTimer(HeartTimeoutTimer) > 12000)  /* 超时断线检测 */
			{
				GPRSState = GRPS_STATE_OPEN;
			}
			break;
		
		case GPRS_STATE_EXIT:
			if (GPRSExit())
			{
				GPRSState = GPRS_STATE_CFG;
			}
			break;
			
		default:
			break;
	}
	
	if (GPRSState == GPRS_STATE_RUNNING || GPRSState == GPRS_STATE_REGISTER)  /* GPRS在线状态 */
	{
		GPRSOnline = 1;
	}
	else
	{
		GPRSOnline = 0;
	}
	
	if (WakeupState.GPRSStatus == TRIG_INIT)
	{
		GPRSState = GPRS_STATE_IDLE;
		WakeupState.GPRSStatus = TRIG_SLEEP;
	}
}

void GPRSReplyHandle(uint8_t *buffer, uint8_t size)
{
	uint8_t i;
	uint16_t TempID;
	uint8_t ReLink = 0;
	uint8_t WriteFlag = 0;
	uint8_t CommonBack = 1;
	uint8_t NetSendFlag = 1;
	uint32_t TotalData, TempData;
	PT_NetHead ptNetHead = (PT_NetHead)NetSendBuffer;
	
	memcpy(&TempID, buffer+3, 2);
	memcpy(NetSendBuffer, buffer, 23);
	if (buffer[5] == 0x00 && buffer[6] == 0x00)  /* 主站请求 */
	{
		if (TempID == 0x2100)  /* 恢复出厂设置 */
		{
			EEPROMReset();
			ResetFlag = 1;
		}
		else if (TempID == 0x3005)  /* 表具基本参数配置 */
		{
			WriteFlag = 1;
			MeterParm.isUse = 1;
			MeterParm.MeterType  = (tMeterType)buffer[23];
			MeterParm.TotalGas   = ConvertEndian32(*(uint32_t *)(buffer+24));
			MeterParm.TotalMoney = 100 * ConvertEndian32(*(uint32_t *)(buffer+28));
			
			MeterParm.UploadCycle = *(uint16_t *)(buffer+32);
			memcpy(&MeterParm.Wakeup.Time, buffer+34, 3);
			MeterParm.Wakeup.Duration = ConvertEndian16(*(uint16_t *)(buffer+37));
			MeterParm.Wakeup.Offset   = ConvertEndian16(*(uint16_t *)(buffer+39));
			
			MeterParm.CurPrice.PriceType = buffer[41];
			MeterParm.CurPrice.PriceVer  = buffer[42];
			MeterParm.CurPrice.ActPeriod = buffer[43];
			MeterParm.CurPrice.PriceDevide[0] = ConvertEndian16(*(uint16_t *)(buffer+44));
			MeterParm.CurPrice.GasDivide[0]   = ConvertEndian16(*(uint16_t *)(buffer+46));
			MeterParm.CurPrice.PriceDevide[1] = ConvertEndian16(*(uint16_t *)(buffer+48));
			MeterParm.CurPrice.GasDivide[1]   = ConvertEndian16(*(uint16_t *)(buffer+50));
			MeterParm.CurPrice.PriceDevide[2] = ConvertEndian16(*(uint16_t *)(buffer+52));
			
			MeterParm.CurPrice.StepCycle = (tStepCycle)buffer[54];
			MeterParm.CurPrice.ActMonth  = buffer[55];
			MeterParm.MarginWarn         = (uint32_t)buffer[56] * 10000;
			MeterParm.LongtimeProtect    = buffer[57];
			MeterParm.CornerLimit        = buffer[58];
			MeterParm.FlowLimit          = buffer[59];
			MeterParm.LittleUseGas       = buffer[60];
			MeterParm.ContinuousUseTime  = buffer[61];
			MeterParm.OverLimit          = 100 * (uint32_t)ConvertEndian16(*(uint16_t *)(buffer+62));
			MeterParm.MagnetMax = (buffer[64] & 0x03);		 /* 防磁保护次数上限 */
			MeterParm.CountMax  = ((buffer[64]>>2) & 0x03);	 /* 采样保护次数上限 */
			MeterParm.FlowMax   = ((buffer[64]>>4) & 0x0F);	 /* 过流保护次数上限 */
			memcpy((uint8_t *)&MeterParm.MeterOutDate, buffer+65, 4);
			
			MeterParm.RemainMoney = MeterParm.TotalMoney;
			MeterParm.CurUnitPrice = MeterParm.CurPrice.PriceDevide[0];
			MeterParm.MeterSta.bits.newmeter = MeterParm.isUse;
		}
		else if (TempID == 0x3006)  /* 表具基本参数查询 */
		{
			CommonBack = 0;
			ptNetHead->PacketLen = 74;
			ptNetHead->DataLen   = 48;
			memset(NetSendBuffer+23, 0x00, 2);
			NetSendBuffer[25] = MeterParm.MeterType;
			*(uint32_t *)(NetSendBuffer+26) = ConvertEndian32(MeterParm.TotalGas);
			*(uint32_t *)(NetSendBuffer+30) = ConvertEndian32(MeterParm.TotalMoney / 100);
			
			*(uint16_t *)(NetSendBuffer+34) = MeterParm.UploadCycle;
			memcpy(NetSendBuffer+36, (uint8_t *)&MeterParm.Wakeup.Time, 3);
			*(uint16_t *)(NetSendBuffer+39) = ConvertEndian16(MeterParm.Wakeup.Duration);
			*(uint16_t *)(NetSendBuffer+41) = ConvertEndian16(MeterParm.Wakeup.Offset);
			
			NetSendBuffer[43] = MeterParm.CurPrice.PriceType;
			NetSendBuffer[44] = MeterParm.CurPrice.PriceVer;
			NetSendBuffer[45] = MeterParm.CurPrice.ActPeriod;
			*(uint16_t *)(NetSendBuffer+46) = ConvertEndian16(MeterParm.CurPrice.PriceDevide[0]);
			*(uint16_t *)(NetSendBuffer+48) = ConvertEndian16(MeterParm.CurPrice.GasDivide[0]);
			*(uint16_t *)(NetSendBuffer+50) = ConvertEndian16(MeterParm.CurPrice.PriceDevide[1]);
			*(uint16_t *)(NetSendBuffer+52) = ConvertEndian16(MeterParm.CurPrice.GasDivide[1]);
			*(uint16_t *)(NetSendBuffer+54) = ConvertEndian16(MeterParm.CurPrice.PriceDevide[2]);
			
			NetSendBuffer[56] = MeterParm.CurPrice.StepCycle;
			NetSendBuffer[57] = MeterParm.CurPrice.ActMonth;
			NetSendBuffer[58] = (uint8_t)(MeterParm.MarginWarn / 10000);
			NetSendBuffer[59] = MeterParm.LongtimeProtect;
			NetSendBuffer[60] = MeterParm.CornerLimit;
			NetSendBuffer[61] = MeterParm.FlowLimit;
			NetSendBuffer[62] = MeterParm.LittleUseGas;
			NetSendBuffer[63] = MeterParm.ContinuousUseTime;
			NetSendBuffer[64] = MeterParm.OverLimit;
			*(uint16_t *)(NetSendBuffer+64) = ConvertEndian16((uint16_t)(MeterParm.OverLimit/100));
			NetSendBuffer[66] = ((MeterParm.MagnetMax & 0x03) | ((MeterParm.CountMax & 0x03)<<2) | ((MeterParm.FlowMax & 0x0F)<<4));
			memcpy(NetSendBuffer+67, (uint8_t *)&MeterParm.MeterOutDate, 4);
			memset(NetSendBuffer+73, 0x16, 1);
			NetSendSize = 74;
		}
		else if (TempID == 0x3021)  /* 心跳周期设置 */
		{
			WriteFlag = 1;
			MeterParm.GPRSHeartTime = ConvertEndian16(*(uint16_t *)(buffer+30))*100;
		}
		else if (TempID == 0x3022)  /* 心跳周期查询 */
		{
			CommonBack = 0;
			ptNetHead->PacketLen = 37;
			ptNetHead->DataLen   = 11;
			memset(NetSendBuffer+23, 0x00, 2);
			memcpy(NetSendBuffer+25, &MeterParm.SlaveAddr, 7);
			*(uint16_t *)(NetSendBuffer+32) = ConvertEndian16(MeterParm.GPRSHeartTime/100);
			memset(NetSendBuffer+36, 0x16, 1);
			NetSendSize = 37;
		}
		else if (TempID == 0x3023)  /* 上报周期设置 */
		{
			WriteFlag = 1;
			MeterParm.UploadCycle = *(uint16_t *)(buffer+30);
		}
		else if (TempID == 0x3024)  /* 上报周期查询 */
		{
			CommonBack = 0;
			ptNetHead->PacketLen = 37;
			ptNetHead->DataLen   = 11;
			memset(NetSendBuffer+23, 0x00, 2);
			memcpy(NetSendBuffer+25, &MeterParm.SlaveAddr, 7);
			*(uint16_t *)(NetSendBuffer+32) = MeterParm.UploadCycle;
			memset(NetSendBuffer+36, 0x16, 1);
			NetSendSize = 37;
		}
		else if (TempID == 0x3025)  /* 时钟设置 */
		{
			CommonBack = 0;
			ptNetHead->PacketLen = 35;
			ptNetHead->DataLen   = 9;
			RTCWrite((PT_SystemTime)(buffer+30));
			memset(NetSendBuffer+23, 0x00, 2);
			memcpy(NetSendBuffer+25, &MeterParm.SlaveAddr, 7);
			memset(NetSendBuffer+34, 0x16, 1);
			NetSendSize = 35;
		}
		else if (TempID == 0x3026)  /* 时钟查询 */
		{
			CommonBack = 0;
			ptNetHead->PacketLen = 42;
			ptNetHead->DataLen   = 16;
			memset(NetSendBuffer+23, 0x00, 2);
			memcpy(NetSendBuffer+25, &MeterParm.SlaveAddr, 7);
			memcpy(NetSendBuffer+32, (uint8_t *)&SystemTime, 7);
			memset(NetSendBuffer+41, 0x16, 1);
			NetSendSize = 42;
		}
		else if (TempID == 0x3027)  /* 主站IP设置 */
		{
			ReLink = 1;
			WriteFlag  = 1;
			CommonBack = 0;
			ptNetHead->PacketLen = 35;
			ptNetHead->DataLen   = 9;
			
			MeterParm.Socket.IP[0] = ((*(buffer+30) >> 4) & 0x0F) + 0x30;
			MeterParm.Socket.IP[1] = (*(buffer+30) & 0x0F) + 0x30;
			
			MeterParm.Socket.IP[2] = ((*(buffer+31) >> 4) & 0x0F) + 0x30;
			MeterParm.Socket.IP[4] = (*(buffer+31) & 0x0F) + 0x30;
			
			MeterParm.Socket.IP[5] = ((*(buffer+32) >> 4) & 0x0F) + 0x30;
			MeterParm.Socket.IP[6] = (*(buffer+32) & 0x0F) + 0x30;
			
			MeterParm.Socket.IP[8] = ((*(buffer+33) >> 4) & 0x0F) + 0x30;
			MeterParm.Socket.IP[9] = (*(buffer+33) & 0x0F) + 0x30;
			
			MeterParm.Socket.IP[10] = ((*(buffer+34) >> 4) & 0x0F) + 0x30;
			MeterParm.Socket.IP[12] = (*(buffer+34) & 0x0F) + 0x30;
			
			MeterParm.Socket.IP[13] = ((*(buffer+35) >> 4) & 0x0F) + 0x30;
			MeterParm.Socket.IP[14] = (*(buffer+35) & 0x0F) + 0x30;
			
			MeterParm.Socket.Port[0] = ((*(buffer+36) >> 4) & 0x0F) + 0x30;
			MeterParm.Socket.Port[1] = (*(buffer+36) & 0x0F) + 0x30;
			
			MeterParm.Socket.Port[2] = ((*(buffer+37) >> 4) & 0x0F) + 0x30;
			MeterParm.Socket.Port[3] = (*(buffer+37) & 0x0F) + 0x30;
			
			MeterParm.Socket.Port[4] = ((*(buffer+38) >> 4) & 0x0F) + 0x30;
			MeterParm.Socket.Port[5] = (*(buffer+38) & 0x0F) + 0x30;
			
			/* 主站IP参数 */
			for (i = 0; i < 15; i++)  /* IP */
			{
				GPRSOpenData[20+i] = MeterParm.Socket.IP[i];
			}
			
			for (i = 0; i < 5; i++)  /* PORT */
			{
				GPRSOpenData[37+i] = MeterParm.Socket.Port[1+i];
			}
			
			memset(NetSendBuffer+23, 0x00, 2);
			memcpy(NetSendBuffer+25, &MeterParm.SlaveAddr, 7);
			memset(NetSendBuffer+34, 0x16, 1);
			NetSendSize = 35;
		}
		else if (TempID == 0x3028)  /* 主站IP查询 */
		{
			CommonBack = 0;
			ptNetHead->PacketLen = 44;
			ptNetHead->DataLen   = 18;
			memset(NetSendBuffer+23, 0x00, 2);
			memcpy(NetSendBuffer+25, &MeterParm.SlaveAddr, 7);
			NetSendBuffer[32] = (((MeterParm.Socket.IP[0] - 0x30)&0x0F) << 4) + ((MeterParm.Socket.IP[1]-0x30)&0x0F);
			NetSendBuffer[33] = (((MeterParm.Socket.IP[2] - 0x30)&0x0F) << 4) + ((MeterParm.Socket.IP[4]-0x30)&0x0F);
			NetSendBuffer[34] = (((MeterParm.Socket.IP[5] - 0x30)&0x0F) << 4) + ((MeterParm.Socket.IP[6]-0x30)&0x0F);
			NetSendBuffer[35] = (((MeterParm.Socket.IP[8] - 0x30)&0x0F) << 4) + ((MeterParm.Socket.IP[9]-0x30)&0x0F);
			NetSendBuffer[36] = (((MeterParm.Socket.IP[10] - 0x30)&0x0F) << 4) + ((MeterParm.Socket.IP[12]-0x30)&0x0F);
			NetSendBuffer[37] = (((MeterParm.Socket.IP[13] - 0x30)&0x0F) << 4) + ((MeterParm.Socket.IP[14]-0x30)&0x0F);
			
			NetSendBuffer[38] = (((MeterParm.Socket.Port[0] - 0x30)&0x0F) << 4) + ((MeterParm.Socket.Port[1]-0x30)&0x0F);
			NetSendBuffer[39] = (((MeterParm.Socket.Port[2] - 0x30)&0x0F) << 4) + ((MeterParm.Socket.Port[3]-0x30)&0x0F);
			NetSendBuffer[40] = (((MeterParm.Socket.Port[4] - 0x30)&0x0F) << 4) + ((MeterParm.Socket.Port[5]-0x30)&0x0F);
			
			memset(NetSendBuffer+43, 0x16, 1);
			NetSendSize = 44;
		}
		else if (TempID == 0x3029)  /* 自醒上报时间设置 */
		{
			WriteFlag  = 1;
			CommonBack = 0;
			ptNetHead->PacketLen = 35;
			ptNetHead->DataLen   = 9;
			memcpy((uint8_t *)&MeterParm.Wakeup, buffer+30, 7);
			MeterParm.Wakeup.Duration = ConvertEndian16(MeterParm.Wakeup.Duration);
			MeterParm.Wakeup.Offset = ConvertEndian16(MeterParm.Wakeup.Offset);
			memset(NetSendBuffer+23, 0x00, 2);
			memcpy(NetSendBuffer+25, &MeterParm.SlaveAddr, 7);
			memset(NetSendBuffer+34, 0x16, 1);
			NetSendSize = 35;
		}
		else if (TempID == 0x3020)  /* 自醒上报时间查询 */
		{
			CommonBack = 0;
			ptNetHead->PacketLen = 42;
			ptNetHead->DataLen   = 16;
			memset(NetSendBuffer+23, 0x00, 2);
			memcpy(NetSendBuffer+25, &MeterParm.SlaveAddr, 7);
			memcpy(NetSendBuffer+32, (uint8_t *)&MeterParm.Wakeup.Time, 3);
			*(uint16_t *)(NetSendBuffer+35) = ConvertEndian16(MeterParm.Wakeup.Duration);
			*(uint16_t *)(NetSendBuffer+37) = ConvertEndian16(MeterParm.Wakeup.Offset);
			memset(NetSendBuffer+41, 0x16, 1);
			NetSendSize = 42;
		}
		else if (TempID == 0x3042)  /* 中心主动抄表 */
		{
			CommonBack = 0;		
			ptNetHead->PacketLen = 54;
			ptNetHead->DataLen   = 28;
			memset(NetSendBuffer+23, 0x00, 2);
			NetSendBuffer[25] = MeterParm.MeterType;
			*(uint32_t *)(NetSendBuffer+26) = ConvertEndian32(MeterParm.TotalGas);
			*(uint32_t *)(NetSendBuffer+30) = ConvertEndian32((MeterParm.UsedMoney + 50) / 100);
			*(uint32_t *)(NetSendBuffer+34) = ConvertEndian32((MeterParm.RemainMoney + 50) / 100);
			memset(NetSendBuffer+38, GetMeterSta(), 1);
			NetSendBuffer[39] = MotorSta.SW;
			memset(NetSendBuffer+40, GetFaultNumber(), 1);  /* 报警保护次数 */
			memset(NetSendBuffer+41, GetBatStatus(), 1);    /* 电池电量及状态 */
			memset(NetSendBuffer+42, MeterParm.CurPrice.PriceVer, 1);
			memset(NetSendBuffer+43, 0x10, 1);
			memcpy(NetSendBuffer+44, (uint8_t *)&SystemTime, 7);
			memset(NetSendBuffer+53, 0x16, 1);
			NetSendSize = 54;
		}
		else if (TempID == 0x3043)  /* 查询预调气价 */
		{
			CommonBack = 0;
			ptNetHead->PacketLen = 43;
			ptNetHead->DataLen   = 17;
			memset(NetSendBuffer+23, 0x00, 2);
			NetSendBuffer[25] = MeterParm.NewPrice.PriceType;
			NetSendBuffer[26] = MeterParm.NewPrice.PriceVer;
			NetSendBuffer[27] = MeterParm.NewPrice.ActMonth;
			NetSendBuffer[28] = MeterParm.NewPrice.ActPeriod;
			*(uint16_t *)(NetSendBuffer+29) = ConvertEndian16(MeterParm.NewPrice.PriceDevide[0]);
			*(uint16_t *)(NetSendBuffer+31) = ConvertEndian16(MeterParm.NewPrice.GasDivide[0]);
			*(uint16_t *)(NetSendBuffer+33) = ConvertEndian16(MeterParm.NewPrice.PriceDevide[1]);
			*(uint16_t *)(NetSendBuffer+35) = ConvertEndian16(MeterParm.NewPrice.GasDivide[1]);
			*(uint16_t *)(NetSendBuffer+37) = ConvertEndian16(MeterParm.NewPrice.PriceDevide[2]);
			NetSendBuffer[39] = MeterParm.CurPrice.StepCycle;			
			memset(NetSendBuffer+42, 0x16, 1);
			NetSendSize = 43;
		}
		else if (TempID == 0x3044)  /* 日用量历史查询 */
		{
			CommonBack  = 0;
			NetSendFlag = 0;
			memset(&MultiPacket, 0, sizeof(T_MultiPacket));
			memcpy((uint8_t *)&MultiPacket.NetHead, buffer, 23);
			MultiPacket.NetHead.TranDirect   = 0x01;
			MultiPacket.NetHead.ResponseFlag = 0x01;
			MultiPacket.Node = MeterParm.DayPoint;
			MultiPacket.PacketType  = PACKET_DAY;
			MultiPacket.PacketState = PACKET_STATE_INIT;
		}
		else if (TempID == 0x3045)  /* 月用量历史查询 */
		{
			CommonBack  = 0;
			NetSendFlag = 0;
			memset(&MultiPacket, 0, sizeof(T_MultiPacket));
			memcpy((uint8_t *)&MultiPacket.NetHead, buffer, 23);
			MultiPacket.NetHead.TranDirect   = 0x01;
			MultiPacket.NetHead.ResponseFlag = 0x01;
			MultiPacket.Node = MeterParm.MonthPoint;
			MultiPacket.PacketType  = PACKET_MONTH;
			MultiPacket.PacketState = PACKET_STATE_INIT;
		}
		else if (TempID == 0x3051)  /* 开阀 */
		{
			WriteFlag = 1;
			MeterParm.MeterSta.bits.force = 0;  /* 开阀，清零强制关阀标志 */
		}
		else if (TempID == 0x3052)  /* 强制关阀 */
		{
			WriteFlag = 1;
			CloseMotor(1);
			AddNetTask(C3062, 1);				/* 强制关阀，异常上报 */
			MeterParm.MeterSta.bits.force = 1;  /* 关阀，置位强制关阀位 */
		}
		else if (TempID == 0x3053)	/* 充值 */
		{
			WriteFlag = 1;
			if (*(GPRSRecvBuffer+27) > 0)	/* 判断充值次数 */
			{
				TotalData = 100 * ConvertEndian32(*(uint32_t *)(GPRSRecvBuffer+23));
				if (TotalData >= MeterParm.TotalMoney)  /* 充值 */
				{
					TempData = MeterParm.TotalMoney;
					MeterParm.TotalMoney = TotalData;
					TempData = TotalData - TempData;
					if (MeterParm.OverMoney != 0)	/* 已透支 */
					{
						if (TempData >= MeterParm.OverMoney)
						{
							MeterParm.RemainMoney += (TempData - MeterParm.OverMoney);
							MeterParm.OverMoney = 0;
						}
						else
						{
							MeterParm.OverMoney -= TempData;
						}
					}
					else
					{
						MeterParm.RemainMoney += TempData;
					}
				}
				else  /* 扣费 */
				{
					TempData = MeterParm.TotalMoney;
					MeterParm.TotalMoney = TotalData;
					TempData = TempData - TotalData;
					if (MeterParm.OverMoney != 0)	/* 已透支 */
					{
						MeterParm.OverMoney += TempData;
					}
					else
					{
						if (MeterParm.RemainMoney >= TempData)
						{
							MeterParm.RemainMoney -= TempData;
						}
						else
						{
							MeterParm.OverMoney += (TempData - MeterParm.RemainMoney);
							MeterParm.RemainMoney = 0;
						}
					}
				}
				
				/* close motor? */
				if (MeterParm.RemainMoney == 0 && (MeterParm.OverMoney >= MeterParm.OverLimit))
				{
					MotorSta.CloseEn = 1;  /* 使能关阀 */
				}
			}
		}
		else if (TempID == 0x3054)  /* 调价 */
		{
			uint8_t year, month, sum;
			
			year  = BCDToDec(&SystemTime.yearL, 1);
			month = BCDToDec(&SystemTime.month, 1);
			sum   = (year - 16) * 12 + month;  /* 从2016年1月到当前的月数 */
			
			/* 预调价判断 */
			if (sum < *(GPRSRecvBuffer+25))
			{
				MeterParm.NewPrice.PriceType = *(GPRSRecvBuffer+23);
				MeterParm.NewPrice.PriceVer  = *(GPRSRecvBuffer+24);
				MeterParm.NewPrice.ActMonth  = *(GPRSRecvBuffer+25);
				MeterParm.NewPrice.ActPeriod = *(GPRSRecvBuffer+26);
				MeterParm.NewPrice.StepCycle = (tStepCycle)*(GPRSRecvBuffer+37);
				
				MeterParm.NewPrice.PriceDevide[0] = ConvertEndian16(*(uint16_t *)(GPRSRecvBuffer+27));
				MeterParm.NewPrice.GasDivide[0]   = ConvertEndian16(*(uint16_t *)(GPRSRecvBuffer+29));
				MeterParm.NewPrice.PriceDevide[1] = ConvertEndian16(*(uint16_t *)(GPRSRecvBuffer+31));
				MeterParm.NewPrice.GasDivide[1]   = ConvertEndian16(*(uint16_t *)(GPRSRecvBuffer+33));
				MeterParm.NewPrice.PriceDevide[2] = ConvertEndian16(*(uint16_t *)(GPRSRecvBuffer+35));
			}
			else
			{
				MeterParm.CurPrice.PriceType = *(GPRSRecvBuffer+23);
				MeterParm.CurPrice.PriceVer  = *(GPRSRecvBuffer+24);
				MeterParm.CurPrice.ActMonth  = *(GPRSRecvBuffer+25);
				MeterParm.CurPrice.ActPeriod = *(GPRSRecvBuffer+26);
				MeterParm.CurPrice.StepCycle = (tStepCycle)*(GPRSRecvBuffer+37);
				
				MeterParm.CurPrice.PriceDevide[0] = ConvertEndian16(*(uint16_t *)(GPRSRecvBuffer+27));
				MeterParm.CurPrice.GasDivide[0]   = ConvertEndian16(*(uint16_t *)(GPRSRecvBuffer+29));
				MeterParm.CurPrice.PriceDevide[1] = ConvertEndian16(*(uint16_t *)(GPRSRecvBuffer+31));
				MeterParm.CurPrice.GasDivide[1]   = ConvertEndian16(*(uint16_t *)(GPRSRecvBuffer+33));
				MeterParm.CurPrice.PriceDevide[2] = ConvertEndian16(*(uint16_t *)(GPRSRecvBuffer+35));
				MeterParm.CurUnitPrice = MeterParm.CurPrice.PriceDevide[0];
			}
		}
		else if (TempID == 0x3055)  /* 清标志 */
		{
			BYTE_VAL FlagTemp;
			
			FlagTemp.Val = *(GPRSRecvBuffer+23);
			if (FlagTemp.bits.b7 == 1)  /* 表具恢复出厂 */
			{
				MeterParm.isUse = 0;
				EEPROMWriteBuffer((uint8_t *)&MeterParm, METER_PAM_ADDR, (uint8_t)sizeof(T_MeterParm));
			}
			else if (FlagTemp.bits.b5 == 1)  /* 标志位总清 */
			{
				MeterParm.MeterSta.Val &= 0xE0;
			}
			else  /* 清标志 */
			{
				MeterParm.MeterSta.Val &=  (~((*(GPRSRecvBuffer+23)) & 0x1F));
				MeterParm.MeterSta.bits.force = 0;
			}
		}
		else if (TempID == 0x3056)  /* 设置表具到期日 */
		{
			WriteFlag = 1;
			memcpy((uint8_t *)&MeterParm.MeterOutDate, buffer+23, 4);
		}
		else if (TempID == 0x3057)  /* 设置表具限定值 */
		{
			WriteFlag = 1;
			MeterParm.MarginWarn         = (uint32_t)buffer[23] * 10000;
			MeterParm.LongtimeProtect    = buffer[24];
			MeterParm.CornerLimit        = buffer[25];
			MeterParm.FlowLimit          = buffer[26];
			MeterParm.LittleUseGas       = buffer[27];
			MeterParm.ContinuousUseTime  = buffer[28];
			MeterParm.OverLimit          = 100 * (uint32_t)ConvertEndian16(*(uint16_t *)(buffer+29));
			MeterParm.MagnetMax = (buffer[31] & 0x03);		 /* 防磁保护次数上限 */
			MeterParm.CountMax  = ((buffer[31]>>2) & 0x03);	 /* 采样保护次数上限 */
			MeterParm.FlowMax   = ((buffer[31]>>4) & 0x0F);	 /* 过流保护次数上限 */
		}
		else if (TempID == 0x3058)  /* 设置表具用量和金额 */
		{
			WriteFlag = 1;
			MeterParm.TotalGas   = ConvertEndian32(*(uint32_t *)(buffer+23));
			MeterParm.UsedMoney  = 100 * ConvertEndian32(*(uint32_t *)(buffer+27));
			MeterParm.TotalMoney = 100 * ConvertEndian32(*(uint32_t *)(buffer+31));
			MeterParm.RemainMoney = MeterParm.TotalMoney - MeterParm.UsedMoney;
			MeterParm.RechargeNum = buffer[35];
		}
		else if (TempID == 0x3065)  /* 故障消息清除指令 */
		{
			WriteFlag = 1;
			MeterParm.MeterSta.Val = 0;
		}
		else
		{
			CommonBack = 0;
			ptNetHead->DataLen  = 0x02;
			memset(NetSendBuffer+23, 0x01, 1);  /* 2bytes error code */
			memset(NetSendBuffer+24, 0x18, 1);
			memset(NetSendBuffer+27, 0x16, 1);
			NetSendSize = 28;
		}
		
		if (WriteFlag == 1)
		{
			EEPROMWriteBuffer((uint8_t *)&MeterParm, METER_PAM_ADDR, (uint8_t)sizeof(T_MeterParm));
		}
		
		if (CommonBack == 1)
		{
			ptNetHead->PacketLen = 35;
			ptNetHead->DataLen   = 9;
			memset(NetSendBuffer+23, 0x00, 2);
			memcpy(NetSendBuffer+25, &MeterParm.SlaveAddr, 7);
			memset(NetSendBuffer+34, 0x16, 1);
			NetSendSize = 35;
		}
		
		if (NetSendFlag != 0)
		{
			ptNetHead->TranDirect   = 0x01;
			ptNetHead->ResponseFlag = 0x01;
			ptNetHead->PacketLen = ConvertEndian16(ptNetHead->PacketLen);
			ptNetHead->DataLen   = ConvertEndian16(ptNetHead->DataLen);
			NetComputeCRC(NetSendBuffer, NetSendSize-3, NetSendBuffer+NetSendSize-3);  // CRC
			USART3_SendData(NetSendBuffer, NetSendSize);  // send GPRS data
			DATA_PRINTF(NetSendBuffer, NetSendSize);
			if (ReLink == 1)
			{
				Soft_delay_ms(2000);
				GPRSState = GPRS_STATE_IDLE;
			}
		}
	}
	else if (buffer[5] == 0x00 && buffer[6] == 0x01)  /* 主站响应 */
	{
		if (TempID == 0x3041)  /* 主动上报 */
		{
			RTCWrite((PT_SystemTime)(GPRSRecvBuffer+25));
		}
	}
}

/* 打包上传至中心 */
void MultiPacketProcess(void)
{
	uint8_t m;
	T_MeterData tReadData;
	static uint32_t ReadSum;
	static uint32_t ReadAddr;
	static uint32_t MultiPacketTimer;
		
	if ((MultiPacket.PacketType != PACKET_IDLE) && (MultiPacket.PacketState != PACKET_STATE_IDLE))
	{
		if (MultiPacket.PacketState == PACKET_STATE_INIT)
		{
			if (MultiPacket.PacketType == PACKET_DAY)
			{
				ReadSum  = SAVE_DAY;
				ReadAddr = METER_DAY_ADDR;
			}
			else
			{
				ReadSum  = SAVE_MONTH;
				ReadAddr = METER_MONTH_ADDR;
			}
			MultiPacketTimer = TimeCnt;
			MultiPacket.PacketState = PACKET_STATE_RUN;
		}
		
		if (GetTimer(MultiPacketTimer) > 5)
		{
			MultiPacketTimer = TimeCnt;
			for (m = 0; m < NUM_OF_PACKET && MultiPacket.TotalNode < ReadSum; MultiPacket.TotalNode++, MultiPacket.Node=((MultiPacket.Node + ReadSum - 1)%ReadSum))
			{
				SPI_FLASH_ReadBuffer((uint8_t *)&tReadData, ReadAddr+MultiPacket.Node*sizeof(T_MeterData), sizeof(T_MeterData));
				if (tReadData.Time.yearH != 0)
				{
					tReadData.GasPot = ConvertEndian16(tReadData.GasPot);
					tReadData.GasInt = ConvertEndian32(tReadData.GasInt);
					memcpy(NetSendBuffer + 37 + m*sizeof(T_MeterData), (uint8_t *)&tReadData, sizeof(T_MeterData));
					m++;
				}
			}
			
			if (m == 0 || MultiPacket.TotalNode == ReadSum)  /* end */
			{
				MultiPacket.End = 1;
			}
			MultiPacket.FrameSeq++;
			MultiPacket.NetHead.PacketLen = 40 + m*sizeof(T_MeterData);
			MultiPacket.NetHead.DataLen = 14 + m*sizeof(T_MeterData);
			memcpy(NetSendBuffer, (uint8_t *)&MultiPacket.NetHead, 23);
			memset(NetSendBuffer+23, 0x00, 2);  /* response code */
			memcpy(NetSendBuffer+25, &MeterParm.SlaveAddr, 7);
			memset(NetSendBuffer+32, MultiPacket.End, 1);
			*(uint16_t *)(NetSendBuffer+33) = ConvertEndian16(MultiPacket.FrameSeq);
			memcpy(NetSendBuffer+35, &m, 1);
			NetSendBuffer[36] = 0;
			memset(NetSendBuffer+39+m*sizeof(T_MeterData), 0x16, 1);
			if (MultiPacket.End == 1)  /* end */
			{
				memset(&MultiPacket, 0, sizeof(T_MultiPacket));
			}
			else
			{
				MultiPacket.PacketState = PACKET_STATE_INIT;
			}
			NetSendSize = 40 + m*sizeof(T_MeterData);
			*(uint16_t *)(NetSendBuffer+1) = ConvertEndian16(*(uint16_t *)(NetSendBuffer+1));
			*(uint16_t *)(NetSendBuffer+21) = ConvertEndian16(*(uint16_t *)(NetSendBuffer+21));
			NetComputeCRC(NetSendBuffer, NetSendSize-3, NetSendBuffer+NetSendSize-3);
			USART3_SendData(NetSendBuffer, NetSendSize);
			DATA_PRINTF(NetSendBuffer, NetSendSize);
		}
		
		if (GetTimer(MultiPacketTimer) > 500)  /* 5s over time */
		{
			memset(&MultiPacket, 0, sizeof(T_MultiPacket));
		}
	}
}


uint8_t GPRSSendRec(uint8_t *str, uint8_t *find, uint16_t time, uint8_t num)
{
	uint8_t i;
	uint8_t CCLK[] = "CCLK: \"";
	uint8_t result = GPRS_BUSY;
	static uint8_t  SendNum = 0;
	static uint32_t TxTimeoutTimer;
    static enum
    {
        STATE_SEND,
        STATE_RUNNING
    }GPRS_SEND_RECV_STATE = STATE_SEND;
	
    switch(GPRS_SEND_RECV_STATE)
	{
		case STATE_SEND:
			memset(GPRSRecvBuffer, 0, GPRS_BUFFER_SIZE);
			USART3_SendString(str);
			TxTimeoutTimer = TimeCnt;
			GPRS_SEND_RECV_STATE = STATE_RUNNING;
			break;
		
		case STATE_RUNNING:
		  	if (GetTimer(TxTimeoutTimer) < time)
			{
				if (GPRSRecvFlag == 1)
				{
					GPRSRecvFlag = 0;
					DBG_PRINTF("%s \n", GPRSRecvBuffer);  // GPRS recv data
					if (LookForStr(GPRSRecvBuffer, find) != FULL)
					{
						if (LookForStr(str, AT_CCLK) != FULL && (i = LookForStr(GPRSRecvBuffer, CCLK)) != FULL)  /* update concentrator time */
						{
							SystemTime.yearH  = 0x20;
							SystemTime.yearL  = ((GPRSRecvBuffer[i+7] - 0x30) << 4) + ((GPRSRecvBuffer[i+8] - 0x30) & 0x0F);
							SystemTime.month  = ((GPRSRecvBuffer[i+10] - 0x30) << 4) + ((GPRSRecvBuffer[i+11] - 0x30) & 0x0F);
							SystemTime.day    = ((GPRSRecvBuffer[i+13] - 0x30) << 4) + ((GPRSRecvBuffer[i+14] - 0x30) & 0x0F);
							SystemTime.hour   = ((GPRSRecvBuffer[i+16] - 0x30) << 4) + ((GPRSRecvBuffer[i+17] - 0x30) & 0x0F);
							SystemTime.minute = ((GPRSRecvBuffer[i+19] - 0x30) << 4) + ((GPRSRecvBuffer[i+20] - 0x30) & 0x0F);
							SystemTime.second = ((GPRSRecvBuffer[i+22] - 0x30) << 4) + ((GPRSRecvBuffer[i+23] - 0x30) & 0x0F);
							RTCWrite(&SystemTime);
						}
						SendNum = 0;
						memset(GPRSRecvBuffer, 0, GPRS_BUFFER_SIZE);
						GPRS_SEND_RECV_STATE = STATE_SEND;
						result = GPRS_DONE;
					}
					else
					{
						memset(GPRSRecvBuffer, 0, GPRS_BUFFER_SIZE);
					}
				}
			}
			else
			{
				SendNum++;
				if (SendNum == num)
				{
					SendNum = 0;
					DBG_PRINTF("%s send error! \n", str);
					memset(GPRSRecvBuffer, 0, GPRS_BUFFER_SIZE);
					result = GPRS_ERROR;
				}
				GPRSRecvFlag = 0;
				GPRS_SEND_RECV_STATE = STATE_SEND;
			}
			break;
		
		default:
			break;
	}
	
	return result;
}


void GPRSInit(void)
{
	USART3_Init();
	
	GPRSIOInit();
}


INTERRUPT_HANDLER(TIM3_CC_USART3_RX_IRQHandler, 22)
{
  	uint8_t num = 0;
	
	if (USART_GetITStatus(USART3, USART_IT_IDLE) == SET)
    {
		num = num;
		num = USART3->SR;
		num = USART3->DR;
		DMA_Cmd(DMA1_Channel2, DISABLE);
		GPRSRecvSize = GPRS_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel2);
		DMA1_Channel2->CNBTR = GPRS_BUFFER_SIZE;	
		DMA_Cmd(DMA1_Channel2, ENABLE);
		GPRSRecvFlag = 1;
    }
	
	if (USART_GetFlagStatus(USART3, USART_FLAG_OR) == SET)
	{
		num = USART_ReceiveData8(USART3); 		/* 取出来扔掉 */
		USART_ClearFlag(USART3, USART_FLAG_OR); /* 清除OR */
	}
}

#include "config.h"
#include "gprs.h"
#include "net.h"


uint8_t GPRSDataSendFlag = 1;
static uint32_t NetSendTimer;
T_NetTask NetSendData[NET_TASK_SIZE];

void GPRSSendProcess(void)
{
//	uint16_t TempPot;
//	uint32_t TempInt;
	static uint16_t order = 0;
	//static uint16_t SendTimeoutTimer;
	PT_NetTask ptNetCur = g_ptNetTaskHead;
	PT_NetHead ptNetHead = (PT_NetHead)GPRSSendBuffer;
	
	while (ptNetCur)
	{
		if (ptNetCur->State == TASK_READY)
		{
			GPRSSendFlag = 1;
			NetSendTimer = TimeCnt;
			ptNetCur->State = TASK_RUNNING;
			ptNetHead->Preamble = 0x68;
			ptNetHead->TranDirect = 1;
			ptNetHead->ResponseFlag = 0;
			ptNetHead->SlaveAddr = MeterParm.SlaveAddr;
			ptNetHead->PacketID.yearL = SystemTime.yearL;
			ptNetHead->PacketID.month = SystemTime.month;
			ptNetHead->PacketID.day   = SystemTime.day;
			ptNetHead->PacketID.Order = order++;
			switch(ptNetCur->Code)
			{	
				case C3041:
					ptNetHead->FuncCode  = 0x3041;
					ptNetHead->PacketLen = ConvertEndian16(52);
					ptNetHead->DataLen = ConvertEndian16(26);
					GPRSSendBuffer[23] = MeterParm.MeterType;
					*(uint32_t *)(GPRSSendBuffer+24) = ConvertEndian32(MeterParm.TotalGas);
					*(uint32_t *)(GPRSSendBuffer+28) = ConvertEndian32((MeterParm.UsedMoney + 50) / 100);
					*(uint32_t *)(GPRSSendBuffer+32) = ConvertEndian32((MeterParm.RemainMoney + 50) / 100);
					memset(GPRSSendBuffer+36, GetMeterSta(), 1);
					GPRSSendBuffer[37] = MotorSta.SW;
					memset(GPRSSendBuffer+38, GetFaultNumber(), 1);  /* 报警保护次数 */
					memset(GPRSSendBuffer+39, GetBatStatus(), 1);    /* 电池电量及状态 */
					memset(GPRSSendBuffer+40, MeterParm.CurPrice.PriceVer, 1);
					memset(GPRSSendBuffer+41, 0x10, 1);
					memcpy(GPRSSendBuffer+42, (uint8_t *)&SystemTime, 7);
					memset(GPRSSendBuffer+51, 0x16, 1);
					GPRSSendSize = 52;
					break;
					
				case C3062:
					ptNetHead->FuncCode  = 0x3062;
					ptNetHead->PacketLen = ConvertEndian16(44);
					ptNetHead->DataLen = ConvertEndian16(18);
					memcpy(GPRSSendBuffer+23, (uint8_t *)&MeterParm.SlaveAddr, 7);
					memcpy(GPRSSendBuffer+30, (uint8_t *)&SystemTime, 7);
					GPRSSendBuffer[37] = 1;
					GPRSSendBuffer[38] = 0;
					GPRSSendBuffer[39] = (ptNetCur->err >> 4) & 0x0F;
					GPRSSendBuffer[40] = ptNetCur->err & 0xFF;
					memset(GPRSSendBuffer+43, 0x16, 1);
					GPRSSendSize = 44;
					break;
					
				default:
					GPRSSendFlag = 0;
					DelNetTask(ptNetCur);
					break;
			}
			
			if (GPRSFlag == 0)
			{
				GPRSFlag = 1;
			}
			break;
		}
		else if (ptNetCur->State == TASK_RUNNING)  /* task is running */
		{
			if (GPRSOnline == 1 && GPRSSendFlag == 0)
			{
				DelNetTask(ptNetCur);
			}
			break;
		}
		else
		{
			ptNetCur = ptNetCur->ptNext;
		}
	}
	
	if ( GetTimer(NetSendTimer) > ((uint32_t)MeterParm.Wakeup.Duration * 6000) )
	{
		GPRSFlag = 0;
		GPRSSendFlag = 0;
		NetSendTimer = TimeCnt;
		WakeupState.GPRSStatus = TRIG_INIT;
		ptNetCur = g_ptNetTaskHead;
		while (ptNetCur)
		{
			if (ptNetCur->State == TASK_RUNNING)  /* task is running */
			{
				//ptNetCur->State = TASK_READY;
				DelNetTask(ptNetCur);
			}
			else
			{
				ptNetCur = ptNetCur->ptNext;
			}
		}
	}
}

void NetTaskProcess(void)
{
	GPRSSendProcess();
	
	GPRSProcess();
	
	MultiPacketProcess();
}

void NetInit(void)
{
	GPRSInit();
}

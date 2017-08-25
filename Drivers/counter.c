#include "config.h"
#include "counter.h"
#include "beep.h"
#include "key.h"


static uint8_t FlowNum = 0;		/* 过流故障次数 */
static uint8_t CountNum = 0;	/* 采样故障次数 */
static uint8_t MagnetNum = 0;	/* 防磁故障次数 */

static uint8_t FlowFlag = 0;	/* 过流检测标志 */
static uint8_t ExSampCount;     /* 外采样计数 */
static uint32_t InSampTimer;	/* 内采样滤波计时 */
static uint32_t ExSampTimer;	/* 外采样滤波计时 */
static uint32_t MagnetTimer;	/* 防磁滤波计时 */

/* 外采样处理函数 */
static void ExSampProcess(void)
{
	uint32_t UsedGas;
	static uint8_t RemainFlag = 1;
	
	if (CountState.ExSampStatus==TRIG_ACTIVE && GetTimer(ExSampTimer)>=1)
	{
		if (!GPIO_ReadInputDataBit(SAMP_GPIO, EX_SAMP_PIN))
		{
			FlowFlag = 1;
			ExSampCount++;
			MeterParm.TotalGas++;
			UsedGas = MeterParm.TotalGas - MeterParm.CycleBaseGas;
			
			if (UsedGas <= ((uint32_t)MeterParm.CurPrice.GasDivide[0]*100))
			{
				MeterParm.CurUnitPrice = MeterParm.CurPrice.PriceDevide[0];
			}
			else if (UsedGas <= ((uint32_t)MeterParm.CurPrice.GasDivide[1]*100))
			{
				MeterParm.CurUnitPrice = MeterParm.CurPrice.PriceDevide[1];
			}
			else
			{
				MeterParm.CurUnitPrice = MeterParm.CurPrice.PriceDevide[2];
			}
			
			MeterParm.UsedMoney += MeterParm.CurUnitPrice;
			
			if (MeterParm.RemainMoney >= MeterParm.CurUnitPrice)
			{
				MeterParm.RemainMoney -= MeterParm.CurUnitPrice;
				
				if (MeterParm.RemainMoney < MeterParm.MarginWarn && RemainFlag==1)
				{
					AddNetTask(C3062, 10);  /* 余量不足提醒一次 */
					RemainFlag = 0;
				}
				else if (MeterParm.RemainMoney >= MeterParm.MarginWarn && RemainFlag==0)
				{
					RemainFlag = 1;
				}
				
				if ((MeterParm.RemainMoney + MeterParm.OverLimit) < MeterParm.CurUnitPrice)
				{
					CloseMotor(0);  /* 已完全用完，关阀 */
					DBG_PRINTF("No remain money! \n");
				}
			}
			else
			{
				if (MeterParm.RemainMoney == 0)
				{
					MeterParm.OverMoney += MeterParm.CurUnitPrice;
				}
				else
				{
					MeterParm.OverMoney = MeterParm.CurUnitPrice - MeterParm.RemainMoney;
					MeterParm.RemainMoney = 0;
				}
				
				if (MeterParm.OverMoney >= MeterParm.OverLimit)  /* 已完全透支，保存数据且关阀 */
				{
					CloseMotor(0);
					DBG_PRINTF("No overdraft money! \n");
				}
			}
			
			EEPROMWriteBuffer((uint8_t *)&MeterParm, METER_PAM_ADDR, (uint8_t)sizeof(T_MeterParm));
			DBG_PRINTF("The total gas is %ld. \n", MeterParm.TotalGas);
		}
		
		CountState.ExSampStatus = TRIG_SLEEP;
	}
}

/* 内采样处理函数 */
static void InSampProcess(void)
{
	uint32_t ofet;
	static uint8_t InSampCount;  /* 内采样计数 */
	
	if (CountState.InSampStatus==TRIG_ACTIVE && GetTimer(InSampTimer)>=1)
	{
		if (!GPIO_ReadInputDataBit(SAMP_GPIO, IN_SAMP_PIN))
		{
			InSampCount++;
			if (InSampCount > IN_SAMP_SUM)
			{
				if (ExSampCount == 0)
				{
					if (MeterParm.MeterSta.bits.samp == 0)  /* 采样错误 */
					{
						CountNum++;
						if (CountNum == MeterParm.CountMax)
						{
							CloseMotor(1);  		/* 强制关阀 */
							AddNetTask(C3062, 11);	/* 采样故障上报 */
							MeterParm.MeterSta.bits.samp = 1;
							ofet = offset(T_MeterParm, MeterSta);
							EEPROMWriteBuffer((uint8_t *)&MeterParm.MeterSta, METER_PAM_ADDR+ofet, 1);
						}
						else
						{
							CloseMotor(0);			/* 正常关阀 */
							AddNetTask(C3062, 11);	/* 采样故障上报 */
							DBG_PRINTF("In sample error! \n");
						}
					}
				}
				
				InSampCount = 0;
				ExSampCount = 0;
			}
		}
		
		CountState.InSampStatus = TRIG_SLEEP;
	}
}

/* 防磁处理函数 */
static void MagnetProcess(void)
{
	uint32_t ofet;
	
	if (CountState.MagnetStatus==TRIG_ACTIVE && GetTimer(MagnetTimer)>=25)
	{
		if (!GPIO_ReadInputDataBit(SAMP_GPIO, MAGNET_PIN))
		{
			if (MeterParm.MeterSta.bits.magnet == 0)  /* 防磁错误 */
			{
				MagnetNum++;
				if (MagnetNum > MeterParm.MagnetMax)
				{
					MagnetNum = 0;
					CloseMotor(1);  		/* 强制关阀 */
					AddNetTask(C3062, 5);	/* 防磁上报 */
					DBG_PRINTF("Magnet error! \n");
					MeterParm.MeterSta.bits.magnet = 1;
					ofet = offset(T_MeterParm, MeterSta);
					EEPROMWriteBuffer((uint8_t *)&MeterParm.MeterSta, METER_PAM_ADDR+ofet, 1);
				}
				else
				{
					CloseMotor(0);      	/* 正常关阀 */
					AddNetTask(C3062, 5);	/* 防磁上报 */
					DBG_PRINTF("Magnet error! \n");
				}
			}
		}
		
		CountState.MagnetStatus = TRIG_SLEEP;
	}
}

/* 过流处理函数 */
void FlowProcess(void)
{
	uint32_t ofet;
	static uint8_t FlowErrNum = 0;
	static uint16_t FlowTimeCnt = 0;
	
	FlowTimeCnt++;
	if (FlowFlag == 1)
	{
		// DBG_PRINTF("The flow time count is %d. \n", FlowTimeCnt);
		FlowFlag = 0;
		if (36/FlowTimeCnt > MeterParm.FlowLimit)  /* 过流 */
		{
			FlowErrNum++;
			if (FlowErrNum >= 10)
			{
				FlowErrNum = 0;
				FlowNum++;
				if (FlowNum > MeterParm.FlowMax)
				{
					FlowNum = 0;
					CloseMotor(1);  /* 强制关阀 */
					AddNetTask(C3062, 10);
					MeterParm.MeterSta.bits.flow = 1;
					ofet = offset(T_MeterParm, MeterSta);
					EEPROMWriteBuffer((uint8_t *)&MeterParm.MeterSta, METER_PAM_ADDR+ofet, 1);
				}
				else
				{
					CloseMotor(0);  /* 正常关阀 */
					AddNetTask(C3062, 10);
					DBG_PRINTF("Over flow error! \n");
				}
			}
		}
		else
		{
			FlowErrNum = 0;
		}
		
		FlowTimeCnt = 0;
	}
}

/* 获取防磁端口电平状态 */
bool GetMagnetPort(void)
{
	if (!GPIO_ReadInputDataBit(SAMP_GPIO, MAGNET_PIN))
	{
		return FALSE;
	}
	
	return TRUE;
}


void SetFaultNumber(uint8_t data)
{
	MagnetNum = data & 0x03;
	CountNum  = (data>>2) & 0x03;
	FlowNum   = (data>>4) & 0x0F;
}


uint8_t GetFaultNumber(void)
{
	return ((MagnetNum & 0x03) | ((CountNum & 0x03)<<2) | ((FlowNum & 0x0F)<<4));
}

/* 计数单元函数处理 */
void CounterProcess(void)
{
	ExSampProcess();
	
	InSampProcess();
	
	MagnetProcess();
}

/* 外采样初始化 */
static void ExSampInit(void)
{
	GPIO_Init(SAMP_GPIO, EX_SAMP_PIN, GPIO_Mode_In_FL_IT);
	EXTI_ClearITPendingBit(EXTI_IT_Pin2);
	EXTI_SetPinSensitivity(EXTI_Pin_2, EXTI_Trigger_Falling);
	ITC_SetSoftwarePriority(EXTI2_IRQn, ITC_PriorityLevel_3);
}

/* 内采样初始化 */
static void InSampInit(void)
{
	GPIO_Init(SAMP_GPIO, IN_SAMP_PIN, GPIO_Mode_In_FL_IT);
	EXTI_ClearITPendingBit(EXTI_IT_Pin7);
	EXTI_SetPinSensitivity(EXTI_Pin_7, EXTI_Trigger_Falling);
	ITC_SetSoftwarePriority(EXTI7_IRQn, ITC_PriorityLevel_1);
}

/* 防磁初始化 */
static void MagnetInit(void)
{
	GPIO_Init(SAMP_GPIO, MAGNET_PIN, GPIO_Mode_In_FL_IT);
	EXTI_ClearITPendingBit(EXTI_IT_Pin3);
	EXTI_SetPinSensitivity(EXTI_Pin_3, EXTI_Trigger_Falling);
	ITC_SetSoftwarePriority(EXTI3_IRQn, ITC_PriorityLevel_1);
}

/* 计数单元初始化 */
void CounterInit(void)
{
	ExSampInit();

	InSampInit();
	
	MagnetInit();
}

/* 外采样中断 */
INTERRUPT_HANDLER(EXTI2_IRQHandler, 10)
{
	ExSampTimer = TimeCnt;
	EXTI_ClearITPendingBit(EXTI_IT_Pin2);
	WakeupState.CountStatus = TRIG_ACTIVE;
	CountState.ExSampStatus = TRIG_ACTIVE;
}

/* 内采样中断 */
INTERRUPT_HANDLER(EXTI7_IRQHandler, 15)
{
	InSampTimer = TimeCnt;
	EXTI_ClearITPendingBit(EXTI_IT_Pin7);
	WakeupState.CountStatus = TRIG_ACTIVE;
	CountState.InSampStatus = TRIG_ACTIVE;
}

/* 防磁中断 */
INTERRUPT_HANDLER(EXTI3_IRQHandler,11)
{
	MagnetTimer = TimeCnt;
	EXTI_ClearITPendingBit(EXTI_IT_Pin3);
	WakeupState.CountStatus = TRIG_ACTIVE;
	CountState.MagnetStatus = TRIG_ACTIVE;
}

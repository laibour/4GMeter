#ifndef __COMMON_H
#define __COMMON_H


#define	SAVE_DAY			(60)
#define	SAVE_MONTH			(120)
#define	NUM_OF_PACKET		(6)
#define	NET_TASK_SIZE		(100)
#define	METER_PAM_ADDR		(0x00)
#define	METER_DATA_ADDR		sizeof(T_MeterParm)

#define offset(type, member) (uint32_t)(&(((type *)0)->member))

#define ConvertEndian16(X)	((((uint16_t)(X) & 0xff00) >> 8) | (((uint16_t)(X) & 0x00ff) << 8))
#define ConvertEndian32(X)	((((uint32_t)(X) & 0xff000000) >> 24) | \
							(((uint32_t)(X) & 0x00ff0000) >> 8) | \
							(((uint32_t)(X) & 0x0000ff00) << 8) | \
							(((uint32_t)(X) & 0x000000ff) << 24))

/* 阶梯周期 */
typedef enum
{
	CYCLE_YEAR,
	CYCLE_HALF_YEAR,
	CYCLE_QUARTER,
	CYCLE_MONTH,
}tStepCycle;

/* 阶梯价格结构体 */
typedef struct _Price
{
    uint8_t PriceType;   	  // 类别
    uint8_t PriceVer;		  // 版本号
	tStepCycle StepCycle;	  // 阶梯周期, 0:年 1：半年 2：季度 3:月
    uint8_t ActMonth;         // 生效月
	uint8_t ActPeriod;		  // 有效期
    uint16_t GasDivide[2];    // 用量阶梯[方]
    uint16_t PriceDevide[3];  // 价格阶梯[分]
}T_Price, *PT_Price;

/* 主站IP参数 */
typedef struct _Socket
{
	uint8_t IP[15];
	uint8_t Port[6];
} T_Socket, *PT_Socket;

/* 定义从站编号结构体 */
typedef struct SlaveAddr
{
	uint8_t MID[2];
	uint8_t ConAddr[5];
} T_SlaveAddr, *PT_SlaveAddr;

/* 年 月 日 */
typedef struct _Date
{
	uint8_t yearH;
	uint8_t yearL;
	uint8_t month;
	uint8_t day;
} T_Date, *PT_Date;

/* 时 分 秒 */
typedef struct _Time
{
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} T_Time, *PT_Time;

/* 表具自醒参数 */
typedef struct _Wakeup
{
	T_Time   Time;
	uint16_t Duration;
	uint16_t Offset;
} T_Wakeup, *PT_Wakeup;

/* 表具类型 */
typedef enum
{
	PRE_PAY,
	POST_PAY,
}tMeterType;

/* 表具使用参数 */
typedef struct _MeterParm
{
	uint8_t isUse;				/* 开户标志 */
	T_Socket Socket;			/* 主站IP参数 */
	tMeterType MeterType;		/* 表具类型 */
	METER_STA MeterSta;			/* 表具状态 */
	//BYTE_VAL MeterFlag;		/* 表具标志 */
	uint8_t RechargeNum;		/* 充值序号 */
	
	T_SlaveAddr SlaveAddr;		/* 从站地址 */
	uint16_t GPRSHeartTime;		/* 心跳周期 */
	
	uint32_t TotalGas;			/* 累计用气量[0.01方]*/
	uint32_t TotalMoney;		/* 累计充值金额[0.01分] */
	uint16_t UploadCycle;		/* 上报周期 */
	T_Wakeup Wakeup;			/* 表具自醒参数 */
	
	T_Price CurPrice;			/* 当前价格参数 */
	T_Price NewPrice;			/* 预调价格参数 */
	
	uint32_t CornerLimit;		/* 防囤积值 */
	uint8_t FlowLimit;			/* 流量限定值[1立方] */
	uint8_t LittleUseGas;		/* 长时间小流量用气 */
	uint8_t LongtimeProtect;	/* 长时间不用保护值 */
	uint8_t ContinuousUseTime;	/* 连续用气时间 */
	
	uint8_t FlowMax;			/* 过流保护次数上限 */
	uint8_t CountMax;			/* 采样保护次数上限 */
	uint8_t MagnetMax;			/* 防磁保护次数上限 */
	
	uint32_t OverLimit;			/* 透支额度[0.01分] */
	uint32_t MarginWarn;       	/* 余量不足提示[0.01分] */
	uint32_t CycleBaseGas;		/* 当前周期基数用气量[0.01方] */
	uint32_t DayBaseGas;		/* 日用气基数用气量[0.01方] */
	uint32_t MonthBaseGas;		/* 月用气基数用气量[0.01方] */
	uint32_t UsedMoney;			/* 累计使用金额[0.01分] */
	uint32_t RemainMoney;		/* 剩余金额[0.01分] */
	uint32_t OverMoney;			/* 已透支金额[0.01分] */
	uint32_t CurUnitPrice;		/* 当前单价 */
	uint8_t DayPoint;			/* 日用量保存指针 */
	uint8_t MonthPoint;			/* 月用量保存指令 */
	T_Date MeterOutDate;		/* 表具到期日期 */
}T_MeterParm, *PT_MeterParm;

/* 按键显示状态 */
typedef enum
{
	KEY_DISP_INIT,
	KEY_DISP_RUN,
	KEY_DISP_WAIT,
}tKeyDispStates;

/* 休眠过程状态 */
typedef enum
{
	TRIG_SLEEP,
	TRIG_IDLE,
	TRIG_INIT,
	TRIG_ACTIVE,
	TRIG_WAIT,
}tTrigStatus;

/* 休眠结构体 */
typedef struct _WakeupState
{
	bool LowPowerStatus;
	tTrigStatus KeyStatus;
	tTrigStatus RTCStatus;
	tTrigStatus GPRSStatus;
	tTrigStatus CountStatus;
	tTrigStatus BatStatus;
	tTrigStatus MotorStatus;
} T_WakeupState, *PT_WakeupState;

/* 采样休眠结构体 */
typedef struct _CountState
{
	tTrigStatus InSampStatus;
	tTrigStatus ExSampStatus;
	tTrigStatus MagnetStatus;
} T_CountState, *PT_CountState;

/* 系统时钟 */
typedef struct _SystemTime
{
	uint8_t yearH;
	uint8_t yearL;
	uint8_t month;
	uint8_t day;
	uint8_t hour;
	uint8_t minute;
	uint8_t second;
} T_SystemTime, *PT_SystemTime;

/* 表具指数结构体 */
typedef struct _MeterData
{
	T_SystemTime Time;
	uint16_t GasPot;  	/* 表具指数小数 */
	uint32_t GasInt;  	/* 表具指数整数 */
} T_MeterData, *PT_MeterData;

/* 显示结构体 */
typedef struct _DispData
{
	uint8_t month;		  // 月
	uint8_t total;		  // 总
	uint8_t consumption;  // 用量
	uint8_t remain;		  // 余量
	uint8_t lack;		  // 不足
	uint8_t changebat;	  // 请换电池
	uint8_t price;		  // 单价
	uint8_t readcard;	  // 读卡
	uint8_t success;	  // 成功
	uint8_t error;		  // 错误
	uint8_t PreColon;	  // 前分号
	uint8_t PosColon;	  // 后分号
	uint8_t PrePoint;	  // 前点
	uint8_t PosPoint;	  // 后点
	uint8_t valve;		  // 阀门
	uint8_t warning;	  // 警告
	uint8_t voltage;	  // 电压
	uint8_t Yuan;		  // 元
	uint8_t divide;		  // '/'号
	uint8_t cube;		  // 立方米
	uint8_t digit[9];	  // 9个数字
} T_DispData, *PT_DispData;

/* 阀门状态 */
typedef enum
{
	MOTOR_CLOSE,
	MOTOR_OPEN,
}tMoterStatus;

/* 阀门操作 */
typedef struct _MotorSta
{
	tMoterStatus SW;	/* 当前阀门状态: 0 - 关，1 - 开 */
	uint8_t Open;		/* 执行开阀操作: 0 - 无效，1 - 开阀 */
	uint8_t OpenEn;
	uint8_t Close;		/* 执行关阀操作: 0 - 无效，1 - 关阀 */
	uint8_t CloseEn;
} T_MotorSta, *PT_MotorSta;

/* 定义报文ID结构体 */
typedef struct _PacketID
{
	uint8_t yearL;
	uint8_t month;
	uint8_t day;
	uint32_t Order;
} T_PacketID, *PT_PacketID;

/* 网络报文头数据结构体 */
typedef struct NetHead
{
	uint8_t Preamble;        	/* 起始符0x68 */
	uint16_t PacketLen;			/* the whole packet length */
	uint16_t FuncCode;     		/* function code */
	uint8_t TranDirect;      	/* transmission direction, 0: master send out; 1: slave send out */ 
	uint8_t ResponseFlag;      	/* response flag, 0: request; 1: answer */
	T_SlaveAddr SlaveAddr;		/* 从站编号 */
	T_PacketID PacketID;		/* 报文ID */
	uint16_t DataLen;			/* 数据域长度 */
}T_NetHead, *PT_NetHead;

/* 定义心跳包结构体 */
typedef struct _HeartBeat
{
	T_NetHead Head;
	uint8_t MeterAddr[7];
	uint16_t Crc;
	uint8_t End;
}T_HeartBeat, *PT_HeartBeat;

/* 多包传输类型 */
typedef enum
{
	PACKET_IDLE,
	PACKET_DAY,
	PACKET_MONTH,
}tPacketType;

/* 多包传输状态 */
typedef enum
{
	PACKET_STATE_IDLE,
	PACKET_STATE_INIT,
	PACKET_STATE_RUN,
}tPacketState;

/* 多包传输结构体 */
typedef struct _MultiPacket
{
	T_NetHead NetHead;
	uint8_t End;
	uint8_t Node;
	uint8_t TotalNode;
	uint16_t FrameSeq;
	tPacketType PacketType;
	tPacketState PacketState;
} T_MultiPacket, *PT_MultiPacket;


extern float Vapp;
extern uint8_t LongPress;
extern uint8_t GPRSFlag;
extern bool LowPowerStatus;
extern T_MotorSta MotorSta;
extern T_DispData DispData;
extern T_MeterParm MeterParm;
extern T_MeterData MeterData;
extern T_SystemTime SystemTime;
extern T_WakeupState WakeupState;
extern T_CountState CountState;
extern T_HeartBeat HeartBeat;
extern T_MultiPacket MultiPacket;
extern uint8_t GPRSOpenData[];


bool IsBigEndian(void);
void SetMeterParm(void);
uint8_t GetMeterSta(void);
void Soft_delay_ms(uint16_t time);
void Soft_delay_us(uint16_t time);
void GetDecTime(PT_Time ptReadTime);
void SetBCDTime(PT_Time ptReadTime);
uint8_t LookForStr(uint8_t *s, uint8_t *t);
void ConvertEndian(uint8_t *buff, uint8_t num);
bool CompareData(uint8_t *a, uint8_t *b, uint16_t n);
uint16_t BCDToDec(uint8_t *bcd, uint8_t len);
void DecToBCD(uint16_t Dec, uint8_t *BCD, uint8_t len);
uint32_t NetComputeCRC(uint8_t *bufData, uint16_t buflen, uint8_t *pcrc);

#endif

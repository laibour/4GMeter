#include "config.h"
#include "gprs.h"
#include "gprs-Hal.h"
#include "gprs-Misc.h"


static uint8_t OK[] = "OK";
static uint8_t AT[] = "AT\r";
static uint8_t Ready[] = "PB DONE";
uint8_t AT_CCLK[] = "AT+CCLK?\r";

uint8_t GPRSOpen(void)
{
	uint8_t result = GPRS_BUSY;
	static uint32_t GPRSOpenTimer;
	uint8_t FAIL[] = "FAIL";
	uint8_t ERROR[] = "ERROR";
	uint8_t CONNECTOK[] = "CONNECT";
	//uint8_t GPRSOpenData1[] = "AT+CIPOPEN=0,\"UDP\",\"101.204.248.138\",08267,6800\r";
	//uint8_t GPRSOpenData1[] = "AT+CIPOPEN=0,\"TCP\",\"101.204.248.138\",08170\r";
	uint8_t GPRSOpenData1[] = "AT+CIPOPEN=0,\"TCP\",\"120.26.240.190\",08172\r";
	
    static enum
    {
		GPRS_OPEN_INIT,
		GPRS_OPEN_WAIT,
        GPRS_OPEN_SEND,
        GPRS_OPEN_RUNNING,
		GPRS_OPEN_SHUT,
    }GPRS_OPEN_STATE = GPRS_OPEN_INIT;
	
    switch(GPRS_OPEN_STATE)
	{
		case GPRS_OPEN_INIT:
			GPRSOpenTimer   = TimeCnt;
			GPRS_OPEN_STATE = GPRS_OPEN_WAIT;
			break;
		
		case GPRS_OPEN_WAIT:
			if (GetTimer(GPRSOpenTimer) > 150)  // 1.5s
			{
				GPRS_OPEN_STATE = GPRS_OPEN_SEND;
			}
			break;
			
		case GPRS_OPEN_SEND:
			memset(GPRSRecvBuffer, 0, GPRS_BUFFER_SIZE);
			USART3_SendString(GPRSOpenData1);
			GPRSOpenTimer   = TimeCnt;
			GPRS_OPEN_STATE = GPRS_OPEN_RUNNING;
			break;
		
		case GPRS_OPEN_RUNNING:
			if (GetTimer(GPRSOpenTimer) < 2000)  // 20s
			{
				if (GPRSRecvFlag == 1)
				{
					GPRSRecvFlag = 0;
					DBG_PRINTF("%s \n", GPRSRecvBuffer);  // GPRS recv data
					if ( LookForStr(GPRSRecvBuffer, FAIL)!=FULL || LookForStr(GPRSRecvBuffer, ERROR)!=FULL )	// LookForStr(GPRSRecvBuffer, CLOSE)!=FULL )
					{
						memset(GPRSRecvBuffer, 0, GPRS_BUFFER_SIZE);
						GPRS_OPEN_STATE = GPRS_OPEN_INIT;
						result = GPRS_ERROR;
					}
					else if (LookForStr(GPRSRecvBuffer, CONNECTOK) != FULL)
					{
						memset(GPRSRecvBuffer, 0, GPRS_BUFFER_SIZE);
						GPRS_OPEN_STATE = GPRS_OPEN_INIT;
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
				GPRSRecvFlag = 0;
				GPRS_OPEN_STATE = GPRS_OPEN_INIT;
				result = GPRS_ERROR;
			}
			break;
		
		default:
			DBG_PRINTF("GPRSSendRec error! \n");
			break;
	}
	
	return result;
}


uint8_t GPRSCfg(void)
{
	uint8_t ret;
	uint8_t result = GPRS_BUSY;
//	static uint8_t GPRSCfgNum = 0;
    
    uint8_t AT_CGATT[]      = "AT+CGATT?\r";
	uint8_t AT_CGSOCKCONT[] = "AT+CGSOCKCONT=1,\"IP\",\"3GNET\"\r";
    //uint8_t AT_CGSOCKCONT[] = "AT+CGSOCKCONT=1,\"IP\",\"CMNET\"\r";
	uint8_t AT_TIMESET[]    = "AT+CIPTIMEOUT=30000,20000,40000\r";
    uint8_t AT_CIPMODE[]    = "AT+CIPMODE=1\r";
    uint8_t AT_ATD[]        = "AT&D0\r";
	//uint8_t AT_ATW[]        = "AT&W0\r";
    uint8_t AT_NETOPEN[]    = "AT+NETOPEN\r";
    uint8_t AT_IPADDR[]     = "AT+IPADDR\r";
    uint8_t AT_NETCLOSE[]   = "AT+NETCLOSE\r";
	uint8_t AT_CTZU[]       = "AT+CTZU=1\r";
	
	uint8_t CGATT_OK[] = ": 1";
	uint8_t IPADDR_OK[] = ".";
	uint8_t NETOPEN_OK[] = "NETOPEN: 0";
	uint8_t NETCLOSE_OK[] = "NETCLOSE: 0";
	
	static enum
    {
        GPRS_CFG_CGATT,
		GPRS_CFG_CLTS,
        GPRS_CFG_CONT,
		GPRS_CFG_TIMESET,
		GPRS_CFG_CTZU,
		GPRS_CFG_CCLK,
        GPRS_CFG_MODE,
		GPRS_CFG_ATD,
		GPRS_CFG_NETOPEN,
		GPRS_CFG_IPADDR,
		GPRS_CFG_ERROR,
		GPRS_CFG_WAIT,
		GPRS_CFG_CLOSE
    }GPRS_CFG_STATE = GPRS_CFG_CGATT;
	
	switch(GPRS_CFG_STATE)
	{
		case GPRS_CFG_CGATT:
			ret = GPRSSendRec(AT_CGATT, CGATT_OK, 200, 30);
			if (ret == GPRS_DONE)
			{
				GPRS_CFG_STATE = GPRS_CFG_CONT;
			}
			else if (ret == GPRS_ERROR)
			{
				GPRS_CFG_STATE = GPRS_CFG_ERROR;
			}
			break;
            
        case GPRS_CFG_CONT:
			ret = GPRSSendRec(AT_CGSOCKCONT, OK, 100, 3);
			if (ret == GPRS_DONE)
			{
				GPRS_CFG_STATE = GPRS_CFG_TIMESET;
			}
			else if (ret == GPRS_ERROR)
			{
				GPRS_CFG_STATE = GPRS_CFG_ERROR;
			}
			break;
			
		case GPRS_CFG_TIMESET:
			ret = GPRSSendRec(AT_TIMESET, OK, 100, 3);
			if (ret == GPRS_DONE)
			{
				GPRS_CFG_STATE = GPRS_CFG_CTZU;
			}
			else if (ret == GPRS_ERROR)
			{
				GPRS_CFG_STATE = GPRS_CFG_ERROR;
			}
			break;
			
		case GPRS_CFG_CTZU:
			ret = GPRSSendRec(AT_CTZU, OK, 100, 3);
			if (ret == GPRS_DONE)
			{
				GPRS_CFG_STATE = GPRS_CFG_CCLK;
			}
			else if (ret == GPRS_ERROR)
			{
				GPRS_CFG_STATE = GPRS_CFG_ERROR;
			}
			break;
			
		case GPRS_CFG_CCLK:
			ret = GPRSSendRec(AT_CCLK, OK, 500, 3);
			if (ret == GPRS_DONE)
			{
				GPRS_CFG_STATE = GPRS_CFG_MODE;
			}
			else if (ret == GPRS_ERROR)
			{
				GPRS_CFG_STATE = GPRS_CFG_ERROR;
			}
			break;
			
		case GPRS_CFG_MODE:
			ret = GPRSSendRec(AT_CIPMODE, OK, 100, 3);
			if (ret == GPRS_DONE)
			{
				GPRS_CFG_STATE = GPRS_CFG_ATD;
			}
			else if (ret == GPRS_ERROR)
			{
				GPRS_CFG_STATE = GPRS_CFG_ERROR;
			}
			break;
			
		case GPRS_CFG_ATD:
			ret = GPRSSendRec(AT_ATD, OK, 100, 3);
			if (ret == GPRS_DONE)
			{
				GPRS_CFG_STATE = GPRS_CFG_NETOPEN;
			}
			else if (ret == GPRS_ERROR)
			{
				GPRS_CFG_STATE = GPRS_CFG_ERROR;
			}
			break;
			
		case GPRS_CFG_NETOPEN:
			ret = GPRSSendRec(AT_NETOPEN, NETOPEN_OK, 3000, 1);
			if (ret == GPRS_DONE)
			{
				GPRS_CFG_STATE = GPRS_CFG_IPADDR;
			}
			else if (ret == GPRS_ERROR)
			{
				GPRS_CFG_STATE = GPRS_CFG_ERROR;
			}
			break;
			
		case GPRS_CFG_IPADDR:
			ret = GPRSSendRec(AT_IPADDR, IPADDR_OK, 500, 3);
			if (ret == GPRS_DONE)
			{
				result = GPRS_DONE;
				GPRS_CFG_STATE = GPRS_CFG_CGATT;
			}
			else if (ret == GPRS_ERROR)
			{
				GPRS_CFG_STATE = GPRS_CFG_ERROR;
			}
			break;
			
		case GPRS_CFG_ERROR:
			ret = GPRSSendRec(AT_NETCLOSE, NETCLOSE_OK, 500, 1);
			if (ret == GPRS_DONE)
			{
				GPRS_CFG_STATE = GPRS_CFG_CGATT;
			}
			else if (ret == GPRS_ERROR)
			{
				result = GPRS_ERROR;
				GPRS_CFG_STATE = GPRS_CFG_CGATT;
			}
			break;
		
		default:
			break;
	}
	
	return result;
}


bool GPRSPowerOn(void)
{
	uint8_t ret;
	bool result = FALSE;
	static uint8_t ErrorNum = 0;
	static uint32_t GPRSPowerTimer;
	static tGPRSPowerStates GPRSPowerState = GPRS_POWER_INIT;
	
	switch(GPRSPowerState)
	{
		case GPRS_POWER_IDLE:
			break;
		
		case GPRS_POWER_INIT:
			USART3_Init();
			GPRS_POWER_OFF();
			GPRS_PWRKEY_OFF();
			GPRSPowerTimer = TimeCnt;
			GPRSPowerState = GPRS_POWER_ON;
			break;
			
		case GPRS_POWER_ON:
		  	if (GetTimer(GPRSPowerTimer) > 300)
			{
				GPRS_POWER_ON();
				Soft_delay_ms(5);
				GPRS_PWRKEY_ON();
				GPRSPowerTimer = TimeCnt;
				GPRSPowerState = GPRS_POWER_KEY;
			}
			break;
			
		case GPRS_POWER_KEY:
		  	if (GetTimer(GPRSPowerTimer) > 800)
			{
				GPRS_PWRKEY_OFF();
				GPRSPowerState = GPRS_POWER_AT;
			}
			break;
			
		case GPRS_POWER_AT:
			ret = GPRSSendRec(AT, OK, 100, 8);
			if (ret == GPRS_DONE)
			{
				ErrorNum = 0;
				GPRSPowerTimer = TimeCnt;
				GPRSPowerState = GPRS_POWER_WAIT;
			}
			else if (ret == GPRS_ERROR)
			{
				ErrorNum++;
				if (ErrorNum >= 2)
				{
					ErrorNum = 0;
					WWDG_SWReset();
				}
				GPRSPowerState = GPRS_POWER_INIT;
			}
			break;
		
		case GPRS_POWER_WAIT:
			if (GetTimer(GPRSPowerTimer) < 6000)	// wait 40s
			{
				if (GPRSRecvFlag == 1)
				{
					GPRSRecvFlag = 0;
					DBG_PRINTF("%s \n", GPRSRecvBuffer);
					if (LookForStr(GPRSRecvBuffer, Ready) != FULL)
					{
						GPRSPowerState = GPRS_POWER_DONE;
					}
				}
			}
			else
			{
				GPRSPowerState = GPRS_POWER_INIT;
			}
			break;
		
		case GPRS_POWER_DONE:
			result = TRUE;
			memset(GPRSRecvBuffer, 0, GPRS_BUFFER_SIZE);
			GPRSPowerState = GPRS_POWER_INIT;
			break;
			
		default:
			break;
	}
	
	return result;
}

/* 退出透传模式 */
bool GPRSExit(void)
{
	bool result = FALSE;
	static uint8_t ExitNum = 0;
	static uint32_t ExitTimeoutTimer;
	static enum
    {
		GPRS_EXIT_INIT,
		GPRS_EXIT_WAIT,
        GPRS_EXIT_RUNNING,
		GPRS_EXIT_END,
    }GPRS_EXIT_STATE = GPRS_EXIT_INIT;
	
	switch(GPRS_EXIT_STATE)
	{
		case GPRS_EXIT_INIT:
			ExitTimeoutTimer = TimeCnt;
			GPRS_EXIT_STATE  = GPRS_EXIT_WAIT;
			break;
		
		case GPRS_EXIT_WAIT:
			if (GetTimer(ExitTimeoutTimer) > 100)
			{
				ExitNum = 0;
				ExitTimeoutTimer = TimeCnt;
				GPRS_EXIT_STATE  = GPRS_EXIT_RUNNING;
			}
			break;
			
		case GPRS_EXIT_RUNNING:
			if (GetTimer(ExitTimeoutTimer) > 50)
			{
				USART3_SendString((uint8_t *)"+");
				DBG_PRINTF("+");
				ExitTimeoutTimer = TimeCnt;
				ExitNum++;
				if (ExitNum == 3)
				{
					ExitNum = 0;
					GPRS_EXIT_STATE  = GPRS_EXIT_END;
				}
			}
			break;
		
		case GPRS_EXIT_END:
			if (GetTimer(ExitTimeoutTimer) > 100)
			{
				ExitTimeoutTimer = TimeCnt;
				GPRS_EXIT_STATE  = GPRS_EXIT_INIT;
				result = TRUE;
			}
			break;
		
		default:
			break;
	}
	
	return result;
}

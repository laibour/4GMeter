#ifndef __GPRS_H
#define __GPRS_H


#define	GPRS_BUFFER_SIZE		(255)		/* GPRS buffer size */

// GPRS POWER PORT
#define	GPRS_POWER_PIN			GPIO_Pin_7
#define	GPRS_POWER_GPIO			GPIOE
#define GPRS_POWER_ON()			GPIO_SetBits(GPRS_POWER_GPIO, GPRS_POWER_PIN)
#define GPRS_POWER_OFF()		GPIO_ResetBits(GPRS_POWER_GPIO, GPRS_POWER_PIN)

// GPRS PWRKEY PORT
#define	GPRS_PWRKEY_PIN			GPIO_Pin_7
#define	GPRS_PWRKEY_GPIO		GPIOF
#define GPRS_PWRKEY_ON()		GPIO_SetBits(GPRS_PWRKEY_GPIO, GPRS_PWRKEY_PIN)
#define GPRS_PWRKEY_OFF()		GPIO_ResetBits(GPRS_PWRKEY_GPIO, GPRS_PWRKEY_PIN)

typedef enum
{
	GPRS_STATE_IDLE,
	GPRS_STATE_POWERON,
	GPRS_STATE_CFG,
	GRPS_STATE_OPEN,
	GPRS_STATE_RUNNING,
	GPRS_STATE_REGISTER,
	GPRS_STATE_EXIT,
}tGPRSStates;

typedef enum
{
	GPRS_POWER_IDLE,
	GPRS_POWER_INIT,
	GPRS_POWER_ON,
	GPRS_POWER_KEY,
	GPRS_POWER_AT,
	GPRS_POWER_WAIT,
	GPRS_POWER_DONE,
}tGPRSPowerStates;

typedef enum
{
	GPRS_IDLE,
    GPRS_BUSY,
    GPRS_DONE,
	GPRS_ERROR,
}tGPRSReturnCodes;

extern uint8_t GPRSOnline;
extern uint8_t GPRSSendFlag;
extern uint8_t GPRSSendSize;
extern uint8_t GPRSSendBuffer[GPRS_BUFFER_SIZE];
extern uint8_t GPRSRecvFlag;
extern uint8_t GPRSRecvSize;
extern uint8_t GPRSRecvBuffer[GPRS_BUFFER_SIZE];

void GPRSInit(void);
void GPRSProcess(void);
void MultiPacketProcess(void);
uint8_t GPRSSendRec(uint8_t *str, uint8_t *find, uint16_t time, uint8_t num);

#endif
#ifndef __MOTOR_H
#define	__MOTOR_H


// motor check port
#define	MOTOR_CHECK_PIN			GPIO_Pin_1
#define	MOTOR_CHECK_GPIO		GPIOG

// motor open port
#define	MOTOR_OPEN_PIN			GPIO_Pin_0
#define	MOTOR_OPEN_GPIO			GPIOG
#define MOTOR_OPEN_ON()			GPIO_SetBits(MOTOR_OPEN_GPIO, MOTOR_OPEN_PIN)
#define MOTOR_OPEN_OFF()		GPIO_ResetBits(MOTOR_OPEN_GPIO, MOTOR_OPEN_PIN)

// motor close port
#define	MOTOR_CLOSE_PIN			GPIO_Pin_2
#define	MOTOR_CLOSE_GPIO		GPIOG
#define MOTOR_CLOSE_ON()		GPIO_SetBits(MOTOR_CLOSE_GPIO, MOTOR_CLOSE_PIN)
#define MOTOR_CLOSE_OFF()		GPIO_ResetBits(MOTOR_CLOSE_GPIO, MOTOR_CLOSE_PIN)


void MotorInit(void);
void OpenMotor(void);
void MotorProcess(void);
bool MotorOpenOption(void);
void CloseMotor(uint8_t Flag);

#endif

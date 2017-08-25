#ifndef __TASK_H
#define	__TASK_H

typedef enum
{
	CNULL,
	C1001,
	C1002,
	C1003,
	C1004,
	C1005,
	C1006,
	C1007,
	C1008,
	C1009,
	C2405,
	C3041,
	C3043,
	C3062,
}tNetCode;

typedef enum
{
	TASK_IDLE,
	TASK_WAIT,
	TASK_READY,
    TASK_RUNNING,
	TASK_SUSPEND,
	TASK_OVERTIME,
	TASK_DONE,
}tNetState;

typedef struct _NetTask
{
	uint8_t err;
	tNetCode Code;
	tNetState State;
	struct _NetTask *ptPre;
	struct _NetTask *ptNext;
}T_NetTask, *PT_NetTask;

extern PT_NetTask g_ptNetTaskHead;
void DelNetTask(PT_NetTask ptDel);
void AddNetTask(tNetCode Code, uint8_t err);

#endif
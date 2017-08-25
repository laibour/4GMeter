#include "config.h"
#include "task.h"

PT_NetTask g_ptNetTaskHead;

// Add Task
void AddTask(PT_NetTask ptNew)
{
	PT_NetTask ptCur;
	
	if (g_ptNetTaskHead == NULL)
	{
		g_ptNetTaskHead = ptNew;
	}
	else
	{
		ptCur = g_ptNetTaskHead;
		while (ptCur->ptNext)
		{
			ptCur = ptCur->ptNext;
		}
		ptCur->ptNext = ptNew;
		ptNew->ptPre  = ptCur;
	}
}

// Delelte Task
void DelTask(PT_NetTask ptDel)
{
	PT_NetTask ptCur;
	PT_NetTask ptPre;
	PT_NetTask ptNext;
	
	if (g_ptNetTaskHead == ptDel)
	{
		ptCur = ptDel;
		g_ptNetTaskHead = ptDel->ptNext;
	}
	else
	{
		ptCur = g_ptNetTaskHead->ptNext;
		while (ptCur)
		{
			if (ptCur == ptDel)
			{
				ptPre  = ptCur->ptPre;
				ptNext = ptCur->ptNext;
				ptPre->ptNext = ptNext;
				if (ptNext)
				{
					ptNext->ptPre = ptPre;
				}
				break;
			}
			else
			{
				ptCur = ptCur->ptNext;
			}
		}
	}
	ptCur->ptPre  = NULL;
	ptCur->ptNext = NULL;
}


void AddNetTask(tNetCode Code, uint8_t err)
{
	uint8_t i;
	
	for (i = 0; i < NET_TASK_SIZE; i++)
	{
		if (NetSendData[i].Code == CNULL)
		{
			NetSendData[i].err   = err;
			NetSendData[i].Code  = Code;
			NetSendData[i].State = TASK_READY;
			AddTask(&NetSendData[i]);
			break;
		}
	}
}

void DelNetTask(PT_NetTask ptDel)
{
	ptDel->err   = 0;
	ptDel->Code  = CNULL;
	ptDel->State = TASK_IDLE;
	DelTask(ptDel);
}

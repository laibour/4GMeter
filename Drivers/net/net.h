#ifndef __NET_H
#define __NET_H


extern uint8_t GPRSSendEnable;
extern uint8_t GPRSDataSendFlag;
extern T_NetTask NetSendData[NET_TASK_SIZE];

void NetInit(void);
void NetTaskProcess(void);

#endif

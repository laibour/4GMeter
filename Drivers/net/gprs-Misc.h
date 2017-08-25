#ifndef __GPRS_MISC_H
#define __GPRS_MISC_H


extern uint8_t AT_CCLK[];

bool GPRSExit(void);
uint8_t GPRSCfg(void);
uint8_t GPRSOpen(void);
bool GPRSPowerOn(void);

#endif

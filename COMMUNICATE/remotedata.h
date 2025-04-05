#ifndef __REMOTEDATA_H
#define __REMOTEDATA_H

#include "stm32f4xx.h" // Device header

void Remote_Data_ReceiveAnalysis(void);
void WIFI_Data_ReceiveAnalysis(uint8_t *buff, uint8_t cnt);
void NRF_SingalCheck(void);

void SendToRemote(void);

#endif

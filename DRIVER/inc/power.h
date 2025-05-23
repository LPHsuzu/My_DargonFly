#ifndef __POWER_H
#define __POWER_H
#include "stm32f4xx.h"

void BATT_Init(void);
void LowVoltage_Alarm(void);
void WiFi_Switch(uint8_t flag);
void OpenMV_Switch(uint8_t flag);
void NRF24L01_Switch(uint8_t flag);

#endif

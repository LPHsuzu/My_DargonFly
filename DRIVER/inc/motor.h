#ifndef __MOTOR_H
#define __MOTOR_H
#include "stm32f4xx.h"

void MOTOR_Init(void);
void MOTOR_PWM(int16_t MOTO1_PWM, int16_t MOTO2_PWM, int16_t MOTO3_PWM, int16_t MOTO4_PWM);

#endif

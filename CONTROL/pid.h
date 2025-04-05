#ifndef __PID_H
#define __PID_H
#include "stm32f4xx.h"
#include "imu.h"

void PID_Postion_Cal(PID_TYPE *PID, float target, float measure);
void PidParameter_init(void);

#endif

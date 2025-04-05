#ifndef __CONTROL_H
#define __CONTROL_H
#include "stm32f4xx.h"
#include "imu.h"

void Control(FLOAT_ANGLE *att_in, FLOAT_XYZ *gyr_in, RC_TYPE *rc_in, uint8_t armed);
int16_t Yaw_Control(float TARGET_YAW);
void Yaw_Carefree(FLOAT_ANGLE *Target_Angle, const FLOAT_ANGLE *Measure_Angle);
void Safety_Check(void);

#endif

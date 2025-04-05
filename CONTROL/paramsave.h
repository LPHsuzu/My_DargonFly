#ifndef __PARAMSAVE_H
#define __PARAMSAVE_H
#include "stm32f4xx.h"

void ParamsToTable(void);
void TableToParams(void);
void ParamsClearAll(void);
void PID_ClearFlash(void);
void PID_WriteFlash(void);
void PID_ReadFlash(void);
void DefaultParams(void);
void DefaultParams_WriteFlash(void);

#endif

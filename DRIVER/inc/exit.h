#ifndef EXIT_H
#define EXIT_H
#include "../CMSIS/stm32f4xx.h"

void EXTI_GPIOConfig(void);
void exti_init(void);
void NVIC_Config(void);

#endif

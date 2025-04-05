#include "stm32f4xx.h" // Device header
#include "timer.h"

/**
 * @brief TIM4初始化为1ms计数一次
 * @note 更新中断时间 Tout = (ARR-1)*(PSC-1)/CK_INT
 */
void TIM4_Init(void)
{
    TIM_TimeBaseInitTypeDef tim_timebaseinit_structure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    // TIM2~TIM5时钟来源于APB1*2
    tim_timebaseinit_structure.TIM_Period = 1000 - 1;   // 自动重装载值
    tim_timebaseinit_structure.TIM_Prescaler = 100 - 1; // 预分频系数
    tim_timebaseinit_structure.TIM_CounterMode = TIM_CounterMode_Up;
    tim_timebaseinit_structure.TIM_ClockDivision = 0;
    TIM_TimeBaseInit(TIM4, &tim_timebaseinit_structure);

    TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

    TIM_Cmd(TIM4, ENABLE);
}

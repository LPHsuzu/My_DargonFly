#include "nvicconfig.h"

/**
 * @brief 配置工程中所有中断的中断优先级
 *
 */
void NVIC_Config(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断分组2

    /*程序时基TIM4定时器中断*/
    NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        // 响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    /*USART2中断*/
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;        // 响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    /*USART1中断*/
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;        // 响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

    /*NRF24L01的IRQ中断*/
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // 响应优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

#include "exit.h"

/**
 * @brief 配置与NRF的IRQ相连的IO
 *
 */
void EXTI_GPIOConfig(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * @brief 外部中断初始化
 * @note  配置NRF的IRQ引脚触发的外部中断
 */
void exti_init(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_EXTIT, ENABLE);  // 使能外部中断时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE); // 使能系统配置时钟

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource2); // 将中断线EXTI Line2与PB2连接

    EXTI_GPIOConfig();
    EXTI_DeInit(); // 复位外部中断

    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
}

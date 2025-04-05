#include "motor.h"

#define Moto_PwmMax 1000
int16_t MOTO1_PWM = 0;
int16_t MOTO2_PWM = 0;
int16_t MOTO3_PWM = 0;
int16_t MOTO4_PWM = 0;

/**
 * @brief TIM3 PWM输出通道GPIO设置
 * @note TIM3 CH1(PWM1) -> PA6
 *       TIM3 CH2(PWM2) -> PA7
 *       TIM3 CH3(PWM3) -> PB0
 *       TIM3 CH4(PWM4) -> PB1
 */
void TIM3_GPIOConfig(void)
{
    GPIO_InitTypeDef gpio_init_structure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);                         // TIM3 时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE); // GPIOA GPIOB 时钟使能
    /*GPIOB 配置: TIM3 CH1 (PA6), TIM3 CH2 (PA7), TIM3 CH3 (PB0) and TIM3 CH4 (PB2)*/
    gpio_init_structure.GPIO_Mode = GPIO_Mode_AF;       // 复用模式
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;     // 推挽输出
    gpio_init_structure.GPIO_PuPd = GPIO_PuPd_UP;       // 上拉输入
    gpio_init_structure.GPIO_Speed = GPIO_Speed_100MHz; // 速度100M
    /*PA6 PA7 初始化*/
    gpio_init_structure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_Init(GPIOA, &gpio_init_structure);
    /*PB0 PB1 初始化*/
    gpio_init_structure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_Init(GPIOB, &gpio_init_structure);
    /*连接 TIM3 的通道到 AF2*/
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM3);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_TIM3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_TIM3);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_TIM3);
}

/**
 * @brief 输出PWM初始化
 * @note TIMx_ARR  决定方波的周期
 *       TIMx_CCRx 决定方波的占空比
 */
void MOTOR_Init(void)
{
    TIM_TimeBaseInitTypeDef tim_timebase_init_structure;
    TIM_OCInitTypeDef tim_oc_init_structure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM3_GPIOConfig();
    /*更新中断时间 Tout = (ARR-1)*(PSC-1)/CK_INT*/
    /*TIM2~TIM5时钟来源于APB1*2*/
    tim_timebase_init_structure.TIM_Period = 1000 - 1;   // 自动重装载值
    tim_timebase_init_structure.TIM_Prescaler = 100 - 1; // 预分频值
    tim_timebase_init_structure.TIM_CounterMode = TIM_CounterMode_Up;
    tim_timebase_init_structure.TIM_ClockDivision = 0;
    TIM_TimeBaseInit(TIM3, &tim_timebase_init_structure);
    /*PWM1 模式配置 通道1*/
    tim_oc_init_structure.TIM_OCMode = TIM_OCMode_PWM1;
    tim_oc_init_structure.TIM_OutputState = TIM_OutputState_Enable;
    tim_oc_init_structure.TIM_Pulse = 0;
    tim_oc_init_structure.TIM_OCPolarity = TIM_OCPolarity_High;

    TIM_OC1Init(TIM3, &tim_oc_init_structure);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    /*PWM1 模式配置 通道2*/
    tim_oc_init_structure.TIM_OutputState = TIM_OutputState_Enable;
    tim_oc_init_structure.TIM_Pulse = 0;
    TIM_OC2Init(TIM3, &tim_oc_init_structure);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    /*PWM1 模式配置 通道3*/
    tim_oc_init_structure.TIM_OutputState = TIM_OutputState_Enable;
    tim_oc_init_structure.TIM_Pulse = 0;
    TIM_OC3Init(TIM3, &tim_oc_init_structure);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
    /*PWM1 模式配置 通道4*/
    tim_oc_init_structure.TIM_OutputState = TIM_OutputState_Enable;
    tim_oc_init_structure.TIM_Pulse = 0;
    TIM_OC4Init(TIM3, &tim_oc_init_structure);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
    /*ARR 寄存器预装载*/
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    /*使能TIM3*/
    TIM_Cmd(TIM3, ENABLE);
}

/**
 * @brief 电机要输出数值转换成PWM波形输出
 *
 * @param MOTO1_PWM 电机1
 * @param MOTO2_PWM 电机2
 * @param MOTO3_PWM 电机3
 * @param MOTO4_PWM 电机4
 */
void MOTOR_PWM(int16_t MOTO1_PWM, int16_t MOTO2_PWM, int16_t MOTO3_PWM, int16_t MOTO4_PWM)
{
    if (MOTO1_PWM > Moto_PwmMax)
        MOTO1_PWM = Moto_PwmMax;
    if (MOTO2_PWM > Moto_PwmMax)
        MOTO2_PWM = Moto_PwmMax;
    if (MOTO3_PWM > Moto_PwmMax)
        MOTO3_PWM = Moto_PwmMax;
    if (MOTO4_PWM > Moto_PwmMax)
        MOTO4_PWM = Moto_PwmMax;
    if (MOTO1_PWM < 0)
        MOTO1_PWM = 0;
    if (MOTO2_PWM < 0)
        MOTO2_PWM = 0;
    if (MOTO3_PWM < 0)
        MOTO3_PWM = 0;
    if (MOTO4_PWM < 0)
        MOTO4_PWM = 0;

    TIM3->CCR1 = MOTO1_PWM;
    TIM3->CCR2 = MOTO2_PWM;
    TIM3->CCR3 = MOTO3_PWM;
    TIM3->CCR4 = MOTO4_PWM;
}

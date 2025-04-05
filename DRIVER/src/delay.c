#include "stm32f4xx.h"
#include "delay.h"

static uint16_t g_fac_us = 0;

void Delay_Init(void)
{
    SysTick->CTRL = 0;
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8); /*8分频*/
    g_fac_us = SystemCoreClock / 1000000 / 8;
}

void Delay_us(uint32_t nus)
{
    uint32_t temp;
    SysTick->LOAD = nus * g_fac_us; /*时间重载*/
    SysTick->VAL = 0x00;            /*清空计数器*/
    SysTick->CTRL |= 1 << 0;        /*开始倒数*/
    do
    {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16))); /*CTRL.ENABLE位必须为1，并等待时间到达*/

    SysTick->CTRL &= ~(1 << 0); /*关闭SYSTICK*/
    SysTick->VAL = 0x00;        /*清空计数器*/
}

void Delay_ms(uint32_t nms)
{
    uint32_t repeat = nms / 1000; // 100MHz8分频时，Delay_us最大延时1342177us
    uint32_t remain = nms % 1000;
    while (repeat)
    {
        Delay_us(1000 * 1000); // 实现1000ms延时
        repeat--;
    }
    if (remain)
    {
        Delay_us(remain * 1000); // 尾数延时
    }
}

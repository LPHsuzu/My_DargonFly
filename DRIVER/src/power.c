#include "power.h"
#include "si24r1.h"
#include "structconfig.h"
#include "filter.h"
#include <stdio.h>

BATT_TYPE BAT =
    {
        .BattAdc = 0,       // 电池电压采集ADC值
        .BattRealV = 3.31f, // 实际测量的飞机供电电压 (注意此电压必须亲测否则测量的电压不准)
        .BattMeasureV = 0,  // 程序测量的实际电池电压
        .BattAlarmV = 3.2f, // 电池低电压报警瞬时值 (这个值需要根据机身不同重量实测，实测380mh是2.8v)
        .BattFullV = 4.2f,  // 电池充满电值 4.2V
};
uint8_t BATT_LEDFlag = 0;

/**
 * @brief 电源管理初始化
 * @note WiFi和OpenMV默认是关闭状态
 */
void BATT_Init(void)
{
    GPIO_InitTypeDef gpio_init_structure;
    ADC_InitTypeDef adc_init_structure;
    ADC_CommonInitTypeDef adc_common_init_structure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOC, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    /*模拟输入模式选择*/
    gpio_init_structure.GPIO_Mode = GPIO_Mode_AN;
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;
    gpio_init_structure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    gpio_init_structure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOA, &gpio_init_structure);
    /*推挽输出模式选择*/
    gpio_init_structure.GPIO_Mode = GPIO_Mode_OUT;
    gpio_init_structure.GPIO_OType = GPIO_OType_PP;
    gpio_init_structure.GPIO_Speed = GPIO_Speed_100MHz;
    /*ESP8266（wifi） EN,BOOT控制引脚*/
    gpio_init_structure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_Init(GPIOA, &gpio_init_structure);
    GPIO_ResetBits(GPIOA, GPIO_Pin_4); // ESP_EN
    GPIO_ResetBits(GPIOA, GPIO_Pin_5); // ESP_BOOT
    /*OpenMV OM_PWR电源控制引脚*/
    gpio_init_structure.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &gpio_init_structure);
    GPIO_ResetBits(GPIOC, GPIO_Pin_13);

    /*ADC通用配置(ADC时钟频率最好不要超过36MHz)*/
    adc_common_init_structure.ADC_Mode = ADC_Mode_Independent;                     // 独立模式
    adc_common_init_structure.ADC_Prescaler = ADC_Prescaler_Div4;                  // 4分频 fplck2/4 = 25MHz
    adc_common_init_structure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;      // DMA失能
    adc_common_init_structure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles; // 两个采样之间间隔5个时钟
    ADC_CommonInit(&adc_common_init_structure);

    /*ADC1参数初始化*/
    adc_init_structure.ADC_Resolution = ADC_Resolution_12b;                      // 12位采样精度
    adc_init_structure.ADC_ScanConvMode = DISABLE;                               // 失能扫描模式
    adc_init_structure.ADC_ContinuousConvMode = DISABLE;                         // 失能连续转换
    adc_init_structure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // 不开启触发，使用软件触发
    adc_init_structure.ADC_DataAlign = ADC_DataAlign_Right;                      // 数据右对齐
    adc_init_structure.ADC_NbrOfConversion = 1;                                  // 一个转化在规则序列中
    ADC_Init(ADC1, &adc_init_structure);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, ADC_SampleTime_144Cycles); // ADC1的ADC_Channel_0进行规则转换配置

    ADC_Cmd(ADC1, ENABLE); // 使能ADC1
}

/**
 * @brief 获取电池采样点电压的ADC值
 * @note 电池电压采样点的ADC值，电池电压采样电路见原理图
 * @param ch ADC采样通道
 * @return 返回通道AD值
 */
uint16_t Get_BatteryAdc(uint8_t ch)
{
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_144Cycles);
    ADC_SoftwareStartConv(ADC1);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC))
        ;
    return ADC_GetConversionValue(ADC1);
}

/**
 * @brief 获取电池电压
 * @note 电池电压 = ADC检测电压*2 具体原理可看原理图
 */
void BATT_GetVoltage(void)
{
    float V;
    Aver_Filter((float)Get_BatteryAdc(ADC_Channel_0), &BAT.BattAdc, 6); // 滑动滤波一下电压值，提高精度
    if (BAT.BattAdc)
    {
        V = BAT.BattAdc * BAT.BattRealV / 4095.0f;
        BAT.BattMeasureV = 2 * V; // 根据原理电阻分压，可知 电池实际电压 = ADC侧量电压 * 2
    }
}

/**
 * @brief 低电量报警
 * @note 电池电压 = ADC检测电压*2 具体原理可看原理图
 */
void LowVoltage_Alarm(void)
{
    static uint8_t cnt = 0, cnt1 = 0;
    BATT_GetVoltage();
    if (Airplane_Enable)
    {
        if (BAT.BattMeasureV < BAT.BattAlarmV) // 飞行时测量
        {
            if (cnt1++ > 10)
            {
                cnt1 = 0;
                BATT_LEDFlag = 1;
            }
        }
        else
        {
            cnt1 = 0;
            BATT_LEDFlag = 0;
        }
    }
    else
    {
        if (BAT.BattMeasureV < 3.7f) // 落地时测量（380mh时是3.5V）
        {
            if (cnt++ > 10)
            {
                Run_flag = 0;
                cnt = 0;
                BATT_LEDFlag = 1;
            }
        }
        else
        {
            Run_flag = 1;
            cnt = 0;
            BATT_LEDFlag = 0;
        }
    }
}

/**
 * @brief 开关Wifi模块
 * @note ESP8266(wifi) EN，BOOT 控制引脚，当EN和BOOT都置1时wifi进入工作模式，反之进入低功耗
 * @param flag flag=1开启WiFi;flag=0关闭WiFi
 */
void WiFi_Switch(uint8_t flag)
{
    if (flag) // 开启WiFi
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_4); // ESP_EN
        GPIO_SetBits(GPIOA, GPIO_Pin_5); // ESP_BOOT
    }
    else // 关闭WiFi
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_4); // ESP_EN
        GPIO_ResetBits(GPIOA, GPIO_Pin_5); // ESP_BOOT
    }
}

/**
 * @brief 开关OpenMV模块
 * @note OpenMV OM_PWR电源控制引脚，当OM_PWR置1 OpenMV进入工作模式,反之进入低功耗
 * @param flag flag=1开启OpenMV;flag=0关闭OpenMV
 */
void OpenMV_Switch(uint8_t flag)
{
    if (flag) // 开启OpenMV
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_13); // OM_PWR
    }
    else // 关闭OpenMV
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_13); // OM_PWR
    }
}

/**
 * @brief 开关NRF2401模块
 * @note NRF2401 CE电源控制引脚，当CE置1 NRF2401进入工作模式,反之进入低功耗
 * @param flag flag=1开启NRF2401;flag=0关闭NRF2401
 */
void NRF24L01_Switch(uint8_t flag)
{
    if (flag) // 开启NRF2401
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_8); // NRF_CE
        NRF24L01_write_reg(W_REGISTER + CONFIG, 0x0A);
    }
    else // 关闭NRF2401
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_8); // NRF_CE
        NRF24L01_write_reg(W_REGISTER + CONFIG, 0x0F);
    }
}

#include "..\CMSIS\stm32f4xx.h" // Device header
#include "usart.h"
#include "structconfig.h"
#include <stdio.h>

/**
 * @brief USART1 GPIO初始化
 *
 */
void usart1_gpio_config(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    /*连接USART1的通道到AF7*/
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    /*PA9(RX)配置为上拉输入*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /*PA10(TX)配置为推挽输出*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
 * @brief USART2 GPIO初始化
 *
 */
void usart2_gpio_config(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;
    /*连接USART2的通道到AF7*/
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    /*PA3(RX)配置为上拉输入*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    /*PA2(TX)配置为推挽输出*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
 * @brief USART1初始化
 * @note 初始化为双工模式（OpenMV用此串口）。对于连续的数据帧的接收 接收中断与空闲中断配合能解决丢包问题，具体接收方式见stm32f4xx_it.c 中的串口中断处理。
 * @param baudrate 波特率
 */
void usart1_init(uint32_t baudrate)
{
    usart1_gpio_config();
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    USART_InitTypeDef USART_InitStructure;

    USART_DeInit(USART1); // USART1复位

    USART_InitStructure.USART_BaudRate = baudrate;                                  // 波特率
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控制
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                 // 发送模式和接收模式
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无校验位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 一位停止位
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 字长八位
    USART_Init(USART1, &USART_InitStructure);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // 串口接收中断
    USART_ITConfig(USART1, USART_IT_IDLE, ENABLE); // 串口空闲中断

    USART_Cmd(USART1, ENABLE);
}

/**
 * @brief USART2初始化
 * @note 初始化为双工模式（ESP8266(wifi)用此串口）。对于连续的数据帧的接收 接收中断与空闲中断配合能解决丢包问题，具体接收方式见stm32f4xx_it.c 中的串口中断处理。
 * @param baudrate 波特率
 */
void usart2_init(uint32_t baudrate)
{
    usart2_gpio_config();
    RCC_APB2PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    USART_InitTypeDef USART_InitStructure;

    USART_DeInit(USART2); // USART2复位

    USART_InitStructure.USART_BaudRate = baudrate;                                  // 波特率
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控制
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;                 // 发送模式和接收模式
    USART_InitStructure.USART_Parity = USART_Parity_No;                             // 无校验位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;                          // 一位停止位
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;                     // 字长八位
    USART_Init(USART2, &USART_InitStructure);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // 串口接收中断
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE); // 串口空闲中断

    USART_Cmd(USART2, ENABLE);
}

/*printf重定向*/
int fputc(int ch, FILE *f)
{
    USART_SendData(USART1, (uint8_t)ch);
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
        ;

    return ch;
}

/**
 * @brief USART发送指定长度数据
 * @note 宏定义WIFI_DEBUG，在structconfig.h 中定义和取消;
         如果开启了WiFi调参功能，则数据从USART2传到ESP8266,然后经ESP8266再传到上位机;
         如果未开启WiFi调参功能，则数据从USART1的Tx,Rx经线连接eLink32的Rx,Tx传到上位机;
 * @param data 要发送数据的地址
 * @param length 要发送数据的长度
 */
void usart_send(uint8_t *data, uint8_t length)
{
    uint8_t i;
#ifdef WIFI_DEBUG // 开启WiFi(无线)调参
    for (i = 0; i < length; i++)
    {
        USART_SendData(USART2, *(data + i));
        while (USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET)
            ;
    }
#else // 有线调参
    for (i = 0; i < length; i++)
    {
        USART_SendData(USART1, *(data + i));
        while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
            ;
    }
#endif
}

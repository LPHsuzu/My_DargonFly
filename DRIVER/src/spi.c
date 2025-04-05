#include "spi.h"

/**
 * @brief SPI2 GPIO引脚初始化
 * @param 无
 * @retval 无
 */
static void spi_gpio_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

    GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /*SPI SCK 引脚初始化*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*SPI MOSI 引脚初始化*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*SPI MISO 引脚初始化*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /*初始化Si24R1 CS引脚*/
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * @brief SPI2初始化
 * @param 无
 * @retval 无
 */
void spi_init(void)
{
    SPI_InitTypeDef SPI_InitStructure;

    spi_gpio_init();

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;

    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI2, &SPI_InitStructure);

    /*使能Si24R1 SPI*/
    SPI_Cmd(SPI2, ENABLE);
}

/**
 * @brief SPI2读写一个字节
 * @param data 待发送数据
 * @return 接收到的数据
 */
uint16_t spi_readwriteByte(uint16_t data)
{
    uint16_t count = 0;
    while (SPI_GetFlagStatus(SPI2, SPI_FLAG_TXE) != SET) /*等待发送缓冲区为空*/
    {
        if (count++ > 200)
        {
            break;
        }
    }
    SPI_SendData(SPI2, data); /*发送数据*/

    count = 0;
    while (SPI_GetFlagStatus(SPI2, SPI_FLAG_RXNE) != SET) /*等待接收缓冲区为空*/
    {
        if (count++ > 200)
        {
            break;
        }
    }
    return SPI_ReceiveData(SPI2); /*返回接收到的数据*/
}

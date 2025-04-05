#include "iic_analog.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
#include "delay.h"

#define RCC_IIC_SCL RCC_AHB1Periph_GPIOB // 端口时钟
#define IIC_SCL_PORT GPIOB               // 端口
#define IIC_SCL_Pin GPIO_Pin_6           // 引脚

#define RCC_IIC_SDA RCC_AHB1Periph_GPIOB // 端口时钟
#define IIC_SDA_PORT GPIOB               // 端口
#define IIC_SDA_Pin GPIO_Pin_7           // 引脚

/*IO口操作函数*/
#define IIC_SCL_H                                \
    do                                           \
    {                                            \
        GPIO_SetBits(IIC_SCL_PORT, IIC_SCL_Pin); \
    } while (0)

#define IIC_SCL_L                                  \
    do                                             \
    {                                              \
        GPIO_ResetBits(IIC_SCL_PORT, IIC_SCL_Pin); \
    } while (0)

#define IIC_SDA_H                                \
    do                                           \
    {                                            \
        GPIO_SetBits(IIC_SDA_PORT, IIC_SDA_Pin); \
    } while (0)

#define IIC_SDA_L                                  \
    do                                             \
    {                                              \
        GPIO_ResetBits(IIC_SDA_PORT, IIC_SDA_Pin); \
    } while (0)

/**
 * @brief IIC延时
 * @note 移植时只需要将delay_us()换成自己的延时即可
 * @param 无
 * @retval 无
 */
void IIC_Delay(void)
{
    Delay_us(10);
}

/**
 * @brief IIC读取引脚电平
 * @param 无
 * @retval BitValue 电平值
 */
uint8_t IIC_ReadBit(void)
{
    uint8_t BitValue;
    BitValue = GPIO_ReadInputDataBit(IIC_SDA_PORT, IIC_SDA_Pin);
    IIC_Delay();
    return BitValue;
}

/**
 * @brief IIC初始化
 * @param 无
 * @retval 无
 */
void IIC_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_IIC_SCL, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_IIC_SDA, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin = IIC_SCL_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IIC_SCL_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = IIC_SDA_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IIC_SDA_PORT, &GPIO_InitStructure);
}

/**
 * @brief 产生IIC起始信号
 * @param 无
 * @retval 无
 */
void IIC_Start(void)
{
    IIC_SDA_H;
    IIC_SCL_H;
    IIC_Delay();
    IIC_SDA_L;
    IIC_Delay();
    IIC_SCL_L;
}

/**
 * @brief 产生IIC停止信号
 * @param 无
 * @retval 无
 */
void IIC_Stop(void)
{
    IIC_SCL_L;
    IIC_SDA_L;
    IIC_Delay();
    IIC_SCL_H;
    IIC_SDA_H;
    IIC_Delay();
}

/**
 * @brief IIC发送一个字节
 * @param Byte 要发送的数据
 * @retval 无
 */
void IIC_SendByte(uint8_t Byte)
{
    uint8_t i;
    for (i = 0; i < 8; i++)
    {
        if (Byte & (0x80 >> i))
        {
            IIC_SDA_H;
        }
        else
        {
            IIC_SDA_L;
        }
        IIC_Delay();
        IIC_SCL_H;
        IIC_Delay();
        IIC_SCL_L;
        IIC_Delay();
    }
}

/**
 * @brief IIC接收一个字节
 * @param 无
 * @retval Byte 所接收到的数据
 */
uint8_t IIC_ReceiveByte(void)
{
    uint8_t i, Byte = 0x00;
    IIC_SDA_H;
    IIC_Delay();
    for (i = 0; i < 8; i++)
    {
        IIC_SCL_H;
        IIC_Delay();
        if (IIC_ReadBit() == 1)
        {
            Byte |= (0x80 >> i);
        }
        IIC_SCL_L;
    }
    return Byte;
}

/**
 * @brief IIC发送应答
 * @param AckBit：1不应答，0应答
 * @retval 无
 */
void IIC_SendAck(uint8_t AckBit)
{
    if (AckBit)
    {
        IIC_SDA_H;
    }
    else
    {
        IIC_SDA_L;
    }
    IIC_Delay();
    IIC_SCL_H;
    IIC_Delay();
    IIC_SCL_L;
}

/**
 * @brief IIC接收应答
 * @param 无
 * @retval AckBit：1未接收到应答，0接收到应答
 */
uint8_t IIC_ReceiveAck(void)
{
    uint8_t AckBit;
    IIC_SDA_H;
    IIC_Delay();
    IIC_SCL_H;
    IIC_Delay();
    AckBit = IIC_ReadBit();
    IIC_SCL_L;
    return AckBit;
}

/**
 * @brief 向指定设备指定寄存器写入一个字节
 * @param devAddr 目标设备地址
 * @param regAddr 目标寄存器地址
 * @param data 要写入的数据
 * @retval 写入状态：1写入失败
 */
uint8_t IIC_WriteByteToSlave(uint8_t devAddr, uint8_t regAddr, uint8_t data)
{
    IIC_Start();
    IIC_SendByte(devAddr << 1);
    if (IIC_ReceiveAck())
    {
        IIC_Stop();
        return 1; /*从机地址写入失败*/
    }
    IIC_SendByte(regAddr);
    IIC_ReceiveAck();
    IIC_SendByte(data);
    if (IIC_ReceiveAck())
    {
        IIC_Stop();
        return 1; /*数据写入失败*/
    }
    IIC_Stop();

    return 0;
}

/**
 * @brief 从指定设备指定寄存器读出一个字节
 * @param devAddr 目标设备地址
 * @param regAddr 目标寄存器地址
 * @param buf 读出数据存储地址
 * @retval 写入状态：1从机地址写入失败
 */
uint8_t IIC_ReadByetFromSlave(uint8_t devAddr, uint8_t regAddr, uint8_t *buf)
{
    IIC_Start();
    IIC_SendByte(devAddr << 1);
    if (IIC_ReceiveAck())
    {
        IIC_Stop();
        return 1; /*从机地址写入失败*/
    }
    IIC_SendByte(regAddr);
    IIC_ReceiveAck();
    IIC_Start();
    IIC_SendByte((devAddr << 1) | 0x01);
    IIC_ReceiveAck();
    *buf = IIC_ReceiveByte();
    IIC_SendAck(0);
    IIC_Stop();
    return 0;
}

/**
 * @brief 向指定设备指定寄存器写入指定个数数据
 * @param devAddr 目标设备地址
 * @param regAddr 目标寄存器地址
 * @param length 写入的字节长度
 * @param data 待写入数据的地址
 * @retval 写入状态：1写入失败
 */
uint8_t IIC_WriteMultByteToSlave(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data)
{
    uint8_t count = 0;
    IIC_Start();
    IIC_SendByte(devAddr << 1);
    if (IIC_ReceiveAck())
    {
        IIC_Stop();
        return 1; /*从机地址写入失败*/
    }
    IIC_SendByte(regAddr);
    IIC_ReceiveAck();
    for (count = 0; count < length; count++)
    {
        IIC_SendByte(data[count]);
        if (IIC_ReceiveAck())
        {
            IIC_Stop();
            return 1; /*数据写入失败*/
        }
    }
    IIC_Stop();

    return 0;
}

/**
 * @brief 从指定设备指定寄存器读出指定个数数据
 * @param devAddr 目标设备地址
 * @param regAddr 目标寄存器地址
 * @param length 读出数据的字节长度
 * @param data 读出数据存放地址
 * @retval 写入状态：1从机地址写入失败
 */
uint8_t IIC_ReadMultByteFromSlave(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data)
{
    uint8_t count = 0;
    uint8_t temp;
    IIC_Start();
    IIC_SendByte(devAddr << 1);
    if (IIC_ReceiveAck())
    {
        IIC_Stop();
        return 1; /*从机地址写入失败*/
    }
    IIC_SendByte(regAddr);
    IIC_ReceiveAck();
    IIC_Start();
    IIC_SendByte((devAddr << 1) | 0x01);
    IIC_ReceiveAck();
    for (count = 0; count < length; count++)
    {
        if (count != length - 1)
        {
            temp = IIC_ReceiveByte();
            IIC_SendAck(0);
        }
        else
        {
            temp = IIC_ReceiveByte();
            IIC_SendAck(1);
        }

        data[count] = temp;
    }
    IIC_Stop();

    return 0;
}

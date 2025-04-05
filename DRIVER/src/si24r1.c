#include "si24r1.h"
#include "spi.h"
#include "delay.h"
#include "led.h"
#include <stdio.h>
#include <stdlib.h>

#define RCC_NRF_SCN RCC_AHB1Periph_GPIOB // 端口时钟
#define NRF_SCN_PORT GPIOB               // 端口
#define NRF_SCN GPIO_Pin_12              // 引脚

#define RCC_NRF_CE RCC_AHB1Periph_GPIOA // 端口时钟
#define NRF_CE_PORT GPIOA               // 端口
#define NRF_CE GPIO_Pin_8               // 引脚

#define RCC_NRF_IRQ RCC_AHB1Periph_GPIOB // 端口时钟
#define NRF_IRQ_PORT GPIOB               // 端口
#define NRF_IRQ GPIO_Pin_2               // 引脚

#define NRF_SCN_LOW                            \
    do                                         \
    {                                          \
        GPIO_ResetBits(NRF_SCN_PORT, NRF_SCN); \
    } while (0)
#define NRF_SCN_HIGH                         \
    do                                       \
    {                                        \
        GPIO_SetBits(NRF_SCN_PORT, NRF_SCN); \
    } while (0)
#define NRF_CE_LOW                           \
    do                                       \
    {                                        \
        GPIO_ResetBits(NRF_CE_PORT, NRF_CE); \
    } while (0)
#define NRF_CE_HIGH                        \
    do                                     \
    {                                      \
        GPIO_SetBits(NRF_CE_PORT, NRF_CE); \
    } while (0)

#define NRFAddrMax 50   // NRF最后一个字节地址最大为50
uint8_t NRFaddr = 0xFF; // 初始化NRF最后一字节地址

uint8_t NRF_TX_DATA[TX_PAYLO_WIDTH]; // NRF发送缓冲区
uint8_t NRF_RX_DATA[RX_PAYLO_WIDTH]; // NRF接收缓冲区

uint8_t TX_ADDRESS[TX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0xFF}; // 发送地址
uint8_t RX_ADDRESS[RX_ADR_WIDTH] = {0x34, 0x43, 0x10, 0x10, 0xFF}; // 接收地址

/**
 * @brief 写一个字节数据到寄存器
 *
 * @param reg 寄存器地址
 * @param value 要写入的数据
 * @retval status 状态值
 */
uint8_t NRF24L01_write_reg(uint8_t reg, uint8_t value)
{
    uint8_t status;
    NRF_SCN_LOW;
    status = spi_readwriteByte(reg);
    spi_readwriteByte(value);
    NRF_SCN_HIGH;
    return status;
}

/**
 * @brief 从寄存器读一字节数据
 *
 * @param reg 寄存器地址
 * @return reg_val 寄存器上的值
 */
uint8_t NRF24L01_read_reg(uint8_t reg)
{
    uint8_t reg_val;
    NRF_SCN_LOW;
    spi_readwriteByte(reg);
    reg_val = spi_readwriteByte(0xFF);
    NRF_SCN_HIGH;
    return reg_val;
}

/**
 * @brief 写一组数据到寄存器
 *
 * @param reg 寄存器地址
 * @param pbuf 要写入数据的地址
 * @param len 要写入数据的长度
 * @retval status 状态值
 */
uint8_t NRF24L01_Write_Buf(uint8_t reg, uint8_t *pbuf, uint8_t len)
{
    uint8_t status;
    int i;
    NRF_SCN_LOW;
    status = spi_readwriteByte(reg);
    for (i = 0; i < len; i++)
    {
        spi_readwriteByte(pbuf[i]);
    }
    NRF_SCN_HIGH;
    return status;
}

/**
 * @brief 从寄存器读一组数据
 *
 * @param reg 寄存器地址
 * @param pbuf 数据读出地址
 * @param len 要读出的数据长度
 * @retval status 状态值
 */
uint8_t NRF24L01_Read_Buf(uint8_t reg, uint8_t *pbuf, uint8_t len)
{
    uint8_t status;
    int i;
    NRF_SCN_LOW;
    status = spi_readwriteByte(reg);
    for (i = 0; i < len; i++)
    {
        pbuf[i] = spi_readwriteByte(0xFF);
    }
    NRF_SCN_HIGH;
    return status;
}

/**
 * @brief 切换NRF24L01的工作模式
 *
 * @param mode 指定要切换的模式
 *      该参数可以是以下值之一：
 *          @arg IT_TX：发送模式
 *          @arg IT_RX：接收模式
 * @retval 无
 */
void NRF24L01_SetMode(uint8_t mode)
{
    if (mode == IT_TX)
    {
        NRF_CE_LOW;
        NRF24L01_write_reg(W_REGISTER + CONFIG, IT_TX); // 配置为发送模式
        NRF24L01_write_reg(W_REGISTER + STATUS, 0x7E);  // 清除所有中断,防止一进去发送模式就触发中断
        NRF_CE_HIGH;
    }
    else
    {
        NRF_CE_LOW;
        NRF24L01_write_reg(W_REGISTER + CONFIG, IT_RX); // 配置为接收模式
        NRF24L01_write_reg(W_REGISTER + STATUS, 0x7E);  // 清除所有中断,防止一进去发送模式就触发中断
        Delay_us(200);
    }
}

/**
 * @brief NRF基本参数配置，并初始化为接收模式
 *
 */
void NRF24L01_Config(void)
{
    NRF_CE_LOW;
    NRF24L01_write_reg(W_REGISTER + SETUP_AW, 0x03);                                  // 配置通信地址的长度，默认值时0x03,即地址长度为5字节
    NRF24L01_Write_Buf(W_REGISTER + TX_ADDR, (uint8_t *)TX_ADDRESS, TX_ADR_WIDTH);    // 写TX节点地址
    NRF24L01_Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t *)TX_ADDRESS, RX_ADR_WIDTH); // 设置RX节点地址,主要为了使能ACK
    NRF24L01_write_reg(W_REGISTER + SETUP_RETR, 0x1A);                                // 设置自动重发间隔时间:500us + 86us;最大自动重发次数:10次 0x1A

    NRF24L01_write_reg(W_REGISTER + EN_RXADDR, 0x01);                                 // 使能通道0的接收地址
    NRF24L01_write_reg(W_REGISTER + EN_AA, 0x01);                                     // 使能通道0自动应答
    NRF24L01_write_reg(W_REGISTER + RX_PW_P0, RX_PAYLO_WIDTH);                        // 选择通道0的有效数据宽度
    NRF24L01_Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t *)RX_ADDRESS, RX_ADR_WIDTH); // 写RX节点地址
    NRF24L01_write_reg(W_REGISTER + RF_CH, 30);                                       // 设置RF通道为40hz(1-64Hz都可以)
    NRF24L01_write_reg(W_REGISTER + RF_SETUP, 0x27);                                  // 设置TX发射参数,0db增益,2Mbps,低噪声增益关闭 （注意：低噪声增益关闭/开启直接影响通信,要开启都开启，要关闭都关闭0x0f）0x07

    NRF24L01_SetMode(IT_RX); // 默认为接收模式

    NRF_CE_HIGH;
}

/**
 * @brief NRF发送一包数据
 *
 * @param txbuf 要发送数据的地址
 */
void NRF24L01_TXPacket(uint8_t *txbuf)
{
    NRF_CE_LOW;
    NRF24L01_Write_Buf(W_REGISTER + TX_ADDR, (uint8_t *)TX_ADDRESS, TX_ADR_WIDTH);    // 写TX节点地址
    NRF24L01_Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t *)TX_ADDRESS, RX_ADR_WIDTH); // 设置RX节点地址,主要为了使能ACK
    NRF24L01_Write_Buf(W_RX_PAYLOAD, txbuf, TX_PAYLO_WIDTH);                          // 写数据到TX_BUFF
    NRF24L01_write_reg(W_REGISTER + CONFIG, 0x0e);                                    // 设置为发送模式,开启所有中断
    NRF24L01_write_reg(W_REGISTER + STATUS, 0X7E);                                    // 清除所有中断,防止一进去发送模式就触发中断
    NRF_CE_HIGH;
    Delay_us(10); // CE持续高电平10us
}

/**
 * @brief NRF接收一包数据
 *
 * @param rxbuf 读出数据存储地址
 */
void NRF24L01_RxPacket(uint8_t *rxbuf)
{
    NRF_CE_LOW;
    NRF24L01_Read_Buf(R_RX_PAYLOAD, rxbuf, TX_PAYLO_WIDTH); // 读取RX的有效数据
    NRF24L01_write_reg(FLUSH_RX, 0xFF);                     // 清除RX FIFO(注意：这句话很必要)
    NRF_CE_HIGH;
}

/**
 * @brief 检查NRF24L01与MCU的SPI总线是否通信正常
 *
 * @retval 1 已连接
 *         0 未连接
 */
uint8_t NRF24L01_testConnection(void)
{
    uint8_t buf[5] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
    uint8_t i;
    NRF24L01_Write_Buf(W_REGISTER + TX_ADDR, buf, 5); // 写入5个字节的地址
    NRF24L01_Read_Buf(TX_ADDR, buf, 5);               // 读出写入的地址
    for (i = 0; i < 5; i++)
    {
        if (buf[i] != 0XA5)
            break;
    }
    if (i != 5)
        return 0; // 检测24L01错误
    return 1;     // 检测到24L01
}

void NRF24L01_Check(void)
{
    while (!NRF24L01_testConnection())
    {
        printf("\rNRF24L01 no connect...\r\n");
        RGB_LED_Red(); // 红灯常亮
    }
}

/**
 * @brief NRF引脚GPIO初始化
 *
 */
void NRF24L01_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_NRF_SCN | RCC_NRF_CE, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_InitStructure.GPIO_Pin = NRF_SCN; // NRF的片选信号SCN
    GPIO_Init(NRF_SCN_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(NRF_SCN_PORT, NRF_SCN);

    GPIO_InitStructure.GPIO_Pin = NRF_CE; // NRF的片选信号CE
    GPIO_Init(NRF_CE_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(NRF_CE_PORT, NRF_CE);

    spi_init(); // SPI初始化

    NRF24L01_Check(); // 检查NRF24L01是否与MCU通信

    NRF_SCN_HIGH; // 失能NRF
    NRF_CE_LOW;   // 待机模式

    // NRF24L01_Config(); // 配置NRF并初始化为接收模式
}

/**
 * @brief NRF(全双工)的外部中断处理函数
 *
 */
void EXTI2_IRQHandler(void)
{
    uint8_t sta;
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
        RunTimer_Test();
        NRF_CE_LOW;                                   // 拉低CE，以便读取NRF中STATUS中的数据
        sta = NRF24L01_read_reg(R_REGISTER + STATUS); // 读取STATUS中的数据，以便判断是由什么中断源触发的IRQ中断

        /*发送完成中断 TX_OK*/
        if (sta == TX_OK)
        {
            NRF24L01_SetMode(IT_RX);
            NRF24L01_write_reg(W_REGISTER + STATUS, TX_OK); // 清除发送完成标志
            NRF24L01_write_reg(FLUSH_TX, 0xFF);             // 清除TX_FIFO
            // printf("Sent OK!!!\r\n");
        }
        /*接收完成中断 RX_OK*/
        if (sta == RX_OK)
        {
            NRF24L01_RxPacket(NRF_RX_DATA);
            Remote_Data_ReceiveAnalysis();
            NRF24L01_write_reg(W_REGISTER + STATUS, RX_OK); // 清除接收完成标志
            // printf("Receive OK!!!\r\n");
        }
        /*达到最大重发次数中断 MAX_TX*/
        if (sta & MAX_TX)
        {
            NRF24L01_SetMode(IT_RX);
            NRF24L01_write_reg(W_REGISTER + STATUS, MAX_TX); // 清除达到最大重发标志
            NRF24L01_write_reg(FLUSH_TX, 0xFF);              // 清除TX_FIFO
            // printf("Sent Max Data!!!\r\n");
        }
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

/**
 * @brief 给飞机上的NRF配置一个地址
 * @note 此函数需要与遥控器的对频函数联合使用否者NRF通信不成功
 */
void NRF_GetAddr(void)
{
    if (NRFaddr > NRFAddrMax) // 当 NRFaddr大于NRFAddrMax，就说明次时NRF还未初始化完成
    {
        srand(SysTick->VAL); // 提供随机种子
        // printf("SysTick->VAL:%d\r\n", SysTick->VAL);
        NRFaddr = rand() % NRFAddrMax; // 随机获取NRF最后一字节地址（地址:0~50）
        PID_WriteFlash();              // 保存此地址Flash
    }
    else if (NRFaddr != TX_ADDRESS[TX_ADR_WIDTH - 1])
    {
        TX_ADDRESS[TX_ADR_WIDTH - 1] = NRFaddr;
        RX_ADDRESS[TX_ADR_WIDTH - 1] = NRFaddr;
        NRF24L01_Config();
        // printf("NRFAddr:%d\r\n", NRFaddr);
    }
}

/**
 * @brief NRF通信测试函数
 *
 */
void NRF_Test(void)
{
    uint8_t t = 0;
    static uint8_t mode, key;
    mode = ' ';
    key = mode;
    for (t = 0; t < 32; t++)
    {
        key++;
        if (key > ('~'))
            key = ' ';
        NRF_TX_DATA[t] = key;
    }
    mode++;
    if (mode > '~')
        mode = ' ';
    NRF24L01_TXPacket(NRF_TX_DATA);
}

#include "../CMSIS/stm32f4xx.h" // Device header
#include "led.h"
#include "usart.h"
#include "delay.h"
#include "iic_analog.h"
#include <stdio.h>
#include "mpu6500.h"
#include "spi.h"
#include "si24r1.h"

int main(void)
{
//    uint8_t data = 0;

    LED_Init();
//    usart1_init(115200);
    Delay_Init();
//    MPU6500_Init();
//    spi_init();
//    NRF24L01_Init();

    while (1)
    {
//        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
//        spi_readwriteByte(0x20 + 0x01);
//        spi_readwriteByte(0x03);
//        GPIO_SetBits(GPIOB, GPIO_Pin_12);

//        GPIO_ResetBits(GPIOB, GPIO_Pin_12);
//        spi_readwriteByte(0x01);
//        data = spi_readwriteByte(0xFF);
//        GPIO_SetBits(GPIOB, GPIO_Pin_12);

        Delay_ms(500);
//        printf("NRF data:0x%x\r\n", data);
        LED5_Run();
    }
}

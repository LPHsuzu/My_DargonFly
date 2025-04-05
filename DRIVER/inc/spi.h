#ifndef __SPI_H
#define __SPI_H

#include "stm32f4xx.h" // Device header

void spi_init(void);

uint16_t spi_readwriteByte(uint16_t data);

#endif

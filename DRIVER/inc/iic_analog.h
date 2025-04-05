#ifndef __IIC_ANALOG_H
#define __IIC_ANALOG_H

#include "stm32f4xx.h"

void IIC_Init(void);
void IIC_Start(void);
void IIC_Stop(void);
void IIC_SendByte(uint8_t Byte);
uint8_t IIC_ReceiveByte(void);
void IIC_SendAck(uint8_t AckBit);
uint8_t IIC_ReceiveAck(void);

uint8_t IIC_WriteByteToSlave(uint8_t devAddr, uint8_t regAddr, uint8_t data);
uint8_t IIC_ReadByetFromSlave(uint8_t devAddr, uint8_t regAddr, uint8_t *buf);
uint8_t IIC_WriteMultByteToSlave(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);
uint8_t IIC_ReadMultByteFromSlave(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data);

#endif

#ifndef __MPU6500_H
#define __MPU6500_H

uint8_t MPU6500_WriteByte(uint8_t regAddr, uint8_t data);
uint8_t MPU6500_ReadByte(uint8_t regAddr, uint8_t *buf);
uint8_t MPU6500_WriteMultBytes(uint8_t regAddr, uint8_t length, uint8_t *buf);
uint8_t MPU6500_ReadMultBytes(uint8_t regAddr, uint8_t length, uint8_t *buf);

uint8_t MPU6500_GetDeviceID(void);
uint8_t MPU6500_CheckConnection(void);

void MPU6500_Check(void);

void MPU6500_Init(void);
void MPU6500_AccRead(int16_t *accdata);
void MPU6500_GyroRead(int16_t *gyrodata);
void MPU6500_TempRead(float *tempdata);
void MPU6500_Read(void);

void MPU6500_CalOff(void);
void MPU6500_CalOff_Acc(void);
void MPU6500_CalOff_Gyro(void);

void MPU6500_DataProcess(void);

#endif

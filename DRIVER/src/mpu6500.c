#include "stm32f4xx.h" // Device header
#include "delay.h"
#include "iic_analog.h"
#include "led.h"
#include "mpu6500.h"
#include "mpu6500_reg.h"
#include "structconfig.h"
// #include "paramsave.h"
// #include "ICM20948.h"
#include <stdio.h>

static uint8_t MPU6500_buf[14];              // 加速度计、陀螺仪、温度原始数据
INT16_XYZ GYRO_OFFSET_RAW, ACC_OFFSET_RAW;   // 零飘数据
INT16_XYZ MPU6500_ACC_RAW, MPU6500_GYRO_RAW; // 读取值原始数据
uint8_t SENSER_OFFSET_FLAG;                  // 传感器校准标志位

/**
 * @brief 写一字节数据到MPU6500寄存器
 *
 * @param regAddr 寄存器地址
 * @param data 要写入的数据
 * @return 0：成功
 *         1：失败
 */
uint8_t MPU6500_WriteByte(uint8_t regAddr, uint8_t data)
{
    if (IIC_WriteByteToSlave(MPU6500Addr, regAddr, data))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief 从MPU6500指定寄存器读取一字节数据
 *
 * @param regAddr 寄存器地址
 * @param buf 读出数据存放地址
 * @return 0：成功
 *         1：失败
 */
uint8_t MPU6500_ReadByte(uint8_t regAddr, uint8_t *buf)
{
    if (IIC_ReadByetFromSlave(MPU6500Addr, regAddr, buf))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief 向指定寄存器写入指定长度数据
 *
 * @param regAddr 寄存器地址
 * @param length 写入数据长度
 * @param buf 待写入数据的地址
 * @return 0：成功 1：失败
 */
uint8_t MPU6500_WriteMultBytes(uint8_t regAddr, uint8_t length, uint8_t *buf)
{
    if (IIC_WriteMultByteToSlave(MPU6500Addr, regAddr, length, buf))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief 从指定寄存器读取指定长度数据
 *
 * @param regAddr 寄存器地址
 * @param length 读取数据长度
 * @param buf 读出数据存放地址
 * @return 0：成功 1:失败
 */
uint8_t MPU6500_ReadMultBytes(uint8_t regAddr, uint8_t length, uint8_t *buf)
{
    if (IIC_ReadMultByteFromSlave(MPU6500Addr, regAddr, length, buf))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/* 以上代码移植时修改 */

/**
 * @brief 读取MPU6500 WHO_AM_I 标识将返回 0x70
 *
 * @return 读取数据
 */
uint8_t MPU6500_GetDeviceID(void)
{
    uint8_t ID;
    MPU6500_ReadByte(MPU6500_RA_WHO_AM_I, &ID);
    return ID;
}

/**
 * @brief 检测MPU6500是否已连接
 *
 * @return 1：已连接 0：未连接
 */
uint8_t MPU6500_CheckConnection(void)
{
    if (MPU6500_GetDeviceID() == 0x70)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief 检测IIC总线上MPU6500是否存在
 *
 */
void MPU6500_Check(void)
{
    if (!MPU6500_CheckConnection())
    {
        printf("\rMPU6500 no connect...\r\n");
    }
}

/**
 * @brief 初始化MPU6500进入工作状态
 * @note DLPF最好设置为采样频率一半
 */
void MPU6500_Init(void)
{
    IIC_Init();      // 初始化IIC通信
    MPU6500_Check(); // 检查MPU6500是否连接

    MPU6500_WriteByte(MPU6500_RA_PWR_MGMT_1, 0x80); // 复位MPU6500
    Delay_ms(100);
    MPU6500_WriteByte(MPU6500_RA_PWR_MGMT_1, 0x01);           // 唤醒MPU6500，并选择PLL为时钟源
    MPU6500_WriteByte(MPU6500_RA_PWR_MGMT_2, 0x00);           // 设置输出三轴陀螺仪与三轴加速度数据
    MPU6500_WriteByte(MPU6500_RA_INT_ENABLE, 0x00);           // 禁止中断
    MPU6500_WriteByte(MPU6500_RA_SMPLRT_DIV, 0x00);           // 采样分频  （采样频率 = 陀螺仪输出频率 / (1+DIV)，采样频率1000hz）
    MPU6500_WriteByte(MPU6500_RA_GYRO_CONFIG, 0x18);          // 陀螺仪满量程+-2000度/秒     (最低分辨率 = 2^16/4000 = 16.4LSB/度/秒
    MPU6500_WriteByte(MPU6500_RA_ACCEL_CONFIG, 0x08);         // 加速度满量程+-4g            (最低分辨率 = 2^16/8g = 8196LSB/g )
    MPU6500_WriteByte(MPU6500_RA_ACCEL_CONFIG_2, 0x03);       // 设置加速度滤波为DLPF=41Hz
    MPU6500_WriteByte(MPU6500_RA_CONFIG, MPU6500_DLPF_BW_20); // 设置陀螺的输出为1kHZ,DLPF=20Hz
    MPU6500_WriteByte(MPU6500_RA_INT_PIN_CFG, 0x02);          // MPU 可直接访问MPU6500辅助I2C
}

/**
 * @brief 读取加速度的原始数据
 *
 * @param accdata 存放加速度原始数据的指针
 */
void MPU6500_AccRead(int16_t *accdata)
{
    uint8_t buf[6];

    MPU6500_ReadMultBytes(MPU6500_RA_ACCEL_XOUT_H, 6, buf);
    accdata[0] = (int16_t)((buf[0] << 8) | buf[1]);
    accdata[1] = (int16_t)((buf[2] << 8) | buf[3]);
    accdata[2] = (int16_t)((buf[4] << 8) | buf[5]);
}

/**
 * @brief 读取陀螺仪原始数据
 *
 * @param gyrodata 存放陀螺仪原始数据的指针
 */
void MPU6500_GyroRead(int16_t *gyrodata)
{
    uint8_t buf[6];

    MPU6500_ReadMultBytes(MPU6500_RA_GYRO_XOUT_H, 6, buf);
    gyrodata[0] = (int16_t)((buf[0] << 8) | buf[1]);
    gyrodata[1] = (int16_t)((buf[2] << 8) | buf[3]);
    gyrodata[2] = (int16_t)((buf[4] << 8) | buf[5]);
}

/**
 * @brief 读取温度值
 *
 * @param tempdata 存放温度数据的指针
 */
void MPU6500_TempRead(float *tempdata)
{
    uint8_t buf[2];
    short data;

    MPU6500_ReadMultBytes(MPU6500_RA_TEMP_OUT_H, 2, buf);
    data = (int16_t)((buf[0] << 8) | buf[1]);
    *tempdata = ((float)data / 340.0f) + 36.5f;
}

/**
 * @brief 陀螺仪加速度计校准
 *
 */
void MPU6500_CalOff(void)
{
    SENSER_FLAG_SET(ACC_OFFSET);  // 加速度计校准
    SENSER_FLAG_SET(GYRO_OFFSET); // 陀螺仪校准
}

/**
 * @brief 加速度计校准
 *
 */
void MPU6500_CalOff_Acc(void)
{
    SENSER_FLAG_SET(ACC_OFFSET); // 加速度计校准
}

/**
 * @brief 陀螺仪校准
 *
 */
void MPU6500_CalOff_Gyro(void)
{
    SENSER_FLAG_SET(GYRO_OFFSET); // 陀螺仪校准
}

/**
 * @brief 读取陀螺仪加速度计的原始数据
 * @note 查询法读取MPU6500的原始数据
 */
void MPU6500_Read(void)
{
    MPU6500_ReadMultBytes(MPU6500_RA_ACCEL_XOUT_H, 14, MPU6500_buf);
}

/**
 * @brief MPU6500零偏校准
 *
 * @param value MPU6500原始数据
 * @param offset 校准后的零偏值
 * @param sensivity 加速度计的灵敏度
 * @return 1：校准完成 0：校准未完成
 */
uint8_t MPU6500_OffSet(INT16_XYZ value, INT16_XYZ *offset, uint16_t sensivity)
{
    static int32_t tempgx = 0, tempgy = 0, tempgz = 0;
    static uint16_t cnt_a = 0; // 使用static修饰的局部变量，表明变量具有静态存储周期，也就是说该函数执行完后不释放内存
    if (cnt_a == 0)
    {
        value.X = 0;
        value.Y = 0;
        value.X = 0;
        tempgx = 0;
        tempgy = 0;
        tempgz = 0;
        cnt_a = 1;
        sensivity = 0;
        offset->X = 0;
        offset->Y = 0;
        offset->Z = 0;
    }
    tempgx += value.X;
    tempgy += value.Y;
    tempgz += value.Z - sensivity; // 加速度计校准 sensivity 等于 MPU6500初始化时设置的灵敏度值（8196LSB/g）;陀螺仪校准 sensivity = 0；
    if (cnt_a == 200)              // 200个数求平均
    {
        offset->X = tempgx / cnt_a;
        offset->Y = tempgy / cnt_a;
        offset->Z = tempgz / cnt_a;
        cnt_a = 0;
        return 1;
    }
    cnt_a++;
    return 0;
}

/**
 * @brief 对MPU9250进行去零偏处理
 *
 */
void MPU6500_DataProcess(void)
{
    /*加速度去零偏AD值*/
    MPU6500_ACC_RAW.X = (((int16_t)MPU6500_buf[0] << 8) | MPU6500_buf[1]) - ACC_OFFSET_RAW.X;
    MPU6500_ACC_RAW.Y = (((int16_t)MPU6500_buf[2] << 8) | MPU6500_buf[3]) - ACC_OFFSET_RAW.Y;
    MPU6500_ACC_RAW.Z = (((int16_t)MPU6500_buf[4] << 8) | MPU6500_buf[5]) - ACC_OFFSET_RAW.Z;
    /*陀螺仪去零偏值*/
    MPU6500_GYRO_RAW.X = (((int16_t)MPU6500_buf[8] << 8) | MPU6500_buf[9]) - GYRO_OFFSET_RAW.X;
    MPU6500_GYRO_RAW.Y = (((int16_t)MPU6500_buf[10] << 8) | MPU6500_buf[11]) - GYRO_OFFSET_RAW.Y;
    MPU6500_GYRO_RAW.Z = (((int16_t)MPU6500_buf[12] << 8) | MPU6500_buf[13]) - GYRO_OFFSET_RAW.Z;
    if (GET_FLAG(GYRO_OFFSET))
    {
        if (MPU6500_OffSet(MPU6500_GYRO_RAW, &GYRO_OFFSET_RAW, 0))
        {
            SENSER_FLAG_RESET(GYRO_OFFSET);
            //            PID_WriteFlash(); // 保存陀螺仪的零偏数据
            Gyro_OffSet_LED();
            SENSER_FLAG_SET(ACC_OFFSET); // 校准加速度
        }
    }
    if (GET_FLAG(ACC_OFFSET))
    {
        if (MPU6500_OffSet(MPU6500_ACC_RAW, &ACC_OFFSET_RAW, 8196))
        {
            SENSER_FLAG_RESET(ACC_OFFSET);
            //            PID_WriteFlash(); // 保存加速度计的零偏数据
            Acc_OffSet_LED();
        }
    }
}

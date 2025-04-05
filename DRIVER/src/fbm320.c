#include "fbm320.h"
#include "iic_analog.h"
#include "delay.h"
#include "structconfig.h"
#include "led.h"
#include <stdio.h>
#include <math.h>
#include "filter.h"

FBMTYPE FBM;
uint8_t ALTIUDE_OK = 0, ALT_Updated = 0;
float RPFilter;

/**
 * @brief 写一个字节到FBM320寄存器
 *
 * @param regaddr 寄存器地址
 * @param data 待写入的数据
 * @return 0：成功 1：失败
 */
uint8_t FBM320_WriteByte(uint8_t regaddr, uint8_t data)
{
    if (IIC_WriteByteToSlave(FBMAddr2, regaddr, data))
        return 1;
    else
        return 0;
}

/**
 * @brief 从FBM320指定寄存器读一字节数据
 *
 * @param regaddr 寄存器地址
 * @param buf 读出数据存放的地址
 * @return 0:成功 1：失败
 */
uint8_t FBM320_ReadByte(uint8_t regaddr, uint8_t *buf)
{
    if (IIC_ReadByetFromSlave(FBMAddr2, regaddr, buf))
        return 1;
    else
        return 0;
}

/**
 * @brief 向FBM320指定寄存器写入指定字节数据
 *
 * @param regaddr 寄存器地址
 * @param length 数据长度
 * @param data 待写入数据指针
 * @return 0：成功 1：失败
 */
uint8_t FBM320_WriteMultByte(uint8_t regaddr, uint8_t length, uint8_t *data)
{
    if (IIC_WriteMultByteToSlave(FBMAddr2, regaddr, length, data))
        return 1;
    else
        return 0;
}

/**
 * @brief 从FBM320指定寄存器读指定字节数据
 *
 * @param regaddr 寄存器地址
 * @param length 数据长度
 * @param buf 读出数据存放地址
 * @return 0：成功 1：失败
 */
uint8_t FBM320_ReadMultByte(uint8_t regaddr, uint8_t length, uint8_t *buf)
{
    if (IIC_ReadMultByteFromSlave(FBMAddr2, regaddr, length, buf))
        return 1;
    else
        return 0;
}

/**
 * @brief 读取FBM320 WHO_AM_I 标识并返回0x68
 *
 * @return 返回读取数据
 */
uint8_t FBM320_getDeviceID(void)
{
    uint8_t buf;
    FBM320_ReadByte(FBM_ID, &buf);
    return buf;
}

/**
 * @brief 检测FBM320是否已连接
 *
 * @return 1：已连接 0：未连接
 */
uint8_t FBM320_testConnectiom(void)
{
    if (FBM320_getDeviceID() == FBMID)
        return 1;
    else
        return 0;
}

/**
 * @brief 检测IIC总线上的FBM320是否存在
 *
 */
void FBM320_Check(void)
{
    while (!FBM320_testConnectiom())
    {
        printf("\rFBM320 no connect...\r\n");
        RGB_LED_Blue(); // 蓝灯常亮
    }
}

/**
 * @brief 初始化FBM320并获取校准数据
 *
 */
void FBM320_Init(void)
{
    FBM320_Check();

    FBM320_WriteByte(FBM_RESET, FBMRESET); // 复位FBM320
    Delay_ms(100);
    FBM320_GetCoeff(); // 获取FBM320的校准数据
}

/**
 * @brief 初始化起飞时的高度数据
 * @note 此函数初始化的其实是起飞时的大气压(FBM.InitPress)，为后面求相对高度做准备
 * @return 0：初始化未完成 1：初始化已完成
 */
uint8_t FBM320_Init_Altitude(void)
{
    static uint8_t cnt_p = 0;
    static int64_t PressNum = 0;
    if (GET_FLAG(BAR_OFFSET))
    {
        if (cnt_p == 0)
        {
            cnt_p = 1;
            PressNum = 0;
        }
        PressNum += FBM.RP; // 100个气压数据累加
        if (cnt_p == 100)
        {
            FBM.InitPress = (float)PressNum / cnt_p; // 求平均值
            SENSER_FLAG_RESET(BAR_OFFSET);           // 校准气压计结束
            cnt_p = 0;
            return 1;
        }
        cnt_p++;
    }
    return 0;
}

/**
 * @brief FBM320气压值转换成相对高度
 * @note 此函数调用频率最好200Hz,否则获取的高度值误差较大！！！
 */
void FBM320_GetAltitude(void)
{
    uint8_t buf[3];
    static uint8_t timecnt = 0;
    float AltitudeFilter;
    switch (timecnt) // 温度和气压数据转换时间不一样
    {
    case 0:
        FBM320_WriteByte(FBM_CONFIG, TEMP_CONVERSION); // 280us
        break;
    case 1: // 5ms转换一次（836us）
        FBM320_ReadMultByte(DATA_MSB, 3, buf);
        FBM.ADTemp = (buf[0] << 16) | (buf[1] << 8) | buf[2];
        FBM320_WriteByte(FBM_CONFIG, PRES_CONVERSION + OSR8192); // 开启气压转换
        break;

    case 5: // 15ms转换一次（834us）
        FBM320_ReadMultByte(DATA_MSB, 3, buf);
        FBM.ADTemp = (buf[0] << 16) | (buf[1] << 8) | buf[2];
        FBM320_WriteByte(FBM_CONFIG, TEMP_CONVERSION); // 开启温度转换
        FBM320_Calculate(FBM.ADPress, FBM.ADTemp);     // 将FBM320的原始数据转换成物理

        if (FBM320_Init_Altitude())
        {
            Bar_OffSet_LED(); // 气压校准成功指示灯
            ALTIUDE_OK = 1;
        }

        SortAver_Filter((float)FBM.RP, &FBM.RPFilter, 12); // 去极值均值滤波

        if (FBM.RPFilter && ALTIUDE_OK)
        {
            ALT_Updated = 1;
            FBM.Altitude = 44330.0f * (1 - powf((float)FBM.RPFilter / FBM.InitPress, 0.190295f));
        }

        SortAver_Filter1(FBM.Altitude, &AltitudeFilter, 12); // 去极值均值滤波
        if (AltitudeFilter)
        {
            Aver_Filter1(AltitudeFilter, &FBM.AltitudeFilter, 8); // 滑动窗口滤波
        }
        break;

    default:
        break;
    }
    timecnt++; // 转换时间计数
    if (timecnt > 5)
    {
        timecnt = 0;
    }
}

/**
 * @brief 获取存储在FBM320片内的校准数据
 *
 */
void FBM320_GetCoeff(void)
{
    uint8_t data[20];
    uint8_t i = 0;
    uint16_t R[10];
    FBM320_ReadMultByte(FBM_COEFF1, 18, data);
    FBM320_ReadMultByte(FBM_COEFF3, 8, &data[18]);
    FBM320_ReadMultByte(FBM_COEFF4, 8, &data[19]);
    for (i = 0; i < 10; i++)
    {
        R[i] = (uint16_t)(data[2 * i] << 8) | data[2 * i + 1];
    }
    /* Use R0~R9 calculate C0~C12 of FBM320-02	*/
    FBM.C0 = R[0] >> 4;
    FBM.C1 = ((R[1] & 0xFF00) >> 5) | (R[2] & 7);
    FBM.C2 = ((R[1] & 0xFF) << 1) | (R[4] & 1);
    FBM.C3 = R[2] >> 3;
    FBM.C4 = ((uint32_t)R[3] << 2) | (R[0] & 3);
    FBM.C5 = R[4] >> 1;
    FBM.C6 = R[5] >> 3;
    FBM.C7 = ((uint32_t)R[6] << 3) | (R[5] & 7);
    FBM.C8 = R[7] >> 3;
    FBM.C9 = R[8] >> 2;
    FBM.C10 = ((R[9] & 0xFF00) >> 6) | (R[8] & 3);
    FBM.C11 = R[9] & 0xFF;
    FBM.C12 = ((R[0] & 0x0C) << 1) | (R[7] & 7);
}

/**
 * @brief 将FBM320获取的AD值转换成物理量
 * @note 气压单位是帕，温度单位是摄氏度
 * @param UP 气压的AD值
 * @param UT 温度的AD值
 */
void FBM320_Calculate(int32_t UP, int32_t UT)
{
    int32_t DT, DT2, X01, X02, X03, X11, X12, X13, X21, X22, X23, X24, X25, X26, X31, X32, CF, PP1, PP2, PP3, PP4;
    DT = ((UT - 8388608) >> 4) + (FBM.C0 << 4);
    X01 = (FBM.C1 + 4459) * DT >> 1;
    X02 = ((((FBM.C2 - 256) * DT) >> 14) * DT) >> 4;
    X03 = (((((FBM.C3 * DT) >> 18) * DT) >> 18) * DT);
    FBM.RT = ((2500 << 15) - X01 - X02 - X03) >> 15;

    DT2 = (X01 + X02 + X03) >> 12;

    X11 = ((FBM.C5 - 4443) * DT2);
    X12 = (((FBM.C6 * DT2) >> 16) * DT2) >> 2;
    X13 = ((X11 + X12) >> 10) + ((FBM.C4 + 120586) << 4);

    X21 = ((FBM.C8 + 7180) * DT2) >> 10;
    X22 = (((FBM.C9 * DT2) >> 17) * DT2) >> 12;
    X23 = (X22 >= X21) ? (X22 - X21) : (X21 - X22);

    X24 = (X23 >> 11) * (FBM.C7 + 166426);
    X25 = ((X23 & 0x7FF) * (FBM.C7 + 166426)) >> 11;
    X26 = (X21 >= X22) ? (((0 - X24 - X25) >> 11) + FBM.C7 + 166426) : (((X24 + X25) >> 11) + FBM.C7 + 166426);

    PP1 = ((UP - 8388608) - X13) >> 3;
    PP2 = (X26 >> 11) * PP1;
    PP3 = ((X26 & 0x7FF) * PP1) >> 11;
    PP4 = (PP2 + PP3) >> 10;

    CF = (2097152 + FBM.C12 * DT2) >> 3;
    X31 = (((CF * FBM.C10) >> 17) * PP4) >> 2;
    X32 = (((((CF * FBM.C11) >> 15) * PP4) >> 18) * PP4);
    FBM.RP = ((X31 + X32) >> 15) + PP4 + 99880; // 99880气压补偿
}

/**
 * @brief 获取绝对高度
 *
 * @param Press 气压值
 * @return 绝对高度值 单位：mm
 */
int32_t ABS_Altitude(int32_t Press)
{
    int8_t P0;
    int16_t hs1, dP0;
    int32_t h0, hs0, HP1, HP2;

    if (Press >= 103000)
    {
        P0 = 103;
        h0 = -138507;
        hs0 = -21007;
        hs1 = 311;
    }
    else if (Press >= 98000)
    {
        P0 = 98;
        h0 = 280531;
        hs0 = -21869;
        hs1 = 338;
    }
    else if (Press >= 93000)
    {
        P0 = 93;
        h0 = 717253;
        hs0 = -22813;
        hs1 = 370;
    }
    else if (Press >= 88000)
    {
        P0 = 88;
        h0 = 1173421;
        hs0 = -23854;
        hs1 = 407;
    }
    else if (Press >= 83000)
    {
        P0 = 83;
        h0 = 1651084;
        hs0 = -25077;
        hs1 = 450;
    }
    else if (Press >= 78000)
    {
        P0 = 78;
        h0 = 2152645;
        hs0 = -27735;
        hs1 = 501;
    }
    else if (Press >= 73000)
    {
        P0 = 73;
        h0 = 2680954;
        hs0 = -27735;
        hs1 = 560;
    }
    else if (Press >= 68000)
    {
        P0 = 68;
        h0 = 3239426;
        hs0 = -29366;
        hs1 = 632;
    }
    else if (Press >= 63000)
    {
        P0 = 63;
        h0 = 3832204;
        hs0 = -31229;
        hs1 = 719;
    }
    else if (Press >= 58000)
    {
        P0 = 58;
        h0 = 4464387;
        hs0 = -33377;
        hs1 = 826;
    }
    else if (Press >= 53000)
    {
        P0 = 53;
        h0 = 5142359;
        hs0 = -35885;
        hs1 = 960;
    }
    else if (Press >= 48000)
    {
        P0 = 48;
        h0 = 5874268;
        hs0 = -38855;
        hs1 = 1131;
    }
    else if (Press >= 43000)
    {
        P0 = 43;
        h0 = 6670762;
        hs0 = -42434;
        hs1 = 1354;
    }
    else if (Press >= 38000)
    {
        P0 = 38;
        h0 = 7546157;
        hs0 = -46841;
        hs1 = 1654;
    }
    else if (Press >= 33000)
    {
        P0 = 33;
        h0 = 8520395;
        hs0 = -52412;
        hs1 = 2072;
    }
    else
    {
        P0 = 28;
        h0 = 9622536;
        hs0 = -59704;
        hs1 = 2682;
    }

    dP0 = Press - P0 * 1000;
    HP1 = (hs0 * dP0) >> 2;
    HP2 = (((hs1 * dP0) >> 10) * dP0) >> 4;

    return ((h0 << 6) + HP1 + HP2) >> 6; // Return absolute altitude 返回绝度高度
}

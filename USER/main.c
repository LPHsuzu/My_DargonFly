#include "main.h"

int main(void)
{
    NVIC_Config();
    LED_Init();
    Delay_Init();
    usart1_init(460800);
    usart2_init(921600);
    IIC_Init();
    TIM4_Init();
    exti_init();
    NRF24L01_Init(); // NRF初始化（红）
    MPU6500_Init();  // MPU9250初始化（绿）
    spl06_init();    // SPL06初始化(气压计蓝)
    // FBM320_Init();   // FBM320初始化(气压计蓝)
    MOTOR_Init();
    BATT_Init();
    WiFi_Switch(DISABLE);
    // OpenMV_Switch(DISABLE); // OpenMV模块开关
    PID_ReadFlash();     // Flash中的数据读取
    PidParameter_init(); // PID初始化
    RGB_LED_Off();

    while (1)
    {
        if (ANO_Scan) // 500Hz
        {
            ANO_Scan = 0;
            ANO_DT_Data_Exchange(); // 更新数据到上位机
        }
        if (IMU_Scan) // 100Hz
        {
            IMU_Scan = 0;
            Prepare_Data();                                              // 获取姿态解算所需数据
            IMUupdate(&Gyr_rad, &Acc_filt, &Att_Angle);                  // 四元数姿态解算
            Control(&Att_Angle, &Gyr_rad, &RC_Control, Airplane_Enable); // 姿态控制
            RunTimer_Test();

            spl06_update();
            altitude_get();
        }
        if (LED_Scan) // 10Hz
        {
            LED_Scan = 0;
            LED_Run();
            if (!Airplane_Enable && Run_Flag && !WIFI_LEDFlag)
            {
                RGB_LED_Runing(); // 飞机上锁状态灯
            }
            WiFi_OFFON_LED(); // WiFi开关状态灯
            BATT_Alarm_LED(); // 电池低电压报警
        }
        if (IRQ_Scan) // 5Hz
        {
            IRQ_Scan = 0;
            NRF_SingalCheck(); // NRF通信检测
            SendToRemote();    // 发送数据给遥控器
        }
        if (Batt_Scan) // 2.5Hz
        {
            Batt_Scan = 0;
            NRF_GetAddr(); // 分配NRF地址
            LowVoltage_Alarm();
        }
    }
}

#include "..\CMSIS\stm32f4xx.h" // Device header
#include "led.h"
#include "delay.h"
#include "structconfig.h"
#include <stdlib.h>

#define RGB_SET1 GPIOB->BSRRL |= 1 << 9;
#define RGB_SET0 GPIOB->BSRRH |= 1 << 9;

/*跑马灯RGB三元色配出七彩跑马灯*/
static u32 Run_buf[][16] =
	{
		{
			0xFFA500,
			0,
			0,
			0,
			0xFFA500,
			0,
			0,
			0,
			0xFFA500,
			0,
			0,
			0,
			0xFFA500,
			0,
			0,
			0,
		}, // 橙色
		{
			0x00FF00,
			0,
			0,
			0,
			0x00FF00,
			0,
			0,
			0,
			0x00FF00,
			0,
			0,
			0,
			0x00FF00,
			0,
			0,
			0,
		}, // 绿色
		{
			0xFF00FF,
			0,
			0,
			0,
			0xFF00FF,
			0,
			0,
			0,
			0xFF00FF,
			0,
			0,
			0,
			0xFF00FF,
			0,
			0,
			0,
		}, // 紫色
		{
			0x00FFFF,
			0,
			0,
			0,
			0x00FFFF,
			0,
			0,
			0,
			0x00FFFF,
			0,
			0,
			0,
			0x00FFFF,
			0,
			0,
			0,
		}, // 青色
		{
			0x0000FF,
			0,
			0,
			0,
			0x0000FF,
			0,
			0,
			0,
			0x0000FF,
			0,
			0,
			0,
			0x0000FF,
			0,
			0,
			0,
		}, // 蓝色
		{
			0xFFFF00,
			0,
			0,
			0,
			0xFFFF00,
			0,
			0,
			0,
			0xFFFF00,
			0,
			0,
			0,
			0xFFFF00,
			0,
			0,
			0,
		}, // 黄色
		{
			0xFFFFFF,
			0,
			0,
			0,
			0xFFFFFF,
			0,
			0,
			0,
			0xFFFFFF,
			0,
			0,
			0,
			0xFFFFFF,
			0,
			0,
			0,
		}, // 白色

};

uint8_t Run_flag = 1; // 跑马灯标志

/**
 * @brief 初始化LED控制引脚
 * @param 无
 * @retval 无
 */
void LED_Init(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_8);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOC, GPIO_Pin_15);
}

/**
 * @brief 指示MCU是否工作
 * @param 无
 * @retval 无
 */
void LED5_Run(void)
{
	static uint8_t flag = 1;
	if (flag)
	{
		flag = 0;
		GPIO_SetBits(GPIOB, GPIO_Pin_8);
	}
	else
	{
		flag = 1;
		GPIO_ResetBits(GPIOB, GPIO_Pin_8);
	}
}

void RunTimer_Test(void)
{
	static uint8_t flag = 1;
	if (flag)
	{
		flag = 0;
		GPIO_SetBits(GPIOC, GPIO_Pin_15);
	}
	else
	{
		flag = 1;
		GPIO_ResetBits(GPIOC, GPIO_Pin_15);
	}
}

void RunTimer_Test2(void)
{
	static uint8_t flag = 1;
	if (flag)
	{
		flag = 0;
		GPIO_SetBits(GPIOC, GPIO_Pin_14);
	}
	else
	{
		flag = 1;
		GPIO_ResetBits(GPIOC, GPIO_Pin_14);
	}
}

/**
 * @brief 对RGB跑马灯写0
 * @note 不同的系统时钟频率下需要微调（默认HCLK=100MHz）
 * @param 无
 * @retval 无
 */
void RGB_Write0(void)
{
	uint8_t cnt1 = 2, cnt2 = 6;
	RGB_SET1;
	while (cnt1--)
		__nop();
	RGB_SET0;
	while (cnt2--)
		__nop();
}

/**
 * @brief 对RGB跑马灯写1
 * @note 不同的系统时钟频率下需要微调（默认HCLK=100MHz）
 * @param 无
 * @retval 无
 */
void RGB_Write1(void)
{
	uint8_t cnt1 = 6, cnt2 = 6;
	RGB_SET1;
	while (cnt1--)
		__nop();
	__nop();
	RGB_SET0;
	while (cnt2--)
		__nop();
	__nop();
}

/**
 * @brief RGB灯复位
 * @param 无
 * @retval 无
 */
void RGB_Reset(void)
{
	uint16_t cnt1 = 600, cnt2 = 1;
	RGB_SET0;
	while (cnt1--)
		__nop();
	RGB_SET1;
	while (cnt2--)
		__nop();
}

/**
 * @brief 向RGB灯传输一个字节数据
 * @param data 要传输的数据
 * @retval 无
 */
void RGB_WriteByte(uint8_t data)
{
	uint8_t i;
	for (i = 0; i < 8; i++)
	{
		if ((data << i) & 0x80) /*高位写入*/
		{
			RGB_Write1();
		}
		else
		{
			RGB_Write0();
		}
	}
}

/**
 * @brief 设置一个灯的色彩
 * @param red 红光所占比例大小,范围0~255
 * @param green 绿光所占比例大小,范围0~255
 * @param blue 蓝光所占比例大小,范围0~255
 * @retval 无
 */
void RGB_SetColour(uint8_t red, uint8_t green, uint8_t blue)
{
	RGB_WriteByte(green);
	RGB_WriteByte(red);
	RGB_WriteByte(blue);
}

/**
 * @brief RGB灯随机变换颜色
 * @param 无
 * @retval 无
 */
void RGB_LED_Rand(void)
{
	uint8_t i;
	uint8_t red = 0, green = 0, blue = 0;
	for (i = 0; i < 4; i++)
	{
		green = rand() % 18 + 2; /*产生一个0~20的随机数*/
		red = rand() % 18 + 2;
		blue = rand() % 18 + 2;
		RGB_SetColour(red, green, blue);
	}
}

/**
 * @brief RGB跑马灯
 * @param 无
 * @retval 无
 */
void RGB_Running(void)
{
	uint8_t i;
	uint8_t red = 0, green = 0, blue = 0;
	static uint8_t cnt = 0, wcnt = 0, times = 0;
	if (times++ >= 16)
	{
		times = 0;
		wcnt++;
	}
	for (i = 0; i < 4; i++)
	{
		if (cnt > 4)
		{
			cnt = 0;
		}
		red = ((Run_buf[wcnt][cnt] >> 16) & 0xFF);
		green = ((Run_buf[wcnt][cnt] >> 8) & 0xFF);
		blue = ((Run_buf[wcnt][cnt] >> 0) & 0xFF);
		RGB_SetColour(red, green, blue); // 合成颜色
		cnt++;
	}
	if (wcnt == 7)
		wcnt = 0;
	// RGB_Reset(); 	// 复位显示
	// Delay_ms(200);
}

/**
 * @brief 红灯
 * @param 无
 * @retval 无
 */
void RGB_LED_Red(void)
{
	uint8_t i;
	for (i = 0; i < 4; i++)
	{
		RGB_SetColour(0xFF, 0, 0);
	}
}

/**
 * @brief 橙灯
 * @param 无
 * @retval 无
 */
void RGB_LED_Orange(void)
{
	uint8_t i;
	for (i = 0; i < 4; i++)
	{
		RGB_SetColour(0xFF, 0xA5, 0);
	}
}
/**
 * @brief 黄灯
 * @param 无
 * @retval 无
 */
void RGB_LED_Yellow(void)
{
	uint8_t i;
	for (i = 0; i < 4; i++)
	{
		RGB_SetColour(0xFF, 0xFF, 0);
	}
}

/**
 * @brief 绿灯
 * @param 无
 * @retval 无
 */
void RGB_LED_Green(void)
{
	uint8_t i;
	for (i = 0; i < 4; i++)
	{
		RGB_SetColour(0, 0xFF, 0);
	}
}

/**
 * @brief 青灯
 * @param 无
 * @retval 无
 */
void RGB_LED_Cyan(void)
{
	uint8_t i;
	for (i = 0; i < 4; i++)
	{
		RGB_SetColour(0, 0xFF, 0xFF);
	}
}

/**
 * @brief 蓝灯
 * @param 无
 * @retval 无
 */
void RGB_LED_Blue(void)
{
	uint8_t i;
	for (i = 0; i < 4; i++)
	{
		RGB_SetColour(0, 0, 0xFF);
	}
}

/**
 * @brief 紫灯
 * @param 无
 * @retval 无
 */
void RGB_LED_Violet(void)
{
	uint8_t i;
	for (i = 0; i < 4; i++)
	{
		RGB_SetColour(0xCD, 0, 0xCD);
	}
}

/**
 * @brief 两红两绿
 * @param 无
 * @retval 无
 */
void RGB_LED_Fly(void)
{
	uint8_t i;
	for (i = 0; i < 4; i++)
	{
		if (i < 2)
			RGB_SetColour(0, 0xFF, 0);
		else
			RGB_SetColour(0xFF, 0, 0);
	}
}

/**
 * @brief 白灯
 * @param 无
 * @retval 无
 */
void RGB_LED_White(void)
{
	uint8_t i;
	for (i = 0; i < 4; i++)
	{
		RGB_SetColour(0x0F, 0x0F, 0x0F);
	}
}

/**
 * @brief 关灯
 * @param 无
 * @retval 无
 */
void RGB_LED_Off(void)
{
	uint8_t i;
	for (i = 0; i < 4; i++)
	{
		RGB_SetColour(0, 0, 0);
	}
}

/**
 * @brief 陀螺仪校准完成闪烁蓝灯
 * @param 无
 * @retval 无
 */
void Gyro_OffSet_LED(void)
{
	RGB_LED_Off();
	RGB_LED_Blue();
	Delay_ms(100);
	RGB_LED_Off();
	Delay_ms(100);
	RGB_LED_Blue();
	Delay_ms(100);
	RGB_LED_Off();
	Delay_ms(100);
	RGB_LED_Blue();
	Delay_ms(100);
	RGB_LED_Off();
}

/**
 * @brief 加速度计校准完成闪烁绿灯
 * @param 无
 * @retval 无
 */
void Acc_OffSet_LED(void)
{
	RGB_LED_Off();
	RGB_LED_Green();
	Delay_ms(100);
	RGB_LED_Off();
	Delay_ms(100);
	RGB_LED_Green();
	Delay_ms(100);
	RGB_LED_Off();
	Delay_ms(100);
	RGB_LED_Green();
	Delay_ms(100);
	RGB_LED_Off();
}

/**
 * @brief 气压计校准完成闪烁紫灯
 * @param 无
 * @retval 无
 */
void Bar_OffSet_LED(void)
{
	RGB_LED_Off();
	RGB_LED_Violet();
	Delay_ms(100);
	RGB_LED_Off();
	Delay_ms(100);
	RGB_LED_Violet();
	Delay_ms(100);
	RGB_LED_Off();
	Delay_ms(100);
	RGB_LED_Violet();
	Delay_ms(100);
	RGB_LED_Off();
}

/**
 * @brief 闪烁青色灯提示WiFi开关
 * @param 无
 * @retval 无
 */
void WiFi_OFFON_LED(void)
{
	static uint8_t cnt = 0, flag = 0;
	if (WIFI_LEDFlag == 1) // WiFi开启指示灯
	{
		if (cnt++ > 6)
		{
			cnt = 0;
			WIFI_LEDFlag = 0;
			Run_flag = 1; // 打开运行灯
			RGB_LED_Off();
		}
		else
		{
			if (flag)
			{
				flag = 0;
				RGB_LED_Cyan();
			}
			else
			{
				flag = 1;
				RGB_LED_Off();
			}
		}
	}
	else if (WIFI_LEDFlag == 2) // WiFi关闭指示灯
	{
		if (cnt++ > 6)
		{
			cnt = 0;
			WIFI_LEDFlag = 0;
			Run_flag = 1; // 打开运行灯
			RGB_LED_Off();
		}
		else
		{
			if (flag)
			{
				flag = 0;
				RGB_LED_Red();
			}
			else
			{
				flag = 1;
				RGB_LED_Off();
			}
		}
	}
}

/**
 * @brief 低电量红灯快闪
 * @param 无
 * @retval 无
 */
void BATT_Alarm_LED(void)
{
	static uint8_t flag = 0;
	if (BATT_LEDFlag)
	{
		if (flag)
		{
			flag = 0;
			RGB_LED_Red();
		}
		else
		{
			flag = 1;
			RGB_LED_Off();
		}
	}
}

/**
 * @brief 飞机解锁指示彩色灯
 * @param N 彩灯变换频率
 * @param flag 使能变换标志
 * @retval 无
 */
void RGB_Unlock(uint8_t N, uint8_t flag)
{
	static uint8_t cnt = 0;
	if (flag && cnt++ > N)
	{
		cnt = 0;
		RGB_LED_Rand();
	}
}

void OneNET_LED(uint8_t color[], uint8_t num)
{
	uint8_t i;
	for (i = 0; i < num; i++)
	{
		RGB_SetColour(color[0], color[1], color[2]);
	}
}

#ifndef __LED_H
#define __LED_H

void LED_Init(void);
void LED5_Run(void);
void RunTimer_Test(void);

void RGB_Write0(void);
void RGB_Write1(void);
void RGB_Reset(void);

void RGB_WriteByte(uint8_t data);
void RGB_SetColour(uint8_t red, uint8_t green, uint8_t blue);

void RGB_LED_Rand(void);
void RGB_Running(void);

void RGB_LED_Red(void);
void RGB_LED_Orange(void);

void RGB_LED_Yellow(void);
void RGB_LED_Green(void);
void RGB_LED_Cyan(void);
void RGB_LED_Blue(void);
void RGB_LED_Violet(void);
void RGB_LED_White(void);
void RGB_LED_Fly(void);
void RGB_LED_Off(void);

void Gyro_OffSet_LED(void);
void Acc_OffSet_LED(void);
void Bar_OffSet_LED(void);
void WiFi_OFFON_LED(void);
void BATT_Alarm_LED(void);
void RGB_Unlock(uint8_t N, uint8_t flag);
void OneNET_LED(uint8_t color[], uint8_t num);

#endif

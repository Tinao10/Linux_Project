#ifndef __DHT11_H__
#define __DHT11_H__

#include "stm32f4xx.h"

extern char TEMP_DATA[5];
extern char HUME_DATA[5];
extern int Temp_Threshold; // 温度的阈值 超过报警
extern int Hume_Threshold; // 湿度的阈值 超过报警

#define DHT11_PIN GPIO_Pin_3
#define DHT11_GROUP GPIOD

// 函数声明
void DHT11_Show(void);

#endif




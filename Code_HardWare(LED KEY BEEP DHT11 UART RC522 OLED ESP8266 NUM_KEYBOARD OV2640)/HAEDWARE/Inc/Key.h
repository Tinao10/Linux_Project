#ifndef __EXTI_KEY_H__
#define __EXTI_KEY_H__

#include "stm32f4xx.h"
#include "Pwm.h"

// 设置 Key 的引脚
#define KEY_GROUP GPIOF

#define KEY1_PIN GPIO_Pin_9 // KEY0
#define KEY2_PIN GPIO_Pin_8 // KEY1
#define KEY3_PIN GPIO_Pin_7 // KEY2
#define KEY4_PIN GPIO_Pin_6 // KEY3

// 读取 Key 的状态
#define KEY1_STATUS GPIO_ReadInputDataBit(KEY_GROUP, KEY1_PIN)
#define KEY2_STATUS GPIO_ReadInputDataBit(KEY_GROUP, KEY2_PIN)
#define KEY3_STATUS GPIO_ReadInputDataBit(KEY_GROUP, KEY3_PIN)
#define KEY4_STATUS GPIO_ReadInputDataBit(KEY_GROUP, KEY4_PIN)

// 声明函数
void Key_Init(void);

#endif

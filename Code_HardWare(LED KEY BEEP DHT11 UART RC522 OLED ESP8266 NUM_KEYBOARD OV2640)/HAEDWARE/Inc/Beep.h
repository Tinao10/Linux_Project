#ifndef __BEEP_H__
#define __BEEP_H__

#include "stm32f4xx.h"

// Beep 引脚定义
#define Beep_Group GPIOG
#define Beep_Pin GPIO_Pin_7

/*蜂鸣器的状态枚举*/
enum BEEP_STATUS
{
    BEEP_ON,
    BEEP_OFF
};

// 函数声明
void Beep_Init(void);
void Beep_Control(int Status);

#endif

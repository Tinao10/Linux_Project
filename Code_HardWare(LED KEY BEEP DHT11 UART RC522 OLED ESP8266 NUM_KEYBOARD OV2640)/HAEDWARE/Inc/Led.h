#ifndef __LED_H__
#define __LED_H__

#include "stm32f4xx.h"
#include "stdbool.h"

// LED对应的 GPIO引脚 的信息
#define LED1_PIN GPIO_Pin_3 // LED0
#define LED2_PIN GPIO_Pin_4 // LED1
#define LED3_PIN GPIO_Pin_9 // LED2

#define LED1_2_GROUP GPIOE // LED0 和 LED1
#define LED3_GROUP GPIOG   // LED2

/*灯的编号枚举*/
enum LED_NUM
{
  LED1,
  LED2,
  LED3
};

/*灯的状态枚举*/
enum LED_STATUS
{
  LED_ON,
  LED_OFF
};

/* 灯展示形式
  正常采集数据，LED1 呼吸灯 LED2~LED4 流水灯
  数据超过阈值，LED1 呼吸灯 LED2~LED4 闪烁
*/
#define Flow_Lights true
#define Flash_Lights false

// 灯的控制 直接宏定义
#define LED1_ON GPIO_ResetBits(LED1_2_GROUP, LED1_PIN)
#define LED1_OFF GPIO_SetBits(LED1_2_GROUP, LED1_PIN)
#define LED2_ON GPIO_ResetBits(LED1_2_GROUP, LED2_PIN)
#define LED2_OFF GPIO_SetBits(LED1_2_GROUP, LED2_PIN)
#define LED3_ON GPIO_ResetBits(LED3_GROUP, LED3_PIN)
#define LED3_OFF GPIO_SetBits(LED3_GROUP, LED3_PIN)

// 声明函数
void Led_Init(void);
void Led_Control(int Led_Num, int Status);
void Flow_Light_Flash_Light(void);

#endif

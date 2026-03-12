#include "Pwm.h"
#include "Led.h"

int Threshold = 100;

void Pwm_Led_Init(void)
{
    GPIO_InitTypeDef Led9_GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM14_TimeBaseInitStruct;
    TIM_OCInitTypeDef TIM14_OCInitStruct;

    // 1. GPIO使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    // 2.初始化GPIO
    Led9_GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; // 复用
    Led9_GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    Led9_GPIO_InitStruct.GPIO_Pin = LED1_PIN;
    Led9_GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    Led9_GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOF, &Led9_GPIO_InitStruct);

    // 配置GPIO复用
    GPIO_PinAFConfig(GPIOF, GPIO_PinSource9, GPIO_AF_TIM14);

    // 3.配置时基使能
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);

    TIM14_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM14_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM14_TimeBaseInitStruct.TIM_Period = 99;
    TIM14_TimeBaseInitStruct.TIM_Prescaler = 83;
    TIM_TimeBaseInit(TIM14, &TIM14_TimeBaseInitStruct);

    // 4. 配置输出比较单元
    TIM14_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1; // 设置模式
    TIM14_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM14_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM14_OCInitStruct.TIM_Pulse = Threshold;
    TIM_OC1Init(TIM14, &TIM14_OCInitStruct);
    // 5.定时器开始计数
    TIM_Cmd(TIM14, ENABLE);
}

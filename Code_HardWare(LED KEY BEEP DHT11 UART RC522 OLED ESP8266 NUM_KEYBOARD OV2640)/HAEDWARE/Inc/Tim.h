#ifndef __TIME_H__
#define __TIME_H__

#include "stm32f4xx.h"

// 函数声明
void Tim14_Init(uint16_t TIM_Prescaler, uint32_t TIM_Period);
void Tim13_Init(uint16_t TIM_Prescaler, uint32_t TIM_Period);
void Tim12_Init(uint16_t TIM_Prescaler, uint32_t TIM_Period);
void Fuc_TIM14_IRQ_Disable(void);
void Fuc_TIM14_IRQ_Enable(void);

#endif

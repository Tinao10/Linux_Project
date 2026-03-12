#ifndef __ADC_H__
#define __ADC_H__

#include "stm32f4xx.h"

extern char LIGHT_DATA[5];
extern int Light_Threshold;

// 函数声明
void Adc_Init(void);
void ADC_Show(void);

#endif

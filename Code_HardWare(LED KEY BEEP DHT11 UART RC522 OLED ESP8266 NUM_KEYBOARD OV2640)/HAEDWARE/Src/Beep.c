#include "Beep.h"
#include <stdio.h>

/*********************************************************************************
 * @description: 蜂鸣器初始化
 * @Author: Tinao
 * @Date: 2024-11-27 14:35:09
 * @return {*} None
 *********************************************************************************/
void Beep_Init(void)
{
    GPIO_InitTypeDef Beep_GPIO_InitStruct;
    // 1. 使能 Beep
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

    // 2. 初始化
    Beep_GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    Beep_GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; // 输出推挽
    Beep_GPIO_InitStruct.GPIO_Pin = Beep_Pin;
    Beep_GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
    Beep_GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(Beep_Group, &Beep_GPIO_InitStruct);

    // 默认蜂鸣器不响
    GPIO_ResetBits(Beep_Group, Beep_Pin);

    printf("BEEP Initialized.\n");
}

/*********************************************************************************
 * @description:  蜂鸣器控制函数
 * @Author: Tinao
 * @Date: 2024-11-27 14:35:22
 * @param {int} Status ：蜂鸣器状态 BEEP_ON BEEP_OFF
 * @return {*} None
 *********************************************************************************/
void Beep_Control(int Status)
{
    if (Status == BEEP_ON)
    {
        GPIO_SetBits(Beep_Group, Beep_Pin);
    }
    if (Status == BEEP_OFF)
    {
        GPIO_ResetBits(Beep_Group, Beep_Pin);
    }
}

#include "Key.h"
#include "Led.h"
#include "Beep.h"
#include "Delay.h"
#include "Usart.h"
#include <stdio.h>
#include <string.h>

/*********************************************************************************
 * @description: 按键初始化函数
 * @Author: Tinao
 * @Date: 2024-11-27 15:41:26
 * @return {*} None
 *********************************************************************************/
void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    /* 1、 GPIO控制器配置为输入模式 */
    // 1.使能GPIOA和GPIOE组时钟
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    // 2.GPIO初始化
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN; // 输入模式
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = KEY1_PIN | KEY2_PIN | KEY3_PIN | KEY4_PIN; // S1 S2 S3 S4
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(KEY_GROUP, &GPIO_InitStruct);

    /* 2、SYSCFG选择器 */
    // 1.使能SYSCFG时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // 2.配置SYSCFG
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource6);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource7);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource8);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource9);

    // 3. EXTI外部终端控制器配置
    // 由于这个时钟使能是默认打开的，所以不用设置时钟使能
    EXTI_InitStruct.EXTI_Line = EXTI_Line6 | EXTI_Line7 | EXTI_Line8 | EXTI_Line9;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStruct);

    // 4. 配置NVIC
    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;        // 外部中断5-9  刚好 PF6-9 在 EXTI9_5_IRQn 里面
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级0
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;        // 子优先级2
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;           // 使能外部中断通道
    NVIC_Init(&NVIC_InitStruct);                           // 配置

    printf("KEY Initialized.\n");
}

// 外部中断线5-9服务程序
void EXTI9_5_IRQHandler(void)
{
    Delay_ms(10); // 消抖

    if (!GPIO_ReadInputDataBit(KEY_GROUP, KEY1_PIN)) // KEY1
    {
        Beep_Control(BEEP_ON);
        EXTI_ClearITPendingBit(EXTI_Line9); // 清除LINE9上的中断标志位
    }
    else
    {
        EXTI_ClearITPendingBit(EXTI_Line9); // 确保在未按下时清除标志
    }

    if (!GPIO_ReadInputDataBit(KEY_GROUP, KEY2_PIN)) // KEY2
    {
        Beep_Control(BEEP_OFF);
        EXTI_ClearITPendingBit(EXTI_Line8); // 清除LINE8上的中断标志位
    }
    else
    {
        EXTI_ClearITPendingBit(EXTI_Line8); // 确保在未按下时清除标志
    }

    if (!GPIO_ReadInputDataBit(KEY_GROUP, KEY3_PIN)) // KEY3
    {
        EXTI_ClearITPendingBit(EXTI_Line7); // 清除LINE7上的中断标志位
    }
    else
    {
        EXTI_ClearITPendingBit(EXTI_Line7); // 确保在未按下时清除标志
    }

    if (!GPIO_ReadInputDataBit(KEY_GROUP, KEY4_PIN)) // KEY4
    {
        EXTI_ClearITPendingBit(EXTI_Line6); // 清除LINE6上的中断标志位
    }
    else
    {
        EXTI_ClearITPendingBit(EXTI_Line6); // 确保在未按下时清除标志
    }
}

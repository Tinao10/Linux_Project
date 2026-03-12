#include "Tim.h"
#include "Pwm.h"
#include "Led.h"
#include "DHT11.h"
#include "Delay.h"
#include "Oled.h"
#include "Esp8266.h"
#include "RC522.h"
#include "Beep.h"
#include "Num_Keyboard.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void Tim14_Init(uint16_t TIM_Prescaler, uint32_t TIM_Period)
{
    TIM_TimeBaseInitTypeDef TIM14_TimeBaseInitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    // 1. 使能定时器始终 使用 TIM14 使能是 APB1
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);

    // 2. 时基初始化
    TIM14_TimeBaseInitStruct.TIM_Prescaler = TIM_Prescaler;        // 设置计数频率
    TIM14_TimeBaseInitStruct.TIM_Period = TIM_Period;              // 设置自动重载值 最大值
    TIM14_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up; // 设置计数模式
    TIM14_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;     // 输入捕获功能

    TIM_TimeBaseInit(TIM14, &TIM14_TimeBaseInitStruct);

    // 3. 使能中断
    TIM_ITConfig(TIM14, TIM_IT_Update, ENABLE);

    // 4. NVIC初始化
    NVIC_InitStruct.NVIC_IRQChannel = TIM8_TRG_COM_TIM14_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // 5. 开启定时器
    TIM_Cmd(TIM14, ENABLE);

    printf("TIM14 Initialized.\n");
}

/* 关闭TIM14中断（保持定时器运行） */
void Fuc_TIM14_IRQ_Disable(void)
{
    TIM_ITConfig(TIM14, TIM_IT_Update, DISABLE);  // 关闭更新中断
    TIM_ClearITPendingBit(TIM14, TIM_IT_Update);   // 清除可能挂起的中断标志
}

/* 重新使能TIM14中断 */
void Fuc_TIM14_IRQ_Enable(void)
{
    TIM_ClearITPendingBit(TIM14, TIM_IT_Update);   // 先清除残留标志
    TIM_ITConfig(TIM14, TIM_IT_Update, ENABLE);    // 重新使能中断
}

void TIM8_TRG_COM_TIM14_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM14, TIM_IT_Update) == SET)
    {
        if (!MFRC522_Request(PICC_REQIDL, (uint8_t *)RawCardID)) // 寻卡
        {
            if (!MFRC522_Anticoll((uint8_t *)RawCardID)) // 获得卡序列号
            {
                char IDStr[20];
                sprintf(IDStr, "%02X%02X%02X%02X", RawCardID[0], RawCardID[1], RawCardID[2], RawCardID[3]);
                Update_Data_Access(IDStr); // 更新序列号
								char Temp[40];
								sprintf(Temp, "%s:%s", "CARD", IDStr); // 整合需要发送的数据到服务器
								printf("%s\n",Temp);
								ESP8266_SendData(Temp, strlen(Temp));    // 发送数据
								Beep_Control(BEEP_ON);
								Delay_ms(500);
								Beep_Control(BEEP_OFF);
            }
        }
        Keyboard_Scan();
    }
    // 清除
    TIM_ClearITPendingBit(TIM14, TIM_IT_Update);
}

/*********************************************************************************
 * @description: TIM13初始化函数 TIM13实现 呼吸灯 10ms触发一次中断，更新值
 * TIM_Prescaler = 8399
 * TIM_Period = 99
 * @Author: Tinao
 * @Date: 2024-11-27 15:43:15
 * @param {uint16_t} TIM_Prescaler ：预分频系数
 * @param {uint32_t} TIM_Period ： 自动重载值
 * @return {*}
 *********************************************************************************/
void Tim13_Init(uint16_t TIM_Prescaler, uint32_t TIM_Period)
{
    TIM_TimeBaseInitTypeDef TIM13_TimeBaseInitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    // 1. 使能定时器始终 使用 TIM13 使能是 APB1
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);

    // 2. 时基初始化
    TIM13_TimeBaseInitStruct.TIM_Prescaler = TIM_Prescaler;        // 设置计数频率
    TIM13_TimeBaseInitStruct.TIM_Period = TIM_Period;              // 设置自动重载值 最大值
    TIM13_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up; // 设置计数模式
    TIM13_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;     // 输入捕获功能

    TIM_TimeBaseInit(TIM13, &TIM13_TimeBaseInitStruct);

    // 3. 使能中断
    TIM_ITConfig(TIM13, TIM_IT_Update, ENABLE);

    // 4. NVIC初始化
    NVIC_InitStruct.NVIC_IRQChannel = TIM8_UP_TIM13_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    // 5. 开启定时器
    TIM_Cmd(TIM13, ENABLE);
}

/*********************************************************************************
 * @description: TIM13的中断服务函数
 * @Author: Tinao
 * @Date: 2024-11-27 15:44:18
 * @return {*} None
 *********************************************************************************/
void TIM8_UP_TIM13_IRQHandler(void)
{
    // 中断
    if (TIM_GetITStatus(TIM13, TIM_IT_Update) == SET)
    {
        Flow_Light_Flash_Light(); // 流水灯 或 闪烁灯
        Threshold -= 10;
        if (Threshold <= 0)
        {
            Threshold = 100;
        }
        TIM_SetCompare1(TIM14, Threshold);
    }
    // 清除
    TIM_ClearITPendingBit(TIM13, TIM_IT_Update);
}

/*********************************************************************************
 * @description: TIM12的初始化函数 实现OLED显示屏幕
 * @Author: Tinao
 * @Date: 2024-11-29 19:25:18
 * @param {uint16_t} TIM_Prescaler : 预分频系数
 * @param {uint32_t} TIM_Period ： 自动装载值
 * @return {*} None
 *********************************************************************************/
void Tim12_Init(uint16_t TIM_Prescaler, uint32_t TIM_Period)
{
    TIM_TimeBaseInitTypeDef TIM12_TimeBaseInitStruct;
    NVIC_InitTypeDef TIM12_NVIC_InitStruct;
    // 使能化定时器
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM12, ENABLE);

    // 初始化时基
    TIM12_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM12_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM12_TimeBaseInitStruct.TIM_Period = TIM_Period;       // 自动装载值
    TIM12_TimeBaseInitStruct.TIM_Prescaler = TIM_Prescaler; // 预分频器
    TIM_TimeBaseInit(TIM12, &TIM12_TimeBaseInitStruct);

    // 使能中断
    TIM_ITConfig(TIM12, TIM_IT_Update, ENABLE);

    // NVIC初始化
    TIM12_NVIC_InitStruct.NVIC_IRQChannel = TIM8_BRK_TIM12_IRQn;
    TIM12_NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    TIM12_NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
    TIM12_NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&TIM12_NVIC_InitStruct);

    // 开启定时器材
    TIM_Cmd(TIM12, ENABLE);

    printf("TIM12 Initialized.\n");
}

/*********************************************************************************
 * @description: TIM12的中断函数
 * @Author: Tinao
 * @Date: 2024-11-29 19:26:51
 * @return {*} None
 *********************************************************************************/
void TIM8_BRK_TIM12_IRQHandler(void)
{
    //char Temp_Data[50] = "123";
    //    int Temp = 0, Hume = 0;
    // 中断
    if (TIM_GetITStatus(TIM12, TIM_IT_Update) == SET)
    {
        DHT11_Show(); // 采集温湿度

        //        sscanf(TEMP_DATA, "%d", &Temp);   // 去温度整数
        //        sscanf(HUME_DATA, "%d", &Hume);   // 去湿度整数
        //        sscanf(LIGHT_DATA, "%d", &Light); // 去光照整数

//        sprintf(Temp_Data, "%s,%s", TEMP_DATA, HUME_DATA); // 整合需要发送的数据到服务器
//        ESP8266_SendData(Temp_Data, strlen(Temp_Data));    // 发送数据

        Update_Data_Environment(); // 更新展示环境数据
        //        Show_Warn(Temp, Hume, Light); // 标记报警模块并且报警
    }
    // 清除
    TIM_ClearITPendingBit(TIM12, TIM_IT_Update);
}

#include "Led.h"
#include "Delay.h"
#include "Oled.h"
#include <stdio.h>

/*********************************************************************************
 * @description: Led灯初始化
 * @Author: Tinao
 * @Date: 2024-11-27 14:33:12
 * @return {*} None
 *********************************************************************************/
void Led_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct; // 定义了一个结构体
    // 1.时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOG, ENABLE); // 必须需要时钟使能

    // 2.GPIO初始化
    GPIO_InitStruct.GPIO_Pin = LED1_PIN | LED2_PIN; // 可以位或 不会影响
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;      // GPIO设置为输出模式
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;     // 推挽输出
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;   // 下拉
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;  // 速率

    GPIO_Init(LED1_2_GROUP, &GPIO_InitStruct); // 初始化GPIO引脚
    GPIO_InitStruct.GPIO_Pin = LED3_PIN;
    GPIO_Init(LED3_GROUP, &GPIO_InitStruct); // 改变 Pin 初始化新的Pin

    // 3.默认状态 灯灭 输出高电平
    GPIO_SetBits(LED1_2_GROUP, LED1_PIN | LED2_PIN);
    GPIO_SetBits(LED3_GROUP, LED3_PIN);

    printf("LED Initialized.\n");
}

/*********************************************************************************
 * @description: Led灯控制函数
 * @Author: Tinao
 * @Date: 2024-11-27 14:33:30
 * @param {int} Led_num ：灯的序号 0 1 2 3
 * @param {int} Status ：灯的状态 Led_ON Led_OFF
 * @return {*} None
 *********************************************************************************/
void Led_Control(int Led_Num, int Status)
{
    if (Led_Num == LED1)
    {
        Status ? LED1_OFF : LED1_ON;
    }
    else if (Led_Num == LED2)
    {
        Status ? LED2_OFF : LED2_ON;
    }
    else if (Led_Num == LED3)
    {
        Status ? LED3_OFF : LED3_ON;
    }
}

/*********************************************************************************
 * @description: 流水灯实现
 * @Author: Tinao
 * @Date: 2024-11-27 14:40:43
 * @return {*} None
 *********************************************************************************/
void Flow_Light_Flash_Light(void)
{
    int i;
    if (LED_STATE)
    {
        for (i = 0; i < 3; i++)
        {
            Led_Control(i, LED_ON);
            Delay_ms(50);
            Led_Control(i, LED_OFF);
            Delay_ms(50);
        }
    }
    else
    {
        Led_Control(1, LED_ON);
        Led_Control(2, LED_ON);
        Led_Control(3, LED_ON);
        Delay_ms(50);
        Led_Control(1, LED_OFF);
        Led_Control(2, LED_OFF);
        Led_Control(3, LED_OFF);
        Delay_ms(50);
    }
}

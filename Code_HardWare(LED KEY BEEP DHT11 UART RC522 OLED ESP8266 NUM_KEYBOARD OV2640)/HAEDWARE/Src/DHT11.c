#include "DHT11.h"
#include "Delay.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char TEMP_DATA[5] = {0}; // 存储温度
char HUME_DATA[5] = {0}; // 存储湿度

int Temp_Threshold = 27; // 温度的阈值 超过报警
int Hume_Threshold = 70; // 湿度的阈值 超过报警

/*********************************************************************************
 * @description: DHT11温湿度传感器输入初始化
 * @Author: Tinao
 * @Date: 2024-11-27 19:52:49
 * @return {*} None
 *********************************************************************************/
void DHT11_Init_Input(void)
{
    // DHT11 数据引脚使能化
    GPIO_InitTypeDef DHT11_GPIO_InitStruct;
    DHT11_GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;   // 输入模式
    DHT11_GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; // 推挽
    DHT11_GPIO_InitStruct.GPIO_Pin = DHT11_PIN;
    DHT11_GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    DHT11_GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(DHT11_GROUP, &DHT11_GPIO_InitStruct);
}

/*********************************************************************************
 * @description: DHT11温湿度传感器输出初始化
 * @Author: Tinao
 * @Date: 2024-11-27 19:52:49
 * @return {*} None
 *********************************************************************************/
void DHT11_Init_Output(void)
{
    // DHT11 数据引脚使能化
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE); /* 使能化GPIOG */

    GPIO_InitTypeDef DHT11_GPIO_InitStruct;
    DHT11_GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;  // 输出模式
    DHT11_GPIO_InitStruct.GPIO_OType = GPIO_OType_PP; // 推挽
    DHT11_GPIO_InitStruct.GPIO_Pin = DHT11_PIN;
    DHT11_GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    DHT11_GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(DHT11_GROUP, &DHT11_GPIO_InitStruct);
}

void DHT11_Start(void)
{
    DHT11_Init_Output();
    GPIO_WriteBit(DHT11_GROUP, DHT11_PIN, Bit_RESET);
    Delay_ms(20);
    GPIO_WriteBit(DHT11_GROUP, DHT11_PIN, Bit_SET);

    Delay_us(30); // 增加延时
    DHT11_Init_Input();

    u8 TimeOut = 0;
    while (GPIO_ReadInputDataBit(DHT11_GROUP, DHT11_PIN) && TimeOut < 100) // DHT11会拉低40~80us
    {
        TimeOut++;
        Delay_us(1);
    }; /* 等待总线电平变低 因为上一步结束总线为高，不然就一直等待 */
    if (TimeOut >= 100)
    {
        return;
    }
    else
    {
        TimeOut = 0;
    }
    while (!GPIO_ReadInputDataBit(DHT11_GROUP, DHT11_PIN) && TimeOut < 100) // DHT11拉低后会再次拉高40~80us
    {
        TimeOut++;
        Delay_us(1);
    }; /* 等待总线电平变高 因为上一步结束总线为底，不然就一直等待 */
    if (TimeOut >= 100)
    {
        return;
    }
    return;
}

/*********************************************************************************
 * @description: 读取DHT11的一位信号
 * @Author: Tinao
 * @Date: 2024-11-27 20:12:57
 * @return {*} 8bit 的数据
 *********************************************************************************/
u8 DHT11_Read_Byte(void)
{
    int i;
    u8 ReadData = 0; // ReadData用于存放8bit数据，即8个单次读取的1bit数据的组合
    u8 Temp_Bit;     // 临时存放信号电平（0或1）

    for (i = 0; i < 8; i++)
    {
        u8 TimeOut = 0;
        while (GPIO_ReadInputDataBit(DHT11_GROUP, DHT11_PIN) && TimeOut < 100) // 等待变为低电平
        {
            TimeOut++;
            Delay_us(1);
        } /* 等待总线电平变底 因为上一步结束总线为高，不然就一直等待 开始传输数据*/
        TimeOut = 0;
        while (!GPIO_ReadInputDataBit(DHT11_GROUP, DHT11_PIN) && TimeOut < 100) // 等待变高电平
        {
            TimeOut++;
            Delay_us(1);
        } /* 等待总线电平变高 因为上一步结束总线为底，开始传输数据*/
        Delay_us(40);
        if (GPIO_ReadInputDataBit(DHT11_GROUP, DHT11_PIN))
        {
            Temp_Bit = 1;
        }
        else
        {
            Temp_Bit = 0;
        }
        ReadData = ReadData << 1;
        ReadData |= Temp_Bit;
    }
    return ReadData;
}

/*********************************************************************************
 * @description: 展示 DHT11 采集的数据
 * @Author: Tinao
 * @Date: 2024-11-27 20:20:46
 * @return {*}
 *********************************************************************************/
void DHT11_Show(void)
{
    int i;
    u8 Data[5];
    DHT11_Start();
    DHT11_Init_Input();

    for (i = 0; i < 5; i++)
    {
        Data[i] = DHT11_Read_Byte(); // 读取每一位的数据
    }
    if ((Data[0] + Data[1] + Data[2] + Data[3]) == Data[4])
    {
        // printf("Humidity: %d.%dRH ,", Data[0], Data[1]);
        sprintf(HUME_DATA, "%d.%d", Data[0], Data[1]);
        // printf("Temperature: %d.%d ", Data[2], Data[3]);
        sprintf(TEMP_DATA, "%d.%d", Data[2], Data[3]);
    }
    else
    {
        printf("ERROR DATA");
    }
}

/*******************
            IIC 软件模拟时序
            PB6 SCL  时钟线全程由主机控制  所以用推挽输出 外部接有上拉电阻  也可以用开漏输出
            PB7 SDA	 有输入、有输出   设置为开漏模式
                                因为外部有接上拉电阻  设置为开漏输出可以不用在输出和输入模式之间转换
*******************/
#include "Iic.h"
#include "Delay.h"
#include <stdio.h>

// 引脚初始化函数
void Software_IIC_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct_IIC; // 定义GPIOB6、7的结构体

    // 时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE); // 使能时钟

    // GPIO配置
    GPIO_InitStruct_IIC.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStruct_IIC.GPIO_Mode = GPIO_Mode_OUT;    // 输出模式
    GPIO_InitStruct_IIC.GPIO_OType = GPIO_OType_PP;   // 开漏输出
    GPIO_InitStruct_IIC.GPIO_PuPd = GPIO_PuPd_NOPULL; // GPIO内部 不加上拉电阻  外部有上拉电阻
    GPIO_InitStruct_IIC.GPIO_Speed = GPIO_Speed_2MHz;

    GPIO_Init(GPIOF, &GPIO_InitStruct_IIC);

    // 默认输出高电平
    GPIO_SetBits(GPIOF, GPIO_Pin_0);
    GPIO_SetBits(GPIOF, GPIO_Pin_1);

    printf("IIC Initialized.\n");
}

// 主机发送起始信号
void Software_IIC_Start(void)
{
    IIC_SCL_H;
    IIC_SDA_H; // 刚开始SCL、SDA都为高电平
    Delay_us(IIC_Delay_Time);

    // SCL为高的情况下 SDA由高变为低 产生起始信号
    IIC_SDA_L; // SDA 变为低电平
    Delay_us(IIC_Delay_Time);

    IIC_SCL_L; // SCL 时钟线变为低 释放时钟线
}

// 主机发送停止信号
void Software_IIC_Stop(void)
{
    IIC_SCL_L;
    IIC_SDA_L; // 刚开始SCL、SDA都为低电平
    Delay_us(IIC_Delay_Time);

    IIC_SCL_H; // 拉高SCL
    Delay_us(IIC_Delay_Time);

    // SCL为高的情况下 SDA由低变为高 产生停止信号
    IIC_SDA_H;
    Delay_us(IIC_Delay_Time);

    // 停止信号之后 SDA、SCL 都为高电平
}

// 主机发送一个数据
// 参数： 需要发送的数据
// 该函数不检验 是否有应答
void Software_IIC_Send_One_Byte_Data(u8 Data)
{
    int i;

    // 发送数据 从最高位开始发送
    for (i = 0; i < 8; i++)
    {
        // SCL时钟 为低电平的时候  可以在SDA 上变化数据
        IIC_SCL_L;
        Delay_us(1);

        if ((Data >> (7 - i)) & 1) // 数据为1
        {
            IIC_SDA_H; // 拉高 SDA
        }
        else // 数据为0
        {
            IIC_SDA_L; // 拉低 SDA
        }
        Delay_us(1);

        // 拉高SCK 就把数据发送出去了
        IIC_SCL_H;
        Delay_us(IIC_Delay_Time);
    }
    IIC_SCL_L; // 先把SCL时钟线 置为低电平  释放时钟线
    Delay_us(1);
}

// 主机读取从机发送的应答
// 返回值 ： 0 读到应答
//					1 没有读到应答
u8 Software_IIC_Read_ACK(void)
{
    u8 Ret = 0;

    // 拉高SDA   SDA为高阻态  受外部控制
    IIC_SDA_H;
    Delay_us(IIC_Delay_Time);

    // 拉高SCL  SDA数据就不能在改变
    IIC_SCL_H;
    Delay_us(IIC_Delay_Time);

    // 在SCL为高的时候		读取SDA的引脚电平
    if (GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_0)) // 引脚为高电平 是非应答
    {
        Ret = 1;
    }

    IIC_SCL_L; // 先把SCL时钟线 置为低电平  释放时钟线
    Delay_us(1);
    return Ret; // 读取到应答
}

/***********************
主机接收从机发送的数据
参数 ：无
返回值：接收到的数据
***********************/
u8 Software_IIC_Receive_One_Byte_Data(void)
{
    int i;
    u8 dat;

    // 先拉低SCL 时钟线
    IIC_SCL_L;
    Delay_us(1);

    // 拉高SDA线
    IIC_SDA_H;
    Delay_us(1);

    for (i = 0; i < 8; i++)
    {
        dat <<= 1;

        // 拉高SCL
        IIC_SCL_H;
        Delay_us(1);

        // 只有SCL为高的时候 才是正真的数据
        if (GPIO_ReadInputDataBit(GPIOF, GPIO_Pin_0))
            dat |= 1; // 读到高电平 最低为置为1

        // 拉低SCL 时钟线 让AT24C02 准备下一位数据
        IIC_SCL_L;
        Delay_us(4);
    }
    return dat;
}

/***********************
主机发送应答或非应答
参数 ：ACK 发送应答
            NO_ACK 非应答
返回值：无
***********************/
void Software_IIC_Send_ACK(u8 Ack)
{
    IIC_SCL_L; // SCL为低
    Delay_us(1);

    if (Ack == ACK)
    {
        IIC_SDA_L;
    }
    else
    {
        IIC_SDA_H;
    }
    Delay_us(1);

    IIC_SCL_H; // SCL为高 发送应答信号
    Delay_us(2);

    IIC_SCL_L; // 应答发送后 拉低SCL  释放时钟线
    Delay_us(1);
}

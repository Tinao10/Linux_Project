#include "Usart.h"
#include "Led.h"
#include "Esp8266.h"
#include "Beep.h"
#include "Delay.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

// u8 USART3_RX_BUF[USART3_REC_LEN]; // 接收缓冲,最大USART_REC_LEN个字节.
// u16 USART3_RX_STA = 0;

// u8 SEND_BUF[2] = {'O', 'K'};
// u8 SEND_BUF_UP[3] = {'U', 'P', '\0'};
// u8 SEND_BUF_DOWN[5] = {'D', 'O', 'W', 'N', '\0'};
int Esp_Flag = 0;
/*********************************************************************************
 * @description: 串口一初始化
 * @Author: Tinao
 * @Date: 2024-11-27 14:48:38
 * @return {*} None
 *********************************************************************************/
void Usart1_Init(u32 BaudRate)
{
	GPIO_InitTypeDef GPIOA_InitStruct;
	USART_InitTypeDef USART1_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	// 1.使能 GPIO
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	GPIOA_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIOA_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIOA_InitStruct.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10;
	GPIOA_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
	GPIOA_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIOA_InitStruct);

	// 2. 配置复用
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource9, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_USART1);

	// 3. 使能 Usart
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	// 3.1 初始化 Usart
	USART1_InitStruct.USART_BaudRate = BaudRate;								  // 波特率
	USART1_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 硬件流空
	USART1_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;				  // 串口模式
	USART1_InitStruct.USART_Parity = USART_Parity_No;							  // 校验方式
	USART1_InitStruct.USART_StopBits = USART_StopBits_1;						  // 停止位长度
	USART1_InitStruct.USART_WordLength = USART_WordLength_8b;					  // 数据位长度
	USART_Init(USART1, &USART1_InitStruct);

	USART_ClearFlag(USART1, USART_FLAG_TC);
	// 3.2 使能中断
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

	// 4 NVIC控制器
	NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;		   // 响应优先级
	NVIC_Init(&NVIC_InitStruct);

	// 5. 开启
	USART_Cmd(USART1, ENABLE);

	printf("Usart1 Initialized.\n");
}

/*********************************************************************************
 * @description: 串口一 中断服务函数
 * @Author: Tinao
 * @Date: 2024-11-27 14:48:12
 * @return {*} None
 *********************************************************************************/
void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) // 监测
	{
		// 接收信息
		u8 Data = USART_ReceiveData(USART1);
		// 判断，如果为 A a 发生不同先现象
		if ((char)Data == 'a') // D1亮
		{
			Led_Control(LED1, LED_ON);
			USART_SendData(USART1, '1');
		}
		if (Data == 'A')
		{
			Led_Control(LED1, LED_OFF);
			USART_SendData(USART1, '0');
		}
		if (Data == 'b')
		{
			Beep_Control(BEEP_ON);
		}
		if (Data == 'B')
		{
			Beep_Control(BEEP_OFF);
		}
	}
	USART_ClearITPendingBit(USART1, USART_IT_RXNE);
}

/*********************************************************************************
 * @description: 重定义 fputc 函数，实现从中断输出到串口一输出
 * @Author: Tinao
 * @Date: 2024-11-27 14:50:45
 * @param {int} c
 * @param {FILE} *stream
 * @return {*}
 *********************************************************************************/
int fputc(int c, FILE *stream)
{
	USART_SendData(USART1, c & 0xFF);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET)
		;
	return 0;
}

/*********************************************************************************
 * @description: 发送字符串
 * @Author: Tinao
 * @Date: 2024-11-27 14:47:19
 * @param {USART_TypeDef} *USARTx ：串口的编号
 * @param {char} *Str ：发送的字符串
 * @param {int} Length ：发送字符串的长度
 * @return {*} None
 *********************************************************************************/
void USART_SendDatas(USART_TypeDef *USARTx, char *Str, int Length)
{
	int i;
	for (i = 0; i < Length; i++)
	{
		while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET)
			;
		USART_SendData(USARTx, Str[i]);
		// USART_ClearFlag(USARTx, USART_FLAG_TXE);
	}
}

/*********************************************************************************
 * @description: 串口3初始化
 * @Author: Tinao
 * @Date: 2024-11-27 14:48:38
 * @return {*} None
//  *	说明：		TX-PA0		RX-PA1
 *********************************************************************************/
void Uart5_Init(u32 BaudRate)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef UART5_InitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Init(GPIOC, &GPIO_InitStruct);

	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_Init(GPIOD, &GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_UART5);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource2, GPIO_AF_UART5);

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, ENABLE);

	UART5_InitStruct.USART_BaudRate = BaudRate;
	UART5_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件流控
	UART5_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;				 // 接收和发送
	UART5_InitStruct.USART_Parity = USART_Parity_No;							 // 无校验
	UART5_InitStruct.USART_StopBits = USART_StopBits_1;							 // 1位停止位
	UART5_InitStruct.USART_WordLength = USART_WordLength_8b;					 // 8位数据位
	USART_Init(UART5, &UART5_InitStruct);

	USART_ITConfig(UART5, USART_IT_RXNE, ENABLE); // 使能接收中断

	NVIC_InitStruct.NVIC_IRQChannel = UART5_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStruct);

	USART_Cmd(UART5, ENABLE); // 使能串口

	printf("UART5 Initialized.\n");
}

/*********************************************************************************
 * @description: 串口5收发中断
 * @Author: Tinao
 * @Date: 2024-11-30 12:23:45
 * @return {*} None
 *********************************************************************************/
void UART5_IRQHandler(void)
{
	if (USART_GetITStatus(UART5, USART_IT_RXNE) == SET) // 接收中断
	{
		if (Esp8266_Cnt >= sizeof(Esp8266_Buf))
		{
			Esp8266_Cnt = 0; // 防止串口被刷爆
		}
		Esp8266_Buf[Esp8266_Cnt++] = UART5->DR;
		if(Esp_Flag != 0)
		{
			//printf("%s\n", Esp8266_Buf); // 打印接收到的字符串
		}
		
		
		//printf("%s\n",Esp8266_Buf);
		
//		char *PtrIPD = strstr((char *)Esp8266_Buf, "IPD,"); // 搜索“IPD”头
//		if(strstr(strchr(PtrIPD, ':'),"BEEP"))
//		{
//		 Beep_Control(BEEP_ON);
//		 Delay_ms(5000);
//		 Beep_Control(BEEP_OFF);
			
//		}
		
	}
	USART_ClearITPendingBit(UART5, USART_FLAG_RXNE);
}

/*********************************************************************************
 * @description:  串口二初始化
 * @Author: Tinao
 * @Date: 2024-12-02 09:42:29
 * @param {u32} BaudRate ：波特率
 * @return {*} None
 * 说明 ： TX : PA2    RX ： PA3
 *********************************************************************************/
// void Usart2_Init(u32 BaudRate)
// {
//     GPIO_InitTypeDef GPIO_InitStruct;
//     USART_InitTypeDef USART2_InitStruct;
//     NVIC_InitTypeDef NVIC_InitStruct;
//     // GPIO使能化
//     RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

//    // 初始化GPIO引脚 复用模式
//    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
//    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
//    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
//    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
//    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
//    GPIO_Init(GPIOA, &GPIO_InitStruct);

//    // 配置复用模式
//    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
//    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

//    // 使能串口
//    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

//    // 串口初始化
//    USART2_InitStruct.USART_BaudRate = BaudRate;
//    USART2_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
//    USART2_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
//    USART2_InitStruct.USART_Parity = USART_Parity_No;
//    USART2_InitStruct.USART_StopBits = USART_StopBits_1;
//    USART2_InitStruct.USART_WordLength = USART_WordLength_8b;
//    USART_Init(USART2, &USART2_InitStruct);

//    // 串口中断配置
//    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

//    // NVIC 初始化
//    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
//    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
//    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
//    NVIC_Init(&NVIC_InitStruct);

//    // 开启串口
//    USART_Cmd(USART2, ENABLE);
// }

/*********************************************************************************
 * @description: 串口二的中断服务函数
 * @Author: Tinao
 * @Date: 2024-12-02 09:57:41
 * @return {*}
 *********************************************************************************/
// void USART2_IRQHandler(void)
// {
//     u8 Data;
//     if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
//     {
//         Data = USART_ReceiveData(USART2); // 读取接收到的数据
//         RecvBuf[Data_Len++] = Data;
//         if (RecvBuf[0] != 0xFF)
//             Data_Len = 0;
//         if (Data_Len == 9)
//         {
//             Recv_Ok = 1;
//         }
//     }
//     USART_ClearITPendingBit(USART2, USART_IT_RXNE);
// }

// void LTE_uart3_init(u32 bound)
// {
// 	GPIO_InitTypeDef GPIO_InitStructure;   // GPIO配置结构体
// 	USART_InitTypeDef USART_InitStructure; // 串口配置结构体
// 	NVIC_InitTypeDef NVIC_InitStructure;
// 	// 1.串口时钟和GPIO时钟使能
// 	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);  // 使能GPIOB时钟
// 	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE); // 使能USART3时钟
// 	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
// 	// 2.设置引脚复用器映射
// 	GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_USART3); // GPIOB10复用为USART3
// 	GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_USART3); // GPIOB11复用为USART3

// 	// 3.GPIO端口配置
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; // GPIOB10与GPIOB11
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			 // 复用功能
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 // 速度50MHz
// 	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			 // 推挽复用输出
// 	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;			 // 上拉
// 	GPIO_Init(GPIOB, &GPIO_InitStructure);					 // 初始化PB10，PB11

// 	// 4.串口参数初始化：波特率等
// 	USART_InitStructure.USART_BaudRate = bound;										// 波特率设置
// 	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// 字长为8位数据格式
// 	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// 一个停止位
// 	USART_InitStructure.USART_Parity = USART_Parity_No;								// 无奇偶校验位
// 	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // 无硬件数据流控制
// 	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// 收发模式
// 	USART_Init(USART3, &USART_InitStructure);										// 初始化串口3
// 	USART_Cmd(USART3, ENABLE);														// 使能串口1

// 	// 清除中断标志位
// 	USART_ClearFlag(USART3, USART_FLAG_TC);
// 	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE); // 开启相关中断

// 	// Usart1 NVIC 配置
// 	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;		  // 串口3中断通道
// 	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 抢占优先级3
// 	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // 子优先级3
// 	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
// 	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器、

// 	/****************************** 蓝牙状态IO口初始化**********************************/
// 	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;		   // GPIOA7
// 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	   // 普通输入模式
// 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; // 100M
// 	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
// 	GPIO_Init(GPIOF, &GPIO_InitStructure); // 初始化
// }

// void USART3_IRQHandler(void)
// {
// 	u8 i;
// 	// 因为串口的中断类型有很多，因此在进入中断之后，首先需要判断此次中断是那种类型。
// 	// USART_GetITStatus(USART1,USART_IT_RXNE) 函数用来读取中断状态标志位。此函数功能是判断 USARTx 的中断类型 USART_IT 是否产生中断，
// 	if (USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)
// 	{
// 		USART_ClearITPendingBit(USART3, USART_IT_RXNE); // 清除中断标志位

// 		USART3_RX_BUF[USART3_RX_STA] = USART_ReceiveData(USART3); // 读取一位接收到的数据
// 		USART3_RX_STA++;

// 		// 判断是否满足接收完成条件，否则继续接收数据
// 		if (USART3_RX_BUF[USART3_RX_STA - 1] == 0x0a || USART3_RX_BUF[USART3_RX_STA - 1] == '#' || USART3_RX_STA == USART3_REC_LEN) // 如果接收到换行符或者接收的数据已满
// 		{
// 			if (USART3_RX_BUF[USART3_RX_STA - 1] == '#')
// 			{

// 				TIM_Cmd(TIM3, DISABLE);
// 				Send_data_3(SEND_BUF_DOWN);
// 				USART3_RX_STA = 0;
// 			}
// 			// 数据发送完成之后，清空数据接收数组，以便接收新的数据。
// 			for (i = 0; i < USART3_RX_STA; i++)
// 			{
// 				USART3_RX_BUF[i] = '\0';
// 			}
// 			USART3_RX_STA = 0;
// 		}
// 		else if (USART3_RX_BUF[USART3_RX_STA - 1] == 0x0a || USART3_RX_BUF[USART3_RX_STA - 1] == '@' || USART3_RX_STA == USART3_REC_LEN)
// 		{
// 			if (USART3_RX_BUF[USART3_RX_STA - 1] == '@')
// 			{

// 				TIM_Cmd(TIM4, DISABLE);
// 				Send_data_3(SEND_BUF_UP);
// 				USART3_RX_STA = 0;
// 			}
// 			// 数据发送完成之后，清空数据接收数组，以便接收新的数据。
// 			for (i = 0; i < USART3_RX_STA; i++)
// 			{
// 				USART3_RX_BUF[i] = '\0';
// 			}
// 			USART3_RX_STA = 0;
// 		}

// 		// 清除数据
// 		if (USART3_RX_STA > 2)
// 		{
// 			for (i = 0; i < USART3_RX_STA; i++)
// 			{
// 				USART3_RX_BUF[i] = '\0';
// 			}
// 			USART3_RX_STA = 0;
// 		}
// 	}
// }

// ////发送字符串
// void Send_data_3(u8 *s)
// {
// 	while (*s != '\0')
// 	{
// 		while (USART_GetFlagStatus(USART3, USART_FLAG_TC) == RESET)
// 			;
// 		USART_SendData(USART3, *s);
// 		s++; // 指向下一个字节。
// 	}
// }

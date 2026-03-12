#ifndef __USART_H__
#define __USART_H__

#include "stm32f4xx.h"
#include "system.h"

#define LTE_connect PFin(13)     //LTE连接标志  为1与手机APP或者其他设备连接成功
#define USART3_REC_LEN	200  	//定义最大接收字节数 200
#define USART_DEBUG USART1 // 调试打印所使用的串口组

extern u8  USART3_RX_BUF[USART3_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART3_RX_STA;         		//接收状态标记 
extern int Esp_Flag;

void Usart1_Init(u32 BaudRate);
void Uart5_Init(u32 BaudRate);
//void Usart2_Init(u32 BaudRate);
void USART_SendDatas(USART_TypeDef *USARTx, char *Str, int Length);
// void LTE_uart3_init(u32 bound);
// void Send_data_3(u8 *s);

#endif

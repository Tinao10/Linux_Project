#ifndef __ESP8266_H__
#define __ESP8266_H__

#include "stm32f4xx.h"

#define REV_OK 0   // 接收完成标志
#define REV_WAIT 1 // 接收未完成标志



extern unsigned char Esp8266_Buf[128];
extern unsigned short Esp8266_Cnt, Esp8266_CntPre;

// 函数声明
void ESP8266_TCP_Init(void);
void ESP8266_UDP_Init(void);
void ESP8266_Clear(void);
int ESP8266_SendData(char *Data, int Length);
int ESP8266_SendJPEGChunk(int length);
char *ESP8266_GetIPD(unsigned short TimeOut);
_Bool ESP8266_SendCmd(char *Cmd, char *Result);
_Bool ESP8266_WaitRecive(void);

#endif

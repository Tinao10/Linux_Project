#ifndef __OLED_H
#define __OLED_H

#include "stm32f4xx.h"
#include <stdbool.h>

extern bool LED_STATE;

#define OLED_SLAVE_ADDR 0x78 // OLED 从机地址
#define OLED_SLAVE_CMD 0x00  // 0000 0000  下一次发送的字节为指令
#define OLED_SLAVE_DATA 0x40 // 0100 0000  下一次发送的字节为数据

/********通过IIC向OLED发送一个字节命令*********/
u8 Oled_Send_One_Byte_Cmd(u8 cmd);

/********通过IIC向OLED发送一个字节数据*********/
u8 Oled_Send_One_Byte_DATA(u8 Data);

/*****************初始化OLED*****************/
void Oled_Init(void);

// 填充缓存区中的数据
void OLed_Fill(unsigned char Bmp_Data);

// 取消OLED初始化
void Off_Init_OLed(void);

// 设置显示字符位置的函数
void OLed_SetPos(unsigned char X, unsigned char Y);

// 显示一个中文
void OLed_ShowChina(u8 X, u8 Y, u8 *Buf);

// 在指定位置显示ASCLL字符
void OLed_ShowASCII(u8 X, u8 Y, char *Str);

void OLed_ShowASCII_Len(u8 X, u8 Y, char *Str, int Length);

// 显示界面信息
void Show_UI(void);

// 实时更新环境数据
void Update_Data_Environment(void);

// 实时更新门禁方式
void Update_Data_Access(char *IDStr);

void Update_Passwd(char *IDStr, int Length);

void Update_Face(char *IDStr);

// 展示报警模块
void Show_Warn(int Temp, int Hume, int Light);

// 消除报警 ！
void Eliminate_Warn(bool Temp_State, bool Hume_State);

#endif

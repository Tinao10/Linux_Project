#include "Num_Keyboard.h"
#include "Delay.h"
#include "ESP8266.h"
#include "Oled.h"
#include "Beep.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int KeyBoard_Num = -1;

int Passwd[MAX_PASSWD_LENGTH]; // 存储按键输入的数组
char Passwd_Str[MAX_PASSWD_LENGTH];
uint8_t Passwd_Length = 0; // 当前密码长度

void Num_Keyboard_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    // GPIOA和GPIOD时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG | RCC_AHB1Periph_GPIOD, ENABLE);

    // 初始化列为输入
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = COL0 | COL1 | COL2;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOG, &GPIO_InitStruct);

    // 初始化行为输出
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_Pin = ROW0 | ROW1 | ROW2 | ROW3;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOD, &GPIO_InitStruct);

    // 每一行拉高
    GPIO_SetBits(GPIOD, ROW0 | ROW1 | ROW2 | ROW3);

    // SYSCFG时钟使能
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource2);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource3);
    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOG, EXTI_PinSource4);

    // EXTI配置
    EXTI_InitStruct.EXTI_Line = EXTI_Line2 | EXTI_Line3 | EXTI_Line4;
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_Init(&EXTI_InitStruct);

    // NVIC配置
    NVIC_InitStruct.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI3_IRQn;
    NVIC_Init(&NVIC_InitStruct);

    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_Init(&NVIC_InitStruct);

    printf("NUM_KEYBOARD Initialized.\n");
}

void EXTI2_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line2) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line2);
    }
}

void EXTI3_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line3) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line3);
    }
}

void EXTI4_IRQHandler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

char* format_passwd(const int* arr, size_t len) {
   if (arr == NULL || len == 0) {
       return NULL;
   }

   // 计算前缀长度
   const char* prefix = "PASSWD:";
   size_t prefix_len = strlen(prefix);

   // 计算总长度
   size_t total_len = prefix_len + 1; // +1 for null terminator
		size_t i;
   for (i = 0; i < len; i++) {
       int num = arr[i];
       // 计算当前数字转换为字符串后的长度
       int num_len = snprintf(NULL, 0, "%d", num);
       total_len += num_len;
   }

   // 分配内存
   char* buffer = (char*)malloc(total_len);
   if (buffer == NULL) {
       return NULL;
   }

   // 写入前缀
   size_t pos = snprintf(buffer, total_len, "%s", prefix);
   if (pos >= total_len) {
       free(buffer);
       return NULL;
   }

   // 逐个写入数字
   for (i = 0; i < len; i++) {
       int num = arr[i];
       // 写入当前数字到缓冲区剩余位置
       int written = snprintf(buffer + pos, total_len - pos, "%d", num);
       if (written < 0 || written >= (total_len - pos)) {
           free(buffer);
           return NULL;
       }
       pos += written;
   }

   return buffer;
}


void Keyboard_Scan(void)
{
    uint16_t Col_Pins[3] = {COL0, COL1, COL2};
    int i, j;

    for (i = 0; i < 4; i++)
    {
        SetRow_GPIO(i);
        Delay_ms(20);
        for (j = 0; j < 3; j++)
        {
            if (GPIO_ReadInputDataBit(GPIOG, Col_Pins[j]) == 0)
            {
                KeyBoard_Num = i * 3 + j + 1; // 更新按键编号
                printf("KeyBoard_Num:%d\n", KeyBoard_Num);

                // 处理按键输入
                if (KeyBoard_Num == 10)
                {
                    // 如果按下的是删除键（12），删除最后一个数字
                    if (Passwd_Length > 0)
                    {
                        Passwd_Length--;               // 长度减1
                        Passwd[Passwd_Length] = 0;     // 清空最后一个位置
                        Passwd_Str[Passwd_Length] = 0; // 清空最后一个位置
                        Update_Passwd(Passwd_Str, Passwd_Length);
                    }
                }
                else if (KeyBoard_Num == 12)
                {
									//char Temp[20];
									size_t len = sizeof(Passwd) / sizeof(Passwd[0]);
									char* result = format_passwd(Passwd, len);
									//sprintf(Temp, "%s:%s", "PASSWD", result); // 整合需要发送的数据到服务器
									printf("%s\n",result);
									ESP8266_SendData(result, strlen(result));    // 发送数据
									Beep_Control(BEEP_ON);
									Delay_ms(500);
									Beep_Control(BEEP_OFF);
                }
                else
                {
                    if (KeyBoard_Num == 11)
                    {
                        KeyBoard_Num = 0;
                    }
                    // 如果按下的是数字键，添加到数组尾部
                    if (Passwd_Length < MAX_PASSWD_LENGTH)
                    {
                        Passwd[Passwd_Length] = KeyBoard_Num; // 添加到数组
                        Passwd_Str[Passwd_Length] = '*';      // 清空最后一个位置
                        Passwd_Length++;                      // 长度加1
                        Update_Passwd(Passwd_Str, Passwd_Length);
                    }
                }
            }
        }
    }
}

void SetRow_GPIO(int Num)
{
    uint16_t Row_Pins[4] = {ROW0, ROW1, ROW2, ROW3};
    GPIO_SetBits(GPIOD, ROW0 | ROW1 | ROW2 | ROW3);
    GPIO_ResetBits(GPIOD, Row_Pins[Num]);
}

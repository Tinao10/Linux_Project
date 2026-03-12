#ifndef __NUM_KEYBOARD_H__
#define __NUM_KEYBOARD_H__

#include "stm32f4xx.h"

extern int KeyBoard_Num;
extern uint8_t key_Pressed;

#define MAX_PASSWD_LENGTH 6
#define COL0 GPIO_Pin_2
#define COL1 GPIO_Pin_3
#define COL2 GPIO_Pin_4
#define ROW0 GPIO_Pin_0
#define ROW1 GPIO_Pin_1
#define ROW2 GPIO_Pin_14
#define ROW3 GPIO_Pin_15

void Num_Keyboard_Init(void);
void Keyboard_Scan(void);
void SetRow_GPIO(int Num);
void Keyboard_Process(void);

#endif

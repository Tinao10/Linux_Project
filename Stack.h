/***
 * @Description:
 * 顺序栈的结构体和相关函数的声明
 * @Author: Tinao
 * @Date: 2024-07-03 13:31:32
 * @LastEditTime: 2024-07-03 14:58:36
 * @LastEditors: Tinao
 */
#ifndef _STACK_H_
#define _STACK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h> // / 使用bool类型

#define STACKMAXNUM 100
typedef int Myint;

typedef struct STACK
{
    Myint Array[STACKMAXNUM]; // 用于存放元素，创建一个数组
    Myint Top;                // 栈顶
    Myint StackNum;           // 记录栈的元素个数
} stack;

// 初始化栈
struct STACK *StackInit();

// 进栈
void StackPush(stack *Head, Myint Num);

// 出栈
void StackPop(stack *Head);

// 清空栈
void StackClean(stack *Head);

// 返回栈顶元素
Myint StackTopNum(stack *Head);

// 销毁栈
void StackDestory(stack *Head);

// 判断栈是否为空
bool StackIsEmpty(stack *Head);
#endif

/*
 * @Description:
    顺序栈相关声明函数的定义实现
 * @Author: Tinao
 * @Date: 2024-07-03 14:51:19
 * @LastEditTime: 2024-07-03 15:16:55
 * @LastEditors: Tinao
 */
#include "Stack.h"

/**
 * @description:
 * 初始化栈
 * @Author: Tinao
 * @Date: 2024-07-03 15:00:48
 * @return Head  返回一个地址
 */
struct STACK *StackInit()
{
    stack *Head = (struct STACK *)malloc(sizeof(struct STACK));
    Head->StackNum = 0; // 初始化栈的数量为0
    Head->Top = -1;     // 初始化栈顶为-1
    return Head;
}

/**
 * @description:
 * 进栈操作
 * @Author: Tinao
 * @Date: 2024-07-03 15:04:29
 * @return {*}
 * @param {stack *} Head  参数一 初始化函数之后的地址
 * @param {Myint} Num 参数二 需要进栈的元素
 */
void StackPush(stack *Head, Myint Num)
{
    // 栈的元素个数不为满，且Head != NULL 则可以进栈
    if (Head == NULL || Head->StackNum > STACKMAXNUM)
    {
        printf("进栈失败！\n");
        return;
    }
    else
    {
        Head->Array[++(Head->Top)] = Num;
        Head->StackNum++;
        printf("进栈成功！\n");
        return;
    }
}

/**
 * @description:
 * 出栈操作
 * @Author: Tinao
 * @Date: 2024-07-03 15:03:19
 * @return 无返回值
 */
void StackPop(stack *Head)
{
    // 栈的元素个数为0 且Head == NULL 则无法进行出战操作
    if (Head == NULL || Head->Top == -1)
    {
        printf("出栈失败！\n");
        return;
    }
    else
    {
        printf("=> %d\n", Head->Array[Head->Top]);
        Head->Array[(Head->Top)--];
        Head->StackNum--;
        return;
    }
}

/**
 * @description:
 * 清除栈内的元素
 * @Author: Tinao
 * @Date: 2024-07-03 15:04:18
 * @return {*}
 * @param {stack *} Head 参数一 ： 初始化函数之后的地址
 */
void StackClean(stack *Head)
{
    // 如果栈为空这不需要清空，如果Head == NULL 无法操作
    if (Head != NULL && Head->Top != -1)
    {
        Head->Top = -1;
        printf("栈已清空！\n");
        return;
    }
    else
    {
        printf("栈为空！\n");
    }
}

/**
 * @description:
 * 返回栈顶的元素
 * @Author: Tinao
 * @Date: 2024-07-03 15:06:22
 * @return Head->Array[Head->Top] 返回栈顶元素
 * @param {stack *} Head 参数一 初始化函数之后的地址
 */
Myint StackTopNum(stack *Head)
{
    // 栈的元素不为空，且Head != NULL
    if (Head->Top != -1 && Head != NULL)
    {
        printf("栈顶元素 ==> %d\n", Head->Array[Head->Top]);
        return Head->Array[Head->Top];
    }
    else
    {
        printf("Error 无法返回栈顶元素！\n");
    }
}

/**
 * @description:
 *    销毁顺序栈 由于顺序栈是一个内存空间连续的数组，因此只需要
 * 释放初始化栈的地址即可
 * @Author: Tinao
 * @Date: 2024-07-03 15:07:23
 * @return 无返回值
 * @param {stack *} Head 参数一 ： 初始化函数之后的地址
 */
void StackDestory(stack *Head)
{
    free(Head);
    printf("销毁成功！！\n");
    return;
}

/**
 * @description:
 * 判断栈是否为空栈
 * @Author: Tinao
 * @Date: 2024-07-03 15:11:22
 * @return true 为空栈
 *         false 不是空战
 * @param {stack *} Head 参数一 ： 初始化函数之后的地址
 */
bool StackIsEmpty(stack *Head)
{
    // 如果Head != NULL时 或者 Head->Top == -1时为空栈
    if (Head != NULL && Head->Top != -1)
    {
        return false;
    }
    else
    {
        return true;
    }
}
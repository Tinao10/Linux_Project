/*
 * @Description:
    顺序栈的执行入口函数文件
 * @Author: Tinao
 * @Date: 2024-07-03 14:12:21
 * @LastEditTime: 2024-07-03 18:57:11
 * @LastEditors: Tinao
 */
#include "Stack.h"

int main()
{
    stack *Node;
    Node = StackInit(); // 初始化顺序栈
    printf("输入数据进入栈！输入-1则退出输入\n");
    int Num = 0;
    // 入栈
    while (1)
    {
        scanf("%d", &Num);
        if (Num == -1)
        {
            break;
        }
        else
        {
            StackPush(Node, Num);
        }
    }
    // 判断栈是否为空栈
    StackIsEmpty(Node);
    // 弹出栈顶元素
    StackTopNum(Node);
    // 出栈
    while (Node->Top != -1)
    {
        StackPop(Node);
    }
    // 清空栈
    StackClean(Node);
    // 判断是否为空栈
    StackIsEmpty(Node);
    // 销毁栈
    StackDestory(Node);
    return 0;
}
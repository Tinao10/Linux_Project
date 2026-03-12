#include "MQ02.h"
#include "Usart.h"
#include "Delay.h"
#include <stdio.h>

char MQ02_DATA[6] = {0}; // 存储烟雾值

char MQ2_Cmd[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
unsigned char RecvBuf[32] = {0};
unsigned char Data_Len = 0;
unsigned char Recv_Ok = 0;

/*********************************************************************************
 * @description: 上位机发送命令，开始接收信息
 * @Author: Tinao
 * @Date: 2024-12-02 10:05:58
 * @return {*} None
 *********************************************************************************/
void MQ02_Start(void)
{
    USART_SendDatas(USART2, MQ2_Cmd, 9);
}

/*********************************************************************************
 * @description: 获取传感器信息数据
 * @Author: Tinao
 * @Date: 2024-12-02 10:06:42
 * @return {*} None
 *********************************************************************************/
void MQ02_Show(void)
{
    int MQ02 = 0;
    MQ02_Start();
    if (Recv_Ok)
    {
        MQ02 = RecvBuf[2] << 8 | RecvBuf[3];
        sprintf(MQ02_DATA, "%d", MQ02);
			printf("SMOG:%d", MQ02);
    }
    else
    {
			printf("SMOG:%s", MQ02_DATA);
    }
    Recv_Ok = 0;  // 接收到信息标志位 至1
    Data_Len = 0; // 数据长度归0
}




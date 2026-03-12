#include "Esp8266.h"
#include "Delay.h"
#include "Usart.h"
#include <string.h>
#include <stdio.h>

char WIFI_NAME[] = "Tinao";        // 连接的WIFI名字
char WIFI_PASSWD[] = "12345678";   // 连接的WIFI密码
char SERVE_IP[] = "192.168.137.1"; // 连接的服务器IP
char SERVE_PORT[] = "8888";        // 连接的服务器的PORT

char ESP8266_WIFI_INFO[100] = {0};
char ESP8266_ONENET_INFO[100] = {0};

unsigned char Esp8266_Buf[128];
unsigned short Esp8266_Cnt = 0, Esp8266_CntPre = 0;

/*********************************************************************************
 * @description: ESP68266 初始化
 * @Author: Tinao
 * @Date: 2024-11-30 14:54:58
 * @return {*} None
 *********************************************************************************/
void ESP8266_TCP_Init(void)
{
    printf("0. Waiting for ESP8266 .....\r\n");
    while (ESP8266_SendCmd("AT\r\n", "OK"))
        Delay_ms(500);
    printf("0. ESP8266 responded!\r\n");
    printf("1. Resetting ESP8266....\r\n");
    ESP8266_SendCmd("AT+RST\r\n", "OK");
    Delay_ms(500);
    ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK");
    printf("1. ESP8266 reset completed!\r\n");
    printf("2. Setting Wi-Fi mode to Station Mode.....\r\n");
    while (ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK")) /* AT+CWMODE=1 sets ESP8266 to Station Mode (Client Mode) */
        Delay_ms(500);
    printf("2. Wi-Fi mode set to Station Mode successfully!\r\n");
    printf("3. Configuring ESP8266 DHCP function.....\r\n");
    while (ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK")) /* AT+CWDHCP=1,1 enables DHCP to automatically obtain an IP address */
        Delay_ms(500);
    printf("3. ESP8266 DHCP configuration completed!\r\n");
    printf("4. Connecting to the specified Wi-Fi network....\r\n");
    sprintf(ESP8266_WIFI_INFO, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_NAME, WIFI_PASSWD);
    while (ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))
        Delay_ms(500);
    printf("4. Successfully connected to the specified Wi-Fi network!\r\n");
    printf("5. Setting TCP/UDP connection to single-connection mode....\r\n");
    while (ESP8266_SendCmd("AT+CIPMUX=0\r\n", "OK")) /* AT+CIPMUX=0 sets ESP8266 to single-connection mode */
        Delay_ms(500);
    printf("5. TCP/UDP single-connection mode configured!\r\n");
    printf("6. Establishing TCP/UDP connection.....\r\n");
    sprintf(ESP8266_ONENET_INFO, "AT+CIPSTART=\"TCP\",\"%s\",%s\r\n", SERVE_IP, SERVE_PORT);
    while (ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
        Delay_ms(500);
    printf("6. TCP/UDP connection established successfully!\r\n");
    printf("7. ESP8266 initialization completed! Server connection successful~\r\n");
}


void ESP8266_UDP_Init(void) {
    printf("0. Waiting for ESP8266 .....\r\n");
    while (ESP8266_SendCmd("AT\r\n", "OK"))
        Delay_ms(500);
    printf("0. ESP8266 responded!\r\n");

    printf("1. Resetting ESP8266....\r\n");
    ESP8266_SendCmd("AT+RST\r\n", "OK");
    Delay_ms(500);
    ESP8266_SendCmd("AT+CIPCLOSE\r\n", "OK");
    printf("1. ESP8266 reset completed!\r\n");

    printf("2. Setting Wi-Fi mode to Station Mode.....\r\n");
    while (ESP8266_SendCmd("AT+CWMODE=1\r\n", "OK")) /* AT+CWMODE=1 sets ESP8266 to Station Mode (Client Mode) */
        Delay_ms(500);
    printf("2. Wi-Fi mode set to Station Mode successfully!\r\n");

    printf("3. Configuring ESP8266 DHCP function.....\r\n");
    while (ESP8266_SendCmd("AT+CWDHCP=1,1\r\n", "OK")) /* AT+CWDHCP=1,1 enables DHCP to automatically obtain an IP address */
        Delay_ms(500);
    printf("3. ESP8266 DHCP configuration completed!\r\n");

    printf("4. Connecting to the specified Wi-Fi network....\r\n");
    sprintf(ESP8266_WIFI_INFO, "AT+CWJAP=\"%s\",\"%s\"\r\n", WIFI_NAME, WIFI_PASSWD);
    while (ESP8266_SendCmd(ESP8266_WIFI_INFO, "GOT IP"))
        Delay_ms(500);
    printf("4. Successfully connected to the specified Wi-Fi network!\r\n");

    printf("5. Setting UDP connection to single-connection mode....\r\n");
    while (ESP8266_SendCmd("AT+CIPMUX=0\r\n", "OK")) /* AT+CIPMUX=0 sets ESP8266 to single-connection mode */
        Delay_ms(500);
    printf("5. UDP single-connection mode configured!\r\n");

    printf("6. Establishing UDP connection.....\r\n");
    sprintf(ESP8266_ONENET_INFO, "AT+CIPSTART=\"UDP\",\"%s\",%s\r\n", SERVE_IP, SERVE_PORT);
    while (ESP8266_SendCmd(ESP8266_ONENET_INFO, "CONNECT"))
        Delay_ms(500);
    printf("6. UDP connection established successfully!\r\n");

    printf("7. ESP8266 initialization completed! UDP connection successful~\r\n");
}


/*********************************************************************************
 * @description: 清空缓存
 * @Author: Tinao
 * @Date: 2024-11-30 12:15:11
 * @return {*} None
 *********************************************************************************/
void ESP8266_Clear(void)
{
    memset(Esp8266_Buf, 0, sizeof(Esp8266_Buf));
    Esp8266_Cnt = 0;
}

/*********************************************************************************
 * @description: 等待结束USART3数据完成
 * @Author: Tinao
 * @Date: 2024-11-30 14:51:08
 * @return {*} REV_OK-接收完成		REV_WAIT-接收超时未完成
 * 说明：		循环调用检测是否接收完成
 *********************************************************************************/
_Bool ESP8266_WaitRecive(void)
{
    if (Esp8266_Cnt == 0) // 如果接收计数为0 则说明没有处于接收数据中，所以直接跳出，结束函数
        return REV_WAIT;
    if (Esp8266_Cnt == Esp8266_CntPre) // 如果上一次的值和这次相同，则说明接收完毕
    {
        Esp8266_Cnt = 0; // 清0接收计数
        return REV_OK;   // 返回接收完成标志
    }
    Esp8266_CntPre = Esp8266_Cnt; // 置为相同
    return REV_WAIT;              // 返回接收未完成标志
}

/*********************************************************************************
 * @description: 往ESP8266发送AT指令
 * @Author: Tinao
 * @Date: 2024-11-30 14:51:55
 * @param {char} *cmd ：命令
 * @param {char} *res ：需要检查的返回指令
 * @return {*} 0-成功	1-失败
 *********************************************************************************/
_Bool ESP8266_SendCmd(char *Cmd, char *Result)
{
    unsigned char TimeOut = 200;
    USART_SendDatas(UART5, Cmd, strlen(Cmd));
    while (TimeOut--)
    {
        if (ESP8266_WaitRecive() == REV_OK) // 如果收到数据
        {
            if (strstr((const char *)Esp8266_Buf, Result) != NULL) // 如果检索到关键词
            {
                ESP8266_Clear(); // 清空缓存
                return 0;
            }
        }
        Delay_ms(10);
    }
    return 1;
}

/*********************************************************************************
 * @description: 发送数据给到服务器
 * @Author: Tinao
 * @Date: 2024-11-30 14:53:04
 * @param {char} *data ：数据
 * @param {int} len ：数据长度
 * @return {*} None
 *********************************************************************************/
int ESP8266_SendData(char *Data, int Length)
{
    char CmdBuf[32];
    ESP8266_Clear();                              // 清空接收缓存
    sprintf(CmdBuf, "AT+CIPSEND=%d\r\n", Length); // 发送命令
    if (ESP8266_SendCmd(CmdBuf, ">"))             // 收到‘>’时可以发送数据
    {
        ESP8266_Clear();
        USART_SendDatas(UART5, Data, Length); // 发送完整的 HTTP 请求，使用 Length 而不是 strlen
        return 1;
    }
    else
    {
        return 0;
    }
}

// 发送 JPEG 数据块
int ESP8266_SendJPEGChunk(int length)
{
    char cmd[32];
    ESP8266_Clear();
    sprintf(cmd, "AT+CIPSEND=%d\r\n", length); // 发送 AT+CIPSEND 命令
    if (!ESP8266_SendCmd(cmd, ">"))
    {
        return 0;
    }
    return 1; // 发送 JPEG 数据
}

/*********************************************************************************
 * @description: 获取服务器的数据
 * @Author: Tinao
 * @Date: 2024-11-30 14:53:39
 * @param {unsigned short} timeOut ：等待的时间(乘以10ms)
 * @return {*} 平台返回的原始数据
 * 说明：不同网络设备返回的格式不同，需要去调试
 *       如ESP8266的返回格式为	"+IPD,x:yyy"	x代表数据长度，yyy是数据内容
 *********************************************************************************/
char *ESP8266_GetIPD(unsigned short TimeOut)
{
    char *PtrIPD = NULL;
    do
    {
        if (ESP8266_WaitRecive() == REV_OK) // 如果接收完成
        {
            PtrIPD = strstr((char *)Esp8266_Buf, "IPD,"); // 搜索“IPD”头
            if (PtrIPD == NULL)                           // 如果没找到，可能是IPD头的延迟，还是需要等待一会，但不会超过设定的时间
            {
            }
            else
            {
                PtrIPD = strchr(PtrIPD, ':'); // 找到':'
                if (PtrIPD != NULL)
                {
                    PtrIPD++;
                    return PtrIPD;
                }
                else
                    return NULL;
            }
        }
        Delay_ms(5); // 延时等待
        TimeOut--;
    } while (TimeOut > 0);

    return "No_Data"; // 超时还未找到，返回空指针
}

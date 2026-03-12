#include "oled.h"
#include "Iic.h"
#include "DHT11.h"
#include "Font.h"
#include "Beep.h"
#include "RC522.h"
#include <stdbool.h>
#include <stdio.h>

bool LED_STATE = true; // 标记灯的状态 是使用流水灯 还是闪烁灯

bool Temp_State = false; // 标记温度是不是超多阈值 false 没有超过
bool Hume_State = false; // 标记光照是不是超多阈值 false 没有超过

// 给OLED发送命令 初始化
void Oled_Init(void)
{
    Oled_Send_One_Byte_Cmd(0xAE); //--turn off oled panel
    Oled_Send_One_Byte_Cmd(0x00); //---set low column address
    Oled_Send_One_Byte_Cmd(0x10); //---set high column address
    Oled_Send_One_Byte_Cmd(0x40); //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    Oled_Send_One_Byte_Cmd(0x81); //--set contrast control register
    Oled_Send_One_Byte_Cmd(0xCF); // Set SEG Output Current Brightness
    Oled_Send_One_Byte_Cmd(0xA1); //--Set SEG/Column Mapping     0xa0???? 0xa1??
    Oled_Send_One_Byte_Cmd(0xC8); // Set COM/Row Scan Direction   0xc0???? 0xc8??
    Oled_Send_One_Byte_Cmd(0xA6); //--set normal display
    Oled_Send_One_Byte_Cmd(0xA8); //--set multiplex ratio(1 to 64)
    Oled_Send_One_Byte_Cmd(0x3f); //--1/64 duty
    Oled_Send_One_Byte_Cmd(0xD3); //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    Oled_Send_One_Byte_Cmd(0x00); //-not offset
    Oled_Send_One_Byte_Cmd(0xd5); //--set display clock divide ratio/oscillator frequency
    Oled_Send_One_Byte_Cmd(0x80); //--set divide ratio, Set Clock as 100 Frames/Sec
    Oled_Send_One_Byte_Cmd(0xD9); //--set pre-charge period
    Oled_Send_One_Byte_Cmd(0xF1); // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    Oled_Send_One_Byte_Cmd(0xDA); //--set com pins hardware configuration
    Oled_Send_One_Byte_Cmd(0x12);
    Oled_Send_One_Byte_Cmd(0xDB); //--set vcomh
    Oled_Send_One_Byte_Cmd(0x40); // Set VCOM Deselect Level
    Oled_Send_One_Byte_Cmd(0x20); //-Set Page Addressing Mode (0x00/0x01/0x02)
    Oled_Send_One_Byte_Cmd(0x02); //
    Oled_Send_One_Byte_Cmd(0x8D); //--set Charge Pump enable/disable
    Oled_Send_One_Byte_Cmd(0x14); //--set(0x10) disable
    Oled_Send_One_Byte_Cmd(0xA4); // Disable Entire Display On (0xa4/0xa5)
    Oled_Send_One_Byte_Cmd(0xA6); // Disable Inverse Display On (0xa6/a7)
    Oled_Send_One_Byte_Cmd(0xAF); //--turn on oled panel
    Oled_Send_One_Byte_Cmd(0xAF); /*display ON*/
    OLed_Fill(0x00);              // 缓存区数据全为0

    printf("OLED Initialized.\n");
}

/********通过IIC向OLED发送一个字节命令*********/
// 参数：cmd 需要发送的命令
// 返回值 ：0 发送成功 非0发送失败
u8 Oled_Send_One_Byte_Cmd(u8 Cmd)
{
    // 1、发送起始信号
    Software_IIC_Start();

    // 2、发送设备地址
    Software_IIC_Send_One_Byte_Data(OLED_SLAVE_ADDR); // 设备地址最低位是0 表示写操作

    // 3、等待从机响应  读应答
    if (Software_IIC_Read_ACK()) // 如果从机没有应答 直接退出
    {
        Software_IIC_Stop(); // 退出之前 发送停止信号  结束总线占用
        return 1;
    }

    // 4、指令模式
    Software_IIC_Send_One_Byte_Data(OLED_SLAVE_CMD);

    // 5、读应答
    if (Software_IIC_Read_ACK()) // 如果从机没有应答 直接退出
    {
        Software_IIC_Stop(); // 退出之前 发送停止信号  结束总线占用
        return 2;
    }

    // 6、发送指令模式
    Software_IIC_Send_One_Byte_Data(Cmd);

    // 5、读应答
    if (Software_IIC_Read_ACK()) // 如果从机没有应答 直接退出
    {
        Software_IIC_Stop(); // 退出之前 发送停止信号  结束总线占用
        return 3;
    }

    // 8、发送停止信号  结束通信
    Software_IIC_Stop();
    return 0; // 成功
}

/********通过IIC向OLED发送一个字节数据*********/
// 参数：Dat 需要发送的数据
// 返回值 ：0 发送成功 非0发送失败
u8 Oled_Send_One_Byte_DATA(u8 Data)
{
    // 1、发送起始信号
    Software_IIC_Start();

    // 2、发送设备地址
    Software_IIC_Send_One_Byte_Data(OLED_SLAVE_ADDR); // 设备地址最低位是0 表示写操作

    // 3、等待从机响应  读应答
    if (Software_IIC_Read_ACK()) // 如果从机没有应答 直接退出
    {
        Software_IIC_Stop(); // 退出之前 发送停止信号  结束总线占用
        return 1;
    }

    // 4、数据模式
    Software_IIC_Send_One_Byte_Data(OLED_SLAVE_DATA);

    // 5、读应答
    if (Software_IIC_Read_ACK()) // 如果从机没有应答 直接退出
    {
        Software_IIC_Stop(); // 退出之前 发送停止信号  结束总线占用
        return 2;
    }

    // 6、发送数据
    Software_IIC_Send_One_Byte_Data(Data);

    // 5、读应答
    if (Software_IIC_Read_ACK()) // 如果从机没有应答 直接退出
    {
        Software_IIC_Stop(); // 退出之前 发送停止信号  结束总线占用
        return 3;
    }

    // 8、发送停止信号  结束通信
    Software_IIC_Stop();
    return 0; // 成功
}

// 填充缓存区中的数据
void OLed_Fill(unsigned char Bmp_Data)
{
    unsigned char Y, X;

    for (Y = 0; Y < 8; Y++)
    {
        // 设置PAGE地址 //b0 ~ b7
        Oled_Send_One_Byte_Cmd(0xb0 + Y); // 页寻址只有最低三位有效 因为只有8页
                                          // 然后就是B开头 多以第一页起始地址为b0

        // 设置列地址
        Oled_Send_One_Byte_Cmd(0x00); // 列地址低位
        Oled_Send_One_Byte_Cmd(0x10); // 列地址高位

        for (X = 0; X < 128; X++)
        {
            Oled_Send_One_Byte_DATA(Bmp_Data);
        }
    }
}

// 取消OLED初始化
void Off_Init_OLed(void)
{
    Oled_Send_One_Byte_Cmd(0xAE); //--turn off oled panel
    Oled_Send_One_Byte_Cmd(0x00); //---set low column address
    Oled_Send_One_Byte_Cmd(0x10); //---set high column address
    Oled_Send_One_Byte_Cmd(0x40); //--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
    Oled_Send_One_Byte_Cmd(0x81); //--set contrast control register
    Oled_Send_One_Byte_Cmd(0xCF); // Set SEG Output Current Brightness
    Oled_Send_One_Byte_Cmd(0xA1); //--Set SEG/Column Mapping     0xa0???? 0xa1??
    Oled_Send_One_Byte_Cmd(0xC8); // Set COM/Row Scan Direction   0xc0???? 0xc8??
    Oled_Send_One_Byte_Cmd(0xA6); //--set normal display
    Oled_Send_One_Byte_Cmd(0xA8); //--set multiplex ratio(1 to 64)
    Oled_Send_One_Byte_Cmd(0x3f); //--1/64 duty
    Oled_Send_One_Byte_Cmd(0xD3); //-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
    Oled_Send_One_Byte_Cmd(0x00); //-not offset
    Oled_Send_One_Byte_Cmd(0xd5); //--set display clock divide ratio/oscillator frequency
    Oled_Send_One_Byte_Cmd(0x80); //--set divide ratio, Set Clock as 100 Frames/Sec
    Oled_Send_One_Byte_Cmd(0xD9); //--set pre-charge period
    Oled_Send_One_Byte_Cmd(0xF1); // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    Oled_Send_One_Byte_Cmd(0xDA); //--set com pins hardware configuration
    Oled_Send_One_Byte_Cmd(0x12);
    Oled_Send_One_Byte_Cmd(0xDB); //--set vcomh
    Oled_Send_One_Byte_Cmd(0x40); // Set VCOM Deselect Level
    Oled_Send_One_Byte_Cmd(0x20); //-Set Page Addressing Mode (0x00/0x01/0x02)
    Oled_Send_One_Byte_Cmd(0x02); //
    Oled_Send_One_Byte_Cmd(0x8D); //--set Charge Pump enable/disable
    Oled_Send_One_Byte_Cmd(0x14); //--set(0x10) disable
    Oled_Send_One_Byte_Cmd(0xA4); // Disable Entire Display On (0xa4/0xa5)
    Oled_Send_One_Byte_Cmd(0xA6); // Disable Inverse Display On (0xa6/a7)
    Oled_Send_One_Byte_Cmd(0xAF); //--turn on oled panel
}

/***********
设置显示字符位置的函数
参数：
        x--> 列地址
        y--> 页地址 相当于行的(0-7)
返回值：无
************/
void OLed_SetPos(unsigned char X, unsigned char Y)
{
    Oled_Send_One_Byte_Cmd((0xb0 + Y));               // 页的首地址 + y 偏移单位
    Oled_Send_One_Byte_Cmd(((X & 0xf0) >> 4) | 0x10); // 列地址高位
    Oled_Send_One_Byte_Cmd((X & 0x0f) | 0x00);        // 列地址低位
}

/****************
在指定位置显示一个汉字，显示一个汉字时,
参数: x	显示位置，每次递增16个bit
            y 显示位置，每次递增2页 16bit
            buf 需要显示的字符字模
返回值：无
****************/
void OLed_ShowChina(u8 X, u8 Y, u8 *Buf)
{
    u8 i = 0;
    OLed_SetPos(X, Y);
    for (i = 0; i < 16; i++)
    {
        Oled_Send_One_Byte_DATA(Buf[i]);
    }

    OLed_SetPos(X, (Y + 1));
    for (i = 0; i < 16; i++)
    {
        Oled_Send_One_Byte_DATA(Buf[i + 16]);
    }
}

// 在指定位置显示ASCLL字符 str就是一个字符串
void OLed_ShowASCII(u8 X, u8 Y, char *Str)
{
    int i = 0;

    char *Pstr = Str;

    while (*Pstr)
    {
        OLed_SetPos(X, Y);
        for (i = 0; i < 8; i++)
        {
            Oled_Send_One_Byte_DATA(F8X16[((*Pstr) - 32) * 16 + i]);
        }

        OLed_SetPos(X, Y + 1);
        for (i = 0; i < 8; i++)
        {
            Oled_Send_One_Byte_DATA(F8X16[((*Pstr) - 32) * 16 + 8 + i]);
        }

        Pstr++;

        X += 8;
    }
}

// 在指定位置显示指定长度的ASCLL字符 str就是一个字符串
void OLed_ShowASCII_Len(u8 X, u8 Y, char *Str, int Length)
{
    int i = 0, j = 0;

    char *Pstr = Str;

    while (*Pstr)
    {
        j++;
        OLed_SetPos(X, Y);
        for (i = 0; i < 8; i++)
        {
            Oled_Send_One_Byte_DATA(F8X16[((*Pstr) - 32) * 16 + i]);
        }

        OLed_SetPos(X, Y + 1);
        for (i = 0; i < 8; i++)
        {
            Oled_Send_One_Byte_DATA(F8X16[((*Pstr) - 32) * 16 + 8 + i]);
        }

        Pstr++;

        X += 8;

        if (j == Length)
            break;
    }
}

/*********************************************************************************
 * @description: 展示OLED屏幕相关信息
 * @Author: Tinao
 * @Date: 2024-11-29 19:48:42
 * @return {*} None
 *********************************************************************************/
void Show_UI(void)
{
    /*
        实时数据展示
    温度：
    湿度：
    光照：
    */
    OLed_ShowChina(16, 0, HZ10); // 智
    OLed_ShowChina(32, 0, HZ11); // 慧
    OLed_ShowChina(48, 0, HZ18); // 门
    OLed_ShowChina(64, 0, HZ19); // 禁
    OLed_ShowChina(80, 0, HZ20); // 系
    OLed_ShowChina(96, 0, HZ21); // 统

    OLed_ShowChina(0, 2, HZ111);  // 温
    OLed_ShowChina(16, 2, HZ211); // 度
    OLed_ShowChina(32, 2, HZ311); // ：

    OLed_ShowChina(80, 2, HZ32); // 摄氏度

    OLed_ShowChina(0, 4, HZ31);   // 湿
    OLed_ShowChina(16, 4, HZ211); // 度
    OLed_ShowChina(32, 4, HZ311); // ：

    OLed_ShowASCII(80, 4, "RH");

    OLed_ShowChina(0, 6, HZ33);   // 方
    OLed_ShowChina(16, 6, HZ34);  // 式
    OLed_ShowChina(32, 6, HZ311); // ：
}

/*********************************************************************************
 * @description: 实时更新数据
 * @Author: Tinao
 * @Date: 2024-11-29 21:21:01
 * @return {*} None
 *********************************************************************************/
void Update_Data_Environment(void)
{
    // 展示温度
    OLed_ShowASCII(48, 2, TEMP_DATA);

    // 展示湿度
    OLed_ShowASCII(48, 4, HUME_DATA);
}

void Update_Data_Access(char *IDStr)
{
    OLed_ShowChina(0, 6, HZ35);   // 刷
    OLed_ShowChina(16, 6, HZ36);  // 卡
    OLed_ShowChina(32, 6, HZ311); // ：

    OLed_ShowASCII(48, 6, IDStr);
}

void Update_Passwd(char *IDStr, int Length)
{
    int i;
    for (i = 0; i < 128; i++)
    {
        Oled_Send_One_Byte_DATA(0x00);
    }
    OLed_ShowChina(0, 6, HZ39);   // 密
    OLed_ShowChina(16, 6, HZ40);  // 码
    OLed_ShowChina(32, 6, HZ311); // ：

    OLed_ShowASCII_Len(48, 6, IDStr, Length);
}

void Update_Face(char *IDStr)
{
    OLed_ShowChina(0, 6, HZ37);   // 人
    OLed_ShowChina(16, 6, HZ38);  // 脸
    OLed_ShowChina(32, 6, HZ311); // ：
}

/*********************************************************************************
 * @description: 展示报警信息 以及 报警
 * @Author: Tinao
 * @Date: 2024-11-30 10:32:14
 * @param {int} Temp 温度的实时值
 * @param {int} Hume 湿度的实时值
 * @param {int} Light 光照的实时值
 * @return {*} None
 *********************************************************************************/
void Show_Warn(int Temp, int Hume, int Light)
{
    int i, j;
    if ((Temp > Temp_Threshold) || ((Hume > Hume_Threshold))) // 超过阈值 显示 ！
    {
        Eliminate_Warn(Temp_State, Hume_State);
        if (Temp > Temp_Threshold)
        {
            OLed_ShowASCII(112, 2, "!");
            Temp_State = true;
        }
        if (Hume > Hume_Threshold)
        {
            OLed_ShowASCII(112, 4, "!");
            Hume_State = true;
        }
        Beep_Control(BEEP_ON); // 开启蜂鸣器 报警
        LED_STATE = false;     // 开启闪烁灯 LED2 ~ LED4
    }
    else // 隐藏全部 ！
    {
        for (j = 2; j <= 7; j++)
        {
            OLed_SetPos(112, j);
            for (i = 115; i < 128; i++)
            {
                Oled_Send_One_Byte_DATA(0x00);
            }
        }
        Beep_Control(BEEP_OFF); // 关闭蜂鸣器
        LED_STATE = true;       // 开启流水灯 LED2 ~ LED4
    }
}

/*********************************************************************************
 * @description: 消除报警 ！
 * @Author: Tinao
 * @Date: 2024-11-30 10:31:34
 * @param {bool} Temp_State 温度的标志
 * @param {bool} Hume_State 湿度的标志
 * @return {*} None
 *********************************************************************************/
void Eliminate_Warn(bool Temp_State, bool Hume_State)
{
    int i, j;
    if (Temp_State)
    {
        for (j = 2; j <= 3; j++)
        {
            OLed_SetPos(112, j);
            for (i = 115; i < 128; i++)
            {
                Oled_Send_One_Byte_DATA(0x00);
            }
        }
        Temp_State = false;
    }
    if (Hume_State)
    {
        for (j = 4; j <= 5; j++)
        {
            OLed_SetPos(112, j);
            for (i = 115; i < 128; i++)
            {
                Oled_Send_One_Byte_DATA(0x00);
            }
        }
        Hume_State = false;
    }
    Beep_Control(BEEP_OFF); // 关闭蜂鸣器
    LED_STATE = true;       // 开启流水灯 LED2 ~ LED4
}

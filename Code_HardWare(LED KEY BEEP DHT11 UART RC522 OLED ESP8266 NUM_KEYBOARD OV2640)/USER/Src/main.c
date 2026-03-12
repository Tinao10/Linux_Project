/*********************************************************************************
 * @Description:
 * @Author: Tinao
 * @Date: 2024-11-27 17:33:50
 * @LastEditTime: 2024-11-27 20:21:46
 * @LastEditors: Tinao
 *********************************************************************************/
#include <stdio.h>
#include <string.h>
#include "main.h"
#include "Tim.h"
#include "Pwm.h"
#include "Beep.h"
#include "Led.h"
#include "Usart.h"
#include "Delay.h"
#include "Key.h"
#include "Esp8266.h"
#include "Oled.h"
#include "Iic.h"
#include "RC522.h"
#include "OV2640.h"
#include "Dcmi.h"
#include "Num_keyboard.h"

int main(void)
{
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 中断设计为组2
    Usart1_Init(115200);                            // 串口一初始化 发送到串口助手
    Uart5_Init(115200);                             // 串口四初始化 ESP8266使用
    Delay_Init(168);                                // 延迟函数使用
    //Led_Init();                                     // LED初始化
    Beep_Init();                                    // 蜂鸣器初始化
    Key_Init();                                     // 按键初始化
    RC522_Init();                                   // RFID初始化
    Num_Keyboard_Init();                            // 数字键盘的初始化
    ESP8266_TCP_Init(); // 初始化 ESP8266
		//ESP8266_UDP_Init(); // 初始化 ESP8266
    Tim14_Init(8399, 238);                          // TIM14初始化 定时50MS
    //Tim12_Init(8399, 99999);                        // TIM12初始化 定时1S
		Software_IIC_Init();     // IIC初始化	
		Oled_Init();             // OLED初始化
		Show_UI();               // 展示屏幕信息
    while (OV2640_Init()) {} // 初始化OV2640

    Jpeg_Test_Pre();
		//Jpeg_Coll_Photo();	
		//Jpeg_Coll_Voide();
	Esp_Flag = 1;
	 while (1) 
	{
			Flow_Light_Flash_Light();
		 char ReicvData[100] = {0};
		 strcpy(ReicvData, ESP8266_GetIPD(200)); // 延时 2s 接收数据
		 
//		 if (strcmp(ReicvData, "No_Data") != 0) 
//		 {
//			 printf("%s",ReicvData);
//			 ESP8266_SendData("OVER",strlen("OVER"));
//			 
//			 if(strstr(ReicvData,"PASSWD_RIGHT"))
//			 {
//					 Beep_Control(BEEP_ON);
//				 Delay_ms(5);
//				 Beep_Control(BEEP_OFF);
//					Update_Passwd("Succeed",strlen("Succeed"));
//			 }
			 if(strstr(ReicvData,"#"))
			 {
				  Fuc_TIM14_IRQ_Disable();
				 //ESP8266_SendData("FACE", strlen("FACE")); 
				 Jpeg_Coll_Photo();
				 Fuc_TIM14_IRQ_Enable(); 
				 printf("完成");
				 ESP8266_SendData("17580310662", strlen("17580310662")); 
			 }
//			 else if(strstr(ReicvData,"BEEP"))
//			 {
//				 Beep_Control(BEEP_ON);
//				 Delay_ms(5000);
//				 Beep_Control(BEEP_OFF);
//			 }
//			 else if(strstr(ReicvData,"PASSWD_ERROR"))
//			 {
//					Update_Passwd("Error",strlen("Error"));
//			 }
//			 else if(strstr(ReicvData,"CARD_RIGHT"))
//			 {
//					Beep_Control(BEEP_ON);
//				 Delay_ms(500);
//				 Beep_Control(BEEP_OFF);
//					Update_Data_Access("Succeed");
//			 }
//			 else if(strstr(ReicvData,"CARD_ERROR"))
//			 {
//					Update_Data_Access("Error");
//			 }
//			 else if(strstr(ReicvData,"FACE_RIGHT"))
//			 {
//				 Beep_Control(BEEP_ON);
//				 Delay_ms(5);
//				 Beep_Control(BEEP_OFF);
//				Update_Face("Succeed");
//			 }
//			 else if(strstr(ReicvData,"FACE_ERROR"))
//			 {
//					Update_Face("Error");
//			 }
		 //}
	 }
}

// Tim13_Init(8399, 999);                          // TIM13初始化 定时100mS
// Pwm_Led_Init();                                 // PWM呼吸灯初始化

/* 展示OLED屏幕上的文字 */
//
/* 准备连接WIFI 和 服务器 */
//

/* 接收来自服务器的数据 */

// while (1) {
// char ReicvData[100] = {0};
// strcpy(ReicvData, ESP8266_GetIPD(200)); // 延时 2s 接收数据
// if (strcmp(ReicvData, "No_Data") != 0) {
//     if(strstr(ReicvData,"Sclose_beepE"))
//     {
//         Beep_Control(BEEP_OFF);
//     }
// }
// }

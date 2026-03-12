#include "OV2640.h"
#include "OV2640cfg.h"
#include "Delay.h"
#include "Usart.h"
#include "Sccb.h"
#include "Dcmi.h"
#include "Tim.h"
#include "ESP8266.h"
#include <stdio.h>
#include <string.h>

#define CHUNK_SIZE_TCP 1024
#define CHUNK_SIZE_UDP 1024

__align(4) u32 jpeg_buf[jpeg_buf_size]; // JPEG数据缓存buf
volatile u32 jpeg_data_len = 0;			// buf中的JPEG有效数据长度
volatile u8 jpeg_data_ok = 0;			// JPEG数据采集完成标志
										// 0,数据没有采集完;
										// 1,数据采集完了,但是还没处理;
										// 2,数据已经处理完成了,可以开始下一帧接收
										
// JPEG尺寸支持列表
const u16 jpeg_img_size_tbl[][2] =
{
		176, 144,	// QCIF
		160, 120,	// QQVGA
		352, 288,	// CIF
		320, 240,	// QVGA
		640, 480,	// VGA
		800, 600,	// SVGA
		1024, 768,	// XGA
		1280, 1024, // SXGA
		1600, 1200, // UXGA
};
	
const u8 *EFFECTS_TBL[7] = {"Normal", "Negative", "B&W", "Redish", "Greenish", "Bluish", "Antique"}; // 7种特效
const u8 *JPEG_SIZE_TBL[9] = {"QCIF", "QQVGA", "CIF", "QVGA", "VGA", "SVGA", "XGA", "SXGA", "UXGA"}; // JPEG图片 9种尺寸

// 处理JPEG数据
// 当采集完一帧JPEG数据后,调用此函数,切换JPEG BUF.开始下一帧采集.
void jpeg_data_process(void)
{
	if (jpeg_data_ok == 0) // jpeg数据还未采集完?
	{
		DMA_Cmd(DMA2_Stream1, DISABLE); // 停止当前传输
		while (DMA_GetCmdStatus(DMA2_Stream1) != DISABLE)
		{
		} // 等待DMA2_Stream1可配置
		jpeg_data_len = jpeg_buf_size - DMA_GetCurrDataCounter(DMA2_Stream1); // 得到此次数据传输的长度
		jpeg_data_ok = 1;													  // 标记JPEG数据采集完按成,等待其他函数处理
	}
	if (jpeg_data_ok == 2) // 上一次的jpeg数据已经被处理了
	{
		DMA2_Stream1->NDTR = jpeg_buf_size;
		DMA_SetCurrDataCounter(DMA2_Stream1, jpeg_buf_size); // 传输长度为jpeg_buf_size*4字节
		DMA_Cmd(DMA2_Stream1, ENABLE);						 // 重新传输
		jpeg_data_ok = 0;									 // 标记数据未采集
	}
}

// 在文件开头添加以下函数定义
uint16_t swap16(uint16_t x) {
    return (x << 8) | (x >> 8);
}

uint32_t swap32(uint32_t x) {
    return ((x & 0xFF000000) >> 24) |
           ((x & 0x00FF0000) >> 8)  |
           ((x & 0x0000FF00) << 8)  |
           ((x & 0x000000FF) << 24);
}

void Jpeg_Test_Pre(void)
{
	u8 size = 3; // 默认是 QVGA 320*240 尺寸
	OV2640_JPEG_Mode();																			 // JPEG 模式
	My_DCMI_Init();																				 // DCMI 配置
	DCMI_DMA_Init((u32)&jpeg_buf, jpeg_buf_size, DMA_MemoryDataSize_Word, DMA_MemoryInc_Enable); // DCMI DMA 配置
	OV2640_OutSize_Set(jpeg_img_size_tbl[size][0], jpeg_img_size_tbl[size][1]);					 // 设置输出尺寸 (320*240)
	DCMI_Start();																				 // 启动传输
	Delay_ms(200);
	
}

void Jpeg_Coll_Photo(void)
{
	u8 *p;
	u32 i, jpgstart, jpglen;
	u8 headok = 0;
	if (jpeg_data_ok == 1) // 已经采集完一帧图像了
	{
		p = (u8 *)jpeg_buf;
		jpglen = 0; // 设置 jpg 文件大小为 0
		headok = 0; // 清除 jpg 头标记
		// 查找 JPEG 文件头 (0xFFD8) 和文件尾 (0xFFD9)
		for (i = 0; i < jpeg_data_len * 4; i++) // DMA 传输 1 次等于 4 字节，所以乘以 4
		{
			if ((p[i] == 0xFF) && (p[i + 1] == 0xD8)) // 找到 JPEG 文件头 (FF D8)
			{
				printf("HEAD RIGHT\n");
				jpgstart = i;
				headok = 1; // 标记找到 JPEG 文件头
			}
			if ((p[i] == 0xFF) && (p[i + 1] == 0xD9) && headok) // 找到 JPEG 文件尾 (FF D9)
			{
				printf("TAIL RIGHT\n");
				jpglen = i - jpgstart + 2; // 计算 JPEG 文件长度（包括头和尾）
				ESP8266_SendData("FACE", strlen("FACE")); 
				break;
			}
		}
		printf("jpglen: %d\n", jpglen);
		ESP8266_SendData("FACE", strlen("FACE")); 
		if (jpglen > 0)
		{
			p += jpgstart; // 偏移到 JPEG 数据起始位置
			int i = 0;

			while (i < jpglen)
			{
				printf("-1-1-1");
				int remaining = jpglen - i;
				int chunk_len = (remaining > CHUNK_SIZE_TCP) ? CHUNK_SIZE_TCP : remaining;

				// 避免拆分 0xFFD9
				if (chunk_len > 0 && p[i + chunk_len - 1] == 0xFF && remaining > chunk_len)
				{
					chunk_len--;
				}

				char cmd[32];
				sprintf(cmd, "AT+CIPSEND=%d\r\n", chunk_len);

				// 发送数据块
				while (!ESP8266_SendData((char *)p + i, chunk_len + strlen(cmd)))
				{
				};
				Delay_ms(5);
				// 等待 "OVER" 确认
				while (1)
				{
					printf("00000");
					char ReicvData[100] = {0};
					strcpy(ReicvData, ESP8266_GetIPD(0));
					if (strstr(ReicvData, "OVER"))
					{
						//ESP8266_SendData("OVER", strlen("OVER"));   
						printf("1111");
						break;
					}
					else
					{
						printf("11111");
						ESP8266_SendData((char *)p + i, chunk_len + strlen(cmd)); // 重发
					}
				}
				i += chunk_len; // 动态更新偏移
			}
		}
		jpeg_data_ok = 2; // 标记 JPEG 数据处理完了，可以让 DMA 去采集下一帧了
	}
}


void Jpeg_Coll_Voide(void)
{
    u8 *p;
    u32 i, jpgstart, jpglen;
    u8 headok = 0;
    static uint32_t frame_counter = 0;  // 帧计数器
    
    if (jpeg_data_ok == 1) 
    {
        p = (u8 *)jpeg_buf;
        jpglen = 0;
        headok = 0;
        
        // JPEG头尾检测（保持原逻辑）
        for (i = 0; i < jpeg_data_len * 4; i++) 
        {
            if ((p[i] == 0xFF) && (p[i + 1] == 0xD8)) 
            {
                jpgstart = i;
                headok = 1;
            }
            if ((p[i] == 0xFF) && (p[i + 1] == 0xD9) && headok)
            {
                jpglen = i - jpgstart + 2;
                break;
            }
        }
        
        if (jpglen > 0) 
        {
            p += jpgstart;
            uint16_t total_chunks = (jpglen + CHUNK_SIZE_UDP - 1) / CHUNK_SIZE_UDP;
            uint16_t chunk_num;
            // 分片发送
            for (chunk_num = 0; chunk_num < total_chunks; chunk_num++)
            {
                uint16_t chunk_len = (chunk_num == total_chunks-1) ? 
                                    (jpglen % CHUNK_SIZE_UDP) : CHUNK_SIZE_UDP;
                if (chunk_len == 0) chunk_len = CHUNK_SIZE_UDP;
                
                // 构建UDP包头（4字节帧号 + 2字节分片号 + 2字节总分片数）
                uint8_t header[8];
                *(uint32_t*)&header[0] = swap32(frame_counter);
								*(uint16_t*)&header[4] = swap16(chunk_num);
								*(uint16_t*)&header[6] = swap16(total_chunks);
                
                // 构建完整UDP包
                char send_buf[8 + CHUNK_SIZE_UDP];
                memcpy(send_buf, header, 8);
                memcpy(send_buf+8, p + chunk_num*CHUNK_SIZE_UDP, chunk_len);
                
                // 发送UDP包（AT命令）
                char cmd[64];
                sprintf(cmd, "AT+CIPSEND=%d\r\n", 8 + chunk_len);
                if(ESP8266_SendCmd(cmd, ">"))  // 设置发送长度
                {
                    USART_SendDatas(UART5, (char *)send_buf, 8 + chunk_len);
                    //ESP8266_WaitResponse("OK", 500);  // 等待发送完成
                }
            }
            frame_counter++;  // 递增帧号
        }
        jpeg_data_ok = 2;
    }
}

// 初始化OV2640
// 配置完以后,默认输出是1600*1200尺寸的图片!!
// 返回值:0,成功
u8 OV2640_Init(void)
{
	u16 i = 0;
	u16 reg;

	// 设置IO
	GPIO_InitTypeDef GPIO_InitStructure;
	// 使能时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG, ENABLE); // 使能GPIOA B C E G时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;		  // PG9,15推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	  // 推挽输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	  // 推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 100MHz
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	  // 上拉
	GPIO_Init(GPIOG, &GPIO_InitStructure);			  // 初始化

	// PWDN(PD3)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOD, &GPIO_InitStructure); // 初始化

	OV2640_PWDN = 0; // POWER ON
	Delay_ms(10);
	OV2640_RST = 0; // 复位OV2640
	Delay_ms(10);
	OV2640_RST = 1;						   // 结束复位
	SCCB_Init();						   // 初始化SCCB 的IO口
	SCCB_WR_Reg(OV2640_DSP_RA_DLMT, 0x01); // 操作sensor寄存器
	SCCB_WR_Reg(OV2640_SENSOR_COM7, 0x80); // 软复位OV2640
	Delay_ms(50);
	reg = SCCB_RD_Reg(OV2640_SENSOR_MIDH); // 读取厂家ID 高八位
	reg <<= 8;
	reg |= SCCB_RD_Reg(OV2640_SENSOR_MIDL); // 读取厂家ID 低八位
	if (reg != OV2640_MID)
	{
		printf("MID:%d\r\n", reg);
		return 1;
	}
	reg = SCCB_RD_Reg(OV2640_SENSOR_PIDH); // 读取厂家ID 高八位
	reg <<= 8;
	reg |= SCCB_RD_Reg(OV2640_SENSOR_PIDL); // 读取厂家ID 低八位
	if (reg != OV2640_PID)
	{
		printf("HID:%d\r\n", reg);
		return 2;
	}
	// 初始化 OV2640,采用SXGA分辨率(1600*1200)
	for (i = 0; i < sizeof(ov2640_sxga_init_reg_tbl) / 2; i++)
	{
		SCCB_WR_Reg(ov2640_sxga_init_reg_tbl[i][0], ov2640_sxga_init_reg_tbl[i][1]);
	}
	printf("OV2640 Initialized.\n");
	return 0x00; // ok
}

// OV2640切换为JPEG模式
void OV2640_JPEG_Mode(void)
{
	u16 i = 0;
	// 设置:YUV422格式
	for (i = 0; i < (sizeof(ov2640_yuv422_reg_tbl) / 2); i++)
	{
		SCCB_WR_Reg(ov2640_yuv422_reg_tbl[i][0], ov2640_yuv422_reg_tbl[i][1]);
	}

	// 设置:输出JPEG数据
	for (i = 0; i < (sizeof(ov2640_jpeg_reg_tbl) / 2); i++)
	{
		SCCB_WR_Reg(ov2640_jpeg_reg_tbl[i][0], ov2640_jpeg_reg_tbl[i][1]);
	}
}

// OV2640切换为RGB565模式
void OV2640_RGB565_Mode(void)
{
	u16 i = 0;
	// 设置:RGB565输出
	for (i = 0; i < (sizeof(ov2640_rgb565_reg_tbl) / 2); i++)
	{
		SCCB_WR_Reg(ov2640_rgb565_reg_tbl[i][0], ov2640_rgb565_reg_tbl[i][1]);
	}
}
// 自动曝光设置参数表,支持5个等级
const static u8 OV2640_AUTOEXPOSURE_LEVEL[5][8] =
	{
		{
			0xFF,
			0x01,
			0x24,
			0x20,
			0x25,
			0x18,
			0x26,
			0x60,
		},
		{
			0xFF,
			0x01,
			0x24,
			0x34,
			0x25,
			0x1c,
			0x26,
			0x00,
		},
		{
			0xFF,
			0x01,
			0x24,
			0x3e,
			0x25,
			0x38,
			0x26,
			0x81,
		},
		{
			0xFF,
			0x01,
			0x24,
			0x48,
			0x25,
			0x40,
			0x26,
			0x81,
		},
		{
			0xFF,
			0x01,
			0x24,
			0x58,
			0x25,
			0x50,
			0x26,
			0x92,
		},
};

// OV2640自动曝光等级设置
// level:0~4
void OV2640_Auto_Exposure(u8 level)
{
	u8 i;
	u8 *p = (u8 *)OV2640_AUTOEXPOSURE_LEVEL[level];
	for (i = 0; i < 4; i++)
	{
		SCCB_WR_Reg(p[i * 2], p[i * 2 + 1]);
	}
}
// 白平衡设置
// 0:自动
// 1:太阳sunny
// 2,阴天cloudy
// 3,办公室office
// 4,家里home
void OV2640_Light_Mode(u8 mode)
{
	u8 regccval = 0X5E; // Sunny
	u8 regcdval = 0X41;
	u8 regceval = 0X54;
	switch (mode)
	{
	case 0: // auto
		SCCB_WR_Reg(0XFF, 0X00);
		SCCB_WR_Reg(0XC7, 0X10); // AWB ON
		return;
	case 2: // cloudy
		regccval = 0X65;
		regcdval = 0X41;
		regceval = 0X4F;
		break;
	case 3: // office
		regccval = 0X52;
		regcdval = 0X41;
		regceval = 0X66;
		break;
	case 4: // home
		regccval = 0X42;
		regcdval = 0X3F;
		regceval = 0X71;
		break;
	}
	SCCB_WR_Reg(0XFF, 0X00);
	SCCB_WR_Reg(0XC7, 0X40); // AWB OFF
	SCCB_WR_Reg(0XCC, regccval);
	SCCB_WR_Reg(0XCD, regcdval);
	SCCB_WR_Reg(0XCE, regceval);
}
// 色度设置
// 0:-2
// 1:-1
// 2,0
// 3,+1
// 4,+2
void OV2640_Color_Saturation(u8 sat)
{
	u8 reg7dval = ((sat + 2) << 4) | 0X08;
	SCCB_WR_Reg(0XFF, 0X00);
	SCCB_WR_Reg(0X7C, 0X00);
	SCCB_WR_Reg(0X7D, 0X02);
	SCCB_WR_Reg(0X7C, 0X03);
	SCCB_WR_Reg(0X7D, reg7dval);
	SCCB_WR_Reg(0X7D, reg7dval);
}
// 亮度设置
// 0:(0X00)-2
// 1:(0X10)-1
// 2,(0X20) 0
// 3,(0X30)+1
// 4,(0X40)+2
void OV2640_Brightness(u8 bright)
{
	SCCB_WR_Reg(0xff, 0x00);
	SCCB_WR_Reg(0x7c, 0x00);
	SCCB_WR_Reg(0x7d, 0x04);
	SCCB_WR_Reg(0x7c, 0x09);
	SCCB_WR_Reg(0x7d, bright << 4);
	SCCB_WR_Reg(0x7d, 0x00);
}
// 对比度设置
// 0:-2
// 1:-1
// 2,0
// 3,+1
// 4,+2
void OV2640_Contrast(u8 contrast)
{
	u8 reg7d0val = 0X20; // 默认为普通模式
	u8 reg7d1val = 0X20;
	switch (contrast)
	{
	case 0: //-2
		reg7d0val = 0X18;
		reg7d1val = 0X34;
		break;
	case 1: //-1
		reg7d0val = 0X1C;
		reg7d1val = 0X2A;
		break;
	case 3: // 1
		reg7d0val = 0X24;
		reg7d1val = 0X16;
		break;
	case 4: // 2
		reg7d0val = 0X28;
		reg7d1val = 0X0C;
		break;
	}
	SCCB_WR_Reg(0xff, 0x00);
	SCCB_WR_Reg(0x7c, 0x00);
	SCCB_WR_Reg(0x7d, 0x04);
	SCCB_WR_Reg(0x7c, 0x07);
	SCCB_WR_Reg(0x7d, 0x20);
	SCCB_WR_Reg(0x7d, reg7d0val);
	SCCB_WR_Reg(0x7d, reg7d1val);
	SCCB_WR_Reg(0x7d, 0x06);
}
// 特效设置
// 0:普通模式
// 1,负片
// 2,黑白
// 3,偏红色
// 4,偏绿色
// 5,偏蓝色
// 6,复古
void OV2640_Special_Effects(u8 eft)
{
	u8 reg7d0val = 0X00; // 默认为普通模式
	u8 reg7d1val = 0X80;
	u8 reg7d2val = 0X80;
	switch (eft)
	{
	case 1: // 负片
		reg7d0val = 0X40;
		break;
	case 2: // 黑白
		reg7d0val = 0X18;
		break;
	case 3: // 偏红色
		reg7d0val = 0X18;
		reg7d1val = 0X40;
		reg7d2val = 0XC0;
		break;
	case 4: // 偏绿色
		reg7d0val = 0X18;
		reg7d1val = 0X40;
		reg7d2val = 0X40;
		break;
	case 5: // 偏蓝色
		reg7d0val = 0X18;
		reg7d1val = 0XA0;
		reg7d2val = 0X40;
		break;
	case 6: // 复古
		reg7d0val = 0X18;
		reg7d1val = 0X40;
		reg7d2val = 0XA6;
		break;
	}
	SCCB_WR_Reg(0xff, 0x00);
	SCCB_WR_Reg(0x7c, 0x00);
	SCCB_WR_Reg(0x7d, reg7d0val);
	SCCB_WR_Reg(0x7c, 0x05);
	SCCB_WR_Reg(0x7d, reg7d1val);
	SCCB_WR_Reg(0x7d, reg7d2val);
}
// 彩条测试
// sw:0,关闭彩条
//    1,开启彩条(注意OV2640的彩条是叠加在图像上面的)
void OV2640_Color_Bar(u8 sw)
{
	u8 reg;
	SCCB_WR_Reg(0XFF, 0X01);
	reg = SCCB_RD_Reg(0X12);
	reg &= ~(1 << 1);
	if (sw)
		reg |= 1 << 1;
	SCCB_WR_Reg(0X12, reg);
}
// 设置图像输出窗口
// sx,sy,起始地址
// width,height:宽度(对应:horizontal)和高度(对应:vertical)
void OV2640_Window_Set(u16 sx, u16 sy, u16 width, u16 height)
{
	u16 endx;
	u16 endy;
	u8 temp;
	endx = sx + width / 2; // V*2
	endy = sy + height / 2;

	SCCB_WR_Reg(0XFF, 0X01);
	temp = SCCB_RD_Reg(0X03); // 读取Vref之前的值
	temp &= 0XF0;
	temp |= ((endy & 0X03) << 2) | (sy & 0X03);
	SCCB_WR_Reg(0X03, temp);	  // 设置Vref的start和end的最低2位
	SCCB_WR_Reg(0X19, sy >> 2);	  // 设置Vref的start高8位
	SCCB_WR_Reg(0X1A, endy >> 2); // 设置Vref的end的高8位

	temp = SCCB_RD_Reg(0X32); // 读取Href之前的值
	temp &= 0XC0;
	temp |= ((endx & 0X07) << 3) | (sx & 0X07);
	SCCB_WR_Reg(0X32, temp);	  // 设置Href的start和end的最低3位
	SCCB_WR_Reg(0X17, sx >> 3);	  // 设置Href的start高8位
	SCCB_WR_Reg(0X18, endx >> 3); // 设置Href的end的高8位
}
// 设置图像输出大小
// OV2640输出图像的大小(分辨率),完全由改函数确定
// width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
// 返回值:0,设置成功
//     其他,设置失败
u8 OV2640_OutSize_Set(u16 width, u16 height)
{
	u16 outh;
	u16 outw;
	u8 temp;
	if (width % 4)
		return 1;
	if (height % 4)
		return 2;
	outw = width / 4;
	outh = height / 4;
	SCCB_WR_Reg(0XFF, 0X00);
	SCCB_WR_Reg(0XE0, 0X04);
	SCCB_WR_Reg(0X5A, outw & 0XFF); // 设置OUTW的低八位
	SCCB_WR_Reg(0X5B, outh & 0XFF); // 设置OUTH的低八位
	temp = (outw >> 8) & 0X03;
	temp |= (outh >> 6) & 0X04;
	SCCB_WR_Reg(0X5C, temp); // 设置OUTH/OUTW的高位
	SCCB_WR_Reg(0XE0, 0X00);
	return 0;
}
// 设置图像开窗大小
// 由:OV2640_ImageSize_Set确定传感器输出分辨率从大小.
// 该函数则在这个范围上面进行开窗,用于OV2640_OutSize_Set的输出
// 注意:本函数的宽度和高度,必须大于等于OV2640_OutSize_Set函数的宽度和高度
//      OV2640_OutSize_Set设置的宽度和高度,根据本函数设置的宽度和高度,由DSP
//      自动计算缩放比例,输出给外部设备.
// width,height:宽度(对应:horizontal)和高度(对应:vertical),width和height必须是4的倍数
// 返回值:0,设置成功
//     其他,设置失败
u8 OV2640_ImageWin_Set(u16 offx, u16 offy, u16 width, u16 height)
{
	u16 hsize;
	u16 vsize;
	u8 temp;
	if (width % 4)
		return 1;
	if (height % 4)
		return 2;
	hsize = width / 4;
	vsize = height / 4;
	SCCB_WR_Reg(0XFF, 0X00);
	SCCB_WR_Reg(0XE0, 0X04);
	SCCB_WR_Reg(0X51, hsize & 0XFF); // 设置H_SIZE的低八位
	SCCB_WR_Reg(0X52, vsize & 0XFF); // 设置V_SIZE的低八位
	SCCB_WR_Reg(0X53, offx & 0XFF);	 // 设置offx的低八位
	SCCB_WR_Reg(0X54, offy & 0XFF);	 // 设置offy的低八位
	temp = (vsize >> 1) & 0X80;
	temp |= (offy >> 4) & 0X70;
	temp |= (hsize >> 5) & 0X08;
	temp |= (offx >> 8) & 0X07;
	SCCB_WR_Reg(0X55, temp);				// 设置H_SIZE/V_SIZE/OFFX,OFFY的高位
	SCCB_WR_Reg(0X57, (hsize >> 2) & 0X80); // 设置H_SIZE/V_SIZE/OFFX,OFFY的高位
	SCCB_WR_Reg(0XE0, 0X00);
	return 0;
}
// 该函数设置图像尺寸大小,也就是所选格式的输出分辨率
// UXGA:1600*1200,SVGA:800*600,CIF:352*288
// width,height:图像宽度和图像高度
// 返回值:0,设置成功
//     其他,设置失败
u8 OV2640_ImageSize_Set(u16 width, u16 height)
{
	u8 temp;
	SCCB_WR_Reg(0XFF, 0X00);
	SCCB_WR_Reg(0XE0, 0X04);
	SCCB_WR_Reg(0XC0, (width) >> 3 & 0XFF);	 // 设置HSIZE的10:3位
	SCCB_WR_Reg(0XC1, (height) >> 3 & 0XFF); // 设置VSIZE的10:3位
	temp = (width & 0X07) << 3;
	temp |= height & 0X07;
	temp |= (width >> 4) & 0X80;
	SCCB_WR_Reg(0X8C, temp);
	SCCB_WR_Reg(0XE0, 0X00);
	return 0;
}

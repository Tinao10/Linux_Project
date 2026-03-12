#include "RC522.h"
#include "Delay.h"
#include <stdio.h>

char RawCardID[MFRC522_MAX_LEN]; // 存储从MFRC522模块读取到的卡片ID原始数据

void RC522_Init(void)
{
	// 使能 GPIO 时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);
	// 初始化 PA15 (NSS)
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;		  // PA15 端口配置
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;	  // 推挽
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	  // 输出模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // IO口速度为 50MHz
	GPIO_Init(GPIOA, &GPIO_InitStructure);			  // 初始化 GPIOA15
	GPIO_SetBits(GPIOA, GPIO_Pin_15);				  // 初始状态为高电平（未选中）
	// 初始化 PB3 (SCK), PB4 (MISO), PB5 (MOSI)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_5;	// SCK, MOSI
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			// 复用模式
	GPIO_Init(GPIOB, &GPIO_InitStructure);					// 初始化 GPIOB3, GPIOB5
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1); // 复用模式配置
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1); // 复用模式配置
	// 初始化 PB4 (MISO)
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;				// MISO
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;			// 复用模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		// 悬浮引脚
	GPIO_Init(GPIOB, &GPIO_InitStructure);					// 初始化 GPIOB4
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1); // 复用模式配置
	// 使能 SPI 时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE); // 使能 SPI1 时钟
	// SPI 配置初始化
	SPI_InitTypeDef SPI_InitStructure;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex; // 全双工
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;					   // 主模式
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;				   // 8位数据
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;						   // 时钟空闲时为低电平
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;					   // 第1个边沿采样
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;						   // 软件片选
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8; // 降低 SPI 时钟频率
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;				   // MSB在前
	SPI_InitStructure.SPI_CRCPolynomial = 7;						   // CRC多项式
	SPI_Init(SPI1, &SPI_InitStructure);								   // 初始化 SPI1
	SPI_Cmd(SPI1, ENABLE);											   // 使能 SPI1
	// 复位 RC522，确保其处于已知状态
	GPIO_ResetBits(GPIOA, GPIO_Pin_15); // 低电平复位
	Delay_ms(50);						// 延长复位时间
	GPIO_SetBits(GPIOA, GPIO_Pin_15);	// 复位结束，拉高电平
	MFRC522_Init();						// 初始化 RC522
	RC522_EXIT_Interrupt();				// 配置外部中断
	// 打印调试信息
	printf("RC522 Initialized.\n");
}

void RC522_EXIT_Interrupt(void)
{
	// 使能 GPIOF 时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	// 配置 PF3 为输入模式
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	  // 输入模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	  // 上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // 设置速度
	GPIO_Init(GPIOF, &GPIO_InitStructure);
	// 使能 SYSCFG 时钟
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	// 配置 PF3 为外部中断源
	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOF, EXTI_PinSource3);
	// 配置 EXTI 中断
	EXTI_InitTypeDef EXTI_InitStructure;
	EXTI_InitStructure.EXTI_Line = EXTI_Line3;				// EXTI 线 3
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;		// 中断模式
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;				// 使能 EXTI 线
	EXTI_Init(&EXTI_InitStructure);
	// 配置 NVIC 中断优先级
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = EXTI3_IRQn;			 // EXTI 线 3 中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00; // 抢占优先级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;		 // 子优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				 // 使能中断
	NVIC_Init(&NVIC_InitStructure);
}

// void EXTI3_IRQHandler(void)
//{
//     if (EXTI_GetITStatus(EXTI_Line3) != RESET) // 检查是否发生中断
//     {
//         // 清除中断标志
//         EXTI_ClearITPendingBit(EXTI_Line3);
//     }
// }

uint8_t spi_master_send_recv_byte(uint8_t spi_byte)
{

	uint8_t ByteSend, ByteRecv;
	ByteSend = spi_byte;
	while (RESET == SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE))
		;
	SPI_I2S_SendData(SPI1, ByteSend);
	while (RESET == SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE))
		;
	ByteRecv = SPI_I2S_ReceiveData(SPI1);
	return ByteRecv;
}

uint8_t SPI1SendByte(uint8_t data)
{
	unsigned char writeCommand[1];
	unsigned char readValue[1];

	writeCommand[0] = data;

	readValue[0] = spi_master_send_recv_byte(writeCommand[0]);
	return readValue[0];
}

void SPI1_WriteReg(uint8_t address, uint8_t value)
{
	cs_reset();
	Delay_ms(1);
	SPI1SendByte(address);
	SPI1SendByte(value);
	cs_set();
	Delay_ms(1);
}

uint8_t SPI1_ReadReg(uint8_t address)
{
	uint8_t val;

	cs_reset();
	Delay_ms(1);
	SPI1SendByte(address);
	val = SPI1SendByte(0x00);
	cs_set();
	Delay_ms(1);
	return val;
}

void MFRC522_WriteRegister(uint8_t addr, uint8_t val)
{
	addr = (addr << 1) & 0x7E; // Address format: 0XXXXXX0
	SPI1_WriteReg(addr, val);
}

uint8_t MFRC522_ReadRegister(uint8_t addr)
{
	uint8_t val;

	addr = ((addr << 1) & 0x7E) | 0x80;
	val = SPI1_ReadReg(addr);
	return val;
}

uint8_t MFRC522_Check(uint8_t *id)
{
	uint8_t status;
	int i;
	status = MFRC522_Request(PICC_REQIDL, id); // Find cards, return card type
	if (status == MI_OK)
	{
		printf("Card found.\n");
		status = MFRC522_Anticoll(id); // Card detected. Anti-collision, return card serial number 4 bytes
		if (status == MI_OK)
		{
			printf("Card ID: ");
			for (i = 0; i < 4; i++)
			{
				printf("%02X ", id[i]); // Print card ID in hex
			}
			printf("\n");
		}
		else
		{
			printf("Failed to get card ID.\n");
		}
	}
	else
	{
		printf("No card detected.\n");
	}
	MFRC522_Halt(); // Command card into hibernation
	return status;
}

uint8_t MFRC522_Compare(uint8_t *CardID, uint8_t *CompareID)
{
	uint8_t i;
	for (i = 0; i < 5; i++)
	{
		if (CardID[i] != CompareID[i])
			return MI_ERR;
	}
	return MI_OK;
}

void MFRC522_SetBitMask(uint8_t reg, uint8_t mask)
{
	MFRC522_WriteRegister(reg, MFRC522_ReadRegister(reg) | mask);
}

void MFRC522_ClearBitMask(uint8_t reg, uint8_t mask)
{
	MFRC522_WriteRegister(reg, MFRC522_ReadRegister(reg) & (~mask));
}

uint8_t MFRC522_Request(uint8_t reqMode, uint8_t *TagType)
{
	uint8_t status;
	uint16_t backBits; // The received data bits

	MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x07); // TxLastBists = BitFramingReg[2..0]
	TagType[0] = reqMode;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits);
	if ((status != MI_OK) || (backBits != 0x10))
		status = MI_ERR;

	return status;
}

uint8_t MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint16_t *backLen)
{
	uint8_t status = MI_ERR;
	uint8_t irqEn = 0x00;
	uint8_t waitIRq = 0x00;
	uint8_t lastBits;
	uint8_t n;
	uint16_t i;

	switch (command)
	{
	case PCD_AUTHENT:
	{
		irqEn = 0x12;
		waitIRq = 0x10;
		break;
	}
	case PCD_TRANSCEIVE:
	{
		irqEn = 0x77;
		waitIRq = 0x30;
		break;
	}
	default:
		break;
	}

	MFRC522_WriteRegister(MFRC522_REG_COMM_IE_N, irqEn | 0x80);
	MFRC522_ClearBitMask(MFRC522_REG_COMM_IRQ, 0x80);
	MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80);
	MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_IDLE);

	// Writing data to the FIFO
	for (i = 0; i < sendLen; i++)
		MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, sendData[i]);

	// Execute the command
	MFRC522_WriteRegister(MFRC522_REG_COMMAND, command);
	if (command == PCD_TRANSCEIVE)
		MFRC522_SetBitMask(MFRC522_REG_BIT_FRAMING, 0x80); // StartSend=1,transmission of data starts

	// Waiting to receive data to complete
	i = 2000; // i according to the clock frequency adjustment, the operator M1 card maximum waiting time 25ms
	do
	{
		// CommIrqReg[7..0]
		// Set1 TxIRq RxIRq IdleIRq HiAlerIRq LoAlertIRq ErrIRq TimerIRq
		n = MFRC522_ReadRegister(MFRC522_REG_COMM_IRQ);
		i--;
	} while ((i != 0) && !(n & 0x01) && !(n & waitIRq));

	MFRC522_ClearBitMask(MFRC522_REG_BIT_FRAMING, 0x80); // StartSend=0

	if (i != 0)
	{
		if (!(MFRC522_ReadRegister(MFRC522_REG_ERROR) & 0x1B))
		{
			status = MI_OK;
			if (n & irqEn & 0x01)
				status = MI_NOTAGERR;
			if (command == PCD_TRANSCEIVE)
			{
				n = MFRC522_ReadRegister(MFRC522_REG_FIFO_LEVEL);
				lastBits = MFRC522_ReadRegister(MFRC522_REG_CONTROL) & 0x07;
				if (lastBits)
					*backLen = (n - 1) * 8 + lastBits;
				else
					*backLen = n * 8;
				if (n == 0)
					n = 1;
				if (n > MFRC522_MAX_LEN)
					n = MFRC522_MAX_LEN;
				for (i = 0; i < n; i++)
					backData[i] = MFRC522_ReadRegister(MFRC522_REG_FIFO_DATA); // Reading the received data in FIFO
			}
		}
		else
			status = MI_ERR;
	}
	return status;
}

uint8_t MFRC522_Anticoll(uint8_t *serNum)
{
	uint8_t status;
	uint8_t i;
	uint8_t serNumCheck = 0;
	uint16_t unLen;

	MFRC522_WriteRegister(MFRC522_REG_BIT_FRAMING, 0x00); // TxLastBists = BitFramingReg[2..0]
	serNum[0] = PICC_ANTICOLL;
	serNum[1] = 0x20;
	status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, serNum, &unLen);
	if (status == MI_OK)
	{
		// Check card serial number
		for (i = 0; i < 4; i++)
			serNumCheck ^= serNum[i];
		if (serNumCheck != serNum[i])
			status = MI_ERR;
	}
	return status;
}

void MFRC522_CalculateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData)
{
	uint8_t i, n;

	MFRC522_ClearBitMask(MFRC522_REG_DIV_IRQ, 0x04);  // CRCIrq = 0
	MFRC522_SetBitMask(MFRC522_REG_FIFO_LEVEL, 0x80); // Clear the FIFO pointer
	// Write_MFRC522(CommandReg, PCD_IDLE);

	// Writing data to the FIFO
	for (i = 0; i < len; i++)
		MFRC522_WriteRegister(MFRC522_REG_FIFO_DATA, *(pIndata + i));
	MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_CALCCRC);

	// Wait CRC calculation is complete
	i = 0xFF;
	do
	{
		n = MFRC522_ReadRegister(MFRC522_REG_DIV_IRQ);
		i--;
	} while ((i != 0) && !(n & 0x04)); // CRCIrq = 1

	// Read CRC calculation result
	pOutData[0] = MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_L);
	pOutData[1] = MFRC522_ReadRegister(MFRC522_REG_CRC_RESULT_M);
}

uint8_t MFRC522_SelectTag(uint8_t *serNum)
{
	uint8_t i;
	uint8_t status;
	uint8_t size;
	uint16_t recvBits;
	uint8_t buffer[9];

	buffer[0] = PICC_SElECTTAG;
	buffer[1] = 0x70;
	for (i = 0; i < 5; i++)
		buffer[i + 2] = *(serNum + i);
	MFRC522_CalculateCRC(buffer, 7, &buffer[7]); //??
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
	if ((status == MI_OK) && (recvBits == 0x18))
		size = buffer[0];
	else
		size = 0;
	return size;
}

uint8_t MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t *Sectorkey, uint8_t *serNum)
{
	uint8_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[12];

	// Verify the command block address + sector + password + card serial number
	buff[0] = authMode;
	buff[1] = BlockAddr;
	for (i = 0; i < 6; i++)
		buff[i + 2] = *(Sectorkey + i);
	for (i = 0; i < 4; i++)
		buff[i + 8] = *(serNum + i);
	status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
	if ((status != MI_OK) || (!(MFRC522_ReadRegister(MFRC522_REG_STATUS2) & 0x08)))
		status = MI_ERR;
	return status;
}

uint8_t MFRC522_Read(uint8_t blockAddr, uint8_t *recvData)
{
	uint8_t status;
	uint16_t unLen;

	recvData[0] = PICC_READ;
	recvData[1] = blockAddr;
	MFRC522_CalculateCRC(recvData, 2, &recvData[2]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
	if ((status != MI_OK) || (unLen != 0x90))
		status = MI_ERR;
	return status;
}

uint8_t MFRC522_Write(uint8_t blockAddr, uint8_t *writeData)
{
	uint8_t status;
	uint16_t recvBits;
	uint8_t i;
	uint8_t buff[18];

	buff[0] = PICC_WRITE;
	buff[1] = blockAddr;
	MFRC522_CalculateCRC(buff, 2, &buff[2]);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);
	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
		status = MI_ERR;
	if (status == MI_OK)
	{
		// Data to the FIFO write 16Byte
		for (i = 0; i < 16; i++)
			buff[i] = *(writeData + i);
		MFRC522_CalculateCRC(buff, 16, &buff[16]);
		status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);
		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
			status = MI_ERR;
	}
	return status;
}

void MFRC522_Init(void)
{
	MFRC522_Reset();									   // 复位 RC522
	MFRC522_WriteRegister(MFRC522_REG_T_MODE, 0x8D);	   // 定时器模式
	MFRC522_WriteRegister(MFRC522_REG_T_PRESCALER, 0x3E);  // 定时器预分频
	MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_L, 30);	   // 定时器重载值低字节
	MFRC522_WriteRegister(MFRC522_REG_T_RELOAD_H, 0);	   // 定时器重载值高字节
	MFRC522_WriteRegister(MFRC522_REG_RF_CFG, 0x7F);	   // 48dB 增益
	MFRC522_WriteRegister(MFRC522_REG_RX_THRESHOLD, 0x84); // 接收阈值
	MFRC522_WriteRegister(MFRC522_REG_TX_AUTO, 0x40);	   // 自动发送
	MFRC522_WriteRegister(MFRC522_REG_MODE, 0x3D);		   // 模式配置

	// 使能中断
	MFRC522_WriteRegister(MFRC522_REG_COMM_I_EN, 0xA0); // 启用接收中断
	MFRC522_WriteRegister(MFRC522_REG_DIV_I_EN, 0x80);	// 关闭CRC中断
	MFRC522_WriteRegister(MFRC522_REG_COM_IRQ, 0x7F);	// 清除所有中断标志

	MFRC522_AntennaOn(); // 开启天线
}

void MFRC522_Reset(void)
{
	MFRC522_WriteRegister(MFRC522_REG_COMMAND, PCD_RESETPHASE);
}

void MFRC522_AntennaOn(void)
{
	uint8_t temp;

	temp = MFRC522_ReadRegister(MFRC522_REG_TX_CONTROL);
	if (!(temp & 0x03))
		MFRC522_SetBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}

void MFRC522_AntennaOff(void)
{
	MFRC522_ClearBitMask(MFRC522_REG_TX_CONTROL, 0x03);
}

void MFRC522_Halt(void)
{
	uint16_t unLen;
	uint8_t buff[4];

	buff[0] = PICC_HALT;
	buff[1] = 0;
	MFRC522_CalculateCRC(buff, 2, &buff[2]);
	MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &unLen);
}

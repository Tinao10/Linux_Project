#include "Adc.h"
#include "Delay.h"
#include <stdio.h>

char LIGHT_DATA[5] = {0}; // 存储光照值

int Light_Threshold = 92; // 光照的阈值 超过报警

/*********************************************************************************
 * @description: 光敏电阻初始化
 * @Author: Tinao
 * @Date: 2024-11-28 10:43:39
 * @return {*} None
 *********************************************************************************/
void Adc_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	ADC_CommonInitTypeDef ADC_CommonInitStruct;
	ADC_InitTypeDef ADC_InitStruct;

	// PF7--配置成模拟功能
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN; // 模拟输入功能
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOF, &GPIO_InitStruct);

	// 2.常用功能初始化
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

	ADC_CommonInitStruct.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStruct.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStruct.ADC_Prescaler = ADC_Prescaler_Div4;
	ADC_CommonInitStruct.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_20Cycles;
	ADC_CommonInit(&ADC_CommonInitStruct);

	// 3.ADC初始化
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_Ext_IT11;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None; // 软件触发
	ADC_InitStruct.ADC_NbrOfConversion = 1;
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStruct.ADC_ScanConvMode = DISABLE;
	ADC_Init(ADC3, &ADC_InitStruct);

	// 4.ADC采样通道设置
	ADC_RegularChannelConfig(ADC3, ADC_Channel_5, 1, ADC_SampleTime_15Cycles);

	// 5.使能ADC
	ADC_Cmd(ADC3, ENABLE);
}

/*********************************************************************************
 * @description: 光敏电阻显示数据
 * @Author: Tinao
 * @Date: 2024-11-28 10:44:00
 * @return {*} None
 *********************************************************************************/
void ADC_Show(void)
{
	ADC_SoftwareStartConv(ADC3);
	while (ADC_GetFlagStatus(ADC3, ADC_FLAG_EOC) == RESET)
		;
	ADC_ClearFlag(ADC3, ADC_FLAG_EOC);
	printf("Light: %dLUX ,", 100 - ADC_GetConversionValue(ADC3) / 40); /* 模拟量转化为数据量 */
	sprintf(LIGHT_DATA, "%d.0", 100 - ADC_GetConversionValue(ADC3) / 40);
}

#include <stdio.h>
#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
#include "font.h"
#include "lcd.h"
#include "lcd.c"
#include "touch.h"
#include "touch.c"

int color[12] = {WHITE,CYAN,BLUE,RED,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};
int flag = 1;
volatile uint32_t ADC_Value[1];

/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);
void NVIC_Configure(void);

//---------------------------------------------------------------------------------------------------

void RCC_Configure(void)
{
   /* Alternate Function IO clock enable */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE); // ADC1, port C RCC ENABLE
   RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
   // ������ LCD�� �ʿ��� ��Ʈ���� lcd.c���� Ȱ��ȭ �ȴ�.
}

void GPIO_Configure(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // �Ƴ��α� �Է� ����
    GPIO_Init(GPIOC, &GPIO_InitStructure); // C0��Ʈ Ȱ��ȭ, ���������� PC0�� ������ ��ȹ
}

/**
  * @brief  Fills each DMA_InitStruct member with its default value.
  * @param  DMA_InitStruct : pointer to a DMA_InitTypeDef structure which will
  *         be initialized.
  * @retval None
  */
void DMA_Configure(void)
{
   DMA_InitTypeDef DMA_InitStruct;
/*-------------- Reset DMA init structure parameters values ------------------*/
  /* Initialize the DMA_PeripheralBaseAddr member */
  DMA_InitStruct.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
  /* Initialize the DMA_MemoryBaseAddr member */
  DMA_InitStruct.DMA_MemoryBaseAddr = (uint32_t)&ADC_Value[0];
  /* Initialize the DMA_DIR member */
  DMA_InitStruct.DMA_DIR = DMA_DIR_PeripheralSRC;
  /* Initialize the DMA_BufferSize member */
  DMA_InitStruct.DMA_BufferSize = 1;
  /* Initialize the DMA_PeripheralInc member */
  DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  /* Initialize the DMA_MemoryInc member */
  DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Disable;
  /* Initialize the DMA_PeripheralDataSize member */
  DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Word;
  /* Initialize the DMA_MemoryDataSize member */
  DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Word;
  /* Initialize the DMA_Mode member */
  DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
  /* Initialize the DMA_Priority member */
  DMA_InitStruct.DMA_Priority = DMA_Priority_High;
  /* Initialize the DMA_M2M member */
  DMA_InitStruct.DMA_M2M = DMA_M2M_Disable;
  
  DMA_Init(DMA1_Channel1, &DMA_InitStruct);
   DMA_Cmd(DMA1_Channel1, ENABLE);
}

void ADC_Configure(void)
{
	// Analog to Digital Converter
   ADC_InitTypeDef ADC_InitStruct;

   //ADC_StructInit(&ADC_InitStruct);
   ADC_InitStruct.ADC_Mode = ADC_Mode_Independent; // slave-master�� ���� ���� ADC
   ADC_InitStruct.ADC_ScanConvMode = DISABLE; // ����ä���̹Ƿ� ��Ȱ��ȭ
   ADC_InitStruct.ADC_ContinuousConvMode = ENABLE; // �� ���� Ʈ���ŷ� �� ä���� ���ø� ����
   ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // �ܺ� �Է��ɿ� ���� Ʈ���� ��Ȱ��ȭ
   ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right; // Default
   ADC_InitStruct.ADC_NbrOfChannel = 1; // ä���� �ϳ�
   
   ADC_Init(ADC1,&ADC_InitStruct); // �� ������ ADC1�� ����
   
   ADC_RegularChannelConfig(ADC1,ADC_Channel_10,1,ADC_SampleTime_28Cycles5); // ä�� �켱���� ����, 10ä�� �ܵ����
   
   // ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE); // ���к��� �ڵ鷯 ȣ�� ������ ���� �ּ�ó��
   
   ADC_Cmd(ADC1,ENABLE); // ADC1 Ȱ��ȭ
   
   //Calibration reset & start
   ADC_ResetCalibration(ADC1);
   
   while(ADC_GetResetCalibrationStatus(ADC1));
   
   ADC_StartCalibration(ADC1);
   
   while(ADC_GetCalibrationStatus(ADC1));
   
   ADC_SoftwareStartConvCmd(ADC1,ENABLE); 

   
   ADC_DMACmd(ADC1, ENABLE); // DMA
}


int main(void)
{

    SystemInit();
    RCC_Configure();
    GPIO_Configure();
    ADC_Configure();

    
    DMA_Configure();

    //-----------------

    LCD_Init(); // LCD �ʱ�ȭ
   //  Touch_Configuration(); // ��ġ ����
   //  Touch_Adjust(); // ȭ�� ��ġ ���� ���߱�
    LCD_Clear(WHITE); // LCD ��� �ʱ�ȭ

    //-----------------


    while (1) {
       if (flag && ADC_Value[0] > 2000) {
         LCD_Clear(WHITE); // LCD ��� �ʱ�ȭ
         flag = 0;
       } else if (!flag && ADC_Value[0] <= 2000) {
         LCD_Clear(GRAY); // LCD ��� �ʱ�ȭ
         flag = 1;
       }
          LCD_ShowNum(40, 100, ADC_Value[0],4, BLUE, WHITE);
    }
    return 0;
}

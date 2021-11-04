#include <stdio.h>
#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"
#include "font.h"
#include "lcd.h"
#include "lcd.c"
#include "touch.h"
#include "touch.c"

int color[12] = {WHITE,CYAN,BLUE,RED,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};
uint16_t value = 100;

/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);
void NVIC_Configure(void);

//---------------------------------------------------------------------------------------------------

void RCC_Configure(void)
{
   // TODO: Enable the APB2 peripheral clock using the function 'RCC_APB2PeriphClockCmd'
   /* Alternate Function IO clock enable */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE); // ADC1, port C
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);

   
}

void GPIO_Configure(void)
{
    // TODO: Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' and the function 'GPIO_Init'

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
   
}

void ADC_Configure(void)
{
    ADC_InitTypeDef ADC_InitStruct;

   //ADC_StructInit(&ADC_InitStruct);
   ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;
   ADC_InitStruct.ADC_ScanConvMode = DISABLE;
   ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;
   ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
   ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right;
   ADC_InitStruct.ADC_NbrOfChannel=1;
   
   ADC_Init(ADC1,&ADC_InitStruct);
   
   ADC_RegularChannelConfig(ADC1,ADC_Channel_10,1,ADC_SampleTime_28Cycles5);
   
   // ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE);
   
   ADC_Cmd(ADC1,ENABLE);
   
   ADC_ResetCalibration(ADC1);
   
   while(ADC_GetResetCalibrationStatus(ADC1));
   
   ADC_StartCalibration(ADC1);
   
   while(ADC_GetCalibrationStatus(ADC1));
   
   ADC_SoftwareStartConvCmd(ADC1,ENABLE);

}

void NVIC_Configure(void) {
    
   // TODO: Initialize the NVIC using the structure 'NVIC_InitTypeDef' and the function 'NVIC_Init'
   NVIC_InitTypeDef NVIC_InitStructure;

    // TODO: fill the arg you want
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void ADC1_2_IRQHandler(void){
  // printf("Handle\n");
   if(ADC_GetITStatus(ADC1,ADC_IT_EOC) != RESET){
      value = ADC_GetConversionValue(ADC1);
      ADC_ClearITPendingBit(ADC1,ADC_IT_EOC);
   }  
}

int main(void)
{

    SystemInit();

    RCC_Configure();

    GPIO_Configure();
    
    ADC_Configure();
    
    NVIC_Configure();

    //-----------------

    LCD_Init();
    Touch_Configuration();
    Touch_Adjust();
    LCD_Clear(WHITE);

    uint16_t pos_temp[2];
    
    while (1) {
      LCD_ShowString(40, 40, "MON_Team02", MAGENTA, WHITE);
      
      Touch_GetXY(&pos_temp[0], &pos_temp[1], 1);
      Convert_Pos(pos_temp[0],pos_temp[1],&pos_temp[0],&pos_temp[1]);
      Draw_Big_Point(pos_temp[0],pos_temp[1]);
      
      LCD_ShowNum(40, 60, (u32)pos_temp[0],3, BLUE, WHITE);
      LCD_ShowNum(40, 80, (u32)pos_temp[1],3, BLUE, WHITE);
      
      ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE);
      LCD_ShowNum(40, 100, value,4, BLUE, WHITE);
      ADC_ITConfig(ADC1,ADC_IT_EOC,DISABLE);
    }
    return 0;
}

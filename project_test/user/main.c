#include <stdio.h>
#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"
// #include "font.h"
#include "lcd.h"
// #include "lcd.c"

#define CHANNEL_NUM 8
#define ARRAYSIZE CHANNEL_NUM * 4
#define ADC1_DR ((uint32_t)0x4001244C)
#define Cycles ADC_SampleTime_13Cycles5
//#define Cycles ADC_SampleTime_41Cycles5

volatile uint16_t ADC_values[ARRAYSIZE];
static volatile uint32_t __status = 0;

// 4000보다 크면 흔들림, 200보다 작으면 안흔들림

/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);
void NVIC_Configure(void);
void ADC_Configure(void);
void DMA_Configure(void);

//---------------------------------------------------------------------------------------------------

void RCC_Configure(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1  | 
                           RCC_APB2Periph_GPIOA | 
                           RCC_APB2Periph_GPIOC | 
                           RCC_APB2Periph_GPIOE, ENABLE);
}

void GPIO_Configure(void)
{
    //--Enable ADC1 and GPIOA--
    GPIO_InitTypeDef GPIOA_InitStructure,
                     GPIOC_InitStructure,
                     GPIOE_InitStructure; //Variable used to setup the GPIO pins

    // 가속도 센서
    GPIO_StructInit(&GPIOA_InitStructure);
    GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_4 | GPIO_Pin_6;
    GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIOA_InitStructure);

    // 수위 센서
    GPIO_StructInit(&GPIOC_InitStructure);
    GPIOC_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIOC_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIOC_InitStructure);
    
    // 온습도 센서
    GPIO_StructInit(&GPIOC_InitStructure);
    GPIOC_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIOC_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIOC_InitStructure);
    
    // 토양습도 센서
    GPIO_StructInit(&GPIOC_InitStructure);
    GPIOC_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIOC_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIOC_InitStructure);
    
    // 진동 센서
    GPIO_StructInit(&GPIOC_InitStructure);
    GPIOC_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIOC_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIOC_InitStructure);
    
    // // 모터
    // GPIO_StructInit(&GPIOE_InitStructure);
    // GPIOE_InitStructure.GPIO_Pin = GPIO_Pin_0;
    // GPIOE_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    // GPIO_Init(GPIOA, &GPIOE_InitStructure);

    /* UART2 pin setting - Bluetooth 모듈 */
    //TX a2
    GPIO_StructInit(&GPIOA_InitStructure);
    GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIOA_InitStructure); 
   
    //RX a3
    GPIO_StructInit(&GPIOA_InitStructure);
    GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIOA_InitStructure.GPIO_Mode = (GPIO_Mode_IPD) | (GPIO_Mode_IPU);
    GPIO_Init(GPIOA, &GPIOA_InitStructure);
}

void ADC_Configure(void)
{
    ADC_InitTypeDef ADC_InitStructure;
    //ADC1 configuration

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    //We will convert multiple channels
    ADC_InitStructure.ADC_ScanConvMode = ENABLE;
    //select continuous conversion mode
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; //!
    //select no external triggering
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    //right 12-bit data alignment in ADC data register
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    //custom channels conversion
    ADC_InitStructure.ADC_NbrOfChannel = CHANNEL_NUM;
    //load structure values to control and status registers
    ADC_Init(ADC1, &ADC_InitStructure);
    //wake up temperature sensor
    //ADC_TempSensorVrefintCmd(ENABLE);
    //configure each channel
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0,  1, Cycles); // A0
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4,  2, Cycles); // A4
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6,  3, Cycles); // A6
    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 4, Cycles); // PC0
    ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 5, Cycles); // PC1
    ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 6, Cycles); // PC3
    ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 7, Cycles); // PC4
    // ADC_RegularChannelConfig(ADC1, ADC_Channel_7,  8, Cycles); // 모터
    //Enable ADC1
    ADC_Cmd(ADC1, ENABLE);
    //enable DMA for ADC
    ADC_DMACmd(ADC1, ENABLE);
    //Enable ADC1 reset calibration register
    ADC_ResetCalibration(ADC1);
    //Check the end of ADC1 reset calibration register
    while (ADC_GetResetCalibrationStatus(ADC1))
        ;
    //Start ADC1 calibration
    ADC_StartCalibration(ADC1);
    //Check the end of ADC1 calibration
    while (ADC_GetCalibrationStatus(ADC1))
        ;
}

void DMA_Configure(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    //enable DMA1 clock
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    //create DMA structure
    //reset DMA1 channe1 to default values;
    DMA_DeInit(DMA1_Channel1);
    //channel will be used for memory to memory transfer
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    //setting normal mode (non circular)
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //DMA_Mode_Normal;
    //medium priority
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    //source and destination data size word=32bit
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
    //automatic memory destination increment enable.
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    //source address increment disable
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    //Location assigned to peripheral register will be source
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    //chunk of data to be transfered
    DMA_InitStructure.DMA_BufferSize = ARRAYSIZE;
    //source and destination start addresses
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_values;
    //send values to DMA registers
    DMA_Init(DMA1_Channel1, &DMA_InitStructure);
    // Enable DMA1 Channel Transfer Complete interrupt
    DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
    DMA_Cmd(DMA1_Channel1, ENABLE); //Enable the DMA1 - Channel1
}

void DMA1_Channel1_IRQHandler(void)
{
    //Test on DMA1 Channel1 Transfer Complete interrupt
    if (DMA_GetITStatus(DMA1_IT_TC1))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC1 | DMA1_IT_GL1);
        __status++;
    }
}

int adc_view_flag = 1;
void dma_test_adc_CHANNEL_NUM(void)
{
    if (!__status)
        return;

    uint8_t index;
    ADC_SoftwareStartConvCmd(ADC1, DISABLE);
    if (adc_view_flag)
    {
        // for (index = 0; index < CHANNEL_NUM; index++)
        // {
        //     printf("%d ADC value on ch%d = %d ,%d\r\n",
        //            __status, index + 1, (uint16_t)((ADC_values[index] + ADC_values[index + CHANNEL_NUM] + ADC_values[index + CHANNEL_NUM * 2] + ADC_values[index + CHANNEL_NUM * 3]) / 4), ADC_values[index]);
        // }
        // ADC_values[0] - 3400이 250 이상일 때 유지 아니면 넘어짐
        // printf("%d ADC value on ch%d = %d ,%d\r\n",
        //            __status, 0 + 1, (uint16_t)((ADC_values[0] + ADC_values[0 + CHANNEL_NUM] + ADC_values[0 + CHANNEL_NUM * 2] + ADC_values[0 + CHANNEL_NUM * 3]) / 4) - 3400, ADC_values[0]);
        // printf("%d ADC value on ch%d = %d ,%d\r\n",
        //            __status, 1 + 1, (uint16_t)((ADC_values[1] + ADC_values[1 + CHANNEL_NUM] + ADC_values[1 + CHANNEL_NUM * 2] + ADC_values[1 + CHANNEL_NUM * 3]) / 4) - 3400, ADC_values[1]);
        // printf("%d ADC value on ch%d = %d ,%d\r\n",
        //            __status, 2 + 1, (uint16_t)((ADC_values[2] + ADC_values[2 + CHANNEL_NUM] + ADC_values[2 + CHANNEL_NUM * 2] + ADC_values[2 + CHANNEL_NUM * 3]) / 4) - 1600, ADC_values[2]);
        
        // printf("%d ADC value on ch%d = %d ,%d\r\n",
        //            __status, 3 + 1, (uint16_t)((ADC_values[3] + ADC_values[3 + CHANNEL_NUM] + ADC_values[3 + CHANNEL_NUM * 2] + ADC_values[3 + CHANNEL_NUM * 3]) / 4) - 3400, ADC_values[3]);
        // printf("=========================\n\r");
        
        printf("%d ADC value on ch%d = %d ,%d\r\n",
                   __status, 4 + 1, (uint16_t)((ADC_values[4] + ADC_values[4 + CHANNEL_NUM] + ADC_values[4 + CHANNEL_NUM * 2] + ADC_values[4 + CHANNEL_NUM * 3]) / 4), ADC_values[4]);
        printf("=========================\n\r");
    }
    __status = 0;
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void NVIC_Configure(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    //Enable DMA1 channel IRQ Channel */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void delay()
{
    for (int i = 0; i < 10000000; i++)
    {
        continue;
    }
}

int main(void)
{
    SystemInit();
    RCC_Configure();
    GPIO_Configure();
    NVIC_Configure();

    // Sensor
    ADC_Configure();
    DMA_Configure();

    // LCD
    LCD_Init();            // LCD 초기화
    LCD_Clear(WHITE);      // LCD 배경 초기화
    //-----------------

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    LCD_ShowString(40, 10, "MON_Team02", MAGENTA, WHITE); // 팀명 출력
    LCD_ShowString(90, 50, "OFF", RED, WHITE);            // 디폴트로 OFF
    LCD_DrawRectangle(40, 80, 80, 120);                   // 사각형 출력
    LCD_ShowString(60, 100, "Button", MAGENTA, WHITE);    // "Button" 글자 출력
    while (1)
    {
        //  LCD_ShowNum(40, 100, value, 4, BLUE, WHITE);
        dma_test_adc_CHANNEL_NUM();
        delay();
    }
    return 0;
}

#include <stdio.h>
#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

#define ARRAYSIZE 8 * 4
#define ADC1_DR ((uint32_t)0x4001244C)
volatile uint16_t ADC_values[ARRAYSIZE];
static volatile uint32_t __status = 0;

// 4000보다 크면 흔들림, 200보다 작으면 안흔들림

uint16_t value = 100;

/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);
void NVIC_Configure(void);

//---------------------------------------------------------------------------------------------------

// void RCC_Configure(void)
// {
//     /* Alternate Function IO clock enable */
//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE); // ADC1, port C RCC ENABLE
//     RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
//     // 나머지 LCD에 필요한 포트들은 lcd.c에서 활성화 된다.
// }

// void GPIO_Configure(void)
// {
//     GPIO_InitTypeDef GPIO_InitStructure;
//     GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2;
//     GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//     GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // 아날로그 입력 설정
//     GPIO_Init(GPIOC, &GPIO_InitStructure);        // C0포트 활성화, 조도센서를 PC0에 연결할 계획
// }

#define Cycles ADC_SampleTime_239Cycles5
//#define Cycles ADC_SampleTime_41Cycles5

void ADCInit(void)
{
    //--Enable ADC1 and GPIOA--
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure; //Variable used to setup the GPIO pins
    //==Configure ADC pins (PA0 -> Channel 0 to PA7 -> Channel 7) as analog inputs==
    GPIO_StructInit(&GPIO_InitStructure); // Reset init structure, if not it can cause issues...
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

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
    //8 channels conversion
    ADC_InitStructure.ADC_NbrOfChannel = 8;
    //load structure values to control and status registers
    ADC_Init(ADC1, &ADC_InitStructure);
    //wake up temperature sensor
    //ADC_TempSensorVrefintCmd(ENABLE);
    //configure each channel
    ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 2, Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_2, 3, Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_3, 4, Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 5, Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_5, 6, Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 7, Cycles);
    ADC_RegularChannelConfig(ADC1, ADC_Channel_7, 8, Cycles);
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

void DMAInit(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
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

    //Enable DMA1 channel IRQ Channel */
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
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
void dma_test_adc8ch(void)
{
    if (!__status)
        return;

    uint8_t index;
    ADC_SoftwareStartConvCmd(ADC1, DISABLE);
    if (adc_view_flag)
    {
        for (index = 0; index < 8; index++)
        {
            printf("%d ADC value on ch%d = %d ,%d\r\n",
                   __status, index, (uint16_t)((ADC_values[index] + ADC_values[index + 8] + ADC_values[index + 16] + ADC_values[index + 24]) / 4), ADC_values[index]);
        }
        printf("=========================\n\r");
    }
    __status = 0;
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

// void ADC1_Configure(void)
// {
//     // Analog to Digital Converter
//     ADC_InitTypeDef ADC_InitStruct;

//     //ADC_StructInit(&ADC_InitStruct);
//     ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;                  // slave-master가 없는 독립 ADC
//     ADC_InitStruct.ADC_ScanConvMode = DISABLE;                       // 단일채널이므로 비활성화
//     ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;                  // 한 번의 트리거로 한 채널의 샘플링 시행
//     ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 외부 입력핀에 의한 트리거 비활성화
//     ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;              // Default
//     ADC_InitStruct.ADC_NbrOfChannel = 1;                             // 채널은 하나

//     ADC_Init(ADC1, &ADC_InitStruct); // 위 설정을 ADC1에 적용

//     ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_28Cycles5); // 채널 우선순위 설정, 10채널 단독사용

//     // ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE); // 무분별한 핸들러 호출 방지를 위해 주석처리

//     ADC_Cmd(ADC1, ENABLE); // ADC1 활성화

//     //Calibration reset & start
//     ADC_ResetCalibration(ADC1);

//     while (ADC_GetResetCalibrationStatus(ADC1))
//         ;

//     ADC_StartCalibration(ADC1);

//     while (ADC_GetCalibrationStatus(ADC1))
//         ;

//     ADC_SoftwareStartConvCmd(ADC1, ENABLE);
// }

// void ADC_Configure(void)
// {
//     ADC1_Configure();
//     ADC2_Configure();
// }

// void NVIC_Configure(void)
// {

//     NVIC_InitTypeDef NVIC_InitStructure;

//     NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

//     NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn; // ADC IRQ 인터럽트 활성화
//     NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
//     NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
//     NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//     NVIC_Init(&NVIC_InitStructure);
// }

// void ADC1_2_IRQHandler(void)
// {
//     // printf("Handle\n");
//     if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET)
//     {                                         // End Of Conversion, ADC변환이 끝났을때,
//         value = ADC_GetConversionValue(ADC1); // value에 조도센서 값 입력
//         printf("%d\n", value);
//         ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
//     }
// }

void delay()
{
    for (int i = 0; i < 1000000; i++)
    {
        continue;
    }
}

int main(void)
{

    SystemInit();
    // RCC_Configure();
    // GPIO_Configure();
    // ADC_Configure();
    // NVIC_Configure();

    ADCInit();
    //-----------------

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    // ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE); // ADC 변환이 끝났을때 인터럽트 발생
    while (1)
    {
        dma_test_adc8ch();
        delay();
    }
    return 0;
}

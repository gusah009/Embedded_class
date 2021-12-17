#include <stdio.h>
#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"

// 4000보다 크면 흔들림, 200보다 작으면 안흔들림

uint16_t value = 100;

/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);
void NVIC_Configure(void);

//---------------------------------------------------------------------------------------------------

void RCC_Configure(void)
{
    /* Alternate Function IO clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 | RCC_APB2Periph_GPIOC, ENABLE); // ADC1, port C RCC ENABLE
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    // 나머지 LCD에 필요한 포트들은 lcd.c에서 활성화 된다.
}

void GPIO_Configure(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // 아날로그 입력 설정
    GPIO_Init(GPIOC, &GPIO_InitStructure);        // C0포트 활성화, 조도센서를 PC0에 연결할 계획
}

void ADC_Configure(void)
{
    // Analog to Digital Converter
    ADC_InitTypeDef ADC_InitStruct;

    //ADC_StructInit(&ADC_InitStruct);
    ADC_InitStruct.ADC_Mode = ADC_Mode_Independent;                  // slave-master가 없는 독립 ADC
    ADC_InitStruct.ADC_ScanConvMode = DISABLE;                       // 단일채널이므로 비활성화
    ADC_InitStruct.ADC_ContinuousConvMode = ENABLE;                  // 한 번의 트리거로 한 채널의 샘플링 시행
    ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 외부 입력핀에 의한 트리거 비활성화
    ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;              // Default
    ADC_InitStruct.ADC_NbrOfChannel = 1;                             // 채널은 하나

    ADC_Init(ADC1, &ADC_InitStruct); // 위 설정을 ADC1에 적용

    ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 1, ADC_SampleTime_28Cycles5); // 채널 우선순위 설정, 10채널 단독사용

    // ADC_ITConfig(ADC1,ADC_IT_EOC,ENABLE); // 무분별한 핸들러 호출 방지를 위해 주석처리

    ADC_Cmd(ADC1, ENABLE); // ADC1 활성화

    //Calibration reset & start
    ADC_ResetCalibration(ADC1);

    while (ADC_GetResetCalibrationStatus(ADC1))
        ;

    ADC_StartCalibration(ADC1);

    while (ADC_GetCalibrationStatus(ADC1))
        ;

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void NVIC_Configure(void)
{

    NVIC_InitTypeDef NVIC_InitStructure;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);

    NVIC_InitStructure.NVIC_IRQChannel = ADC1_2_IRQn; // ADC IRQ 인터럽트 활성화
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void ADC1_2_IRQHandler(void)
{
    // printf("Handle\n");
    if (ADC_GetITStatus(ADC1, ADC_IT_EOC) != RESET)
    {                                         // End Of Conversion, ADC변환이 끝났을때,
        value = ADC_GetConversionValue(ADC1); // value에 조도센서 값 입력
        printf("%d\n", value);
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
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

    uint16_t pos_temp[2]; // x, y좌표를 담을 배열

    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE); // ADC 변환이 끝났을때 인터럽트 발생
    while (1)
    {
    }
    return 0;
}

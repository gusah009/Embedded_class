#include <stdio.h>
#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"

// 온습도

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

   GPIO_Init(GPIOC, &GPIO_InitStructure); // C0포트 활성화, 조도센서를 PC0에 연결할 계획
}

void ADC_Configure(void)

{

   // Analog to Digital Converter

   ADC_InitTypeDef ADC_InitStruct;

   //ADC_StructInit(&ADC_InitStruct);

   ADC_InitStruct.ADC_Mode = ADC_Mode_Independent; // slave-master가 없는 독립 ADC

   ADC_InitStruct.ADC_ScanConvMode = DISABLE; // 단일채널이므로 비활성화

   ADC_InitStruct.ADC_ContinuousConvMode = ENABLE; // 한 번의 트리거로 한 채널의 샘플링 시행

   ADC_InitStruct.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // 외부 입력핀에 의한 트리거 비활성화

   ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right; // Default

   ADC_InitStruct.ADC_NbrOfChannel = 1; // 채널은 하나

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
   { // End Of Conversion, ADC변환이 끝났을때,

      value = ADC_GetConversionValue(ADC1); // value에 조도센서 값 입력

      printf("%d\n", value);

      ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
   }
}

//--------------------------------

double temperature = 0.0;

double moisture = 0.0;

typedef struct
{

   uint32_t u;

   uint8_t c;

} dht11_result;

typedef struct
{

   uint16_t gpio;

   uint16_t pin;

} gpio_pin;

static gpio_pin __D;

static dht11_result Dr = {0, 0};

void init_dht11(gpio_pin g)
{

   __D = g;

   init_gpio_p(__D, GPIO_Mode_Out_OD); // GPIO_Mode_Out_PP
}

void dht11_status()
{

   printf("Temperature = %5.2f moisture = %5.2f\n\r", temperature, moisture);
}

static int waitchange_pin(gpio_pin g, uint16_t usec, int hl)
{

   uint16_t t;

   if (hl)
   {

      for (t = get_utime(); (is_gpio_H(g) && elapsed_us(t) < usec);)
         ;

      if (is_gpio_H(g))
         return -1;
   }
   else
   {

      for (t = get_utime(); (is_gpio_L(g) && elapsed_us(t) < usec);)
         ;

      if (is_gpio_L(g))
         return -1;
   }

   return (int)elapsed_us(t);
}

int read_dht11()
{

   int val;

   static uint32_t dht11_delay = 0;

   static int dht11_step = 0;

   char *p = (char *)(&Dr);

   switch (dht11_step)
   {

   case 0:

      Dr.u = 0;

      Dr.c = 0;

      dht11_delay = get_mtime();

      gpio_bit_H(__D.gpio, __D.pin);

      dht11_step++;

      break;

   case 1:

      if (elapsed_ms(dht11_delay) > 980)
      {

         gpio_bit_L(__D.gpio, __D.pin);

         dht11_delay = get_mtime();

         dht11_step++;
      }

      break;

   case 2:

      if (elapsed_ms(dht11_delay) > 18)
      { // > 18msec

         dht11_step++;
      }

      break;

   case 3:

      gpio_bit_H(__D.gpio, __D.pin);

      while (is_gpio_H(__D))
         ;

      if (waitchange_pin(__D, (80 + 10), 0) < 0)
         return -1;

      if (waitchange_pin(__D, (80 + 10), 1) < 0)
         return -1;

      for (int i = 0; i < 40; i++)
      {

         if (waitchange_pin(__D, (50 + 10), 0) < 0)
            return -1;

         if ((val = waitchange_pin(__D, (70 + 10), 1)) < 0)
            return -1;

         val = (val < 35) ? 0 : 1;

         if (i < 32)
         {

            Dr.u |= (val)&0x1;

            if (i < 31)
               Dr.u <<= 1;
         }
         else
         {

            Dr.c |= (val)&0x1;

            if (i < 39)
               Dr.c <<= 1;
         }
      }

      dht11_step = 10;

      break;

   case 10:

      if ((uint8_t)((p[0] + p[1] + p[2] + p[3]) & 0xff) != Dr.c)
      {

         dht11_step = -1;
      }
      else
      {

         dht11_step = 0;

         moisture = (double)p[3];

         temperature = (double)p[1];
      }

      return 1;

   default:

      return -1;
   }

   return 0;
}

//--------------------------------

int main(void)

{

   SystemInit();

   RCC_Configure();

   GPIO_Configure();

   ADC_Configure();

   NVIC_Configure();

   //-----------------

   ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE); // ADC 변환이 끝났을때 인터럽트 발생

   //-----------------

   uint16_t pos_temp[2]; // x, y좌표를 담을 배열

   while (1)
   {
   }

   return 0;
}
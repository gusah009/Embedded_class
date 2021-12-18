#include "timerConfigure.h"

volatile uint32_t TIM3_Counter = 0;

void RCC_Configure(void)
{
  /* PWM */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // TIM3
}

void GPIO_Configure(void)
{
}

void PWM_Configure()
{
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_TimeBaseStructure.TIM_Prescaler = (uint16_t)(SystemCoreClock / 1000000);
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = 20000;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  printf("SysClk: %d\n", (uint16_t)(SystemCoreClock / 1000000));
  printf("TIM_Period: %d\n", TIM_TimeBaseStructure.TIM_Period);
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
  TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
  TIM_Cmd(TIM3, ENABLE);
}

void NVIC_Configure(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;

  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void timerInit(void)
{
  RCC_Configure();
  GPIO_Configure();
  PWM_Configure();
  NVIC_Configure();
}
#include <stdio.h>
#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_tim.h"
#include "font.h"
#include "lcd.h"
#include "lcd.c"
#include "touch.h"
#include "touch.c"

int color[12] = {WHITE,CYAN,BLUE,RED,MAGENTA,LGRAY,GREEN,YELLOW,BROWN,BRRED,GRAY};
uint16_t value = 100;
int flag = 1;
int ledCount = 0;

TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
TIM_OCInitTypeDef       TIM_OCInitStructure;
uint16_t prescale;

/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);
void NVIC_Configure(void);

//---------------------------------------------------------------------------------------------------

void RCC_Configure(void)
{
      /* LCD */
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // port C RCC ENABLE
      /* PWM */
      RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE); // TIM2
      /* LED */
      RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); // Port D

   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configure(void)
{
    GPIO_InitTypeDef GPIO_InitStructure3;
    GPIO_InitStructure3.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_7;
    GPIO_InitStructure3.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure3.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure3); // led 활성화
}

void NVIC_Configure(void) {
    
   NVIC_PriorityGroupConfig(NVIC_PriorityGroup_0);
   NVIC_InitTypeDef NVIC_InitStructure;

   /* Enable TIM2 Global Interrupt */
   NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
}

void TIM2_IRQHandler(void) {
  if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
  {
      if (ledCount % 2 == 0) {
      GPIO_SetBits(GPIOD, GPIO_Pin_4);
      } else{
      GPIO_ResetBits(GPIOD, GPIO_Pin_4); 
      }

      if(ledCount == 0) {
      GPIO_ResetBits(GPIOD, GPIO_Pin_7);
      } else if(ledCount == 5) {
      GPIO_SetBits(GPIOD, GPIO_Pin_7);
      }

      ledCount++;
      ledCount %= 10;

      TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
  }
}

void PWM_Configure()
{
   prescale = (uint16_t) (SystemCoreClock / 10000);
   TIM_TimeBaseStructure.TIM_Period = 10000;
   TIM_TimeBaseStructure.TIM_Prescaler = prescale;
   TIM_TimeBaseStructure.TIM_ClockDivision = 0;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
   
   TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
   TIM_ARRPreloadConfig(TIM2, ENABLE);
   TIM_Cmd(TIM2, ENABLE);
}


int main(void)
{

    SystemInit();
    RCC_Configure();
    GPIO_Configure();
    PWM_Configure();
    NVIC_Configure();

    //-----------------

    LCD_Init(); // LCD 초기화
    Touch_Configuration(); // 터치 설정
    Touch_Adjust(); // 화면 터치 초점 맞추기
    LCD_Clear(WHITE); // LCD 배경 초기화

    //-----------------

    uint16_t pos_temp[2]; // x, y좌표를 담을 배열

   LCD_ShowString(40, 10, "MON_Team02", MAGENTA, WHITE); // 팀명 출력
   LCD_ShowString(90, 50, "OFF", RED, WHITE); // 디폴트로 OFF 
   LCD_DrawRectangle(40, 80, 80, 120); // 사각형 출력
   LCD_ShowString(60,100, "Button", MAGENTA, WHITE); // "Button" 글자 출력
    
    while (1) {
         Touch_GetXY(&pos_temp[0], &pos_temp[1], 1); // 터치 좌표 받아서 배열에 입력
         Convert_Pos(pos_temp[0],pos_temp[1],&pos_temp[0],&pos_temp[1]); // 받은 좌표를 LCD 크기에 맞게 변환
         printf("%d, %d\n", pos_temp[0], pos_temp[1]);

         if ((uint16_t)40 < pos_temp[0] && pos_temp[0] < (uint16_t)80 && \
             (uint16_t)80 < pos_temp[1] && pos_temp[1] < (uint16_t)120) {
            if (flag) {
               LCD_ShowString(90, 50, "    ", RED, WHITE); 
               LCD_ShowString(40, 50, "ON", RED, WHITE); 
               TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE); // interrupt enable
            } else {
               LCD_ShowString(90, 50, "OFF", RED, WHITE); 
               LCD_ShowString(40, 50, "    ", RED, WHITE); 
               TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE); // interrupt enable
            }
            flag++;
            flag %= 2;
         }
    }
    return 0;
}
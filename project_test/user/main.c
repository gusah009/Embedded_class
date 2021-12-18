#include <stdio.h>
#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "lcd.h"
#include "sensorConfigure.h"
#include "motorConfigure.h"
#include "bluetoothConfigure.h"
#include "timerConfigure.h"
#include "utility.h"

extern volatile uint32_t __status;
extern volatile uint32_t TIM3_Counter;

/*================== Start Bluetooth ==================*/
void sendDataUART1(uint16_t data)
{
    USART_SendData(USART1, data);
    /* Wait till TC is set */
    // while ((USART1->SR & USART_SR_TC) == 0);
}

void sendDataUART2(uint16_t data)
{
    USART_SendData(USART2, data);
    /* Wait till TC is set */
    // while ((USART2->SR & USART_SR_TC) == 0);
}

void USART1_IRQHandler()
{
    uint16_t word;
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {

        // the most recent received data by the USART1 peripheral
        word = USART_ReceiveData(USART1);
        sendDataUART2(word); // send to USART2
        sendDataUART1(word); // send to USART1

        // clear 'Read data register not empty' flag
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

void USART2_IRQHandler()
{
    uint16_t word;
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {

        // the most recent received data by the USART2 peripheral
        word = USART_ReceiveData(USART2);
        sendDataUART1(word); // send to USART1

        // clear 'Read data register not empty' flag
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}
/*================== End Bluetooth ==================*/

/*================== Start Sensor Interrupt ==================*/
void DMA1_Channel1_IRQHandler(void)
{
    //Test on DMA1 Channel1 Transfer Complete interrupt
    if (DMA_GetITStatus(DMA1_IT_TC1))
    {
        SensorVal a = getSensorValue();

        DMA_ClearITPendingBit(DMA1_IT_TC1 | DMA1_IT_GL1);
        __status++;
    }
}
/*================== End Sensor Interrupt ==================*/

/*================== Start Timer Interrupt ==================*/
void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
        if (TIM3_Counter % 100 == 0) printf("TIMER: %d\n", TIM3_Counter);
        TIM3_Counter++;
    }
}
/*================== End Timer Interrupt ==================*/

int main(void)
{
    /*================== Start Initilaize ==================*/
    SystemInit();

    /* NVIC은 Priority Group Config는 모든 설정에 공통되야 하므로 main에서 앞쪽에 선언해주었습니다. */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* Sensor */
    sensorInit();

    /* LCD */
    LCD_Init(); // LCD 초기화

    /* Bluetooth */
    bluetoothInit();

    /* Motor */
    motorInit();

    /* Timer */
    timerInit();
    /*================== End Initilaize ==================*/

    LCD_Clear(WHITE);                                     // LCD 배경 초기화
    LCD_ShowString(40, 10, "MON_Team02", MAGENTA, WHITE); // 팀명 출력
    LCD_ShowString(90, 50, "OFF", RED, WHITE);            // 디폴트로 OFF
    LCD_DrawRectangle(40, 80, 80, 120);                   // 사각형 출력
    LCD_ShowString(60, 100, "Button", MAGENTA, WHITE);    // "Button" 글자 출력
    GPIOE->ODR = (GPIO_ODR_ODR1);
    while (1)
    {
        //  LCD_ShowNum(40, 100, value, 4, BLUE, WHITE);
        dma_test_adc_CHANNEL_NUM();
        delay();
    }
    return 0;
}

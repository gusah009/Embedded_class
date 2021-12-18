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

#define COMMAND_MAX 5

extern volatile uint32_t __status;
extern volatile uint32_t TIME;
extern volatile uint16_t LOG_INDEX;
extern const volatile uint32_t MAX_TIME;
extern Log logs[1000];
uint16_t command[COMMAND_MAX];
volatile uint8_t COMMAND_INDEX = 0;
volatile uint32_t TIM3_COUNTER = 0;
volatile uint16_t COMMAND_LOG[COMMAND_MAX] = {'l', '\n'};

volatile int NO_COMMAND_FLAG = 0;

/*================== Start Bluetooth ==================*/
void USART1_IRQHandler()
{
    uint16_t word;
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {

        // the most recent received data by the USART1 peripheral
        word = USART_ReceiveData(USART1);
          
        printf("word: %d\n", word);
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
        command[COMMAND_INDEX] = word;
        if (word == '\n')
        {
            printf("!!!\n");
            int cmdIndex = 0;
            for (; cmdIndex <= COMMAND_INDEX; cmdIndex++)
            {
                if (command[cmdIndex] != COMMAND_LOG[cmdIndex])
                {
                    break;
                }
            }

            if (cmdIndex == COMMAND_INDEX + 1)
            {
            printf("def\n");
                Ts ts;
                for (int logIndex = 0; logIndex < LOG_INDEX; logIndex++)
                {
                    ts = ConvertTimeInFormat(logs[logIndex].time);

                    sendLogData(ts.month / 10 + '0',
                                ts.month % 10 + '0',
                                ts.day / 10 + '0',
                                ts.day % 10 + '0',
                                ts.hour / 10 + '0',
                                ts.hour % 10 + '0',
                                ts.min / 10 + '0',
                                ts.min % 10 + '0',
                                ts.sec / 10 + '0',
                                ts.sec % 10 + '0',
                                logs[logIndex].soil_moisture / 10 + '0',
                                logs[logIndex].soil_moisture % 10 + '0',
                                logs[logIndex].temperature / 10 + '0',
                                logs[logIndex].temperature % 10 + '0',
                                logs[logIndex].is_pump,
                                logs[logIndex].is_vibrate);
                }
            }
            else
            {
                printf("%d\n", 'R');
                NO_COMMAND_FLAG = 1;
            }
            COMMAND_INDEX = 0;
        }
        else
        {
            COMMAND_INDEX++;
            COMMAND_INDEX %= COMMAND_MAX;
        }

        /* ============ FOR DEBUGGING ============= */
        sendDataUART1(word); // send to USART1
        sendDataUART2(word); // send to USART2
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
        if (TIME == MAX_TIME)
            TIME = 0;
        if (TIM3_COUNTER == 100)
        {
            TIME++;
            TIM3_COUNTER = 0;
        }
        TIM3_COUNTER++;
    }
}
/*================== End Timer Interrupt ==================*/

/*================== Start Log Func ==================*/

/*================== End Log Func ==================*/
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
    // LCD_ShowString(40, 10, "MON_Team02", MAGENTA, WHITE); // 팀명 출력
    // LCD_ShowString(90, 50, "OFF", RED, WHITE);            // 디폴트로 OFF
    // LCD_DrawRectangle(40, 80, 80, 120);                   // 사각형 출력
    // LCD_ShowString(60, 100, "Button", MAGENTA, WHITE);    // "Button" 글자 출력

    GPIOE->ODR = (GPIO_ODR_ODR1);// 모터 상시 작동
    while (1)
    {
        //  LCD_ShowNum(40, 100, value, 4, BLUE, WHITE);
        // dma_test_adc_CHANNEL_NUM();
        delay(10000000);
        if (NO_COMMAND_FLAG) {
            uint16_t w = 0x52;
            sendDataUART2(w);
            sendDataUART2(69);
            sendDataUART2((uint16_t)'T');
            sendDataUART2((uint16_t)'R');
            sendDataUART2((uint16_t)'Y');
            sendDataUART2((uint16_t)'!');
            sendDataUART2((uint16_t)'\n');
            NO_COMMAND_FLAG = 0;
        }
    }
    return 0;
}

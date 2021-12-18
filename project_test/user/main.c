#include <stdio.h>
#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_usart.h"
#include "lcd.h"
#include "sensorConfigure.h"
// #include "motorConfigure.h"
// #include "bluetoothConfigure.h"
#include "utility.h"

int main(void)
{
    SystemInit();

    /* NVIC은 Priority Group Config는 모든 설정에 공통되야 하므로 main에서 앞쪽에 선언해주었습니다. */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /* Sensor */
    sensorInit();

    /* LCD */
    LCD_Init();       // LCD 초기화
    LCD_Clear(WHITE); // LCD 배경 초기화

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

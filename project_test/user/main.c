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

// extern 함수는 각 변수가 선언된 고유 위치에 주석을 달아 놓았습니다.
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
volatile int SEND_LOG_FLAG = 0;

uint8_t presentFace = 'S'; // 초기 표정은 웃는표정 가정
/*================== Value Check ==================*/

void startPump(){
    GPIOE->ODR = (GPIO_ODR_ODR1);// 모터작동
}

void stopPump(){
    GPIOE->ODR = 0;// 모터 중지
}

uint8_t checkMoisture(uint16_t moisture){
    printf("Moist : %d\n",moisture);

    if(moisture < 1000/*임시 토양 습도*/){
        startPump(); // 펌프모터 작동
        SaveLog(0,0); // 토양습도 기록
    }
    else{
        stopPump(); // 모터 중지
    }
}

uint8_t checkVibration(uint16_t vibration){
    printf("Vib : %d\n",vibration);

    if(vibration > 3000/*임시진동기준치*/){
        return 1;
    }
    return 0;
}

uint8_t checkTemperature(uint16_t temperature){

    uint16_t calcedTemp = temperature; /* 계산된 온도 입력*/
    printf("temp : %d\n",calcedTemp);

    if(20/*임시최소온도*/ <= calcedTemp
        && calcedTemp <= 30/*임시최대온도*/ ){
            return 0;
    }
    return 1;
}

void getFace(SensorVal sensorValue){
    uint16_t vibration = sensorValue.vibration;
    uint16_t temperature = sensorValue.temperature;

    if(checkVibration(vibration)){ // 진동 발생여부 확인
        SaveLog(0,1); // 진동 감지 로그 남기기
        if (presentFace != 'F'){
            LCD_Frown(); // 찌푸린 표정
            presentFace = 'F';
        }
    }
    else if(checkTemperature(temperature)){ // 적정 온도여부 확인
        if (presentFace != 'D'){
            LCD_Sad(); // 슬픈 표정
            presentFace = 'D';
        }
    }
    else{
        if (presentFace != 'S'){
            LCD_Smile(); // 웃는 표정
            presentFace = 'S';
        }
    }
}

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
        word = USART_ReceiveData(USART2);
        command[COMMAND_INDEX] = word; // 사용자의 입력값을 command라는 char배열에 저장
        
        if (word == '\n') // 사용자가 엔터를 눌렀을 때,
        {
            int cmdIndex = 0;
            for (; cmdIndex <= COMMAND_INDEX; cmdIndex++)
            {
                if (command[cmdIndex] != COMMAND_LOG[cmdIndex])
                {
                    // 사용자 명령어와 예약명령어(현재는 log기능을 나타내는 'l'명령어만 존재)를 비교하고
                    // 예약된 명령어와 다르면 break로 cmdIndex를 COMMAND_INDEX와 다른 값으로 만들어줌.
                    break;
                }
            }

            // cmdIndex가 COMMAND_INDEX와 같은 값이면 명령어라는 뜻이므로 LOG 출력
            if (cmdIndex == COMMAND_INDEX + 1)
            {
                // Interrupt 도중에 출력하지 않고, Interrupt가 끝나면 main으로 돌아가서 실행.
                SEND_LOG_FLAG = 1;
            }
            // cmdIndex가 COMMAND_INDEX와 다른 값이면 명령어가 아니라는 뜻이므로 다시 입력하라는 RETRY 출력
            else
            {
                // Interrupt 도중에 출력하지 않고, Interrupt가 끝나면 main으로 돌아가서 실행.
                NO_COMMAND_FLAG = 1;
            }
            // 엔터키가 눌러졌다면 command를 초기화.
            COMMAND_INDEX = 0;
        }
        else
        {
            // 명령어를 최대 명령어 크기(COMMAND_MAX)까지 받아오는 모습.
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

        getFace(a); // LCD 표정 결정
        checkMoisture(a.soil_moisture); // 습도확인 및 펌프동작 결정

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
        // MAX_TIME인 12월 31일 23:59:59가 지나가는 순간 0으로 초기화 시켜주는 부분
        if (TIME == MAX_TIME)
            TIME = 0;
        // COUNTER가 0.01s마다 1씩 증가하기 때문에 100이 되면 1초 증가시켜주는 부분.
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
        delay(100000);

        if (NO_COMMAND_FLAG) {
            sendDataUART2('R');
            sendDataUART2('E');
            sendDataUART2('T');
            sendDataUART2('R');
            sendDataUART2('Y');
            sendDataUART2('!');
            sendDataUART2('\n');
            NO_COMMAND_FLAG = 0;
        }

        if (SEND_LOG_FLAG) {
            Ts ts;
            for (int logIndex = 0; logIndex < LOG_INDEX; logIndex++)
            {
                ts = ConvertTimeInFormat(logs[logIndex].time);
                // 시간, 온도, 습도같은 int형 변수를 char단위로 쪼개서 인자로 보내주는 함수
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
                SEND_LOG_FLAG = 0;
            }
        }
    }
    return 0;
}

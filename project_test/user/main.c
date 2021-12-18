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
#define DATE_MAX 4
#define SET_MONTH 1
#define SET_DAY 2
#define SET_HOUR 3
#define SET_MIN 4
#define FALL_CHECK_TIME 5

// extern 함수는 각 변수가 선언된 고유 위치에 주석을 달아 놓았습니다.
extern volatile uint32_t __status;
extern volatile uint32_t TIME;
extern volatile uint16_t LOG_INDEX;
extern const volatile uint32_t MAX_TIME;
extern uint32_t MONTH_PER_DAY[12];
extern Log logs[1000];

// 명령어 버퍼
uint16_t command[COMMAND_MAX];

// 명령어 문자열의 크기
volatile uint8_t COMMAND_INDEX = 0;
// 10ms 단위로 증가하는 COUNTER
volatile uint32_t TIM3_COUNTER = 0;
// log를 출력하라는 명령어인 'l' 명령어
volatile uint16_t COMMAND_LOG[COMMAND_MAX] = {'l', 'o', 'g', '\r', '\n'};
// 진동 발생 후 넘어졌는지 체크하는 시간
volatile uint32_t elapsed_time = 0;

// 현재 시간 버퍼
uint16_t date[DATE_MAX];
// 현재 시간을 담는 문자열의 크기
volatile uint8_t DATE_INDEX = 0;

// 입력한 명령어가 명령어가 아닐 경우 1
volatile int RETRY_FLAG = 0;
// 입력한 명령어가 LOG 출력 명령어일 경우 1
volatile int SEND_LOG_FLAG = 0;
// 물통에 물이 없을 때 1
volatile int SEND_WATER_FLAG = 0;
// 물을 펑핑 했을 때 1
volatile int SEND_PUMP_FLAG = 0;
// 넘어졌을 경우 1
volatile int SEND_FALL_FLAG = 0;
// 시간 설정 상태면 1 이상
volatile int SET_CLOCK_FLAG = SET_MONTH;
// MONTH 설정 문구를 보낼 때 1
volatile int SET_MONTH_MSG_FLAG = 0;
// DAY 설정 문구를 보낼 때 1
volatile int SET_DAY_MSG_FLAG = 0;
// HOUR 설정 문구를 보낼 때 1
volatile int SET_HOUR_MSG_FLAG = 0;
// MINUTE 설정 문구를 보낼 때 1
volatile int SET_MIN_MSG_FLAG = 0;
// 설정이 완료되었다는 문구를 보낼 때 1
volatile int SET_SUCCESS_MSG_FLAG = 0;

uint8_t presentFace = 'N'; // 초기
/*================== Value Check ==================*/

void startPump(){
    GPIOE->ODR = (GPIO_ODR_ODR1);// 모터작동
}

void stopPump(){
    GPIOE->ODR = 0;// 모터 중지
}

uint8_t checkMoisture(uint16_t moisture){
    // printf("Moist : %d\n",moisture);

    if(moisture > 3000/*임시 토양 습도*/){
        SEND_PUMP_FLAG = 1;
        startPump(); // 펌프모터 작동
        SaveLog(1,0); // 토양습도 기록
    }
    else{
        stopPump(); // 모터 중지
    }
}

uint8_t checkVibration(uint16_t vibration){
    // printf("Vib : %d\n",vibration);

    if(vibration > 4000/*임시진동기준치*/){
        return 1;
    }
    return 0;
}

uint8_t checkTemperature(uint16_t temperature){

    uint16_t calcedTemp = temperature; /* 계산된 온도 입력*/
    // printf("temp : %d\n",calcedTemp);

    if(440 >= calcedTemp){ // 적정온도 일때
        return 0;
    }
    return 1; // 적정온도가 아닐때
}

void getFace(SensorVal sensorValue){
    uint16_t vibration = sensorValue.vibration;
    uint16_t temperature = (uint16_t)((sensorValue.temperature * 5.0 / 1024) * 22 / 2.275);

    printf("temp : %d\n",temperature);
    printf("vibr : %d\n",vibration);
    if(checkVibration(vibration)){ // 진동 발생여부 확인
        SaveLog(0,1); // 진동 감지 로그 남기기

        if (presentFace != 'F'){
            LCD_Frown(); // 찌푸린 표정
            presentFace = 'F';
        }
    }
    else if(32 > temperature){ // 적정 온도여부 확인
        // 적정온도가 아닐 때

        if (presentFace != 'D'){
            LCD_Sad(); // 슬픈 표정
            // LCD_ShowString()
            presentFace = 'D';
        }
    }
    else{
        // 적정온도 일때

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

        // printf("word: %d\n", word);
        sendDataUART2(word); // send to USART2
        sendDataUART1(word); // send to USART1

        // clear 'Read data register not empty' flag
        // USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

void USART2_IRQHandler()
{
    uint16_t word;
    if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
    {
        word = USART_ReceiveData(USART2);
        printf("USART2 : %c \n", word);

        // 현재 시간 입력 상태면
        if (SET_CLOCK_FLAG)
        {
            date[DATE_INDEX] = word; // 사용자의 입력값을 date라는 char배열에 저장
            DATE_INDEX++;
            if (word == '\n')              // 사용자가 엔터를 눌렀을 때,
            {
                if (SET_CLOCK_FLAG == SET_MONTH)
                {
                    // 입력 포멧에 맞게 입력했을 때
                    if (DATE_INDEX == DATE_MAX)
                    {
                        // 1 ~ 12 값이면
                        if ((date[0] == '0' && '0' < date[1] && date[1] <= '9') ||
                            (date[0] == '1' && (date[1] == '1' || date[1] == '2'))) {
                            for (int i = 0; i < (date[0] - '0') * 10 + (date[1] - '0') - 1; i++) {
                                TIME += MONTH_PER_DAY[i] * 86400;
                            }
                            SET_CLOCK_FLAG = SET_DAY;
                            SET_DAY_MSG_FLAG = 1;
                        } else SET_MONTH_MSG_FLAG = 1; // Interrupt 도중에 출력하지 않고, Interrupt가 끝나면 main으로 돌아가서 실행.
                    } else SET_MONTH_MSG_FLAG = 1; // Interrupt 도중에 출력하지 않고, Interrupt가 끝나면 main으로 돌아가서 실행.
                }
                else if (SET_CLOCK_FLAG == SET_DAY)
                {
                    // 입력 포멧에 맞게 입력했을 때
                    if (DATE_INDEX == DATE_MAX)
                    {
                        // 1 ~ 31 값이면
                        if ((date[0] == '0' && '0' < date[1] && date[1] <= '9') ||
                            (date[0] == '1' && '0' <= date[1] && date[1] <= '9') ||
                            (date[0] == '2' && '0' <= date[1] && date[1] <= '9') ||
                            (date[0] == '3' && '0' <= date[1] && date[1] <= '1')) {
                                TIME += ((date[0] - '0') * 10 + (date[1] - '0') - 1) * 86400;
                                SET_CLOCK_FLAG = SET_HOUR;
                                SET_HOUR_MSG_FLAG = 1;
                            } else SET_DAY_MSG_FLAG = 1; // Interrupt 도중에 출력하지 않고, Interrupt가 끝나면 main으로 돌아가서 실행.
                    } else SET_DAY_MSG_FLAG = 1; // Interrupt 도중에 출력하지 않고, Interrupt가 끝나면 main으로 돌아가서 실행.
                }
                else if (SET_CLOCK_FLAG == SET_HOUR)
                {
                    // 입력 포멧에 맞게 입력했을 때
                    if (DATE_INDEX == DATE_MAX)
                    {
                        // 0 ~ 23 값이면
                        if ((date[0] == '0' && '0' <= date[1] && date[1] <= '9') ||
                            (date[0] == '1' && '0' <= date[1] && date[1] <= '9') ||
                            (date[0] == '2' && '0' <= date[1] && date[1] <= '3')) {
                                TIME += ((date[0] - '0') * 10 + (date[1] - '0')) * 3600;
                                SET_CLOCK_FLAG = SET_MIN;
                                SET_MIN_MSG_FLAG = 1;
                        } else SET_HOUR_MSG_FLAG = 1; // Interrupt 도중에 출력하지 않고, Interrupt가 끝나면 main으로 돌아가서 실행.
                    } else SET_HOUR_MSG_FLAG = 1; // Interrupt 도중에 출력하지 않고, Interrupt가 끝나면 main으로 돌아가서 실행.
                }
                else if (SET_CLOCK_FLAG == SET_MIN)
                {
                    // 입력 포멧에 맞게 입력했을 때
                    if (DATE_INDEX == DATE_MAX)
                    {
                        // 0 ~ 59 값이면
                        if ((date[0] == '0' && '0' <= date[1] && date[1] <= '9') ||
                            (date[0] == '1' && '0' <= date[1] && date[1] <= '9') ||
                            (date[0] == '2' && '0' <= date[1] && date[1] <= '9') ||
                            (date[0] == '3' && '0' <= date[1] && date[1] <= '9') ||
                            (date[0] == '4' && '0' <= date[1] && date[1] <= '9') ||
                            (date[0] == '5' && '0' <= date[1] && date[1] <= '9')) {
                                TIME += ((date[0] - '0') * 10 + (date[1] - '0')) * 60;
                                SET_CLOCK_FLAG = 0;
                                SET_SUCCESS_MSG_FLAG = 1;
                                // 시간 설정이 끝나면 TIMER 가동
                                TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
                            }else SET_MIN_MSG_FLAG = 1; // Interrupt 도중에 출력하지 않고, Interrupt가 끝나면 main으로 돌아가서 실행.
                    } else SET_MIN_MSG_FLAG = 1; // Interrupt 도중에 출력하지 않고, Interrupt가 끝나면 main으로 돌아가서 실행.

                }
                // 엔터키가 눌러졌다면 date를 초기화.
                DATE_INDEX = 0;
            }
            else
            {
                // 명령어를 최대 명령어 크기(DATE_MAX)까지 받아오는 모습.
                DATE_INDEX %= DATE_MAX;
            }
        }
        else
        {
            command[COMMAND_INDEX] = word; // 사용자의 입력값을 command라는 char배열에 저장
            if (word == '\n')              // 사용자가 엔터를 눌렀을 때,
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
                    RETRY_FLAG = 1;
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
        }

        /* ============ FOR DEBUGGING ============= */
        sendDataUART1(word); // send to USART1
        sendDataUART2(word); // send to USART2
    }
}
/*================== End Bluetooth ==================*/

/*================== Start Sensor Interrupt ==================*/

void DMA1_Channel1_IRQHandler(void)
{
    //Test on DMA1 Channel1 Transfer Complete interrupt
    if (DMA_GetITStatus(DMA1_IT_TC1))
    {

        
        
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
        if (TIME == MAX_TIME) TIME = 0;
        // 10초마다 log 저장
        // COUNTER가 0.01s마다 1씩 증가하기 때문에 100이 되면 1초 증가시켜주는 부분.
        if (TIM3_COUNTER == 100)
        {
            if (TIME % 10 == 0) SaveLog(0, 0);
            SensorVal a = getSensorValue();
            
            getFace(a); // LCD 표정 결정
            checkMoisture(a.soil_moisture); // 습도확인 및 펌프동작 결정
            
            // printf("WaterHegiht : %d\n",a.water_height);
            if (a.water_height < 300 ){ /*물이 없을 때*/
                SEND_WATER_FLAG = 1;
            }
            
            if(checkVibration(a.vibration)){ // 진동 발생여부 확인
                elapsed_time = 0;
            }

            if(elapsed_time < FALL_CHECK_TIME && a.gyro_x > 3400 + 130){ /*자이로 센서로 화분이 넘어졌는지 체크*/
                SEND_FALL_FLAG = 1;
            }

            // 진동 발생 후 5초가 지났는 지 확인.
            if (elapsed_time < FALL_CHECK_TIME) {
                elapsed_time++;
            }
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

    LCD_Clear(WHITE); // LCD 배경 초기화
    SET_MONTH_MSG_FLAG = 1;
    while (1)
    {
        //  LCD_ShowNum(40, 100, value, 4, BLUE, WHITE);
        // dma_test_adc_CHANNEL_NUM();
        delay(100000);

        if (SET_MONTH_MSG_FLAG) {
            Send_Input_Month_MSG();
            SET_MONTH_MSG_FLAG = 0;
        } else if (SET_DAY_MSG_FLAG) {
            Send_Input_Day_MSG();
            SET_DAY_MSG_FLAG = 0;
        } else if (SET_HOUR_MSG_FLAG) {
            Send_Input_Hour_MSG();
            SET_HOUR_MSG_FLAG = 0;
        } else if (SET_MIN_MSG_FLAG) {
            Send_Input_Minute_MSG();
            SET_MIN_MSG_FLAG = 0;
        } else if (SET_SUCCESS_MSG_FLAG) {
            Send_Input_Success_MSG();
            SET_SUCCESS_MSG_FLAG = 0;
        }
        if (SEND_PUMP_FLAG){
            sendDataUART2('P'); /*블루투스로 메시지 전송*/ 
            sendDataUART2('U'); 
            sendDataUART2('M'); 
            sendDataUART2('P');  
            sendDataUART2('E');  
            sendDataUART2('D');  
            sendDataUART2('!');
            sendDataUART2('\n');
            SEND_PUMP_FLAG = 0;
        }
        
        if (SEND_WATER_FLAG){
            sendDataUART2('N'); /*블루투스로 메시지 전송*/ 
            sendDataUART2('O'); 
            sendDataUART2(' '); 
            sendDataUART2('W');  
            sendDataUART2('A');  
            sendDataUART2('T');  
            sendDataUART2('E');  
            sendDataUART2('R');  
            sendDataUART2('!');
            sendDataUART2('\n');
            SEND_WATER_FLAG = 0;
        }
        
        if (SEND_FALL_FLAG){
            sendDataUART2('F'); /*블루투스로 메시지 전송*/
            sendDataUART2('A');
            sendDataUART2('L');
            sendDataUART2('L');
            sendDataUART2('!');
            sendDataUART2('!');
            sendDataUART2('!');
            sendDataUART2('!');
            sendDataUART2('!');
            sendDataUART2('\n');
            SEND_FALL_FLAG = 0;
        }

        if (RETRY_FLAG)
        {
            sendDataUART2('R');
            sendDataUART2('E');
            sendDataUART2('T');
            sendDataUART2('R');
            sendDataUART2('Y');
            sendDataUART2('!');
            sendDataUART2('\n');
            RETRY_FLAG = 0;
        }

        if (SEND_LOG_FLAG)
        {
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
                            logs[logIndex].temperature / 10 + '0',
                            logs[logIndex].temperature % 10 + '0',
                            logs[logIndex].soil_moisture / 10 + '0',
                            logs[logIndex].soil_moisture % 10 + '0',
                            logs[logIndex].is_pump,
                            logs[logIndex].is_vibrate);
                SEND_LOG_FLAG = 0;
            }
        }
    }
    return 0;
}

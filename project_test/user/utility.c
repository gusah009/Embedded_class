#include "utility.h"

extern volatile uint32_t TIME;

// LOG의 개수를 나타내는 변수
volatile uint16_t LOG_INDEX = 0;

// LOG는 최대 1000개 까지 저장할 수 있게 제작
Log logs[1000];

uint32_t MONTH_PER_DAY[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

// 우리 프로젝트에서 표현가능한 최대 시간인 12-31 23:59:59를 초단위로 계산
const volatile uint32_t MAX_TIME = 31 * 86400 +
                                   28 * 86400 +
                                   31 * 86400 +
                                   30 * 86400 +
                                   31 * 86400 +
                                   30 * 86400 +
                                   31 * 86400 +
                                   31 * 86400 +
                                   30 * 86400 +
                                   31 * 86400 +
                                   30 * 86400 +
                                   31 * 86400;

void delay(uint32_t s)
{
    for (int i = 0; i < s; i++)
    {
        continue;
    }
}

// 로그를 저장하는 함수.
// 펌핑이 일어나서 기록하는 log와 진동이 감지되서 기록하는 log를 표현하기 위해 아래와 같이 is_pump, is_vibrate 매개변수를 전달받음.
// 나머지 센서값은 getSensorValue로 현재 센서의 값을 받아옴.
void SaveLog(uint8_t is_pump, uint8_t is_vibrate)
{
    SensorVal sensorVal = getSensorValue();
    logs[LOG_INDEX].time = TIME;
    logs[LOG_INDEX].temperature = sensorVal.temperature;
    logs[LOG_INDEX].soil_moisture = sensorVal.soil_moisture; 
    logs[LOG_INDEX].is_pump = is_pump;
    logs[LOG_INDEX].is_vibrate = is_vibrate;

    // 로그를 모두 기록하고 INDEX를 1만큼 증가시킴.
    LOG_INDEX++;
}

// 초단위로 기록되던 시간을 Time Struct 구조체 형식에 맞게 변환해서 Return해줌.
Ts ConvertTimeInFormat(uint32_t secs)
{
    Ts ts;
    ts.month = 0;
    for (; ts.month < 12; ts.month++) {
        if ((int)(secs - MONTH_PER_DAY[ts.month] * 86400) < 0) {
            break;
        } else {
            secs -= MONTH_PER_DAY[ts.month] * 86400;
        }
    }
    ts.month += 1;
    ts.day = secs / 86400 + 1;
    secs %= 86400;
    ts.hour = secs / 3600;
    secs %= 3600;
    ts.min = secs / 60;
    secs %= 60;
    ts.sec = secs;

    return ts;
}

// 사용자에게 Bluetooth로 LogData를 전달하는 함수.
void sendLogData(uint16_t month1, uint16_t month2, uint16_t day1, uint16_t day2, uint16_t hour1, uint16_t hour2, uint16_t minute1, uint16_t minute2, uint16_t second1, uint16_t second2,
                 uint16_t temperature1, uint16_t temperature2, uint16_t soil_moisture1, uint16_t soil_moisture2, uint16_t is_pump, uint16_t is_vibrate)
{

    sendDataUART2(month1);
    sendDataUART2(month2);
    sendDataUART2('-');
    sendDataUART2(day1);
    sendDataUART2(day2);
    sendDataUART2(' ');
    sendDataUART2(hour1);
    sendDataUART2(hour2);
    sendDataUART2(':');
    sendDataUART2(minute1);
    sendDataUART2(minute2);
    sendDataUART2(':');
    sendDataUART2(second1);
    sendDataUART2(second2);
    sendDataUART2(' ');
    sendDataUART2(temperature1);
    sendDataUART2(temperature2);
    sendDataUART2('C');
    sendDataUART2(' ');
    sendDataUART2(soil_moisture1);
    sendDataUART2(soil_moisture2);
    sendDataUART2('%');
    sendDataUART2(' ');
    if (is_pump)
    {
        sendDataUART2('P');
        sendDataUART2('U');
        sendDataUART2('M');
        sendDataUART2('P');
        sendDataUART2('E');
        sendDataUART2('D');
        sendDataUART2('!');
        sendDataUART2(' ');
    }
    if (is_vibrate)
    {
        sendDataUART2('V');
        sendDataUART2('I');
        sendDataUART2('B');
        sendDataUART2('R');
        sendDataUART2('A');
        sendDataUART2('T');
        sendDataUART2('E');
        sendDataUART2('D');
        sendDataUART2('!');
        sendDataUART2(' ');
    }
    sendDataUART2('\n');
}

void Send_Input_Month_MSG()
{
    sendDataUART2('I');
    sendDataUART2('N');
    sendDataUART2('P');
    sendDataUART2('U');
    sendDataUART2('T');
    sendDataUART2(' ');
    sendDataUART2('M');
    sendDataUART2('O');
    sendDataUART2('N');
    sendDataUART2('T');
    sendDataUART2('H');
    sendDataUART2('\n');
}


void Send_Input_Day_MSG()
{
    sendDataUART2('I');
    sendDataUART2('N');
    sendDataUART2('P');
    sendDataUART2('U');
    sendDataUART2('T');
    sendDataUART2(' ');
    sendDataUART2('D');
    sendDataUART2('A');
    sendDataUART2('Y');
    sendDataUART2('\n');
}

void Send_Input_Hour_MSG()
{
    sendDataUART2('I');
    sendDataUART2('N');
    sendDataUART2('P');
    sendDataUART2('U');
    sendDataUART2('T');
    sendDataUART2(' ');
    sendDataUART2('H');
    sendDataUART2('O');
    sendDataUART2('U');
    sendDataUART2('R');
    sendDataUART2('\n');
}

void Send_Input_Minute_MSG()
{
    sendDataUART2('I');
    sendDataUART2('N');
    sendDataUART2('P');
    sendDataUART2('U');
    sendDataUART2('T');
    sendDataUART2(' ');
    sendDataUART2('M');
    sendDataUART2('I');
    sendDataUART2('N');
    sendDataUART2('U');
    sendDataUART2('T');
    sendDataUART2('E');
    sendDataUART2('\n');
}
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
    while ((USART2->SR & USART_SR_TC) == 0);
}

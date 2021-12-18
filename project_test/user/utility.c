#include "utility.h"

extern volatile uint32_t TIME;
volatile uint16_t LOG_INDEX = 0;
Log logs[1000];
uint8_t MONTH_PER_DAY[12] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
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


void SaveLog(uint8_t is_pump, uint8_t is_vibrate)
{
    SensorVal sensorVal = getSensorValue();
    logs[LOG_INDEX].time = TIME;
    logs[LOG_INDEX].temperature = sensorVal.temperature;
    logs[LOG_INDEX].soil_moisture = sensorVal.soil_moisture; 
    logs[LOG_INDEX].is_pump = is_pump;
    logs[LOG_INDEX].is_vibrate = is_vibrate;

    LOG_INDEX++;
}

Ts ConvertTimeInFormat(uint32_t secs)
{
    Ts ts;
    ts.month = 0;
    for (; ts.month < 12; ts.month++) {
        if (secs - MONTH_PER_DAY[ts.month] * 86400 < 0) {
            break;
        } else {
            secs -= MONTH_PER_DAY[ts.month] * 86400;
        }
    }
    ts.month += 1;
    ts.sec = secs % 60;
    secs /= 60;
    ts.min = secs % 60;
    secs /= 60;
    ts.hour = secs % 24;
    secs /= 24;
    ts.day = secs + 1;

    return ts;
}


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
    sendDataUART2(soil_moisture1);
    sendDataUART2(soil_moisture2);
    sendDataUART2('%');
    if (is_pump)
    {
        sendDataUART2('P');
        sendDataUART2('U');
        sendDataUART2('M');
        sendDataUART2('P');
        sendDataUART2('E');
        sendDataUART2('D');
        sendDataUART2('!');
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
    }
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
    printf("%c", data);
    USART_SendData(USART2, data);
    /* Wait till TC is set */
    while ((USART2->SR & USART_SR_TC) == 0);
}

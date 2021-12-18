#include "utility.h"

extern volatile uint32_t TIME;
volatile uint16_t LOG_INDEX = 0;
Log logs[UINT16_MAX];
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

void delay(void)
{
    for (int i = 0; i < 10000000; i++)
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
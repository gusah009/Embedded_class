#ifndef __UTILITY_H_
#define __UTILITY_H_

#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "sensorConfigure.h"
#include "timerConfigure.h"

typedef struct
{
  uint32_t time;          // log 남긴 시간
  uint16_t temperature;   // 온도
  uint16_t soil_moisture; // 토양 습도
  uint8_t is_pump;        // 펌프 가동 여부
  uint8_t is_vibrate;     // 진동 가동 여부
} Log;

typedef struct
{
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t min;
  uint8_t sec;
} Ts; // Time Struct

void delay(void);
void SaveLog(uint8_t is_pump, uint8_t is_vibrate);
Ts ConvertTimeInFormat(uint32_t secs);

#endif
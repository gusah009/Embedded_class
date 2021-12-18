#ifndef __SENSOR_H_
#define __SENSOR_H_

#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

// 사용할 CHANNEL의 개수가 바뀔 수 있어서 유동적으로 사용하기 위해 define으로 구현.
#define CHANNEL_NUM 8
// ARRAYSIZE는 CHANNEL_NUM에 의존적이므로 아래와 같이 구현.
#define ARRAYSIZE CHANNEL_NUM * 4

#define ADC1_DR ((uint32_t)0x4001244C)
#define Cycles ADC_SampleTime_13Cycles5

// 각 센서들의 값을 저장하는 구조체.
typedef struct
{
  uint16_t gyro_x;          // 자이로 센서 x
  uint16_t gyro_y;          // 자이로 센서 y
  uint16_t gyro_z;          // 자이로 센서 y
  uint16_t water_height;    // 수위 센서
  uint16_t temperature;     // 온도 센서
  uint16_t soil_moisture;   // 토양 센서
  uint16_t vibration;       // 진동 감지 센서
}  SensorVal;

/* function prototype */
void RCC_Configure_sensor(void);
void GPIO_Configure_sensor(void);
void NVIC_Configure_sensor(void);
void ADC_Configure_sensor(void);
void DMA_Configure_sensor(void);
void DMA1_Channel1_IRQHandler(void);
void dma_test_adc_CHANNEL_NUM(void);
void sensorInit(void);
SensorVal getSensorValue(void);

#endif
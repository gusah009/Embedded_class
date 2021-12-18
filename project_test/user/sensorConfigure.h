#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_dma.h"

#define CHANNEL_NUM 8
#define ARRAYSIZE CHANNEL_NUM * 4
#define ADC1_DR ((uint32_t)0x4001244C)
#define Cycles ADC_SampleTime_13Cycles5
//#define Cycles ADC_SampleTime_41Cycles5

typedef struct
{
  uint16_t gyro_x;
  uint16_t gyro_y;
  uint16_t gyro_z;
  uint16_t water_height;
  uint16_t temperature;
  uint16_t soil_moisture;
  uint16_t vibration;
}  SensorVal;
// 4000보다 크면 흔들림, 200보다 작으면 안흔들림

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
#include "sensorConfigure.h"

// 모든 센서의 값들이 해당 배열안에 저장됨.
volatile uint16_t ADC_values[ARRAYSIZE];

// DEBUG: DMA 인터럽트가 얼마나 자주 일어나는 지 보기 위함.
volatile uint32_t __status = 0;

void RCC_Configure_sensor(void)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1 |
                         RCC_APB2Periph_GPIOA |
                         RCC_APB2Periph_GPIOC |
                         RCC_APB2Periph_GPIOE, ENABLE);
}

void GPIO_Configure_sensor(void)
{
  //--Enable ADC1 and GPIOA--
  GPIO_InitTypeDef GPIOA_InitStructure,
      GPIOC_InitStructure,
      GPIOE_InitStructure; //Variable used to setup the GPIO pins

  // 가속도 센서
  GPIO_StructInit(&GPIOA_InitStructure);
  GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_4 | GPIO_Pin_6;
  GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIOA_InitStructure);

  // 수위 센서
  GPIO_StructInit(&GPIOC_InitStructure);
  GPIOC_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIOC_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIOC_InitStructure);

  // 온습도 센서
  GPIO_StructInit(&GPIOC_InitStructure);
  GPIOC_InitStructure.GPIO_Pin = GPIO_Pin_1;
  GPIOC_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIOC_InitStructure);

  // 토양습도 센서
  GPIO_StructInit(&GPIOC_InitStructure);
  GPIOC_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIOC_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIOC_InitStructure);

  // 진동 센서
  GPIO_StructInit(&GPIOC_InitStructure);
  GPIOC_InitStructure.GPIO_Pin = GPIO_Pin_4;
  GPIOC_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
  GPIO_Init(GPIOA, &GPIOC_InitStructure);
}

void ADC_Configure_sensor(void)
{
  ADC_InitTypeDef ADC_InitStructure;
  //ADC1 configuration

  ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
  //We will convert multiple channels
  ADC_InitStructure.ADC_ScanConvMode = ENABLE;
  //select continuous conversion mode
  ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; //!
  //select no external triggering
  ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
  //right 12-bit data alignment in ADC data register
  ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
  //custom channels conversion
  ADC_InitStructure.ADC_NbrOfChannel = CHANNEL_NUM;
  //load structure values to control and status registers
  ADC_Init(ADC1, &ADC_InitStructure);
  //wake up temperature sensor
  //ADC_TempSensorVrefintCmd(ENABLE);
  //configure each channel
  ADC_RegularChannelConfig(ADC1, ADC_Channel_0, 1, Cycles);  // A0
  ADC_RegularChannelConfig(ADC1, ADC_Channel_4, 2, Cycles);  // A4
  ADC_RegularChannelConfig(ADC1, ADC_Channel_6, 3, Cycles);  // A6
  ADC_RegularChannelConfig(ADC1, ADC_Channel_10, 4, Cycles); // PC0
  ADC_RegularChannelConfig(ADC1, ADC_Channel_11, 5, Cycles); // PC1
  ADC_RegularChannelConfig(ADC1, ADC_Channel_13, 6, Cycles); // PC3
  ADC_RegularChannelConfig(ADC1, ADC_Channel_14, 7, Cycles); // PC4
  //Enable ADC1
  ADC_Cmd(ADC1, ENABLE);
  //enable DMA for ADC
  ADC_DMACmd(ADC1, ENABLE);
  //Enable ADC1 reset calibration register
  ADC_ResetCalibration(ADC1);
  //Check the end of ADC1 reset calibration register
  while (ADC_GetResetCalibrationStatus(ADC1))
    ;
  //Start ADC1 calibration
  ADC_StartCalibration(ADC1);
  //Check the end of ADC1 calibration
  while (ADC_GetCalibrationStatus(ADC1))
    ;
}

void DMA_Configure_sensor(void)
{
  DMA_InitTypeDef DMA_InitStructure;
  //enable DMA1 clock
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
  //create DMA structure
  //reset DMA1 channe1 to default values;
  DMA_DeInit(DMA1_Channel1);
  //channel will be used for memory to memory transfer
  DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
  //setting normal mode (non circular)
  DMA_InitStructure.DMA_Mode = DMA_Mode_Circular; //DMA_Mode_Normal;
  //medium priority
  DMA_InitStructure.DMA_Priority = DMA_Priority_High;
  //source and destination data size word=32bit
  DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
  DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
  //automatic memory destination increment enable.
  DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
  //source address increment disable
  DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
  //Location assigned to peripheral register will be source
  DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
  //chunk of data to be transfered
  DMA_InitStructure.DMA_BufferSize = ARRAYSIZE;
  //source and destination start addresses
  DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)ADC1_DR;
  DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADC_values;
  //send values to DMA registers
  DMA_Init(DMA1_Channel1, &DMA_InitStructure);
  // Enable DMA1 Channel Transfer Complete interrupt
  DMA_ITConfig(DMA1_Channel1, DMA_IT_TC, ENABLE);
  DMA_Cmd(DMA1_Channel1, ENABLE); //Enable the DMA1 - Channel1
}

// DEBUG: 각 센서의 값을 확인하기 위한 함수
void dma_test_adc_CHANNEL_NUM(void)
{
  if (!__status)
    return;

  uint8_t index;
  ADC_SoftwareStartConvCmd(ADC1, DISABLE);
  for (index = 0; index < CHANNEL_NUM; index++)
  {
      printf("%d ADC value on ch%d = %d ,%d\r\n",
              __status, index + 1, (uint16_t)((ADC_values[index] + ADC_values[index + CHANNEL_NUM] + ADC_values[index + CHANNEL_NUM * 2] + ADC_values[index + CHANNEL_NUM * 3]) / 4), ADC_values[index]);
  }
__status = 0;
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

// 각 센서값을 SensorVal이라는 구조체에 넣어서 반환해주는 함수.
SensorVal getSensorValue(void)
{
  SensorVal sensorVal;

  ADC_SoftwareStartConvCmd(ADC1, DISABLE);

  sensorVal.gyro_x = (uint16_t)((ADC_values[0]                   + 
                                 ADC_values[0 + CHANNEL_NUM]     + 
                                 ADC_values[0 + CHANNEL_NUM * 2] + 
                                 ADC_values[0 + CHANNEL_NUM * 3]) / 4);

  sensorVal.gyro_y = (uint16_t)((ADC_values[1]                   + 
                                 ADC_values[1 + CHANNEL_NUM]     + 
                                 ADC_values[1 + CHANNEL_NUM * 2] + 
                                 ADC_values[1 + CHANNEL_NUM * 3]) / 4);

  sensorVal.gyro_z = (uint16_t)((ADC_values[2]                   + 
                                 ADC_values[2 + CHANNEL_NUM]     + 
                                 ADC_values[2 + CHANNEL_NUM * 2] + 
                                 ADC_values[2 + CHANNEL_NUM * 3]) / 4);

  sensorVal.water_height = (uint16_t)((ADC_values[3]                   + 
                                       ADC_values[3 + CHANNEL_NUM]     + 
                                       ADC_values[3 + CHANNEL_NUM * 2] + 
                                       ADC_values[3 + CHANNEL_NUM * 3]) / 4);

  sensorVal.temperature = (uint16_t)((ADC_values[4]                   + 
                                      ADC_values[4 + CHANNEL_NUM]     + 
                                      ADC_values[4 + CHANNEL_NUM * 2] + 
                                      ADC_values[4 + CHANNEL_NUM * 3]) / 4);

  sensorVal.soil_moisture = (uint16_t)((ADC_values[5]                   + 
                                        ADC_values[5 + CHANNEL_NUM]     + 
                                        ADC_values[5 + CHANNEL_NUM * 2] + 
                                        ADC_values[5 + CHANNEL_NUM * 3]) / 4);
                                        
  sensorVal.vibration = (uint16_t)((ADC_values[6]                   + 
                                    ADC_values[6 + CHANNEL_NUM]     + 
                                    ADC_values[6 + CHANNEL_NUM * 2] + 
                                    ADC_values[6 + CHANNEL_NUM * 3]) / 4);
  __status = 0;
  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  
  return sensorVal;
}

void NVIC_Configure_sensor(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  //Enable DMA1 channel IRQ Channel */
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel1_IRQn;
  // Sensor의 입력을 가장 최우선으로 받아옴.
  // Sensor > Bluetooth > TIME
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void sensorInit(void)
{
  RCC_Configure_sensor();
  GPIO_Configure_sensor();
  NVIC_Configure_sensor();
  ADC_Configure_sensor();
  DMA_Configure_sensor();

  ADC_SoftwareStartConvCmd(ADC1, ENABLE);
  
  printf("SENSOR INIT\n");
}

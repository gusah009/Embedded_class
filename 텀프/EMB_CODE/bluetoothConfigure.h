#ifndef __BLUETOOTH_H_
#define __BLUETOOTH_H_

#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_usart.h"


void RCC_Configure_bluetooth(void);
void GPIO_Configure_bluetooth(void);
void USART1_Init(void);
void USART2_Init(void);
void NVIC_Configure_bluetooth(void);
void bluetoothInit(void);

#endif
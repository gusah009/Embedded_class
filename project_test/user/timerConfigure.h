#ifndef __TIMER_H_
#define __TIMER_H_

#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"

void RCC_Configure(void);
void GPIO_Configure(void);
void PWM_Configure(void);
void NVIC_Configure(void);
void timerInit(void);

#endif
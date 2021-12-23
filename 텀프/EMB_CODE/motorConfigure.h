#ifndef __MOTOR_H_
#define __MOTOR_H_

#include "stm32f10x.h"
#include "core_cm3.h"
#include "misc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_rcc.h"

void RCC_Configure_motor(void);
void GPIO_Configure_motor(void);
void motorInit(void);

#endif
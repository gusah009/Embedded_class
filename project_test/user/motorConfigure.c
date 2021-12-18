#include "motorConfigure.h"

void RCC_Configure_motor(void)
{
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE); // port A
}

void GPIO_Configure_motor(void)
{
  //--Enable GPIOA--
  GPIO_InitTypeDef GPIOE_InitStructure;

  // 모터 드라이버
  // Speed나 Mode 미완성
  GPIO_StructInit(&GPIOE_InitStructure);
  GPIOE_InitStructure.GPIO_Pin = GPIO_Pin_0;
  GPIOE_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIOE_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOE, &GPIOE_InitStructure);

}

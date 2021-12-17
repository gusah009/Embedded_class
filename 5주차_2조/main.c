#include "stm32f10x.h"

void delay()
{
  int i;
  for (i = 0; i < 10000000; i++)
  {
  }
}

int main(void)
{
  *((volatile unsigned int *)0x40021018) |= 0x78; // RCC clock enable B, C, D, E

  *((volatile unsigned int *)0x40011000) &= 0xff0000ff; // port c init
  *((volatile unsigned int *)0x40011000) |= 0x00888800; // port c set to input

  *((volatile unsigned int *)0x40011404) &= 0xffff0fff; // port d init
  *((volatile unsigned int *)0x40011404) |= 0x00008000; // port d set to input

  *((volatile unsigned int *)0x40011800) &= 0xfffffff0; // port e init
  *((volatile unsigned int *)0x40011800) |= 0x3;        // port e set to output

  while (1)
  {

    if ((~(*((volatile unsigned int *)0x40011008)) & (1 << 5))) // port c 5 IDR UP
    {
      *((volatile unsigned int *)0x4001180C) |= 0x1; // port e 0
      delay();
    }
    if ((~(*((volatile unsigned int *)0x40011408)) & (1 << 11))) // port d 11
    {
      *((volatile unsigned int *)0x4001180C) &= 0xfffffffe; // port e 0
      delay();
    }
  }
}
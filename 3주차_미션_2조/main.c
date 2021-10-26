#include "stm32f10x.h"
int main(void)
{
  *((volatile unsigned int *)0x40021018) |= 0x78; // RCC clock enable B, C, D
  
  *((volatile unsigned int *)0x40011000) &= 0xff0000ff; // port c init
  *((volatile unsigned int *)0x40011000) |= 0x00888800; // port c set to input
  
  *((volatile unsigned int *)0x40011400) &= 0x0ff000ff; // port d init
  *((volatile unsigned int *)0x40011400) |= 0x30033300; //port d set to output
  
  *((volatile unsigned int *)0x40011404) &= 0x0ff000ff; // port d init  
  *((volatile unsigned int *)0x40011404) |= 0x00008000; //port d set to output->d11 input  
  
  *((volatile unsigned int *)0x40011800) &= 0xfffffff0; // port e init
  *((volatile unsigned int *)0x40011800) |= 0x3; // port e set to input ->output
  
  unsigned int flag = 0;
  while(1)
  {

    if ((~(*((volatile unsigned int *)0x40011008)) & (1 << 5))) // port c 5 IDR UP
    {
      *((volatile unsigned int *)0x4001180C) |= 0x1; // port e
    }
    if ((~(*((volatile unsigned int *)0x40011408)) & (1 << 11))) // port d11 
    {
      *((volatile unsigned int *)0x4001180C) &= 0xfffffffe; // port e
    }
    
    //-------------------------------
    if ((~(*((volatile unsigned int *)0x40011408)) & (1 << 11))) // port d11 
    {
      *((volatile unsigned int *)0x40011410) |= 0x0000000c; // port d 2 3 ON
    }
    
    if ((~(*((volatile unsigned int *)0x40011008)) & (1 << 5))) // port c 5 IDR UP
    {
      *((volatile unsigned int *)0x40011410) |= 0x00000090; // port d 4 7 ON
    }


    if ((~(*((volatile unsigned int *)0x40011008)) & (1 << 4))) // port c 4 IDR RIGHT
    {
      *((volatile unsigned int *)0x40011410) |= 0x000c0000; // port d 2 3 OFF
    }

    if ((~(*((volatile unsigned int *)0x40011008)) & (1 << 2))) // port c 2 IDR DOWN
    {
      *((volatile unsigned int *)0x40011410) |= 0x00900000; // port d 4 7 OFF
    }
  }
}
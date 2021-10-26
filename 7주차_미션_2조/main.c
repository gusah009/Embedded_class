
#include "stm32f10x.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"

#include "misc.h"

#define FORWARD 1
#define REVERSE -1

/* global variable */
int flag = FORWARD;

/* function prototype */
void RCC_Configure(void);
void GPIO_Configure(void);
void EXTI_Configure(void);
void USART1_Init(void);
void NVIC_Configure(void);

void EXTI15_10_IRQHandler(void);
void Delay(void);

void sendDataUART1(uint16_t data);

//---------------------------------------------------------------------------------------------------

void RCC_Configure(void)
{
   // TODO: Enable the APB2 peripheral clock using the function 'RCC_APB2PeriphClockCmd'

   /* UART TX/RX port clock enable */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // port A
   /* JoyStick Up/Down port clock enable */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // port C
   /* JoyStick Selection port clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // port B
   /* LED port clock enable && S1 Port clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); // port D
   /* USART1 clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); // USART1
   /* Alternate Function IO clock enable */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configure(void)
{
    // TODO: Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' and the function 'GPIO_Init'

    /* JoyStick up, down pin setting */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5; // 2,3,4,5번 핀 사용
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; // Default Speed
    GPIO_InitStructure.GPIO_Mode = (GPIO_Mode_IPD) | (GPIO_Mode_IPU); // input pull down, pull up
    GPIO_Init(GPIOC, &GPIO_InitStructure); // port C에 적용
    
    /* button pin setting */
    GPIO_InitTypeDef GPIO_InitStructure2;
    GPIO_InitStructure2.GPIO_Pin = GPIO_Pin_11; // 11번핀 사용
    GPIO_InitStructure2.GPIO_Speed = GPIO_Speed_50MHz; // Default Speed
    GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_IPD; // input pull down
    GPIO_Init(GPIOD, &GPIO_InitStructure2); // port D에 적용   
    
    /* LED pin setting*/
    GPIO_InitTypeDef GPIO_InitStructure3;
    GPIO_InitStructure3.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_7;
    GPIO_InitStructure3.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure3.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure3);
   
    /* UART pin setting */
    //TX a9
    GPIO_InitTypeDef GPIO_InitStructure4;
    GPIO_InitStructure4.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure4.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure4.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure4); 
   
    //RX a10
    GPIO_InitTypeDef GPIO_InitStructure5;
    GPIO_InitStructure5.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure5.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure5.GPIO_Mode = (GPIO_Mode_IPD) | (GPIO_Mode_IPU);
    GPIO_Init(GPIOA, &GPIO_InitStructure5);
   
}

void EXTI_Configure(void)
{
    EXTI_InitTypeDef EXTI_InitStructure;

   // TODO: Select the GPIO pin (Joystick, button) used as EXTI Line using function 'GPIO_EXTILineConfig'
   // TODO: Initialize the EXTI using the structure 'EXTI_InitTypeDef' and the function 'EXTI_Init'
   
    /* Joystick Down */
   GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);
    EXTI_InitStructure.EXTI_Line = EXTI_Line2;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* Joystick Up */
   GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource5);
    EXTI_InitStructure.EXTI_Line = EXTI_Line5;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
   
    /* S1 Button */
   GPIO_EXTILineConfig(GPIO_PortSourceGPIOD, GPIO_PinSource11);
    EXTI_InitStructure.EXTI_Line = EXTI_Line11;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);
   
   // NOTE: do not select the UART GPIO pin used as EXTI Line here
}

void USART1_Init(void)
{
   USART_InitTypeDef USART1_InitStructure;

   // Enable the USART1 peripheral
   USART_Cmd(USART1, ENABLE);
   
   // TODO: Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600; // Baud Rate 9600
    USART_InitStructure.USART_WordLength = USART_WordLength_8b; // word length 8bit
    USART_InitStructure.USART_StopBits = USART_StopBits_1; // stop bit 1bit
    USART_InitStructure.USART_Parity = USART_Parity_No ; // no parity bits
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // rx&tx mode
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
    USART_Init(USART1,&USART_InitStructure); // USART1에 적용
   
   // TODO: Enable the USART1 RX interrupts using the function 'USART_ITConfig' and the argument value 'Receive Data register not empty interrupt'
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
   
}

void NVIC_Configure(void) {

    NVIC_InitTypeDef NVIC_InitStructure;
    
    // TODO: fill the arg you want
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

   // TODO: Initialize the NVIC using the structure 'NVIC_InitTypeDef' and the function 'NVIC_Init'
    
    // Joystick Down
    NVIC_InitStructure.NVIC_IRQChannel = EXTI2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // Joystick Up
    NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);    
    // User S1 Button
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 7;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    // UART1
   // 'NVIC_EnableIRQ' is only required for USART setting
    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void USART1_IRQHandler() {
   uint16_t word;
    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET){
        
       // the most recent received data by the USART1 peripheral
        word = USART_ReceiveData(USART1);
        // TODO implement
        if (word == 'a') { // Receive 'a'
            flag = FORWARD;
        } else if (word == 'b') { // Receive 'b'
            flag = REVERSE;
        }

        // clear 'Read data register not empty' flag
       USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
}

void EXTI15_10_IRQHandler(void) { // when the button is pressed

   if (EXTI_GetITStatus(EXTI_Line11) != RESET) {
      if (GPIO_ReadInputDataBit(GPIOD, GPIO_Pin_11) == Bit_RESET) {
         // TODO implement
        
         sendDataUART1('T');
         sendDataUART1('E');
         sendDataUART1('A');
         sendDataUART1('M');
         sendDataUART1('0');
         sendDataUART1('2');
         sendDataUART1('.');
         sendDataUART1('\r');
         sendDataUART1('\n');
      }
        EXTI_ClearITPendingBit(EXTI_Line11);
   }
}

// TODO: Create Joystick interrupt handler functions
// DOWN
void EXTI2_IRQHandler(void) {

   if (EXTI_GetITStatus(EXTI_Line2) != RESET) {
      if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) == Bit_RESET) {
         // TODO implement
            flag = REVERSE;
      }
        EXTI_ClearITPendingBit(EXTI_Line2);
   }
}

// UP
void EXTI9_5_IRQHandler(void) {

   if (EXTI_GetITStatus(EXTI_Line5) != RESET) {
      if (GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_5) == Bit_RESET) {
         // TODO implement
            flag = FORWARD;
      }
        EXTI_ClearITPendingBit(EXTI_Line5);
   }
}

void Delay(void) {
   int i;

   for (i = 0; i < 2000000; i++) {}
}

void sendDataUART1(uint16_t data) {
   /* Wait till TC is set */
   while ((USART1->SR & USART_SR_TC) == 0);
   USART_SendData(USART1, data);
}

int main(void)
{

    SystemInit();

    RCC_Configure();

    GPIO_Configure();

    EXTI_Configure();

    USART1_Init();

    NVIC_Configure();

    while (1) {
       // TODO: implement 
        uint16_t arr[4][4] ={
            {UINT16_MAX, 0 ,0 ,0},
            {0, UINT16_MAX ,0 ,0},
            {0, 0 ,UINT16_MAX ,0},
            {0, 0 ,0 ,UINT16_MAX}};

        
       for (int j = 0; j < 4; j++) {
            GPIOD->ODR = ((GPIO_ODR_ODR2 & arr[(j * flag + (flag == REVERSE ? 3 : 0))][0]) | // port d2
                         (GPIO_ODR_ODR3 & arr[(j * flag + (flag == REVERSE ? 3 : 0))][1]) | // port d3
                         (GPIO_ODR_ODR4 & arr[(j * flag + (flag == REVERSE ? 3 : 0))][2]) | // port d4
                         (GPIO_ODR_ODR7 & arr[(j * flag + (flag == REVERSE ? 3 : 0))][3])); // port d7
            Delay(); // Delay
        }
    }
    return 0;
}

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
void USART1_Init(void);
void USART2_Init(void);
void NVIC_Configure(void);

void sendDataUART1(uint16_t data);
void sendDataUART2(uint16_t data);

//---------------------------------------------------------------------------------------------------

void RCC_Configure(void)
{
   // TODO: Enable the APB2 peripheral clock using the function 'RCC_APB2PeriphClockCmd'

   /* UART TX/RX port clock enable */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE); // port A
   /* USART1 clock enable */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE); // USART1
   /* USART2 clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); // USART2
   /* Alternate Function IO clock enable */
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
}

void GPIO_Configure(void)
{
    // TODO: Initialize the GPIO pins using the structure 'GPIO_InitTypeDef' and the function 'GPIO_Init'

    /* UART1 pin setting */
    //TX a9
    GPIO_InitTypeDef GPIO_InitStructure1;
    GPIO_InitStructure1.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure1.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure1.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure1); 
   
    //RX a10
    GPIO_InitTypeDef GPIO_InitStructure2;
    GPIO_InitStructure2.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure2.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure2.GPIO_Mode = (GPIO_Mode_IPD) | (GPIO_Mode_IPU);
    GPIO_Init(GPIOA, &GPIO_InitStructure2);
    
    /* UART2 pin setting */
    //TX a2
    GPIO_InitTypeDef GPIO_InitStructure3;
    GPIO_InitStructure3.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure3.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure3.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure3); 
   
    //RX a3
    GPIO_InitTypeDef GPIO_InitStructure4;
    GPIO_InitStructure4.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure4.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure4.GPIO_Mode = (GPIO_Mode_IPD) | (GPIO_Mode_IPU);
    GPIO_Init(GPIOA, &GPIO_InitStructure4);
   
}

void USART1_Init(void)
{
   // Enable the USART1 peripheral
   USART_Cmd(USART1, ENABLE);
   
   // TODO: Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'
    USART_InitTypeDef USART1_InitStructure;
    USART1_InitStructure.USART_BaudRate = 9600; // Baud Rate 9600
    USART1_InitStructure.USART_WordLength = USART_WordLength_8b; // word length 8bit
    USART1_InitStructure.USART_StopBits = USART_StopBits_1; // stop bit 1bit
    USART1_InitStructure.USART_Parity = USART_Parity_No ; // no parity bits
    USART1_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // rx&tx mode
    USART1_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
    USART_Init(USART1,&USART1_InitStructure); // USART1       
   
   // TODO: Enable the USART1 RX interrupts using the function 'USART_ITConfig' and the argument value 'Receive Data register not empty interrupt'
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
   
}

void USART2_Init(void)
{
   // Enable the USART2 peripheral
   USART_Cmd(USART2, ENABLE);
   
   // TODO: Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'
    USART_InitTypeDef USART2_InitStructure;
    USART2_InitStructure.USART_BaudRate = 9600; // Baud Rate 9600
    USART2_InitStructure.USART_WordLength = USART_WordLength_8b; // word length 8bit
    USART2_InitStructure.USART_StopBits = USART_StopBits_1; // stop bit 1bit
    USART2_InitStructure.USART_Parity = USART_Parity_No ; // no parity bits
    USART2_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // rx&tx mode
    USART2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  
    USART_Init(USART2,&USART2_InitStructure); // USART2       
   
   // TODO: Enable the USART2 RX interrupts using the function 'USART_ITConfig' and the argument value 'Receive Data register not empty interrupt'
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
   
}
void NVIC_Configure(void) {
    
   // TODO: Initialize the NVIC using the structure 'NVIC_InitTypeDef' and the function 'NVIC_Init'
    NVIC_InitTypeDef NVIC_InitStructure;

    // TODO: fill the arg you want
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    NVIC_EnableIRQ(USART1_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    
    // UART2
   // 'NVIC_EnableIRQ' is only required for USART setting
    NVIC_EnableIRQ(USART2_IRQn);
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void USART1_IRQHandler() {
   uint16_t word;
    if(USART_GetITStatus(USART1,USART_IT_RXNE)!=RESET){
        
       // the most recent received data by the USART1 peripheral
        word = USART_ReceiveData(USART1);
        sendDataUART2(word); // send to USART2
        sendDataUART1(word); // send to USART1

        // clear 'Read data register not empty' flag
       USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
}

void USART2_IRQHandler() {
   uint16_t word;
   if(USART_GetITStatus(USART2,USART_IT_RXNE)!=RESET){
      
      // the most recent received data by the USART2 peripheral
      word = USART_ReceiveData(USART2);
      sendDataUART1(word); // send to USART1

      // clear 'Read data register not empty' flag
      USART_ClearITPendingBit(USART2,USART_IT_RXNE);
   }
}

void sendDataUART1(uint16_t data) {
   USART_SendData(USART1, data);
   /* Wait till TC is set */
   // while ((USART1->SR & USART_SR_TC) == 0);
}

void sendDataUART2(uint16_t data) {
   USART_SendData(USART2, data);
   /* Wait till TC is set */
   // while ((USART2->SR & USART_SR_TC) == 0);
}
int main(void)
{

    SystemInit();

    RCC_Configure();

    GPIO_Configure();

   //  EXTI_Configure();

    USART1_Init();

    USART2_Init();
    
    NVIC_Configure();

    while (1) {
    }
    return 0;
}
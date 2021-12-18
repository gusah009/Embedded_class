#include "bluetoothConfigure.h"

void RCC_Configure_bluetooth(void)
{
  /* UART TX/RX port clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
  /* Alternate Function IO clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
  /* USART1 clock enable */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

  /* USART2 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
}

void GPIO_Configure_bluetooth(void)
{
  //--Enable GPIOA--
  GPIO_InitTypeDef GPIOA_InitStructure;

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

  /* UART2 pin setting - Bluetooth 모듈 */
  //TX a2
  GPIO_StructInit(&GPIOA_InitStructure);
  GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_2;
  GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIOA_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_Init(GPIOA, &GPIOA_InitStructure);

  //RX a3
  GPIO_StructInit(&GPIOA_InitStructure);
  GPIOA_InitStructure.GPIO_Pin = GPIO_Pin_3;
  GPIOA_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIOA_InitStructure.GPIO_Mode = (GPIO_Mode_IPD) | (GPIO_Mode_IPU);
  GPIO_Init(GPIOA, &GPIOA_InitStructure);
}

void USART1_Init(void)
{
  // Enable the USART1 peripheral
  USART_Cmd(USART1, ENABLE);

  // TODO: Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'
  USART_InitTypeDef USART1_InitStructure;
  USART1_InitStructure.USART_BaudRate = 9600;                      // Baud Rate 9600
  USART1_InitStructure.USART_WordLength = USART_WordLength_8b;     // word length 8bit
  USART1_InitStructure.USART_StopBits = USART_StopBits_1;          // stop bit 1bit
  USART1_InitStructure.USART_Parity = USART_Parity_No;             // no parity bits
  USART1_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // rx&tx mode
  USART1_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(USART1, &USART1_InitStructure); // USART1

  // TODO: Enable the USART1 RX interrupts using the function 'USART_ITConfig' and the argument value 'Receive Data register not empty interrupt'
  USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
}

void USART2_Init(void)
{
  // Enable the USART2 peripheral
  USART_Cmd(USART2, ENABLE);

  // TODO: Initialize the USART using the structure 'USART_InitTypeDef' and the function 'USART_Init'
  USART_InitTypeDef USART2_InitStructure;
  USART2_InitStructure.USART_BaudRate = 9600;                      // Baud Rate 9600
  USART2_InitStructure.USART_WordLength = USART_WordLength_8b;     // word length 8bit
  USART2_InitStructure.USART_StopBits = USART_StopBits_1;          // stop bit 1bit
  USART2_InitStructure.USART_Parity = USART_Parity_No;             // no parity bits
  USART2_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; // rx&tx mode
  USART2_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_Init(USART2, &USART2_InitStructure); // USART2

  // TODO: Enable the USART2 RX interrupts using the function 'USART_ITConfig' and the argument value 'Receive Data register not empty interrupt'
  USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
}

void NVIC_Configure_bluetooth(void)
{

  NVIC_InitTypeDef NVIC_InitStructure;

  // UART1
  // 'NVIC_EnableIRQ' is only required for USART setting
  NVIC_EnableIRQ(USART1_IRQn);
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  // UART2
  NVIC_EnableIRQ(USART2_IRQn);
  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

void USART1_IRQHandler()
{
  uint16_t word;
  if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
  {

    // the most recent received data by the USART1 peripheral
    word = USART_ReceiveData(USART1);
    sendDataUART2(word); // send to USART2
    sendDataUART1(word); // send to USART1

    // clear 'Read data register not empty' flag
    USART_ClearITPendingBit(USART1, USART_IT_RXNE);
  }
}

void USART2_IRQHandler()
{
  uint16_t word;
  if (USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
  {

    // the most recent received data by the USART2 peripheral
    word = USART_ReceiveData(USART2);
    sendDataUART1(word); // send to USART1

    // clear 'Read data register not empty' flag
    USART_ClearITPendingBit(USART2, USART_IT_RXNE);
  }
}

void sendDataUART1(uint16_t data)
{
  USART_SendData(USART1, data);
  /* Wait till TC is set */
  // while ((USART1->SR & USART_SR_TC) == 0);
}

void sendDataUART2(uint16_t data)
{
  USART_SendData(USART2, data);
  /* Wait till TC is set */
  // while ((USART2->SR & USART_SR_TC) == 0);
}

void bluetoothInit(void)
{
  RCC_Configure_bluetooth();
  GPIO_Configure_bluetooth();
  NVIC_Configure_bluetooth();
  USART1_Init();
  USART2_Init();
}

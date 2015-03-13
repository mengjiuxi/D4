/*
 * IncFile1.h
 *
 * Created: 05/10/2012 21:47:31
 *  Author: hslovett
 */ 


#ifndef UART_H_
#define UART_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#define UART0

volatile char Received0;

#ifdef UART0 //UART0 methods
#define USART0_BITRATE 57600
#define MYUBBR0 F_CPU/16/USART0_BITRATE-1

void USART0_Init (unsigned int ubrr);
void USART0_SendChar(char data );
unsigned char USART0_Receive( void );
void USART0_SendString(char str[]);
void USART0_SendInt( int Data);
void USART0_SendHex( uint8_t Data);
void USART0_Senduint16 (uint16_t Data);
void USART0_GetLine(char *buff, uint8_t len);
#endif



#endif /* INCFILE1_H_ */
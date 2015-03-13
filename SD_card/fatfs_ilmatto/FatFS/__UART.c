/*
 * UART.c
 *	Uart methods
 * Created: 05/10/2012 21:47:43
 *  Author: seblovett
 */ 
// #include <avr/io.h>
// #include <avr/interrupt.h>
#include "UART.h"
#include <stdlib.h>

void USART0_Init (unsigned int ubrr)
{
	//Set baud rate
	UBRR0H = (unsigned char)(ubrr>>8);
	UBRR0L = (unsigned char)ubrr;
	//Enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	
	UCSR0C = 0x06; //set asynchronous, no parity, one stop bit, 8 bit transfer.
	
	//UCSR0B |= (1 << RXCIE0); //set RX interrupt on

}
void USART0_SendChar(char data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) )
	;
	/* Put data into buffer, sends the data */
	UDR0 = data;
}
void USART0_SendInt( int Data)
{
	char buff[5];
	itoa(Data, buff, 10);
	USART0_SendString(buff);
}
void USART0_SendHex( uint8_t Data)
{
	char buff[5];
	itoa(Data, buff, 16);
	USART0_SendString("0x");
	USART0_SendString(buff);
}
unsigned char USART0_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)) )
	;
	/* Get and return received data from buffer */
	return UDR0;
}
void USART0_Senduint16 (uint16_t Data)
{	
	USART0_SendChar(Data >> 8);
	USART0_SendChar(Data & 0xFF);
}
ISR(USART0_RX_vect) //trigger interrupt when uart1 receives data   USART_RXC1_vect
{
	// Code to be executed when the USART receives a byte here
	Received0 = UDR0; // Fetch the received byte value into the variable "Received0"
	USART0_SendChar(Received0);
}

void USART0_SendString( char str[] )
{
	uint8_t ptr = 0;
	while(str[ptr])
	{
		USART0_SendChar(str[ptr++]);
	}
	
}

void USART0_GetLine(char *buff, uint8_t len)
{
	USART0_SendString("\n\r>");
	uint8_t i;
	char c;

	i = 0;
	for (;;) 
	{
		c = USART0_Receive();
		USART0_SendChar(c);
		if (c == '\r') //Return Char
			break;
		if ((c == '\b') && i) //backspace
			i--;
		if ((c >= ' ') && (i < len - 1))//character, so long as within the bounds of the buffer
			buff[i++] = c;
	}
	buff[i] = 0;
	USART0_SendChar('\n');
}

/*
 * Lab2.c
 *
 * Created: 4/18/2017 3:48:34 PM
 * Author : Dustin
 */ 

#include <avr/io.h>
#include <stdio.h>

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

void UART_Init(unsigned int ubrr)
{
	UBRR0H = (unsigned char) (ubrr>>8);
	UBRR0L = (unsigned char) ubrr;
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
}

void USART_Transmit(unsigned char data)
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)));
	/* Put data into buffer, sends the data */
	UDR0 = data;
}

unsigned char USART_Receive( void )
{
	/* Wait for data to be received */
	while ( !(UCSR0A & (1<<RXC0)));
	/* Get and return received data from buffer */
	return UDR0;
}

int main(void)
{
	unsigned char last_received = 'r';
	UART_Init(MYUBRR);
	DDRB |= 1<<DDB5;
	while(1){
		unsigned char receive;
		receive = USART_Receive();
		if(receive == 'q' && last_received != 'q'){
			USART_Transmit('O');
			USART_Transmit('N');
			USART_Transmit(' ');
			last_received = receive;
			PORTB |= 1<<PORTB5;
			
		}
		else if (receive == 'r' && last_received != 'r'){
			USART_Transmit('O');
			USART_Transmit('F');
			USART_Transmit('F');
			USART_Transmit(' ');
			last_received = receive;
			PORTB &= ~(1<<PORTB5);
				
		}
		
	}
return 0;
}


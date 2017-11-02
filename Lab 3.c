/*
 * Lab3.c
 *
 * Created: 4/25/2017 4:15:57 PM
 * Author : Dustin
 */ 

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/io.h>
#include <util/delay.h> 

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

void LcdCommandWrite_UpperNibble(unsigned char cm){
	PORTC = (PORTC & 0xf0) | (cm >> 4);
	PORTC &= ~(1<<4); // RS = 0
	PORTC |= 1<<5; // E = 1
	_delay_ms(1);
	PORTC &= ~(1<<5); // E = 0
	_delay_ms(1);
}
	
void LcdCommandWrite(unsigned char cm){
	// Write upper nibble
	PORTC = (PORTC & 0xf0) | (cm >> 4);
	PORTC &= ~(1<<4); // RS = 0
	PORTC |= 1<<5; // E = 1
	_delay_ms(1);
	PORTC &= ~(1<<5); // E = 0
	_delay_ms(1);
	
	// Write lower nibble
	PORTC = (PORTC & 0xf0) | (cm & 0x0f);
	PORTC &= ~(1<<4); // RS = 0
	PORTC |= 1<<5; // E = 1
	_delay_ms(1);
	PORTC &= ~(1<<5); // E = 0
	_delay_ms(1);
}

void LcdDataWrite(unsigned char cm)
{
	// Write upper nibble
	PORTC = (PORTC & 0xf0) | (cm >> 4);
	PORTC |= 1<<4; // RS = 1
	PORTC |= 1<<5; // E = 1
	_delay_ms(1);
	PORTC &= ~(1<<5); // E = 0
	_delay_ms(1);
	
	// Write lower nibble
	PORTC = (PORTC & 0xf0) | (cm & 0x0f);
	PORTC |= 1<<4; // RS = 1
	PORTC |= 1<<5; // E = 1
	_delay_ms(1);
	PORTC &= ~(1<<5); // E = 0
	_delay_ms(1);
}

void LCD_Init( void ){
	LcdCommandWrite_UpperNibble(0x30);
	_delay_ms(4.1);
	LcdCommandWrite_UpperNibble(0x30);
	_delay_us(100);
	LcdCommandWrite_UpperNibble(0x30);
	LcdCommandWrite_UpperNibble(0x20);
	LcdCommandWrite(0x28);
	 //function set: 0x28 means, 4-bit interface, 2 lines, 5x8 font
	LcdCommandWrite(0x08);
	//display control: turn display off, cursor off, no blinking
	LcdCommandWrite(0x01);
	// clear display, set address counter to zero
	LcdCommandWrite(0x06); // entry mode set:
	LcdCommandWrite(0x0f); // display on, cursor on, cursor blinking
	_delay_ms(120);
}

int main(void)
{
	UART_Init(MYUBRR);
	DDRC |= 1<<DDC0;
	DDRC |= 1<<DDC1;
	DDRC |= 1<<DDC2;
	DDRC |= 1<<DDC3;
	DDRC |= 1<<DDC4;
	DDRC |= 1<<DDC5;
	LCD_Init();

	unsigned char receive;
	int nextLineCounter = 0;
    while (1) 
    {
		if(nextLineCounter == 16)
			LcdCommandWrite(0xc0);
		if(nextLineCounter == 32){
			LcdCommandWrite(0x80);
			LcdCommandWrite (0x01);
			nextLineCounter = 0;
		}
			receive = USART_Receive();
			USART_Transmit(receive);
			LcdDataWrite(receive);
			nextLineCounter++;
	}
	return 0;
}


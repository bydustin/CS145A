/*
 * Lab 6.c
 *
 * Created: 6/1/2017 3:56:11 PM
 * Author : Dustin
 */ 

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <assert.h>
#include <inttypes.h>

volatile uint32_t duty_cycle;
volatile uint32_t time_period = 0;
volatile uint32_t duty_cycles;

#define DDR_SPI DDRB
#define DD_SS PB2
#define DD_MOSI PB3
#define DD_MISO PB4
#define DD_SCK PB5

int in_min = 0;
int in_max = 4096;
int out_min = 1229; //approx 1.5 V
int out_max = 4096; // approx 5.0 V

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

void InitADC( void ){
	ADMUX = (1<<REFS0) | (1 << MUX0); //5V Reference + ADC1 as Input
	ADCSRA |= (1<<ADPS2) |(1<<ADPS1) | (1<<ADPS0); //128 Pre-scalar
	ADCSRA |= (1<<ADEN); //Enable ADC
}

void InitTimer1( void ){
	//DDRB = (1 << PORTB3); //PB3 as Output Done in MasterInit
	TCCR1A = (1<<WGM11) | (1<<WGM10);
	TCCR1B = (1<<WGM13) | (1<<WGM12) | (1<<CS10);
	TIMSK1 = (1<<OCIE1A) | (1<<OCIE1B);
}

void StartConversion( void ){
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC)){}
}


void SPI_MasterInit( void ){
	//Sets MOSI(PB3) and SCK(PB5) as Outputs
	DDR_SPI =  (1<<DD_SCK) | (1<< DD_MOSI) | (1<<DD_SS) | (1 << PORTB0);
	PORTB |= (1<<DD_SS); //CS High;
	// Enable SPI / Atmega as Master Mode / Setup -> Sample / 16 Clock Rate
	SPCR |= (1<<SPE)|(1<<MSTR)|(1<<SPR0);
}

void SPI_MasterTransmit(uint8_t cData){
	//Start transmission
	SPDR = cData;
	//Wait for transmission to be complete
	while (!(SPSR & (1 << SPIF)));
}


int main(void)
{
	SPI_MasterInit();
	UART_Init(MYUBRR);
	InitTimer1();
	InitADC();
	sei();
	uint8_t config = 0b00111111;
	//uint16_t data = 0;
	OCR1A = time_period;
	/* Replace with your application code */
	while (1)
	{
		StartConversion();
		uint8_t theLowADC = ADCL;
		time_period = ADCH<<8 | theLowADC;
		time_period = time_period << 2;
		
		float ratio = 0.699951171875; //(out_max - out_min) / in_max;
		int flat_add = out_min / ratio;
		time_period = (time_period + flat_add) * ratio;	
		

		if(time_period <= 4091 && time_period > 1227){ //Ranges from 1227 - 4090
			USART_Transmit('S');
		}
		
		//time_period = 1229;
		uint8_t upper_byte = (config & 0xF0)|(0x0F & (time_period>>8));
		uint8_t lower_byte = (time_period & 0xFF);
		PORTB &= ~(1<<DD_SS); //CS Low
		SPI_MasterTransmit(upper_byte);
		SPI_MasterTransmit(lower_byte);			
		PORTB |= (1<<DD_SS); //CS High
		PORTB &= ~(1<< PORTB0); // Generate Low Pulse at LDAC after transmission
		OCR1A = time_period;
		_delay_ms(50);
	}
}

ISR (TIMER1_COMPA_vect){
	PORTB = 0xFF;
	OCR1A = time_period;
}

ISR (TIMER1_COMPB_vect){
	PORTB = 0x00;
}




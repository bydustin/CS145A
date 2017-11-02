/*
 * 145_Project5.c
 *
 * Created: 5/22/2017 1:37:35 PM
 * Author : Dustin Ngo
 */ 

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <assert.h>
#include <inttypes.h>

// set starting time_period to 16,000,000Mhz/15,250 is approx. ~1046Hz
volatile uint32_t duty_cycle;
volatile uint32_t time_period = 15250;
volatile uint32_t duty_cycles;

//ADC value ranges from 0-1023 and 7650-15250, which with pre-scaler of 1 gives us a frequency between ~1043Hz and ~2096Hz
int in_min = 0;
int in_max = 1023;
int out_min = 7650;
int out_max = 15250;

void InitADC( void ){
	ADMUX = (1<<REFS0) | (1 << MUX0); //5V Reference + ADC1 as Input
	ADCSRA |= (1<<ADPS2) |(1<<ADPS1) | (1<<ADPS0); //128 Pre-scalar
	ADCSRA |= (1<<ADEN); //Enable ADC
}

void InitTimer1( void ){
		DDRD = (1 << PORTD6); //PD6 as Output
		TCCR1A = (1<<WGM11) | (1<<WGM10);
		TCCR1B = (1<<WGM13) | (1<<WGM12) | (1<<CS10);
		TIMSK1 = (1<<OCIE1A) | (1<<OCIE1B);
}

void StartConversion( void ){
	ADCSRA |= (1<<ADSC);
	while(ADCSRA & (1<<ADSC)){}
}


int main(void){
	InitTimer1();
	InitADC();
	sei();
	OCR1A = time_period;
	while(1){
		StartConversion();
		time_period = ADC;
		time_period = ((time_period - in_min) * (out_max - out_min) / (in_max - in_min)) + out_min;
		// set Top time_period
		OCR1A = time_period;
		_delay_ms(10);
	}
	return 0;
}

ISR (TIMER1_COMPA_vect){
	PORTD = 0xFF;
	OCR1A = time_period;
}

ISR (TIMER1_COMPB_vect){
	PORTD = 0x00;
}






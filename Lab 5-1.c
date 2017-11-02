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
#include <avr/interrupt.h>
#include <util/delay.h>
unsigned char ADC_Input;


void startConversion(){
	ADCSRA |= (1 << ADSC); //Starts conversion     
}

void ADC_Setup(){
	ADMUX = (1 << REFS0) | (1 << MUX0); // 5V Reference + ADC1 as Input
	ADCSRA = (1 << ADEN) | (1 << ADIE) |(1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Enable ADC Converter + AD Interrupt Enabled + 128 Prescalar
}

void PWM_Setup(){
	DDRD = (1 << PORTD6); //PD6 as Output
	TCCR0A = (1 << COM0A1) | (1 << WGM00) | (1 << WGM01); //Clear on Match + 1028 Prescalar
	TIMSK0 = (1 << TOIE0); // Allows OVF on Timer
	OCR0A = 0;
	TCCR0B = (1 << CS00) | (1 << CS02); 
}



int main ( void ){
	sei();
	ADC_Setup();
	PWM_Setup();
	
	while(1){
		
		startConversion();

	}
	return 0;
	
}

ISR(ADC_vect){
	ADC_Input = ADCH;  
}

ISR(TIMER0_OVF_vect){
		if(ADC_Input == 3){
			if(OCR0A < 255)
			OCR0A += 1;
		}
		else if(ADC_Input == 2){
			if(OCR0A < 170)
			OCR0A += 1;
			else
			OCR0A -= 2;
		}
		else if(ADC_Input == 1){
			if(OCR0A < 85)
				OCR0A += 1;
			else{
				OCR0A -= 2;
			}

		}
		else if(ADC_Input == 0){
			if(OCR0A > 2)
			OCR0A -= 2;

		}
}



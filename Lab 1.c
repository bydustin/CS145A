/*
 * Lab1.c
 *
 * Created: 4/8/2017 2:35:08 AM
 * Author : Dustin
 */ 

#include <avr/io.h>


int main(void)
{
DDRB |= 1<<DDB5;
while(1)
{
	if(!(PINB & (1<<PINB7))) {
		PORTB &= ~(1<<PORTB5);
	}
	else {
		PORTB |= 1<<PORTB5;
	}
}
return 0;
}


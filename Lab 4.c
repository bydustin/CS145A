/*
 * 145_Project4.c
 *
 * Created: 5/9/2017 5:17:05 PM
 * Author : Billy Guan
 */
 
#define F_CPU 16000000UL

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define NOT_PUSHED 0
#define MAYBE_PUSHED 1
#define JUST_PUSHED 2
#define OLD_PUSHED 3
#define OLD_MAYBE_PUSHED 4

void LcdCommandWrite_UpperNibble(unsigned char cm);
void LcdCommandWrite(unsigned char cm);
void LcdDataWrite(unsigned char cm);
void initializeLcd();
void LcdWriteInt(int number);

void initTimer1();

void runButtonState();
void notPushedState();
void maybePushedState();
void justPushedState();
void oldPushedState();
void oldMaybePushedState();

volatile int seconds = 0;
volatile int mode = 0;
volatile int buttonState = 0;
volatile int restart = 0;

int main(void)
{
	DDRC |= 0x3f;
	initializeLcd();
	
	DDRD = 0x00;
	initTimer1();
	
	while (1)
	{
		runButtonState();
	}
	
	return 0;
}

void runButtonState(){
	switch(buttonState) {
		case NOT_PUSHED: {
			notPushedState();
			break;
		}
		case MAYBE_PUSHED: {
			maybePushedState();
			break;
		}
		case JUST_PUSHED: {
			justPushedState();
			break;
		}
		case OLD_PUSHED: {
			oldPushedState();
			break;
		}
		case OLD_MAYBE_PUSHED: {
			oldMaybePushedState();
			break;
		}
	}
	_delay_ms(100);
}

void notPushedState(){
	if ((PIND & (1<<PORTD3))) { //Button is pushed
		buttonState = MAYBE_PUSHED;
	}
	else {
		if (mode % 3 == 0) { //Reset Mode
			seconds = 0;
			LcdWriteInt(seconds);
		}
		else if (mode % 3 == 1) { //Count Mode
			if (restart == 1) {
				TCCR1B |= (1 << WGM12)|(1 << CS10)|(1 << CS12);
				restart = 0;
			}
		}
		else if (mode % 3 == 2) { //Stop mode
			TCCR1B = 0x00; //pauses the timer
			restart = 1;
			LcdWriteInt(seconds-1);
		}
	}	
}

void maybePushedState(){
	if ((PIND & (1<<PORTD3))) {
		buttonState = JUST_PUSHED;
	}
	else {
		buttonState = NOT_PUSHED;
	}	
}

void justPushedState(){
	if ((PIND & (1<<PORTD3))) {
		buttonState = OLD_PUSHED;
		LcdCommandWrite(0x01);
		mode = mode + 1;
		if (mode % 3 == 2) {
			TCCR1B = 0x00;
			restart = 1;
		}
	}
	else {
		buttonState = OLD_MAYBE_PUSHED;
	}	
}

void oldPushedState(){
	if (!(PIND & (1<<PORTD3))) {
		buttonState = OLD_MAYBE_PUSHED;
	}	
}

void oldMaybePushedState(){
	if ((PIND & (1<<PORTD3))) {
		buttonState = OLD_PUSHED;
	}
	else {
		buttonState = NOT_PUSHED;
	}	
}

void initTimer1(){
	 // Make sure the timer is really stopped
	 TCCR1A = 0;
	 TCCR1B = 0;
	 
	 //Set ticks to 15625, 1 tick = 62.5 nanoseconds
	 OCR1A = 0x3d09;
	 TCCR1B |= (1 << WGM12);
	 
	 //Set prescalar to 1024
	 TCCR1B |= (1<<CS12)|(1<<CS10);
	 
	 // initialize counter
	 TCNT1 = 0;
	 
	 TIMSK1 |= (1 << OCIE1A);
	 
	 // enable global interrupts
	 sei();
}

ISR (TIMER1_COMPA_vect) {
	 //action to be done every 1 second
	 LcdWriteInt(seconds);
	 if (seconds == 255){
		 seconds = 0;
	 }
	 else {
		 seconds++;
	 }
}

void LcdWriteInt(int number) {
	LcdCommandWrite(0x01);
	if (number < 10) {
		LcdDataWrite(number + '0');
	}
	else if (number >= 10 && number < 100) {
		int tenthDigit = number/10;
		LcdDataWrite(tenthDigit + '0');
		int oneDigit = number - (tenthDigit * 10);
		LcdDataWrite(oneDigit + '0');
	}
	else if (number >= 100) {
		int hundredthDigit = number/100;
		LcdDataWrite(hundredthDigit + '0');
		int tenthDigit = (number - (hundredthDigit * 100))/10;
		LcdDataWrite(tenthDigit + '0');
		int oneDigit = number - (hundredthDigit * 100) - (tenthDigit * 10);
		LcdDataWrite(oneDigit + '0');
	}	
}

void LcdCommandWrite_UpperNibble(unsigned char cm){
	PORTC = (PORTC & 0xf0) | (cm >> 4);
	PORTC &= ~(1 << 4);
	PORTC |= 1 << 5;
	_delay_ms(1);
	PORTC &= ~(1 << 5);
	_delay_ms(1);
}

void LcdCommandWrite(unsigned char cm){
	PORTC = (PORTC & 0xf0) | (cm >> 4);
	PORTC &= ~(1 << 4);
	PORTC |= 1 << 5;
	_delay_ms(1);
	PORTC &= ~(1 << 5);
	_delay_ms(1);
	
	PORTC = (PORTC & 0xf0) | (cm & 0x0f); //f0 doesn't work - it's hexadecimal so use 0xf0 and 0x0f
	PORTC &= ~(1 << 4);
	PORTC |= 1 << 5;
	_delay_ms(1);
	PORTC &= ~(1 << 5);
	_delay_ms(1);
}

void LcdDataWrite(unsigned char cm){
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

void initializeLcd(){
	LcdCommandWrite_UpperNibble(0x30);
	_delay_ms(4.1);
	LcdCommandWrite_UpperNibble(0x30);
	_delay_us(100);
	LcdCommandWrite_UpperNibble(0x30);
	LcdCommandWrite_UpperNibble(0x20);
	LcdCommandWrite(0x28); // function set: 0x28 means, 4-bit interface, 2 lines, 5x8 font
	LcdCommandWrite(0x08); // display control: turn display off, cursor off, no blinking
	LcdCommandWrite(0x01); // clear display, set address counter to zero
	LcdCommandWrite(0x06); // entry mode set:
	//LcdCommandWrite(0x0f); // display on, cursor on, cursor blinking
	LcdCommandWrite(0x0c); //display on, cursor off, cursor do not blink
	_delay_ms(120);
}
/*
 * LightstickSoftware.c
 *
 * Created: 6/22/2023 9:30:34 PM
 * Author : alial
 */

#ifndef F_CPU
#define F_CPU 8000000UL
#endif //F_CPU

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// THESE ARE FOR PORT B
#define bitmask_G 0b00000010 // Green channel is on TIM1A, PORTB1
#define bitmask_W 0b00000100 // White channel is on TIM1B, PORTB2
// THESE ARE FOR PORT D
#define bitmask_R 0b00000001 // Red channel is on TIM3A, PORTD0
#define bitmask_B 0b00000100 // Blue channel is on TIM3B, PORTD2

uint16_t colortable[][4] = {
	{0x1ff,	0x0,	0x0,	0x0},
	{0x0,	0x1ff,	0x0,	0x0},
	{0x0,	0x0,	0x1ff,	0x0},
	{0x0,	0x0,	0x0,	0x1ff}
};


// Interrupt subroutine for the button debouncer's 1ms timer
ISR(TIMER0_COMPA_vect) {
	// Do garbage here
}


int main(void)
{
	// Set GPIO direction for the 4 PWM channels
	DDRB |= (bitmask_G | bitmask_W);
	DDRD |= (bitmask_R | bitmask_B);

	// test
	PORTD |= (bitmask_B);

	// Enables interrupts globally by setting the I-bit of SREG
	sei();
	
	uint16_t dutycycle_R, dutycycle_G, dutycycle_B, dutycycle_W;
	// Set thresholds here from a list of colors and button toggling
	dutycycle_R = 0x0ff;
	dutycycle_G = 0x0;
	dutycycle_B = 0x1ff;
	dutycycle_W = 0x0;
	
	// Update the PWM generators with the new threshold
	OCR1A = dutycycle_G;
	OCR1B = dutycycle_W;
	OCR3A = dutycycle_R;
	OCR3B = dutycycle_B;
	
	// Set some data control registers.
	// COMn[1:0] = 2'b10, for "normal" PWM that widens with a larger dutycycle value
	// WGMn[3:0] = 4'b1110, for fast mode PWM with the counter's max value in ICRn
	// CSn[2:0] = 3'b001, to clock directly from clkio (no prescaler)
	// ICRn = 0x0fff, as our counter top, to create 12-bit PWM timer resolution
	TCCR1A |= (0b10 << COM1A0) | (0b10 << COM1B0) | (0b10 << WGM10);
	ICR1 = 0x0fff;
	TCCR1B |= (0b11 << WGM12) | (0b001 << CS10);
	TCCR3A |= (0b10 << COM3A0) | (0b10 << COM3B0) | (0b10 << WGM30);
	ICR3 = 0x0fff;
	TCCR3B |= (0b11 << WGM32) | (0b001 << CS30);
	
	// TIM0A makes an internal 1ms interrupt clock for switch debounce
	// clkio is 8 MHz, so clkio/64 and 125 counts gives us a perfect 1ms timer
	// WGM0[2:0] = 3'b010, for CTC (Clear Timer on Compare match) mode
	// CS0[2:0] = 3'b011, for clkio/64
	// Set OCIE0A to trigger an interrupt every time the compare matches
	OCR0A = 125;
	TIMSK0 |= (1 << OCIE0A);
	TCCR0A |= (0b10 << WGM00);
	TCCR0B |= (0b0 << WGM02) | (0b011 << CS00);

	// Forever loop here.
	while (1) {
		// abc
	}
}

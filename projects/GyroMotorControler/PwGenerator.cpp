/*
 * PwGenerator.cpp
 *
 *  Created on: 13.01.2013
 *  Updated on: 02.01.2018 - making it work and calibrate
 *  Updaten on: 07.12.2019 - mit Basis EcuSurface - für 328P minianpassung
 *      Author: CK
 */

// Description
//
// Uses Port C - Bit 0 till Bit 5 as Outputs
// Uses Timer1 with an Interrupt and in Counter Mode with OCR1A

// Measured Values with a real RC-Control (Hitec)
// Overallwidth = 20ms till 22ms
// Channelwidth = first try 1,4ms (zero Position) 1,9ms(max) 1,1ms(min) - calibrating to 0%: 1,56 till 1,96 and 1,1ms


#include "PwGenerator.h"
#include <avr/io.h>
#include <avr/interrupt.h>


#define OVERALLWIDTH 0xA000 // 20,4ms  (0x8000 is 8ms)
//#define CHANNELSTEPWIDTH 7 // makes 0,44ms by 127 Steps (Values 6 is 0,38ms by 127 Steps)
//#define CHANNELMIDWIDTH 0xC2C // 1,56ms  (0x1000 is 2,05ms)
#define CHANNELSTEPWIDTH 8 // makes 0,51ms by 127 Steps 
#define CHANNELMIDWIDTH 0xB9B // 1,5ms

void PwGenerator::Initialize()
{
	//// Using Timer 1 with 16bit Register
	//// Running in CTC-Modus with Interrupt to Output Compare Register A

	// Set Timer Prescaler (same Values as PwReader)
	// 010 CS10 - CS12
	// Prescaler -> divide by 8 from internal Clock
	TCCR1B |= (1<<CS11); 
	TCCR1B &= ~((1<<CS12) | (1<<CS10));
	

	// CTC-Modus for Timer 1 - Clear Timer or Compare Match mode)
	// Using OCR1A for Matching
	TCCR1A &= ~((1<<WGM11) | (1<<WGM10));
	TCCR1B |= (1<<WGM12);
	TCCR1B &= ~(1<<WGM13);

	// Using TIMER1_COMPA_vect for Interrupt
	// Setting TIMSK = Timer Interrupt Mask,
	//TIMSK |= (1 << OCIE1A); // Output Compare Interrupt Enable
	// ATMel328P: hier von TIMSK auf TIMSK1 geändert
	TIMSK1 = (1 << OCIE1A); // Output Compare Interrupt Enable
	// COM1A1 ... Bits not to be changed, because output pins OC1A/OC1B are not used.

	// Enable the port 
	DDRC  |= 0x3F;  // prepare outputports bit 0 to bit 5
	
	// Initialize the first channel
	channel = 0;
	PORTC = 0x01;  	// Set bit 0 to 1
	OCR1A = CHANNELMIDWIDTH + PwAdjustValues[channel] + (PwValues[channel] * CHANNELSTEPWIDTH); // set Timerwidth
}

void PwGenerator::TimerInterrupt()
{
	// Implements TIMER1_COMPA_vect

	// Clear Bit and select next channel
	if (channel >= 0) PORTC &= ~(1<<channel);
    channel += 1;
    if (channel <= 5) {
        // Set Channel Pin
        PORTC |= (1<<channel);
        // Set the Timer
        OCR1A = CHANNELMIDWIDTH + PwAdjustValues[channel] + (PwValues[channel] * CHANNELSTEPWIDTH);
    } else {
    	channel = -1; // Select -1 for the pause

		// Calculate the rest to be executed for the whole period
    	int i = 0;
    	uint16_t pausetimervalue = OVERALLWIDTH;
    	while (i<6) {
	    	pausetimervalue -= (CHANNELMIDWIDTH + PwAdjustValues[i] + PwValues[i] * CHANNELSTEPWIDTH);
	    	i++;
    	}
		// Set Timer
    	OCR1A = pausetimervalue;
    	PwMainCyclesCount += 1;
    }
}

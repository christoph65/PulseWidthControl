/*
* StickReader.cpp
*
* Created: 07.01.2020 21:33:59
* Author: Christoph
*/


#include "StickReader.h"
#include <avr/io.h>
#include <stdio.h>

#define defStep1InitX 1
#define defStep2ConvertingX 2
#define defStep3InitY 3
#define defStep4ConvertingY 4


void StickReader::Initialize()
{
	// Input frequency should be set between 50 and 200 khz for 10bit resolution, but can be faster.
	//   200.000 durch 1024 bedeutet ca. 200 samples pro Sekunde
	//   with 16 MHZ divided by 128 (highest) you get 125khz
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // Next is 64 and so on
	
	// Enable conversion
	// Disable Power Reduction bit PRADC to logical zero (not done because should not be in low power mode)
	
	// Enable the ADC
	ADCSRA |= _BV(ADEN);

	// Reference voltage external from AVCC at 5V
	ADMUX |= (1 << REFS0);
	ADMUX &= ~(1 << REFS1);

	iReaderState = defStep1InitX;
	bNotInMiddlePos = false;
}

void StickReader::TimerInterrupt()
{

}

void StickReader::ReadADC()
{
	// and 1 writing to ADSC (start conversion bit) in ADCSRA. Will stay to one as long a conversion is running
	//   so I should do alway pollng and switching the input chanel
	//   but auto triggering must be selected
	
	// Different input chanel can be selected but will first be activated when running conversion is ready
	// Select ADC7
	
	if(iReaderState == defStep1InitX) {
		ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0)); // all to zero
		ADMUX |= (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0);

		ADCSRA |= (1<<ADSC); // start conversion
		iReaderState = defStep2ConvertingX;
		
		
		
		
	}
	
	
	
	

	if(iReaderState == defStep2ConvertingX) {
		
		
		

		
		
		// wait for ADSC to get to zero to show that conversion is ready
		if (!(ADCSRA & _BV(ADSC))) {
			// Read Low and then High
			AdcValueADC7 = ADC;
			iReaderState = defStep3InitY;
		}
	}
	
	if(iReaderState == defStep3InitY) {
		// Different input chanel can be selected but will first be activated when running conversion is ready
		// Select ADC6
		ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0)); // all to zero
		ADMUX |= (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0);

		ADCSRA |= (1<<ADSC); // start conversion
		
		
		

		iReaderState = defStep4ConvertingY;
	}
	
	if(iReaderState == defStep4ConvertingY) {
		
		// wait for ADSC to get to zero to show that conversion is ready
		if ( !(ADCSRA & _BV(ADSC))) {
			// Read Low and then High
			AdcValueADC6 = ADC;
			
			
			
			

			// Print out to serial interface
			//sprintf (strptr, "ADC: %u  %u\n\r", AdcValueADC7, AdcValueADC6);
			//serialInterface.SendString(strptr);

			int16_t PwValueCalc16;
			// int8_t PwValueCalc8;
			PwValueCalc16 = (AdcValueADC6 - 487);
			// PwValueCalc8 = PwValueCalc16;
			StickX = PwValueCalc16;
			if (PwValueCalc16 > 127) StickX = 127;
			if (PwValueCalc16 < -127) StickX = -127;
			
			PwValueCalc16 = - (AdcValueADC7 - 497);
			// PwValueCalc8 = PwValueCalc16;
			StickY = PwValueCalc16;
			if (PwValueCalc16 > 127) StickY = 127;
			if (PwValueCalc16 < -127) StickY = -127;
			
			iReaderState = defStep1InitX;
			bNotInMiddlePos = true;
		}
	}
}
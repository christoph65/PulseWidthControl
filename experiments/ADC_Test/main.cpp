/*
 * PwGenerator
 * main.cpp
 *
 * Testapplication for SerialInterface class
 *
 * For Putty: don't forget to turn xon/xoff off (see settings for serial)
 *
 *  Created on: 16.12.2012
 *  Updated on: 02.01.2018 to make it work
 *  Updaten on: 07.12.2019 adaption for Arduino Nano
 *      Author: Christoph Kunz
 */

#include "SerialInterface.h"
#include "PwGenerator.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

SerialInterface serialInterface;
PwGenerator pwGenerator;

void echo() {
	char readChar;
	serialInterface.Cyclic();
	if (serialInterface.GetChar(&readChar)) {
		if (readChar == '\r') serialInterface.SendChar('\n');
		serialInterface.SendChar(readChar);
	}
	serialInterface.Cyclic();
}

class Application {

	public: void MyMain()
	{
		sei(); // Interrupts enablen
		
		// set inital values for the interface variables
		for (int i = 0; i<=5; i++) {
			pwGenerator.PwValues[i] = 0;
			pwGenerator.PwAdjustValues[i] = 0;
		}
		// Initialize Port and Timers
		pwGenerator.Initialize();

		// Say hello
		serialInterface.Initialize();
		char strptr[50];
		sprintf (strptr, "Started ADC Tester\n\r");
		serialInterface.SendString(strptr);

		//bool waitForRisingEdge;
		// waitForRisingEdge = true;
		// int8_t direction;
		// direction = 1;
		// int16_t lastValue;


		// Wake up Signal über die LED (3 mal blinken)
		// Erster Test ob es denn wirklich flasht
		DDRB |= 0x01;
		int j;
		
		for (j=0;j<3;j++) {
			echo();
			int i,k;
			pwGenerator.PwValues[0] = 50;
			for (k=0;k<10;k++) {
				for(i=0;i<2000;i++) {
					echo();			
					PORTB |= 0x01;
				}
			}
			for (k=0;k<10;k++) {
				for(i=0;i<2000;i++) {
					echo();
					PORTB &= 0xFE;
				}
			}
		}
		pwGenerator.PwValues[0] = 0;
		PORTB &= 0xFE;
	
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
		
		while (true) {
			echo();			
			// and 1 writing to ADSC (start conversion bit) in ADCSRA. Will stay to one as long a conversion is running
			//   so I should do alway pollng and switching the input chanel
			//   but auto triggering must be selected
			
			// Different input chanel can be selected but will first be activated when running conversion is ready
			// Select ADC7
			ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0)); // all to zero
			ADMUX |= (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0);

			ADCSRA |= (1<<ADSC); // start conversion
			
			// wait for ADSC to get to zero to show that conversion is ready
			while ( (ADCSRA & _BV(ADSC)) ) {
				echo();
			}
			
			// Read Low and then High
			uint16_t AdcValueADC7;
			AdcValueADC7 = ADC;

			// Different input chanel can be selected but will first be activated when running conversion is ready
			// Select ADC7
			ADMUX &= ~((1 << MUX3) | (1 << MUX2) | (1 << MUX1) | (1 << MUX0)); // all to zero
			ADMUX |= (0 << MUX3) | (1 << MUX2) | (1 << MUX1) | (0 << MUX0);

			ADCSRA |= (1<<ADSC); // start conversion
			
			// wait for ADSC to get to zero to show that conversion is ready
			while ( (ADCSRA & _BV(ADSC)) ) {
				echo();
			}
			
			// Read Low and then High
			uint16_t AdcValueADC6;
			AdcValueADC6 = ADC;

			
			// Print out to serial interface
			sprintf (strptr, "ADC: %u  %u\n\r", AdcValueADC7, AdcValueADC6);
			serialInterface.SendString(strptr);
			
			// Delay for a while an make a short blink of the LED
			echo();
			int i,k;
			for (k=0;k<3;k++) {
				for(i=0;i<8000;i++) {
					echo();
					PORTB |= 0x01;
				}
			}
			for (k=0;k<10;k++) {
				for(i=0;i<8000;i++) {
					echo();
					PORTB &= 0xFE;
				}
			}
			PORTB &= 0xFE;
		}
	}
} app;

extern "C" {
	//-----------------------------------------------------------------
	ISR (TIMER1_COMPA_vect)          // Interrupt 0 vector
	{
		pwGenerator.TimerInterrupt();
		//serialInterface.SendChar('i');
	}
}

int main(void)
{
	app.MyMain();
}


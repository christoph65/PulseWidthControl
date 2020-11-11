/*
 * Controler for manaaging my boat with a joystick
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
#include "StickReader.h"
#include "MotorControler.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

SerialInterface serialInterface;
PwGenerator pwGenerator;
StickReader stickReader;
MotorControler motorControler;
int iEvaluationCounter;

void echo() {
	char readChar;
	serialInterface.Cyclic();
	if (serialInterface.GetChar(&readChar)) {
		if (readChar == '\r') serialInterface.SendChar('\n');
		serialInterface.SendChar(readChar);
	}
	serialInterface.Cyclic();
}


void BlinkThreeTimes()
{
	// Wake up Signal über die LED (3 mal blinken)
	// Erster Test ob es denn wirklich flasht
	// Drive Servo to left and then to right
	DDRB |= 0x01;
	int j,i,k;
	
	for (j=0;j<3;j++) {
		echo();
		// pwGenerator.PwValues[0] = 127;
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
	for (j=0;j<3;j++) {
		echo();
		// pwGenerator.PwValues[0] = -127;
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
	// pwGenerator.PwValues[0] = 0;
	PORTB &= 0xFE;
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
		motorControler.Initialize();
		serialInterface.Initialize();
		stickReader.Initialize();
		iEvaluationCounter = 0;

		// Say hello
		char strptr[50];
		sprintf (strptr, "Started ADC Controler\n\r");
		serialInterface.SendString(strptr);

		BlinkThreeTimes();

		// Go to main Loop		
		int iReportingCounter;
		iReportingCounter = 0;
		while (true) {
			iReportingCounter += 1;	
			echo();
			stickReader.ReadADC();
			if (iEvaluationCounter > 10) {
				iEvaluationCounter = 0;
				motorControler.Evaluate(stickReader.StickX,stickReader.StickY);
			}
			
			// positive Value always moves forward
			pwGenerator.PwValues[0] = motorControler.PwValueLeft;
			pwGenerator.PwValues[1] = motorControler.PwValueRight;

			if (iReportingCounter == 1000) {
				iReportingCounter = 0;
				// sprintf (strptr, "ADC: X %d Y %d\n\r", stickReader.StickX, stickReader.StickY);
				sprintf (strptr, "PW: L %d R %d D %d\n\r", motorControler.PwValueLeft, motorControler.PwValueRight, motorControler.Direction);
				serialInterface.SendString(strptr);				
			}
		}
	}
} app;

extern "C" {
	//-----------------------------------------------------------------
	ISR (TIMER1_COMPA_vect)          // Interrupt 0 vector
	{
		pwGenerator.TimerInterrupt();
		//serialInterface.SendChar('i');
		iEvaluationCounter += 1;
	}
}

int main(void)
{
	app.MyMain();
}


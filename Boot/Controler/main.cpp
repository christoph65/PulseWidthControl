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
#include "StickReader.h"
#include "MotorControler.h"
#include "KeysReader.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

SerialInterface serialInterface;
PwGenerator pwGenerator;
StickReader stickReader;
MotorControler motorControler;
KeysReader keysReader;
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
		keysReader.Initialize();
		iEvaluationCounter = 0;

		// Say hello
		char strptr[50];
		sprintf (strptr, "Started ADC Controler %d %d %d\n\r",1 << PIND0,1 << PIND6, 1 << PIND7);
		serialInterface.SendString(strptr);

		BlinkThreeTimes();

		// Go to main Loop		
		int iReportingCounter = 0;
		int iEvaluationThreshold = 10;
		int iBlinkCounter = 8;		
		while (true) {
						   
			echo();
			stickReader.ReadADC();
			keysReader.ReadKeys();
			if (iEvaluationCounter > iEvaluationThreshold) {
				iEvaluationCounter = 0;
				if (iBlinkCounter-- > 0) {
					PORTB ^= 0x01; // ^ XOR Operator
					iBlinkCounter = 8;				
				}
				motorControler.Evaluate(stickReader.StickX,stickReader.StickY);
			}
			
			if (keysReader.bSwitchDownChange) {
				keysReader.bSwitchDownChange = false;
				if((keysReader.bSwitchDownPressed==true) & (iEvaluationThreshold < 30)) iEvaluationThreshold++;
 			}
			if (keysReader.bSwitchUpChange) {
				keysReader.bSwitchUpChange = false;
				if((keysReader.bSwitchUpPressed==true) & (iEvaluationThreshold > 2)) iEvaluationThreshold--;
			}
			
			// positive Value always moves forward
			pwGenerator.PwValues[0] = motorControler.PwValueLeft;
			pwGenerator.PwValues[1] = motorControler.PwValueRight;

			iReportingCounter += 1;
			if (iReportingCounter == 2500) {
				iReportingCounter = 0;
				// alt sprintf (strptr, "ADC: X %d Y %d\n\r", stickReader.StickX, stickReader.StickY);
				sprintf (strptr, "PW: L %3d R %3d D %2d T %2d\r", motorControler.PwValueLeft, motorControler.PwValueRight, motorControler.Direction,iEvaluationThreshold);
				serialInterface.SendString(strptr);
			}

			//if (keysReader.bSwitchUpChange){
				//keysReader.bSwitchUpChange = false;
				//if (keysReader.bSwitchUpPressed) sprintf (strptr, "Up Pressed\n\r");
				//else sprintf (strptr, "Up Released\n\r");
				//serialInterface.SendString(strptr);
			//}			
			//if (keysReader.bSwitchDownChange){
				//keysReader.bSwitchDownChange = false;
				//if (keysReader.bSwitchDownPressed) sprintf (strptr, "Down Pressed\n\r");
				//else sprintf (strptr, "Down Released\n\r");
				//serialInterface.SendString(strptr);
			//}			
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


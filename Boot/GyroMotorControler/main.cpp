/*
 * EcuSubmarine
 * main.cpp
 *
 * Testapplication for CommandInterpreter class
 *
 * For Putty: don't forget to turn xon/xoff off (see settings for serial)
 *
 *  Created on: 30.03.2013
 *  Updated on: 03.01.2018
 *      Author: Christoph Kunz
 */

#include "SerialInterface.h"
#include "CommandInterpreter.h"
#include "PwGenerator.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

SerialInterface serialInterface;
CommandInterpreter commandInterpreter;
PwGenerator pwGenerator;

// The real application
//
// Enter the command at the prompt '>'
// If the values of the arrays changed, then the new value is printed out
//

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

		serialInterface.Initialize();
		char strptr[50];
		sprintf (strptr, "Started Submarine Neu Ganzes 01052020\n\r");
		serialInterface.SendString(strptr);
		
		BlinkThreeTimes();
		echo();

		// PW Generator init 
		// set the interface Arrays to zero
		for (int i = 0; i<5; i++) {
			pwGenerator.PwValues[i] = 0;
			pwGenerator.PwAdjustValues[i] = 0;
		}
		pwGenerator.Initialize();
		
		// command Interpreter Initialize
		commandInterpreter.Inject(pwGenerator.PwValues, pwGenerator.PwAdjustValues);
		commandInterpreter.InterpretState = STATE_WAITCOMMAND;
		commandInterpreter.setcvChannel = 0;
		commandInterpreter.setavChannel = 0;
		sprintf (strptr, ">");
		serialInterface.SendString(strptr);
		

		char readChar;
		readChar = 0;
		// char debugChar;
		bool enteredWaitMode;
		enteredWaitMode = false;


		// Todo - test adjustment command
		// do some test with the motor (wie das VErhalten ist, wenn sich beide Werte addieren - setzen und adjust)

		// always coming back to STATE_WAITCOMMAND, then send ">".
		// Will be executed with the "Enter"-Button
		// to finish wrong Input - Press "Enter"
		while (true) {
			
			
			
			// Left ist 0
			// Right ist 1
			
			serialInterface.Cyclic();
			if (serialInterface.GetChar(&readChar)) {
				
				// send echo
				// if (readChar == '\r') serialInterface.SendChar('\n');  // only for use with a terminal
				serialInterface.SendChar(readChar);
				
				// interpret Char
				commandInterpreter.Interpret(readChar);
				if (commandInterpreter.InterpretState == STATE_WAITCOMMAND) {
					// for terminal use
					sprintf (strptr, "\r\n>");
					serialInterface.SendString(strptr);
					
					// debug start
					//if (enteredWaitMode == true) {
						//enteredWaitMode = false;
						//sprintf (strptr, "Channel %d : %d \r\n", commandInterpreter.setcvChannel + 1, pwGenerator.PwValues[commandInterpreter.setcvChannel]);
						//serialInterface.SendString(strptr);
						//sprintf (strptr, "Adjust %d : %i \r\n", commandInterpreter.setavChannel + 1, pwGenerator.PwAdjustValues[commandInterpreter.setavChannel]);
						//serialInterface.SendString(strptr);
						//
						//serialInterface.Cyclic();
						////serialInterface.SendChar('\r');
						////serialInterface.SendChar('\n');
						//sprintf (strptr, ">");
						//serialInterface.SendString(strptr);
					//}
					// debug end
				} 
				//debug start 
				else {
					enteredWaitMode = true;
					//debugChar = '0' + commandInterpreter.InterpretState;
					//serialInterface.SendChar(debugChar);
				}
				// debug end
				
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
	}
}

int main(void)
{
	app.MyMain();
}


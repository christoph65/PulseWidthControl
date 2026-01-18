/*
 * GyroMotorControler and ManualMotorControler
 * main.cpp
 *
 * This application is for controlling a boat with two motors (left/right)
 * - one input is over a joystick with two inputkey
 * - one input is over the serial port where an ECU with Gyro is connected
 *
 * For Putty: don't forget to turn xon/xoff off (see settings for serial)
 *
 *  Created on: 30.03.2013
 *  Updated on: 26.02.2021
 *
 *      Author: Christoph Kunz, Munich/Krailling, Germany
 */

#include "SerialInterface.h"
#include "CommandInterpreter.h"
#include "PwGenerator.h"
#include "StickReader.h"
#include "MotorControler.h"
#include "KeysReader.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

SerialInterface serialInterface;
CommandInterpreter commandInterpreter;
PwGenerator pwGenerator;
StickReader stickReader;
MotorControler motorControler;
KeysReader keysReader;
int iEvaluationCounter;

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
	// Wake up Signal �ber die LED (3 mal blinken)
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
		sprintf (strptr, "Started Submarine Neu 28022021\r\n");
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
		stickReader.Initialize();
		motorControler.Initialize();
		keysReader.Initialize();
		iEvaluationCounter = 0;
		
				
		// command Interpreter Initialize
		commandInterpreter.Inject(pwGenerator.PwValues, pwGenerator.PwAdjustValues);
		commandInterpreter.InterpretState = STATE_WAITCOMMAND;
		commandInterpreter.setcvChannel = 0;
		commandInterpreter.setavChannel = 0;
		sprintf (strptr, ">");
		serialInterface.SendString(strptr);
		

		char readChar;
		readChar = 0;



		// Todo - test adjustment command
		// do some test with the motor (wie das VErhalten ist, wenn sich beide Werte addieren - setzen und adjust)

		// always coming back to STATE_WAITCOMMAND, then send ">".
		// Will be executed with the "Enter"-Button
		// to finish wrong Input - Press "Enter"
		
		bool GyroActiveBool = false;
		int iEvaluationThreshold = 30;
		int iBlinkCounter = 8;
				
		while (true) {
			
			stickReader.ReadSwitches();
			keysReader.ReadKeys();
			serialInterface.Cyclic();
			
			if (GyroActiveBool)	{
				if (serialInterface.GetChar(&readChar)) {
					serialInterface.SendChar(readChar); //echo
					commandInterpreter.Interpret(readChar);
					if (commandInterpreter.InterpretState == STATE_WAITCOMMAND) {
						// for terminal use
						sprintf (strptr, "\r\n>");
						serialInterface.SendString(strptr);
					}			
				}
			} else {
				if (iEvaluationCounter > iEvaluationThreshold) {
					iEvaluationCounter = 0;
					if (iBlinkCounter-- > 0) {
						PORTB ^= 0x01; // ^ XOR Operator
						iBlinkCounter = 8;
					}
					motorControler.Evaluate(stickReader.StickX,stickReader.StickY);
				}
				// positive Value always moves forward
				pwGenerator.PwValues[0] = motorControler.PwValueLeft;
				pwGenerator.PwValues[1] = motorControler.PwValueRight;
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

// after reading stick or keys
			//if (stickReader.bSwitchForwardChange){
				//stickReader.bSwitchForwardChange = false;
				//sprintf (strptr, "\r\nf");
				//serialInterface.SendString(strptr);
				//
			//}
			//if (stickReader.bSwitchBackwardChange){
				//stickReader.bSwitchBackwardChange = false;
				//sprintf (strptr, "\r\nb");
				//serialInterface.SendString(strptr);
				//
			//}
			//if (stickReader.bSwitchLeftChange){
				//stickReader.bSwitchLeftChange = false;
				//sprintf (strptr, "\r\nl");
				//serialInterface.SendString(strptr);
				//
			//}
			//if (stickReader.bSwitchRightChange){
				//stickReader.bSwitchRightChange = false;
				//sprintf (strptr, "\r\nr");
				//serialInterface.SendString(strptr);
				//
			//}
			//
			//if (keysReader.bSwitchDownChange){
				//keysReader.bSwitchDownChange = false;
				//sprintf (strptr, "\r\nd");
				//serialInterface.SendString(strptr);
				//
			//}
			//if (keysReader.bSwitchUpChange){
				//keysReader.bSwitchUpChange = false;
				//sprintf (strptr, "\r\nu");
				//serialInterface.SendString(strptr);
				//
			//}
//



// for debugging of commandinterpreter

		//// char debugChar;
		//bool enteredWaitMode;
		//enteredWaitMode = false;
				//if (commandInterpreter.InterpretState == STATE_WAITCOMMAND) {
					//// for terminal use
					//sprintf (strptr, "\r\n>");
					//serialInterface.SendString(strptr);
// debug code should come now
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
				////debug start
				//else {
				//enteredWaitMode = true;
				////debugChar = '0' + commandInterpreter.InterpretState;
				////serialInterface.SendChar(debugChar);
				//}
				//// debug end

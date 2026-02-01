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
 *  Updated on: 01.02.2026 adaptions for new usage concept 
 * 		New Stick Behavoir: cccording to Drwa Io diagram "MotorControlerStateMachine.drawio"
 * 		No tested so far
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
int iDownButtonCounter;

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
		
				
		// command Interpreter Initialize
		commandInterpreter.Inject(pwGenerator.PwValues, pwGenerator.PwAdjustValues);
		commandInterpreter.InterpretState = STATE_WAITCOMMAND;
		commandInterpreter.setcvChannel = 0;
		commandInterpreter.setavChannel = 0;
		sprintf (strptr, ">");
		serialInterface.SendString(strptr);
		

		char readChar;
		readChar = 0;

		// always coming back to STATE_WAITCOMMAND, then send ">".
		// Will be executed with the "Enter"-Button
		// to finish wrong Input - Press "Enter"
		
		const int constOneSecondDelay                	= 1000; // 3 seconds first estimate
		const int constOneSecondDelayShortOn         	= 50; // 3 seconds first estimate
		const int constBlinkLowLightCounterOff 			= 8;
		const int constBlinkLowLightCounterOn 			= 3;
		const int constBlinkHighLightCounterOff 		= 3;
		const int constBlinkHighLightCounterOn 			= 8;

		iEvaluationCounter = 0;
		int iEvaluationThreshold = 30;
		iDownButtonCounter = 0;
		int iDownButtonThreshold = 100;

		int iDelayCounter = constOneSecondDelay;
		bool bLedStateOn = true;
		PORTB |= 0x01;

		const int constSTATE_ManualMode                = 0;
		const int constSTATE_Stop                      = 1;
		const int constSTATE_ExternalMode              = 2;
		const int constSTATE_SensitivityChange         = 3;
		const int constSTATE_QuittingSensitivityChange = 4;
		int iActualState = constSTATE_Stop;

		while (true) {
			
			stickReader.ReadSwitches();
			keysReader.ReadKeys();
			serialInterface.Cyclic();
			
			// '''''''''''''''''''''''''''''''''''''''''''''
			// State Machine for Mode Selection
			// '''''''''''''''''''''''''''''''''''''''''''''
			// For every state this possible transitions are checked first

			// Top Button -> go to Stop Mode from any state
			if (keysReader.bSwitchUpChange == true && keysReader.bSwitchUpPressed == true) {
				iActualState = constSTATE_Stop;
				keysReader.bSwitchUpChange = false;
				sprintf (strptr, "\r\nEntering Stop Mode\r\n");
				serialInterface.SendString(strptr);				
					stickReader.bValueChange = false; // prepare to detect stick movement
					// stop the motors
					pwGenerator.PwValues[0] = 0;
					pwGenerator.PwValues[1] = 0;
			}

			// Goto state specific code
			switch (iActualState)
			{
				case constSTATE_Stop:
					// any stick button pressed goto Manual Mode
					if (stickReader.bValueChange == true) {
						iActualState = constSTATE_ManualMode;
						// stickReader.bValueChange = false;
						sprintf (strptr, "\r\nFrom Stop to Manual Mode\r\n");
						serialInterface.SendString(strptr);
					}
					if (iDelayCounter-- > 0) {
						if (bLedStateOn == true) {
							// turn off after short time
							PORTB &= 0xFE; // turn off
							bLedStateOn = false;
							iDelayCounter = constOneSecondDelay;
						} else {
							// turn on after long time
							PORTB |= 0x01; // turn on
							bLedStateOn = true;
							iDelayCounter = constOneSecondDelayShortOn;
						}	
					}					
					/* code */
					break;
				
				case constSTATE_ManualMode:
				
					// Bottom Button short -> go to External Mode
					// must be pressed -> then iDownButtonCounter counts up
					// when released before threshold -> go to External Mode
					if (keysReader.bSwitchDownChange == true && keysReader.bSwitchDownPressed == true) {
						// button pressed
						iDownButtonCounter = 0;
						keysReader.bSwitchDownChange = false;
					} else {
						if (keysReader.bSwitchDownPressed == true) {
							// button still pressed
							iDownButtonCounter += 1;
						}
					}
					if (iDownButtonCounter < iDownButtonThreshold && keysReader.bSwitchDownChange == true) {
						// check if button released
						if (keysReader.bSwitchDownPressed == false) {
							// released before threshold -> go to External Mode
							iDownButtonCounter = 0;
							iActualState = constSTATE_ExternalMode;
							keysReader.bSwitchDownChange = false;
							stickReader.bValueChange = false;
							sprintf (strptr, "\r\nEntering External Mode\r\n");
							serialInterface.SendString(strptr);
						}
					}

					// Bottom Button long -> go to QuittingSensitivityChange
					// when released after threshold -> go to QuittingSensitivityChange
					if (iDownButtonCounter >= iDownButtonThreshold && keysReader.bSwitchDownChange == true) {
						// button released
						if (keysReader.bSwitchDownPressed == false) {
							// released after threshold -> go to QuittingSensitivityChange
							iDownButtonCounter = 0;
							iActualState = constSTATE_QuittingSensitivityChange;
							iDelayCounter = constOneSecondDelay * 3;
							// turn led off for 3 seconds
							PORTB &= 0xFE; // turn off
							bLedStateOn = false;							
							keysReader.bSwitchDownChange = false;
							stickReader.bValueChange = false;
							sprintf (strptr, "\r\nFrom manual mode to Quitting Sensitivity Change\r\n");
							serialInterface.SendString(strptr);
						}
					}
					
					// modify motor controler according to stick position
					if (iEvaluationCounter > iEvaluationThreshold) {
						iEvaluationCounter = 0;
						// blink LED with short pulse to show we are in Sensitivity Change Mode
						if (iDelayCounter-- <= 0) {
							if (bLedStateOn == true) {
								// turn off after short time
								PORTB &= 0xFE; // turn off
								bLedStateOn = false;
								iDelayCounter = constBlinkLowLightCounterOff;
							} else {
								// turn on after long time
								PORTB |= 0x01; // turn on
								bLedStateOn = true;
								iDelayCounter = constBlinkLowLightCounterOn;
							}	
						}
						// evaluate stick and set motor controler
						if (stickReader.bValueChange == true	) {
							stickReader.bValueChange = false;
							motorControler.Evaluate(stickReader.StickX,stickReader.StickY);
					}
					// positive Value always moves forward
					pwGenerator.PwValues[0] = motorControler.PwValueLeft;
					pwGenerator.PwValues[1] = motorControler.PwValueRight;
					break;
				
				case constSTATE_ExternalMode:
					// Led on continuously
					PORTB |= 0x01; // turn on
					bLedStateOn = true;

					// Any Stick Movement -> go to Manual Mode
					if (stickReader.bValueChange == true) {
						iActualState = constSTATE_ManualMode;
						// turn led off to show mode change
						PORTB &= 0xFE; // turn off
						bLedStateOn = false;
						// stickReader.bValueChange = false;
						sprintf (strptr, "\r\nFrom External to Manual Mode\r\n");
						serialInterface.SendString(strptr);
					}

					// in External Mode the commands are read from the serial interface
					if (serialInterface.GetChar(&readChar)) {
						serialInterface.SendChar(readChar); //echo
						commandInterpreter.Interpret(readChar);
						if (commandInterpreter.InterpretState == STATE_WAITCOMMAND) {
							// for terminal use
							sprintf (strptr, "\r\n>");
							serialInterface.SendString(strptr);
						}			
					}
					break;
				
				case constSTATE_SensitivityChange:
					// LowButton short -> go to Manual Mode
					if (keysReader.bSwitchDownChange == true && keysReader.bSwitchDownPressed == true) {
						keysReader.bSwitchDownChange = false;
						iActualState = constSTATE_ManualMode;
						// turn led off to show mode change
						PORTB &= 0xFE; // turn off
						bLedStateOn = false;
						iDelayCounter = 0;
						// stickReader.bValueChange = false;
						sprintf (strptr, "\r\nFrom Sensitivity Change to Manual Mode\r\n");
						serialInterface.SendString(strptr);
					}

					// Led blinks with long pulse to show we are in Sensitivity Change Mode
					// and according to the stick evaluation speed
					if (iEvaluationCounter > iEvaluationThreshold) {
						iEvaluationCounter = 0;
						if (iDelayCounter-- <= 0) {
							if (bLedStateOn == true) {
								// turn off after short time
								PORTB &= 0xFE; // turn off
								bLedStateOn = false;
								iDelayCounter = constBlinkHighLightCounterOff;
							} else {
								// turn on after long time
								PORTB |= 0x01; // turn on
								bLedStateOn = true;
								iDelayCounter = constBlinkHighLightCounterOn;
							}	
						}
					}

					// evaluate stick and adapt sensitivity
					if (stickReader.bValueChange == true	) {	
						stickReader.bValueChange = false;
						// adapt sensitivity according to stick position
						if (stickReader.StickY > 20) {
							// increase sensitivity
							if (pwGenerator.PwAdjustValues[2] < 2000) {
								pwGenerator.PwAdjustValues[2] += 10;
								sprintf (strptr, "\r\nIncreasing Sensitivity to %d\r\n", pwGenerator.PwAdjustValues[2]);
								serialInterface.SendString(strptr);
							}
						} else if (stickReader.StickY < -20) {
							// decrease sensitivity
							if (pwGenerator.PwAdjustValues[2] > 0) {
								pwGenerator.PwAdjustValues[2] -= 10;
								sprintf (strptr, "\r\nDecreasing Sensitivity to %d\r\n", pwGenerator.PwAdjustValues[2]);
								serialInterface.SendString(strptr);
							}
						}
					}
					break;
				
				case constSTATE_QuittingSensitivityChange:
					// 3 seconds Led on then go to Sensitivity Change
					if (iDelayCounter-- <= 0) {
						iActualState = constSTATE_SensitivityChange	;
						sprintf (strptr, "\r\nFrom Quitting Sensitivity Change to Sensivtivity change\r\n");
						serialInterface.SendString(strptr);
						stickReader.bValueChange = false; // prepare to detect stick movement
					}
					// Any Stick Movement -> go to Manual Mode
					if (stickReader.bValueChange == true) {
						iActualState = constSTATE_ManualMode;
						// turn led off to show mode change
						PORTB &= 0xFE; // turn off
						bLedStateOn = false;
						iDelayCounter = 0;
						// stickReader.bValueChange = false;
						sprintf (strptr, "\r\nFrom Quitting Sensitivity Change to Manual Mode\r\n");
						serialInterface.SendString(strptr);
					}
					break;
				
				default:
					break;
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

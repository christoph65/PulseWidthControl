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

		// for stick evaluation in Manual Mode and Sensitivity Change Mode
		iEvaluationCounter = 0;
		int iEvaluationThreshold = 5;



		// for LED Blink Management
		long longLedDelayCounterDown = 0;
		bool bLedStateOn = true;
		PORTB |= 0x01;
		// used for blinking the LED in stop mode
		const long constOneSecondDelay                	= 70000; // 1 seconds first estimate
		const long constOneSecondDelayShortOn         	= 6000; // 0.1 seconds first estimate
		// Delay used for LED off in QuittingSensitivityChange State
		const long constOneSecondDelayOff             	= 50000; // 3 seconds first estimate
		// used for blinking the LED in Manual Mode and Sensitivity Change Mode (low in man mode) 
		const long constBlinkLowLightCounterOff 		= 16;
		const long constBlinkLowLightCounterOn 			= 4;
		const long constBlinkHighLightCounterOff 		= 4;
		const long constBlinkHighLightCounterOn 		= 16;

		const long constDownButtonThreshold 			= 60000; // about 1 second to press
		long longDownButtonCounterUp = 0;
		bool bDownButtonCountingRuns = false;




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
				longLedDelayCounterDown = 0;

				keysReader.bSwitchUpChange = false;
				sprintf (strptr, "\r\nStpMde\r\n");
				serialInterface.SendString(strptr);				
				stickReader.bStickMoved = false; // prepare to detect stick movement
				// stop the motors
				pwGenerator.PwValues[0] = 0;
				pwGenerator.PwValues[1] = 0;
				motorControler.PwValueLeft = 0;
				motorControler.PwValueRight = 0;
			}

			// Goto state specific code
			switch (iActualState)
			{
				case constSTATE_Stop:
					// any stick button pressed goto Manual Mode
					if (stickReader.bStickMoved == true) {
						iActualState = constSTATE_ManualMode;
						longLedDelayCounterDown = 0;
						bDownButtonCountingRuns = false;
						longDownButtonCounterUp = 0;
						keysReader.bSwitchDownChange = false;

						iEvaluationCounter = 0;
						// stickReader.bValueChange = false;
						sprintf (strptr, "\r\nStp2ManMde\r\n");
						serialInterface.SendString(strptr);
						break;
					}
					// in Stop Mode the LED blinks with short pulse to show we are in Stop Mode
					if (longLedDelayCounterDown-- <= 0) {
						if (bLedStateOn == true) {
							// turn off after short time
							PORTB &= 0xFE; // turn off
							bLedStateOn = false;
							longLedDelayCounterDown = constOneSecondDelay;
						} else {
							// turn on after long time
							PORTB |= 0x01; // turn on
							bLedStateOn = true;
							longLedDelayCounterDown = constOneSecondDelayShortOn;
						}	
					}
										
					// Initialise counting if pressed for first time
					if (keysReader.bSwitchDownChange == true && keysReader.bSwitchDownPressed == true) {
						// button pressed and start counting
						longDownButtonCounterUp = 0;
						bDownButtonCountingRuns = true;
						keysReader.bSwitchDownChange = false;
					// if button is still pressed count up
					} else {
						if (keysReader.bSwitchDownPressed == true) {
							// button still pressed so count up
							longDownButtonCounterUp += 1;
						} 
					}

					// Bottom Button long -> go to QuittingSensitivityChange
					// when released after threshold -> go to QuittingSensitivityChange
					if (longDownButtonCounterUp >= constDownButtonThreshold && bDownButtonCountingRuns == true) {
						// if button still pressed then go to next State
						if (keysReader.bSwitchDownPressed == true) {
							iActualState = constSTATE_QuittingSensitivityChange;

							longDownButtonCounterUp = 0;
							bDownButtonCountingRuns = false;
							// turn led off for about 1 seconds
							longLedDelayCounterDown = constOneSecondDelayOff;
							PORTB &= 0xFE; // turn off
							bLedStateOn = false;

							keysReader.bSwitchDownChange = false;
							stickReader.bStickMoved = false;
							sprintf (strptr, "\r\nStopMde2QuitSensChnge\r\n");
							serialInterface.SendString(strptr);
							break;
						}
					}
					break;
				
				case constSTATE_ManualMode:
				
					// Bottom Button short -> go to External Mode
					// Botto Buttong long -> go to QuittingSensitivityChange

					// Initialise counting if pressed for first time
					if (keysReader.bSwitchDownChange == true && keysReader.bSwitchDownPressed == true) {
						// button pressed and start counting
						longDownButtonCounterUp = 0;
						bDownButtonCountingRuns = true;
						keysReader.bSwitchDownChange = false;
					// if button is still pressed count up
					} else {
						if (keysReader.bSwitchDownPressed == true) {
							// button still pressed so count up
							longDownButtonCounterUp += 1;
						} 
					}

					
					// Bottom Button short -> go to External Mode
					// when released before threshold -> go to External Mode
					if (longDownButtonCounterUp < constDownButtonThreshold && keysReader.bSwitchDownChange == true && bDownButtonCountingRuns == true	) {
						// check if button released
						if (keysReader.bSwitchDownPressed == false) {
							// released before threshold -> go to External Mode
							iActualState = constSTATE_ExternalMode;
							longDownButtonCounterUp = 0;
							bDownButtonCountingRuns = false;
							keysReader.bSwitchDownChange = false;
							stickReader.bStickMoved = false;
							sprintf (strptr, "\r\nManMde2ExtMde\r\n");
							serialInterface.SendString(strptr);
							break;
						}
					}

					// Bottom Button long -> go to QuittingSensitivityChange
					// when released after threshold -> go to QuittingSensitivityChange
					if (longDownButtonCounterUp >= constDownButtonThreshold && bDownButtonCountingRuns == true) {
						// if button still pressed then go to next State
						if (keysReader.bSwitchDownPressed == true) {
							iActualState = constSTATE_QuittingSensitivityChange;

							longDownButtonCounterUp = 0;
							bDownButtonCountingRuns = false;
							// turn led off for about 1 seconds
							longLedDelayCounterDown = constOneSecondDelayOff;
							PORTB &= 0xFE; // turn off
							bLedStateOn = false;

							keysReader.bSwitchDownChange = false;
							stickReader.bStickMoved = false;
							sprintf (strptr, "\r\nManMde2QuitSensChnge\r\n");
							serialInterface.SendString(strptr);
							break;
						}
					}
					
					// modify motor controler according to stick position
					if (iEvaluationCounter > iEvaluationThreshold) {
						iEvaluationCounter = 0;
						// blink LED with short pulse to show we are in Sensitivity Change Mode
						longLedDelayCounterDown -= 1;
						if (longLedDelayCounterDown <= 0) {
							if (bLedStateOn == true) {
								// turn off after short time
								PORTB &= 0xFE; // turn off
								bLedStateOn = false;
								longLedDelayCounterDown = constBlinkLowLightCounterOff;
							} else {
								// turn on after long time
								PORTB |= 0x01; // turn on
								bLedStateOn = true;
								longLedDelayCounterDown = constBlinkLowLightCounterOn;
							}	
						}
						// evaluate stick and set motor controler
						if (stickReader.bNotInMiddlePos == true	) {
							motorControler.Evaluate(stickReader.StickX,stickReader.StickY);
						}
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
					if (stickReader.bStickMoved == true) {
						iActualState = constSTATE_ManualMode;
						bDownButtonCountingRuns = false;
						longDownButtonCounterUp = 0;
						iEvaluationCounter = 0;
						// turn led off to show mode change
						longLedDelayCounterDown = 0;
						PORTB &= 0xFE; // turn off
						bLedStateOn = false;
						// stickReader.bValueChange = false;
						sprintf (strptr, "\r\nExtMod2ManMod\r\n");
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
					// LowButton pressed then go to Manual Mode
					if (keysReader.bSwitchDownChange == true && keysReader.bSwitchDownPressed == true) {
						keysReader.bSwitchDownChange = false;
						longDownButtonCounterUp = 0;
						bDownButtonCountingRuns = false;
						iActualState = constSTATE_ManualMode;
						// turn led off to show mode change
						PORTB &= 0xFE; // turn off
						bLedStateOn = false;
						longLedDelayCounterDown = 0;
						// stickReader.bValueChange = false;
						sprintf (strptr, "\r\nSensChg2ManMod\r\n");
						serialInterface.SendString(strptr);
						break;
					}

					// Led blinks with long pulse to show we are in Sensitivity Change Mode
					// and according to the stick evaluation speed
					if (iEvaluationCounter > iEvaluationThreshold) {
						iEvaluationCounter = 0;
						if (longLedDelayCounterDown-- <= 0) {
							if (bLedStateOn == true) {
								// turn off after short time
								PORTB &= 0xFE; // turn off
								bLedStateOn = false;
								longLedDelayCounterDown = constBlinkHighLightCounterOff;
							} else {
								// turn on after long time
								PORTB |= 0x01; // turn on
								bLedStateOn = true;
								longLedDelayCounterDown = constBlinkHighLightCounterOn;
							}	
						}
					}

					// evaluate stick and adapt sensitivity
					if (stickReader.bStickMoved == true	) {	
						stickReader.bStickMoved = false;
						// adapt sensitivity according to stick position
						if (stickReader.StickY > 20) {
							// increase sensitivity
							if (iEvaluationThreshold < 20) {
								iEvaluationThreshold += 1;
								sprintf (strptr, "\r\nIncSensTo %i\r\n", iEvaluationThreshold);
								serialInterface.SendString(strptr);
							}
						} else if (stickReader.StickY < -20) {
							// decrease sensitivity
							if (iEvaluationThreshold > 0) {
								iEvaluationThreshold -= 1;
								sprintf (strptr, "\r\nDecSensTo %i\r\n", iEvaluationThreshold);
								serialInterface.SendString(strptr);
							}
						}
					}
					break;
				
				case constSTATE_QuittingSensitivityChange:
					// 3 seconds Led on then go to Sensitivity Change
					if (longLedDelayCounterDown-- <= 0) {
						iActualState = constSTATE_SensitivityChange	;
						longLedDelayCounterDown = 0;
						sprintf (strptr, "\r\nQuitSC2SensChg\r\n");
						serialInterface.SendString(strptr);
						stickReader.bStickMoved = false; // prepare to detect stick movement
						keysReader.bSwitchDownChange = false; // prepare to detect button change
						break;
					}
					// Any Stick Movement -> go to Manual Mode
					if (stickReader.bStickMoved == true) {
						iActualState = constSTATE_ManualMode;
						// turn led off to show mode change
						PORTB &= 0xFE; // turn off
						bLedStateOn = false;
						longLedDelayCounterDown = 0;
						// stickReader.bValueChange = false;
						sprintf (strptr, "\r\nQuitSC2ManMod\r\n");
						serialInterface.SendString(strptr);
						break;
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

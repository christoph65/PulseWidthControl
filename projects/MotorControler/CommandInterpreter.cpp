/*
 * CommandInterpreter.cpp
 *
 *  Created on: 30.03.2013
 *      Author: CK
 */

#include "CommandInterpreter.h"


// The following commands can be used
//
// Set Channel Value (SETCV)
// <Cnxx>
//   n = 1 to 6 the channel
//   xx = two digit hexvalue for the command
//
// End is /r or /n
//
// Set Channel Adjust Value (SETAV)
// <Anxxxx>
//   n = 1 to 6 the channel
//   xxxx = four digit hexvalue for the command
//
// End is /r or /n
//


// ***********************************************************************
// Must be called with each new value coming in.
//
void CommandInterpreter::Interpret(char inputChar)
{
	switch (InterpretState){
		case STATE_WAITCOMMAND:
			if (inputChar == 'C')
				InterpretState = STATE_SETCV_WAITCHANNEL;
			else if (inputChar == 'A')
				InterpretState = STATE_SETAV_WAITCHANNEL;
			else if (inputChar == '\r' || inputChar == '\n')
				InterpretState = STATE_WAITCOMMAND;
			else
				InterpretState = STATE_WAITLINEEND;
			break;
		case STATE_SETCV_WAITCHANNEL:
			if (inputChar >= '1' && inputChar <= '6'){
				setcvChannel = (int)(inputChar - '1');
				InterpretState = STATE_SETCV_WAITVALUE1;
			} else
				InterpretState = STATE_WAITLINEEND;
			break;
		case STATE_SETCV_WAITVALUE1:
			if (inputChar >= '0' && inputChar <= '9') {
				setcvValue = (int8_t)(inputChar - '0') * 16;
				InterpretState = STATE_SETCV_WAITVALUE2;
				break;
			}
			else if (inputChar >= 'A' && inputChar <= 'F') {
				setcvValue = (int8_t)(inputChar - 'A' + 10) * 16;
				InterpretState = STATE_SETCV_WAITVALUE2;
				break;
			}
			InterpretState = STATE_WAITLINEEND;
			break;
		case STATE_SETCV_WAITVALUE2:
			if (inputChar >= '0' && inputChar <= '9') {
				setcvValue += (int8_t)(inputChar - '0');
				InterpretState = STATE_SETCV_EXECUTE;
				break;
			}
			else if (inputChar >= 'A' && inputChar <= 'F') {
				setcvValue += (int8_t)(inputChar - 'A' + 10);
				InterpretState = STATE_SETCV_EXECUTE;
				break;
			}
			InterpretState = STATE_WAITLINEEND;
			break;
		case STATE_SETCV_EXECUTE:
			if (inputChar == '\r' || inputChar == '\n'){
				pwArray[setcvChannel] = setcvValue;
				InterpretState = STATE_WAITCOMMAND;
				break;
			}
			InterpretState = STATE_WAITLINEEND;
			break;

		//
		// All for Adjusting Values
		//
		case STATE_SETAV_WAITCHANNEL:
			if (inputChar >= '1' && inputChar <= '6'){
				setavChannel = (int)(inputChar - '1');
				InterpretState = STATE_SETAV_WAITVALUES;
				setavCountInput = 0;
			} else
				InterpretState = STATE_WAITLINEEND;
			break;
		case STATE_SETAV_WAITVALUES:
			setavInputChars[setavCountInput++] = inputChar;
			if (setavCountInput == 4){
				if (convertHexInput(setavInputChars,&setavValue))
					InterpretState = STATE_SETAV_EXECUTE;
				else
					InterpretState = STATE_WAITLINEEND;
			}
			break;
		case STATE_SETAV_EXECUTE:
			if (inputChar == '\r' || inputChar == '\n'){
				pwAdjustArray[setavChannel] = setavValue;
				InterpretState = STATE_WAITCOMMAND;
				break;
			}
			InterpretState = STATE_WAITLINEEND;
			break;

		//
		// In Case of an wrong detection of a character
		//
		case STATE_WAITLINEEND:
			if (inputChar == '\r' || inputChar == '\n')
				InterpretState = STATE_WAITCOMMAND;
			break;
	}
}

// Injection of the pulsewidth Array
// This array will be modified by the CommandInterpreter

void CommandInterpreter::Inject(int8_t *injectedPwArray,int16_t *injectedPwAdjustArray)
{
	pwArray = injectedPwArray;
	pwAdjustArray = injectedPwAdjustArray;
	InterpretState = STATE_WAITCOMMAND;
	setavInputChars[5] = 0;
}

// Converts 4 hex Characters into a number

bool CommandInterpreter::convertHexInput(char *inputArray,int16_t *returnValue)
{
	char inputChar;
	*returnValue = 0;
	int16_t multiValue = 1;

	for (int i = 3; i >= 0; multiValue *= 16){
		inputChar = inputArray[i--];
		if (inputChar >= '0' && inputChar <= '9') {
			*returnValue += (int16_t)(inputChar - '0') * multiValue;
		}
		else if (inputChar >= 'A' && inputChar <= 'F') {
			*returnValue += (int16_t)(inputChar - 'A' + 10) * multiValue;
		}
		else
			return false;
	}
	return true;
}

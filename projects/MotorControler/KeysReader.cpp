/*
* KeysReader.cpp
*
* Created: 21.05.2020 10:22:53
* Author: Christoph
*/


#include "KeysReader.h"
#include <avr/io.h>
#include <stdio.h>

// default constructor
void KeysReader::Initialize()
{
	// Init all for the Switsches on PD6 and PD7 as Input
	// Normally PULLUp is already enabled for all ports
	DDRD &= ~((1<<PIND7) | (1<<PIND6));
	PORTD |= (1<<PIND7) | (1<<PIND6);
	MCUCR &= ~(1 << PUD);
	bSwitchUpPressed = false;
	bSwitchDownPressed = false;
	iSwitchUpDelay = 0;
	iSwitchDownDelay = 0;

} //KeysReader

void KeysReader::ReadKeys()
{
	// Read the Keys
	
	// Key must remain in one position for at least 10 Cycles
	if ((PIND & (1 << PIND7)) > 0) { // PD7 is released meaning bSwitchDownPressed should be false
		if (bSwitchDownPressed == false) iSwitchDownDelay = 0; // reset counter if state is already correct
		else iSwitchDownDelay += 1;
		
		if(iSwitchDownDelay >= 10) {
			bSwitchDownPressed = false;
			bSwitchDownChange = true;
			iSwitchDownDelay = 0;
		}
	} else { // now the opposite is the case PD7 is pressed meaning bSwitchDownPressed should be true
		if (bSwitchDownPressed == true) iSwitchDownDelay = 0;
		else iSwitchDownDelay += 1;
		
		if(iSwitchDownDelay >= 10) {
			bSwitchDownPressed = true;
			bSwitchDownChange = true;
			iSwitchDownDelay = 0;
		}
	}

	if ((PIND & (1 << PIND6)) > 0) { // PD6 is pressed
		if (bSwitchUpPressed == false) iSwitchUpDelay = 0;
		else iSwitchUpDelay += 1;
		
		if(iSwitchUpDelay >= 10) {
			bSwitchUpPressed = false;
			bSwitchUpChange = true;
			iSwitchUpDelay = 0;
		}
	} else {
		if (bSwitchUpPressed == true) iSwitchUpDelay = 0;
		else iSwitchUpDelay += 1;
		
		if(iSwitchUpDelay >= 10) {
			bSwitchUpPressed = true;
			bSwitchUpChange = true;
			iSwitchUpDelay = 0;
		}
	}
}
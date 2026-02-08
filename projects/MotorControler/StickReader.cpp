/*
* StickReader.cpp
*
*  Created on: 07.01.2020 21:33:59
*  Updated on: 26.02.2021 adaption for Serial Interface to Raspberry Pi and stick based on switches
*
*      Author: Christoph Kunz, Munich/Krailling, Germany
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
	// Init all for the Switsches on PD2 and PD5 as Input
	// Normally PULLUp is already enabled for all ports
	
	//D2, PD2 Forward (blau wei�)
	//D3, PD3 Right (orange)
	//D4, PD4 Left (orange wei�)
	//D5, PD5 Backward (braun wei�)
	DDRD &= ~((1<<PIND2) | (1<<PIND3) | (1<<PIND4) | (1<<PIND5));
	PORTD |=  (1<<PIND2) | (1<<PIND3) | (1<<PIND4) | (1<<PIND5);
	
	iSwitchForwardDelay = 0;
	iSwitchBackwardDelay = 0;
	iSwitchLeftDelay = 0;
	iSwitchRightDelay =  0;
	
	bSwitchForwardPressed = false;
	bSwitchBackwardPressed = false;
	bSwitchLeftPressed = false;
	bSwitchRightPressed = false;
	
	bSwitchForwardChange = false;
	bSwitchBackwardChange = false;
	bSwitchLeftChange = false;
	bSwitchRightChange = false;


 	bNotInMiddlePos = false;
	bStickMoved = false;
}


void StickReader::ReadSwitches()
{
	// this is for debouncing the key (should be ten times stable)
	
	// forward
	if ((PIND & (1 << PIND2)) > 0) { // PD2 is pressed Forward
		if (bSwitchForwardPressed == false) iSwitchForwardDelay = 0;
		else iSwitchForwardDelay += 1;
		
		if(iSwitchForwardDelay >= 10) {
			bSwitchForwardPressed = false;
			bSwitchForwardChange = true;
			iSwitchForwardDelay = 0;
		}
	} else {
		if (bSwitchForwardPressed == true) iSwitchForwardDelay = 0;
		else iSwitchForwardDelay += 1;
		
		if(iSwitchForwardDelay >= 10) {
			bSwitchForwardPressed = true;
			bSwitchForwardChange = true;
			iSwitchForwardDelay = 0;
		}
	}

	// Right
	if ((PIND & (1 << PIND3)) > 0) { // PD3 is pressed Right
		if (bSwitchRightPressed == false) iSwitchRightDelay = 0;
		else iSwitchRightDelay += 1;
		
		if(iSwitchRightDelay >= 10) {
			bSwitchRightPressed = false;
			bSwitchRightChange = true;
			iSwitchRightDelay = 0;
		}
	} else {
		if (bSwitchRightPressed == true) iSwitchRightDelay = 0;
		else iSwitchRightDelay += 1;
		
		if(iSwitchRightDelay >= 10) {
			bSwitchRightPressed = true;
			bSwitchRightChange = true;
			iSwitchRightDelay = 0;
		}
	}

	// Left
	if ((PIND & (1 << PIND4)) > 0) { // PD4 is pressed Left
		if (bSwitchLeftPressed == false) iSwitchLeftDelay = 0;
		else iSwitchLeftDelay += 1;
		
		if(iSwitchLeftDelay >= 10) {
			bSwitchLeftPressed = false;
			bSwitchLeftChange = true;
			iSwitchLeftDelay = 0;
		}
	} else {
		if (bSwitchLeftPressed == true) iSwitchLeftDelay = 0;
		else iSwitchLeftDelay += 1;
		
		if(iSwitchLeftDelay >= 10) {
			bSwitchLeftPressed = true;
			bSwitchLeftChange = true;
			iSwitchLeftDelay = 0;
		}
	}
	
	// Backward
	if ((PIND & (1 << PIND5)) > 0) { // PD5 is pressed Backward
		if (bSwitchBackwardPressed == false) iSwitchBackwardDelay = 0;
		else iSwitchBackwardDelay += 1;
			
		if(iSwitchBackwardDelay >= 10) {
			bSwitchBackwardPressed = false;
			bSwitchBackwardChange = true;
			iSwitchBackwardDelay = 0;
		}
	} else {
		if (bSwitchBackwardPressed == true) iSwitchBackwardDelay = 0;
		else iSwitchBackwardDelay += 1;
			
		if(iSwitchBackwardDelay >= 10) {
			bSwitchBackwardPressed = true;
			bSwitchBackwardChange = true;
			iSwitchBackwardDelay = 0;
		}
	}
	
	bool bStickYNotCentered = false;
	bool bStickXNotCentered = false;

	if (bSwitchForwardPressed)  {
		bStickYNotCentered = true;
		if (StickY == 0 or StickY == -40) {
			StickY = 40;
			bStickMoved = true;
		}
	}
	if (bSwitchBackwardPressed) {
		bStickYNotCentered = true;
		if (StickY == 0 or StickY == 40) {
			StickY = -40;
			bStickMoved = true;
		}
	}
	if (bSwitchLeftPressed) {
		bStickXNotCentered = true;
		if (StickX == 0 or StickX == 40) {
			StickX = -40;
			bStickMoved = true;
		}
	}
	if (bSwitchRightPressed) {
		bStickXNotCentered = true;
		if (StickX == 0 or StickX == -40) {
			StickX = 40;
			bStickMoved = true;
		}
	}

	if (bStickXNotCentered == false ) StickX = 0;
	if (bStickYNotCentered == false ) StickY = 0;

	if (StickX == 0 && StickY == 0) {
		bNotInMiddlePos = false;
	} else {
		bNotInMiddlePos = true;
	}
}
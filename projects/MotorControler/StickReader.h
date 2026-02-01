/*
* StickReader.h
*
*  Created on: 07.01.2020 21:33:59
*  Updated on: 26.02.2021 adpation for Serial Interface to Raspberry Pi and stick based on switches
*
*      Author: Christoph Kunz, Munich/Krailling, Germany
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#ifndef __STICKREADER_H__
#define __STICKREADER_H__

class StickReader {
	private:
	int iSwitchForwardDelay;
	int iSwitchBackwardDelay;
	int iSwitchLeftDelay;
	int iSwitchRightDelay;
	
	bool bSwitchForwardPressed;
	bool bSwitchBackwardPressed;
	bool bSwitchLeftPressed;
	bool bSwitchRightPressed;
	
	bool bSwitchForwardChange;
	bool bSwitchBackwardChange;
	bool bSwitchLeftChange;
	bool bSwitchRightChange;

	public:		
	void Initialize();
	void ReadSwitches();
	bool bValueChange;
	
	// this are historical values
	// used for an analog stick, but now changed to switches
	
	// Both Values between -127 and 128
	// Both Values like in X,Y coordination system like normal algebra function
	// the threshold is above 20 to react
	// for the keys here is 0 or 40 or -40 a good value selection
	// the value in X Direction
	int8_t StickX;
	// the value in Y Direction
	int8_t StickY;
};

#endif //__STICKREADER_H__

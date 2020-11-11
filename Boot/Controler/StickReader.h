/* 
* StickReader.h
*
* Created: 07.01.2020 21:34:00
* Author: Christoph
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#ifndef __STICKREADER_H__
#define __STICKREADER_H__

class StickReader {
	private:
	// the actual channel from 0 to 5. Is "-1" when in the restphase (pause) of the period
	// int8_t channel;


	public:
	void Initialize();
	void TimerInterrupt();
	void ReadADC();

	// Both Values between -127 and 128
	// Both Values like in X,Y coordination system like normal algebra function
	// the value in X Direction
	int8_t StickX;
	// the value in Y Direction
	int8_t StickY;
};

#endif //__STICKREADER_H__

/*
 * PwGenerator.h
 *
 *  Created on: 13.01.2013
 *      Author: CK
 */

#ifndef PWGENERATOR_H_
#define PWGENERATOR_H_
#include <stdint.h>

class PwGenerator {
private:
	// the actual channel from 0 to 5. Is "-1" when in the restphase (pause) of the period
	int8_t channel;


public:
	void Initialize();
	void TimerInterrupt();
	
	// The actual Values for the actuator 
	int8_t PwValues[6];
	// The trimmed Values for each channel
	int16_t PwAdjustValues[6];
	// Counting how many Cycles already happened
	int16_t PwMainCyclesCount;
};

#endif /* PWGENERATOR_H_ */

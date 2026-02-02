/* 
* MotorControler.cpp
*
* Created: 11.01.2020 21:07:19
* Author: Christoph
*/

#include "MotorControler.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdint.h>

#define defDirMiddle 0
#define defDirRight 1
#define defDirLeft 2
#define defDirForward 3
#define defDirBackward 4
#define defDirRightForward 5
#define defDirLeftForward 6
#define defDirRightBackward 7
#define defDirLeftBackward 8


void MotorControler::Initialize()
{
} //MotorControler

// default destructor
void MotorControler::Evaluate(int8_t stickX, int8_t stickY)
{
	// identify direction
	//PwValueRight = stickX;
	//PwValueLeft = stickY;
	//return;
	
	// stickX and stickY should behave like in coordination system of X,Y Axis
	// Values should be from -127 to +128
	// First evaluating in which quarter we are.
	
	if (((stickX > -20) & (stickX < 20)) & ((stickY > -20) & (stickY < 20)) )
		Direction = defDirMiddle;
    else if ((stickX >= 20) & (stickY >= 20))
		Direction = defDirRightForward;
    else if ((stickX >= 20) & (stickY <= -20) )
	    Direction = defDirRightBackward;
    else if ((stickX <= -20) & (stickY >= 20) )
		Direction = defDirLeftForward;
    else if ((stickX <= -20) & (stickY <= -20) )
		Direction = defDirLeftBackward;
    else if (((stickX > -20) & (stickX < 20)) & (stickY >= 20) )
	    Direction = defDirForward;
    else if (((stickX > -20) & (stickX < 20)) & (stickY <= -20) )
		Direction = defDirBackward;
    else if ((stickX >= 20) & ((stickY > -20) & (stickY < 20)))
        Direction = defDirRight;
	else if ((stickX <= -20) & ((stickY > -20) & (stickY < 20)))
		Direction = defDirLeft;
	else 
		Direction = defDirMiddle;
	
	
	switch (Direction) {
		case defDirMiddle:
			// nichts machen
			break;

		case defDirForward: 
			// Keep direction as it is accelarating. Means difference between both motors stays constant
			// If one reaches limit, then turn by also increasing other motor
			if (PwValueRight < 122) PwValueRight += 1;
			if (PwValueLeft < 122) PwValueLeft += 1;
			break;

		case defDirLeft: 
			// left Motor more forward, right motor more reduced till one reaches end
			if (PwValueRight < 122) PwValueRight += 1;
			if (PwValueLeft > -121) PwValueLeft -= 1;
			break;

		case defDirRight:
			// left Motor more forward, right motor more reduced till one reaches end
			if (PwValueRight > -121) PwValueRight -= 1;
			if (PwValueLeft < 122) PwValueLeft += 1;
			break;

		case defDirBackward:
			// Keep direction as it is accelarating. Means difference between both motors stays constant
			// If one reaches limit, then turn by also increasing other motor
			// But in backward direction	
			if (PwValueRight > -121) PwValueRight -= 1;
			if (PwValueLeft > -121) PwValueLeft -= 1;
			break;

		case defDirLeftForward:
			// Increase right motor stronger but also accelerate left motor
			// When maximus is reached of right motor, then only declerate left motor
			if ( PwValueRight < 122)
				PwValueRight += 1;
			if ( PwValueRight < 122)
				PwValueRight += 1;
			if ( PwValueRight < 122) {
				if (PwValueLeft < 122) PwValueLeft += 1;
			} else {
				// right motor at maximum, now decelerate left motor
				if (PwValueLeft > -121) PwValueLeft -= 1;
			}
			break;
			
		case defDirRightForward:
			// Increase left motor stronger but also accelerate right motor
			// When maximus is reached of left motor, then only declerate right motor
			if ( PwValueLeft < 122)
				PwValueLeft += 1;
			if ( PwValueLeft < 122)
				PwValueLeft += 1;
			if ( PwValueLeft < 122) {
				if (PwValueRight < 122) PwValueRight += 1;
			} else {
				// left motor at maximum, now decelerate right motor
				if (PwValueRight > -121) PwValueRight -= 1;
			}
			break;

		case defDirLeftBackward:
			// decelerate left motor stronger but also decelerate right motor
			// When minimum is reached of left motor, then only accelerate right motor	
			if ( PwValueLeft > -121)
				PwValueLeft -= 1;	
			if ( PwValueLeft > -121)
				PwValueLeft -= 1;	
			if ( PwValueLeft > -121) {
				if (PwValueRight > -121) PwValueRight -= 1;
			} else {
				// left motor at minimum, now accelerate right motor
				if (PwValueRight < 122) PwValueRight += 1;
			}
			break;

		case defDirRightBackward:
			// decelerate right motor stronger but also decelerate left motor
			// When minimum is reached of right motor, then only accelerate left motor	
			if ( PwValueRight > -121)
				PwValueRight -= 1;	
			if ( PwValueRight > -121)
				PwValueRight -= 1;	
			if ( PwValueRight > -121) {
				if (PwValueLeft > -121) PwValueLeft -= 1;
			} else {
				// right motor at minimum, now accelerate left motor
				if (PwValueLeft < 122) PwValueLeft += 1;
			}
			break;
	}


} //~MotorControler

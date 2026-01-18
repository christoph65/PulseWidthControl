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
			// In the direction as it is accelarating
			// If one reaches limit, then don't extend
			if ((PwValueLeft >= 122) | (PwValueRight >= 122)) 
				break; // do nothing
			else {
				PwValueRight += 1;
				PwValueLeft += 1;
			}
			break;
		case defDirLeft: 
			// left Motor more forward, right motor more reduced till one reaches end
			if ((PwValueRight >= 122) | (PwValueLeft <= -121))
				break; // do nothing
			else {
				PwValueRight += 1;
				PwValueLeft -= 1;
			}
			break;
		case defDirRight:
			// left Motor more forward, right motor more reduced till one reaches end
			if ((PwValueRight <= -121) | (PwValueLeft >= 122))
				break; // do nothing
			else {
				PwValueRight -= 1;
				PwValueLeft += 1;
			}
			break;
		case defDirBackward:
			// both motor more reduced till one reaches end
			if ((PwValueRight <= -121) | (PwValueLeft <= -121))
				break; // do nothing
			else {
				PwValueRight -= 1;
				PwValueLeft -= 1;
			}
			break;
		case defDirLeftForward:
			// Only accelerating right Motor
			if ( PwValueRight < 122)
				PwValueRight += 1;
			break; 
		case defDirRightForward:
			// Only accelerating left Motor
			if ( PwValueLeft < 122)
				PwValueLeft += 1;
			break;
		case defDirLeftBackward:
			// Only reducing right Motor
			if ( PwValueRight > -121)
				PwValueRight -= 1;
			break;
		case defDirRightBackward:
			// Only reducing left Motor
			if ( PwValueLeft > -121)
				PwValueLeft -= 1;
			break;
			
	}


} //~MotorControler

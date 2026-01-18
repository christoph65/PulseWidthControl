/* 
* MotorControler.h
*
* Created: 11.01.2020 21:07:20
* Author: Christoph
*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>

#ifndef __MOTORCONTROLER_H__
#define __MOTORCONTROLER_H__


class MotorControler
{
//functions and variables
private:
	int MoveTimerX;
	int MoveTimerY;
	

//functions and variables
public:
	void Initialize();
    void Evaluate(int8_t stickX, int8_t stickY);

	int8_t PwValueRight;
	int8_t PwValueLeft;	
	int8_t Direction;
	
}; //MotorControler

#endif //__MOTORCONTROLER_H__

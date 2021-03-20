/*
* KeysReader.h
*
* Created: 21.05.2020 10:22:54
* Author: Christoph
*/


#ifndef __KEYSREADER_H__
#define __KEYSREADER_H__


class KeysReader
{
	//variables
	private:
	int iSwitchUpDelay;
	int iSwitchDownDelay;

	//functions
	public:
	void Initialize();
	void ReadKeys();

	bool bSwitchUpPressed;
	bool bSwitchDownPressed;
	bool bSwitchUpChange;
	bool bSwitchDownChange;

}; //KeysReader

#endif //__KEYSREADER_H__

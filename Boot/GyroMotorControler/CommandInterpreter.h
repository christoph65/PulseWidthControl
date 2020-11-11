/*
 * CommandInterpreter.h
 *
 *  Created on: 30.03.2013
 *      Author: CK
 */

#ifndef COMMANDINTERPRETER_H_
#define COMMANDINTERPRETER_H_
#include <stdint.h>

#define STATE_WAITCOMMAND 0
// all for Set Channel Value
#define STATE_SETCV_WAITCHANNEL 1
#define STATE_SETCV_WAITVALUE1 2
#define STATE_SETCV_WAITVALUE2 3
#define STATE_SETCV_EXECUTE 4
// all for Set Adjust Value
#define STATE_SETAV_WAITCHANNEL 5
#define STATE_SETAV_WAITVALUES 6
#define STATE_SETAV_EXECUTE 7
// Wait for CR LF
#define STATE_WAITLINEEND 8

class CommandInterpreter {

private:
	int8_t *pwArray;
	int16_t *pwAdjustArray;
	char setavInputChars[6];
	int setavCountInput;
	bool convertHexInput(char *, int16_t *);

public:
	//TODO wieder private
	int InterpretState;
	void Inject(int8_t *,int16_t *);
	void Interpret (char);

	// debug start
	// normalerweise private
	int setcvChannel;
	int8_t setcvValue;
	int setavChannel;
	int16_t setavValue;
	// debug end

};

#endif /* COMMANDINTERPRETER_H_ */

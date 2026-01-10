/*
 * SerialInterface.h
 *
 *  Created on: 29.12.2012
 *      Author: CK
 */

#define SERIAL_BUFFER_SIZE 32
#ifndef SERIALINTERFACE_H_
#define SERIALINTERFACE_H_

class SerialInterface {

private:
	char sendRingBuffer[SERIAL_BUFFER_SIZE];
	int inputPointerSendRingBuffer; // points to the next free place
	int outputPointerSendRingBuffer; // points to the last taken char

	char receiveRingBuffer[SERIAL_BUFFER_SIZE];
	int inputPointerReceiveRingBuffer; // points to the next free place
	int outputPointerReceiveRingBuffer; // points to the last taken char

	char out;

public:
	void Initialize();
	void Cyclic();
	bool GetChar(char *);
	bool SendChar(char);
	bool SendString (char *);
};

#endif /* SERIALINTERFACE_H_ */

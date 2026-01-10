/*
 * SerialInterface.cpp
 *
 *  Created on: 29.12.2012
 *      Author: CK
 */

#include <avr/io.h>
// #include <avr/iom328p.h>
#include <avr/interrupt.h>
// #include <avr/iom32.h>
#include "SerialInterface.h"

//---------------------------------------------------------------------------------------
// Serielles Interface wird auf die Baudrate von UART_BAUD_RATE gesetzt

void SerialInterface::Initialize()
{
#ifndef F_CPU
#define F_CPU 16000000
#endif
// 38400 geht noch. Verdoppelung geht nicht mehr
#define UART_BAUD_RATE 38400

// Hilfsmakro zur UBRR-Berechnung ("Formel" laut Datenblatt)
#define UART_UBRR_CALC(BAUD_,FREQ_) ((FREQ_)/((BAUD_)*16L)-1)


    // Adapted for 382P on 7.12.2019 - inerting 0
	// Enable receiver and transmitter
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	// Set frame format: 8data, 2stop bit
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);

	// Set Baudrate
    UBRR0H = (uint8_t)( UART_UBRR_CALC( UART_BAUD_RATE, F_CPU ) >> 8 );
    UBRR0L = (uint8_t)UART_UBRR_CALC( UART_BAUD_RATE, F_CPU );

    inputPointerSendRingBuffer = 0;
    outputPointerSendRingBuffer = 0;
    inputPointerReceiveRingBuffer = 0;
    outputPointerReceiveRingBuffer = 0;
}

//---------------------------------------------------------------------------------------
// This must be called in cyclic way - it sends and receives the characters

void SerialInterface::Cyclic()
{
	//if ( _SFR_BYTE(UCSRA) | (1<<UDRE) ) // would also be correct
	if ( UCSR0A & (1<<UDRE0) ) {// Check if sendbuffer is empty
		if (outputPointerSendRingBuffer != inputPointerSendRingBuffer)
		{
			UDR0 = sendRingBuffer[outputPointerSendRingBuffer];
			outputPointerSendRingBuffer += 1;
			if (outputPointerSendRingBuffer > (SERIAL_BUFFER_SIZE - 1)) outputPointerSendRingBuffer = 0;
		}
	}

	// Check if something has to be received
	if ( UCSR0A & (1<<RXC0) ) {
		// if next increase of inputPointer is same as output, then no read
		if ( !(
				((inputPointerReceiveRingBuffer + 1 ) > (SERIAL_BUFFER_SIZE - 1)) && (outputPointerReceiveRingBuffer == 0)
			  )  ) {
			if ( !(
					(inputPointerReceiveRingBuffer + 1) == outputPointerReceiveRingBuffer
				  )  ) {
				// put char to ringbuffer and increase pointer
				receiveRingBuffer[inputPointerReceiveRingBuffer] = UDR0;
				inputPointerReceiveRingBuffer += 1;
				if (inputPointerReceiveRingBuffer > (SERIAL_BUFFER_SIZE - 1)) inputPointerReceiveRingBuffer = 0;
			}
		}
	}
}


//---------------------------------------------------------------------------------------
// Gets a character from the receivebuffer

bool SerialInterface::GetChar(char *receivedCharPtr)
{
	if (outputPointerReceiveRingBuffer != inputPointerReceiveRingBuffer)
	{
		*receivedCharPtr = receiveRingBuffer[outputPointerReceiveRingBuffer];
		outputPointerReceiveRingBuffer += 1;
		if (outputPointerReceiveRingBuffer > (SERIAL_BUFFER_SIZE - 1)) outputPointerReceiveRingBuffer = 0;
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------------------
// Take Char to the Ringbuffer

bool SerialInterface::SendChar(char charToSend)
{
	// if next increase of inputPointer is same as output, then drop char
	if (((inputPointerSendRingBuffer + 1 ) > (SERIAL_BUFFER_SIZE - 1)) && (outputPointerSendRingBuffer == 0)) return false;
	if ((inputPointerSendRingBuffer + 1) == outputPointerSendRingBuffer) return false;

	// put char to ringbuffer and increase pointer
	sendRingBuffer[inputPointerSendRingBuffer] = charToSend;
	inputPointerSendRingBuffer += 1;
	if (inputPointerSendRingBuffer > (SERIAL_BUFFER_SIZE - 1)) inputPointerSendRingBuffer = 0;
	return true;
}

//---------------------------------------------------------------------------------------
// Sends a complete String

bool SerialInterface::SendString(char *sendString)
{
	int i = 0;
	bool retval = true;

	while( (sendString[i] != 0) && retval) {
		retval = SendChar(sendString[i++]);

	}
	return retval;
}





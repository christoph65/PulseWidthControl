### OpenFishPilot – Component MotorControler

This is the Core, to be flashed on an Atmelprocessor.
Idealwise you can take a Arduino Nano and just flash it baremetal.

It will run without an raspberry PI. The raspberry PI is necessary for the connection to the electronic compass.

### Hardware and flashing

I am just using an Diamex AVR flasher, connected to my USB Port:
> avrdude -c stk500v2 -p m328p -P COM3 -b 57600 -U flash:w:MotorControler.hex:i

The serial output of the Nano connect to your computer at a baudrate of 38400.


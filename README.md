<p align="center"> <img src="assets/Logo.png" alt="OpenFishPilot logo" width="600" /> </p>

### OpenFishPilot – Hands-Free Boat Steering for Anglers

OpenFishPilot is an open-source embedded boat steering system designed for anglers using small inflatable boats who want precise control, stable heading, and hands-free operation while fishing.

In addition to automatic heading control, the system is optimized for manual low-speed maneuvering along depth contours. This makes it especially useful when following depth lines with a wobbler while closely monitoring a fish finder.

The system uses two electric outboard motors mounted on the left and right side of the boat. Each motor is controlled by a brushed RC speed controller and driven via PWM signals from an ATmega328P microcontroller.

### Manual Control

A joystick connected to the ATmega328P allows intuitive and precise manual control:

- Tap left or right to steer
- Push forward or backward to accelerate or decelerate
- Adjustable motor response for more aggressive or smoother handling
- When the joystick is released, the current motor settings are held
- Emergency stop button

By increasing the motor response, the system allows fast and precise corrections when following depth contours.  
This enables the angler to focus on the fish finder instead of constant steering adjustments.

The joystick-based control also allows steering from a comfortable position within the boat, eliminating the need to sit directly at the stern on longer inflatable boats.

### Automatic Heading Hold

To reduce the need for constant manual correction, an electronic compass is connected to a Raspberry Pi.
The Raspberry Pi runs a simple PT controller that:

- Measures the current heading
- Compares it to a target heading
- Automatically corrects the course by adjusting the left and right motors

The Raspberry Pi can take over steering control from the ATmega328P via a serial interface, allowing seamless switching between manual contour tracking and automatic heading stabilization.

### System Architecture

- **ATmega328P**: PWM motor control, joystick input, manual steering
- **Raspberry Pi Zero**: Heading control, PT controller, compass processing
- **Electronic Compass**: Absolute heading feedback
- **Dual Thrusters**: Differential steering without rudder

### Features

- Differential steering with two independent motors
- Manual joystick control with configurable motor response
- Stable low-speed control for following depth contours
- Automatic compass-based heading stabilization
- Mode switching between manual and automatic steering
- Modular and extendable open-source architecture

### Use Case
OpenFishPilot is intended for anglers who want a simple, low-cost and open-source solution to keep their boat on a steady heading while focusing on fishing or for comfortable manual trolling along a depth line.
OpenFishPilot is an independent open-source project and is not affiliated with any commercial product or brand.

⚠️ This project is experimental and intended for personal and recreational use only.

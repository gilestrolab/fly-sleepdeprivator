fly-sleepdeprivator
===================

Firmware and software for a shaker based fly sleep deprivator

REQUIREMENTS

* Arduino 1.0+  - http://arduino.cc
* SerialCommand - http://github.com/kroimon/Arduino-SerialCommand
* pySerial      - http://pyserial.sourceforge.net/
* pySolo-Video  - http://github.com/gilestrolab/pySolo-Video

What's NEW

*Version 0.97*
* Changed the order of the tubes to reflect the mask in pySolo-Video
* Auto mode will start automatically when the arduino is not connected to the computer
* IR LEDs are not longer controlled by the ARDUINO

*Version 0.99
* Auto mode will not start automatically because MEGA cannot detect if Serial port is connected or not.
* The sleepdeprivator script will be able to start auto mode
* added support for online check of new version
* Added LED status for auto On or Off on LED 13

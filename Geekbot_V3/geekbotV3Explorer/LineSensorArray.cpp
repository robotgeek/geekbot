#include <Arduino.h>
#include "LineSensorArray.h"


// Uncomment one of the four lines to match your SX1509's address
//  pin selects. SX1509 breakout defaults to [0:0] (0x3E).
const uint8_t SX1509_ADDRESS = 0x3E;  // SX1509 I2C address (00)
//const byte SX1509_ADDRESS = 0x3F;  // SX1509 I2C address (01)
//const byte SX1509_ADDRESS = 0x70;  // SX1509 I2C address (10)
//const byte SX1509_ADDRESS = 0x71;  // SX1509 I2C address (11)

SensorBar mySensorBar(SX1509_ADDRESS);

void LineSensorArrayInit()
{
	/* Sensor Bar Configuration */
	mySensorBar.setBarStrobe(); //Default: the IR will only be turned on during reads.
	//mySensorBar.clearBarStrobe(); //Alternate option: Command IR to run all the time
	if (LINE_COLOR == 0)
	{
		mySensorBar.setInvertBits(); //Alternate option: light line on dark
	}

	if (LINE_COLOR == 1)
	{
		mySensorBar.clearInvertBits(); //Default: dark on light
	}

	//Don't forget to call .begin() to get the bar ready. This configures HW.
	uint8_t returnStatus = mySensorBar.begin();
	if (returnStatus == 0)
	{
		Serial.println("sx1509 IC communication FAILED!");
	}
}

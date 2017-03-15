#include "GeekBot.h"

/* Robot Config Parameters */
#include "Config.h"

/* Motor Control */
#include "Motors.h"

/* Navigation and Map */
#include "Navigation.h"
#include "myMap.h"

/* I2C Line Sensor Array */
#include "LineSensorArray.h"

/* Line Following Logic and Intersection handling */
#include "LineFollower.h"

/* Sound effects */
#include <PiezoEffects.h>
PiezoEffects mySounds( BUZZER_PIN );


void GeekBot::init()
{
	pinMode(LED_LEFT_PIN, OUTPUT);
	pinMode(LED_RIGHT_PIN, OUTPUT);
	digitalWrite(LED_LEFT_PIN, HIGH);
	digitalWrite(LED_RIGHT_PIN, HIGH);
	motorsInit( LEFT_SERVO_PIN, RIGHT_SERVO_PIN );
	navigationInit( &mySounds );
	lineFollowerInit( &mySounds );
	LineSensorArrayInit();
}


void GeekBot::lineFollow(int state)	//Follow Line until Intersection, state = NAV_CMD for next action.
{
	Serial.println("Starting lineFollow Function");
	do
	{
		Serial.println("Updating!");
	}
	while (lineFollowCommand() == 0);

	Serial.println("intersectionDetected");
	intersectionDetected(state);

}

void GeekBot::timedDrive(unsigned int time, int speed)	// time in mS, Speed -100 to 100
{
	speed = constrain( speed, -100, 100 );
	if ( speed < 0 ) motorsReverse( speed );
	else motorsForward(speed);
	delay(time);
	motorsStop();
}

void GeekBot::timedRotate(unsigned int time, int speed)	// time in mS, rotation speed CCW -100 to CW +100
{
	if (speed > 0)
	{
		motorsRotateRight(speed);
		delay(time);
		motorsStop();
	}
	if (speed < 0)
	{
		motorsRotateLeft(speed);
		delay(time);
		motorsStop();
	}
	if (speed == 0)
	{
		delay(time);
		motorsStop();
	}
}

void GeekBot::recoverLine(int rotation)	//Rotate until line recovered, -1 = CCW, 1 = CW
{
	if (rotation > 0)
	{
		recoverLineRight();
	}
	if (rotation < 0)
	{
		recoverLineLeft();
	}
}

void GeekBot::sound(int sound)	// play sound from Piezo library
{
	mySounds.play( sound );
}

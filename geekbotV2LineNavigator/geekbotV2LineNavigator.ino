/***********************************************************************************
 *                         RobotGeek Geekbot V2 Line Navigator
 *  __________________
 *   |              |
 *   |     ___      |
 *  _|____/___\ ____|_
 *   \_/  \___/   \_/
 *
 *  Wiring
 *
 *    Rotation Knob - Analog Pin 0
 *    Left Servo - Digital Pin 10
 *    Right Servo - Digital Pin 11
 *    Buzzer - Digital Pin 12
 *    IR Receiver - Digital Pin 2
 *    Right LED - Digital Pin 4
 *    Left LED - Digital Pin 7
 *    Line Sensor Array - I2C
 *    LCD (4 line) - I2C
 *    LCD Up Button - Digital Pin 3
 *    LCD Down Button - Digital Pin 9
 *    LCD Play Button - Digital Pin 1
 *
 *    Jumper for pins 9/10/11 should be set to 'VIN'
 *    Jumper for pins 3/5/6 should be set to '5V'
 *
 *  Control Behavior:
 *
 *    Put it on a line and use the LCD panel to navigate between destinations.
 *    Use the Rotation Knob to adjust forward driving trim.
 *    Customize your nagivation routes in myMap.cpp
 *
 *  External Resources
 *
 ***********************************************************************************/

/* Pin constants */
const int BUZZER_PIN = 12;
const int LED_RIGHT_PIN = 4;
const int LED_LEFT_PIN = 7;
const int LEFT_SERVO_PIN = 10;
const int RIGHT_SERVO_PIN = 11;

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

void setup()
{
  Serial.begin(38400);
  Serial.println("Geekbot Navigator Starting...");

  motorsInit( LEFT_SERVO_PIN, RIGHT_SERVO_PIN );
  navigationInit( &mySounds );
  lineFollowerInit( &mySounds );
  LineSensorArrayInit();

  pinMode(LED_LEFT_PIN, OUTPUT);
  pinMode(LED_RIGHT_PIN, OUTPUT);
  digitalWrite(LED_LEFT_PIN, HIGH);
  digitalWrite(LED_RIGHT_PIN, HIGH);
}

void loop()
{
  //Update the line following state if the navigation location and destination are set
  if ( navigationCheckLocation() && navigationCheckDestination() )
  {
    lineFollowerUpdate(); //Will follow the line and handle any intersections
  }
}

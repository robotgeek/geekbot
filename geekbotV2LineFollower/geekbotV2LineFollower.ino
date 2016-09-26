/***********************************************************************************
 *                         RobotGeek Geekbot V2 Line Follower
 *  __________________
 *   |              |
 *   |     ___      |
 *  _|____/___\ ____|_
 *   \_/  \___/   \_/
 *
 *  The
 *
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
 *
 *    Jumper for pins 9/10/11 should be set to 'VIN'
 *    Jumper for pins 3/5/6 should be set to '5V'
 *
 *  Control Behavior:
 *  Pressing directional button(s) on the gamepad will start the Geekbot rolling in that direction.
 *  Pressing Select will trim the robot's drive to the left, Start will trim the drive right.
 *  Pressing B and TA (Left and Right) buttons will decrease or increase the amount of turn while driving.
 *  Pressing A and TB (Bottom and Top) buttons will decrease or increase the drive speed.
 *
 *  External Resources
 *
 ***********************************************************************************/

#include <sensorbar.h>
// Uncomment one of the four lines to match your SX1509's address
//  pin selects. SX1509 breakout defaults to [0:0] (0x3E).
const uint8_t SX1509_ADDRESS = 0x3E;  // SX1509 I2C address (00)
//const byte SX1509_ADDRESS = 0x3F;  // SX1509 I2C address (01)
//const byte SX1509_ADDRESS = 0x70;  // SX1509 I2C address (10)
//const byte SX1509_ADDRESS = 0x71;  // SX1509 I2C address (11)
SensorBar mySensorBar(SX1509_ADDRESS);
#define IDLE_STATE 0
#define READ_LINE 1
#define GO_FORWARD 2
#define GO_LEFT 3
#define GO_RIGHT 4
#define GO_INTERSECTION 5

uint8_t state;

//Includes
#include <Servo.h>     //include servo library to control continous turn servos
#include "Sounds.h"

//pin constants
const int TRIM_KNOB_PIN = 0;
const int BUZZER_PIN = 12;
const int LED_RIGHT_PIN = 4;
const int LED_LEFT_PIN = 7;
const int LEFT_SERVO_PIN = 10;
const int RIGHT_SERVO_PIN = 11;

//Servo control
const int SERVO_STOP = 1500; //servo pulse in microseconds for stopped servo

//Speed constants from RGS-4C No Load Test Data
const int CCW_MIN_SPEED = 1580;
const int CW_MIN_SPEED = 1400;
const int CCW_MAX_SPEED = 1750;
const int CW_MAX_SPEED = 1250;
//Experimental speed constant
const int CCW_MED_SPEED = 1660;
const int CW_MED_SPEED = 1300;

int SERVO_DRIVE_TURN_SPEED = 70; //For turning while driving (controller selectable)
const int SERVO_TURN_SPEED_HIGH = 50;
const int SERVO_TURN_SPEED_LOW = 25;
int SERVO_TURN_SPEED = SERVO_TURN_SPEED_LOW; //For in place turning. Applied to CW and CCW_MIN_SPEEDs


Servo servoLeft, servoRight;      //wheel servo objects

int servoSpeedLeft = SERVO_STOP;   //left servo speed.
int servoSpeedRight = SERVO_STOP;  //right servo speed.

//Wheel speeds from "gear" selection
int leftFwdSpeed = CCW_MIN_SPEED;
int leftRevSpeed = CW_MIN_SPEED;

int rightFwdSpeed = CW_MIN_SPEED;
int rightRevSpeed = CCW_MIN_SPEED;

enum SpeedSelections
{
  SPEED_MIN,
  SPEED_MED,
  SPEED_MAX
};

int currentSpeed = SPEED_MAX;

//Trim testing
int _wheel_speed_trim = 0;
void updateDriveTrim()
{
  int knob_value = analogRead( TRIM_KNOB_PIN );
  _wheel_speed_trim = map( knob_value, 0, 1023, -100, 100 );
}

int _wheel_speed_trim_override = 0;

void setup()
{
  //Default: the IR will only be turned on during reads.
  mySensorBar.setBarStrobe();
  //Other option: Command to run all the time
  //mySensorBar.clearBarStrobe();

  //Default: dark on light
  mySensorBar.clearInvertBits();
  //Other option: light line on dark
  //mySensorBar.setInvertBits();

  //Don't forget to call .begin() to get the bar ready.  This configures HW.
  uint8_t returnStatus = mySensorBar.begin();
  if(returnStatus)
  {
    Serial.println("sx1509 IC communication OK");
  }
  else
  {
    Serial.println("sx1509 IC communication FAILED!");
  }
  Serial.println();


  // put your setup code here, to run once:
  pinMode(LED_LEFT_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_LEFT_PIN, HIGH);
  digitalWrite(LED_RIGHT_PIN, HIGH);

  Serial.begin(38400);

  servoLeft.attach(LEFT_SERVO_PIN);
  servoRight.attach(RIGHT_SERVO_PIN);
  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);

  SoundEnable();

  Serial.println("Geekbot V2 Start");
}

void intersectionForward()
{
  motorsForward();
  while ( mySensorBar.getDensity() > 5 )
  {
    delay(100);
  }
}
void intersectionRight()
{
  motorsRotateRight();
  while( true )
  {
    if( mySensorBar.getDensity() < 3 )
    {
      uint8_t lastBarRawValue = mySensorBar.getRaw();

      if ( lastBarRawValue == 0x08 || lastBarRawValue == 0x18 || lastBarRawValue == 0x10 )
      {
        break; //Stop rotating when line is detected in middle.
      }
    }
  }
}
void intersectionLeft()
{
  motorsRotateLeft();
  while( true )
  {
    if( mySensorBar.getDensity() < 3 )
    {
      uint8_t lastBarRawValue = mySensorBar.getRaw();

      if ( lastBarRawValue == 0x08 || lastBarRawValue == 0x18 || lastBarRawValue == 0x10 )
      {
        break; //Stop rotating when line is detected in middle.
      }
    }
  }
}
int currentIntersection = 0;
void intersectionDetected()
{
  motorsStop();
  switch( ++currentIntersection )
  {
  case 1:
    SoundPlay(UP);
    intersectionForward();
    break;
  case 2:
    SoundPlay(DOWN);
    intersectionRight();
    break;
  case 3:
    SoundPlay(LAUGH);
    intersectionLeft();
    break;
  default:
    motorsStop();
    SoundPlay(WHISTLE);
    while (1);
  }
}
void motorsSetSpeed()
{
  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);
}

void motorsStop()
{
  servoSpeedLeft = SERVO_STOP;
  servoSpeedRight = SERVO_STOP;
  motorsSetSpeed();
}
void motorsForward()
{
  currentSpeed = SPEED_MIN;
  SERVO_TURN_SPEED = SERVO_TURN_SPEED_LOW;
  leftFwdSpeed = CCW_MIN_SPEED + 20;
  leftRevSpeed = CW_MIN_SPEED - 20;
  rightFwdSpeed = CW_MIN_SPEED - 20;
  rightRevSpeed = CCW_MIN_SPEED + 20;

  updateDriveTrim();
  servoSpeedLeft = leftFwdSpeed + _wheel_speed_trim + _wheel_speed_trim_override;
  servoSpeedRight = rightFwdSpeed + _wheel_speed_trim + _wheel_speed_trim_override;

  motorsSetSpeed();
}
void motorsTurnLeft()
{
  motorsForward();
  servoSpeedLeft -= SERVO_DRIVE_TURN_SPEED;
  motorsSetSpeed();
}
void motorsTurnRight()
{
  motorsForward();
  servoSpeedRight += SERVO_DRIVE_TURN_SPEED;
  motorsSetSpeed();
}
void motorsRotateLeft()
{
  servoSpeedLeft = CW_MIN_SPEED - SERVO_TURN_SPEED;
  servoSpeedRight = CW_MIN_SPEED - SERVO_TURN_SPEED;
  motorsSetSpeed();
}
void motorsRotateRight()
{
  servoSpeedLeft = CCW_MIN_SPEED + SERVO_TURN_SPEED;
  servoSpeedRight = CCW_MIN_SPEED + SERVO_TURN_SPEED;
  motorsSetSpeed();
}

void loop()
{
  uint8_t nextState = state;
  switch (state)
  {
  case IDLE_STATE:
    motorsStop();       // Stops both motors
    nextState = READ_LINE;
    break;
  case GO_INTERSECTION:
    intersectionDetected(); //This function will handle all drive and sensor commands until intersection is complete
    nextState = IDLE_STATE;
    break;
  case READ_LINE:
    if( mySensorBar.getDensity() < 7 )
    {
      nextState = GO_FORWARD;
      if( mySensorBar.getPosition() < -50 )
      {
        nextState = GO_LEFT;
      }
      if( mySensorBar.getPosition() > 50 )
      {
        nextState = GO_RIGHT;
      }
    }
    else //all 8 on means we are at black line intersection
    {
      nextState = GO_INTERSECTION;
    }
    break;
  case GO_FORWARD:
    motorsForward();
    nextState = READ_LINE;
    break;
  case GO_LEFT:
    motorsTurnLeft();
    nextState = READ_LINE;
    break;
  case GO_RIGHT:
    motorsTurnRight();
    nextState = READ_LINE;
    break;
  default:
    motorsStop();       // Stops both motors
    break;
  }
  state = nextState;
}


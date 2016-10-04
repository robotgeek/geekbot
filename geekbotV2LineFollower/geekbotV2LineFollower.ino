/***********************************************************************************
 *                         RobotGeek Geekbot V2 Line Follower
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
 *    IR Sensor Array - I2C
 *
 *    Jumper for pins 9/10/11 should be set to 'VIN'
 *    Jumper for pins 3/5/6 should be set to '5V'
 *
 *  Control Behavior:
 *
 *    Put it on a line!
 *
 *  External Resources
 *
 ***********************************************************************************/

//Includes
#include <Servo.h>     //include servo library to control continous turn servos
#include "Sounds.h"

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

uint8_t lineFollowingState; //State of line following

//pin constants
const int TRIM_KNOB_PIN = 0;
const int BUZZER_PIN = 12;
const int LED_RIGHT_PIN = 4;
const int LED_LEFT_PIN = 7;
const int LEFT_SERVO_PIN = 10;
const int RIGHT_SERVO_PIN = 11;

//Servo control
Servo servoLeft, servoRight; //wheel servo objects
const int SERVO_STOP = 1500; //servo pulse in microseconds for stopped servo
const int CCW_MIN_SPEED = 1580; //Speed constants from RGS-4C No Load Test Data
const int CW_MIN_SPEED = 1400;

//Turning speed constants
const int SERVO_DRIVE_TURN_SPEED = 70; //For turning while driving
const int SERVO_TURN_SPEED = 25; //For in place turning. Applied to left and rightFwdSpeed

//Servo speeds
const int leftFwdSpeed = CCW_MIN_SPEED + 20;
const int rightFwdSpeed = CW_MIN_SPEED - 20;
int servoSpeedLeft = SERVO_STOP;   //left servo speed.
int servoSpeedRight = SERVO_STOP;  //right servo speed.

//Servo speed trim
int _wheel_speed_trim = 0;
void updateDriveTrim()
{
  int knob_value = analogRead( TRIM_KNOB_PIN );
  _wheel_speed_trim = map( knob_value, 0, 1023, -50, 50 );
}

//Motor Speed Control
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
  updateDriveTrim();
  servoSpeedLeft = leftFwdSpeed + _wheel_speed_trim;
  servoSpeedRight = rightFwdSpeed + _wheel_speed_trim;
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

  Serial.println("Geekbot V2 Line Follower Starting");
}

void loop()
{
  uint8_t nextState = lineFollowingState;
  switch (lineFollowingState)
  {
  case IDLE_STATE:
    motorsStop(); //Stops both motors
    nextState = READ_LINE;
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
    motorsStop(); //Stops both motors
    while(1)
    {
      SoundPlay(WHISTLE);
      delay(1000);
    }
    break;
  }
  lineFollowingState = nextState;
}

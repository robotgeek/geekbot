/***********************************************************************************
 *                         RobotGeek Geekbot V2 Gamepad
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

//Includes
#include <Servo.h>     //include servo library to control continous turn servos
#include "Sounds.h"
#include "Gamepad.h"
unsigned long last_gamepad_command = millis(); //milliseconds since last command was received
const unsigned long gamepad_timeout = 250; //milliseconds to wait without control input for stopping robot (150 minimum)

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
int SERVO_TURN_SPEED = 50; //For in place turning. Applied to CW and CCW_MIN_SPEEDs
const int SERVO_TURN_SPEED_HIGH = 50;
const int SERVO_TURN_SPEED_LOW = 25;

Servo servoLeft, servoRight;      //wheel servo objects

int servoSpeedLeft = SERVO_STOP;   //left servo speed.
int servoSpeedRight = SERVO_STOP;  //right servo speed.

//Wheel speeds from "gear" selection
int leftFwdSpeed = CCW_MAX_SPEED;
int leftRevSpeed = CW_MAX_SPEED;

int rightFwdSpeed = CW_MAX_SPEED;
int rightRevSpeed = CCW_MAX_SPEED;

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

  GamepadEnable();

  SoundEnable();

  Serial.println("Geekbot V2 Start");
}

void loop()
{
  if ( my_gamepad.update_button_states() )
  {
    updateDriveTrim();
    last_gamepad_command = millis();
    servoSpeedLeft = SERVO_STOP;
    servoSpeedRight = SERVO_STOP;

    if ( my_gamepad.button_press_up() )
    {
      Serial.print( "UP" );
      servoSpeedLeft = leftFwdSpeed + _wheel_speed_trim + _wheel_speed_trim_override;
      servoSpeedRight = rightFwdSpeed + _wheel_speed_trim + _wheel_speed_trim_override;
      if ( my_gamepad.button_press_left() )
      {
        Serial.print( " and LEFT" );
        servoSpeedLeft -= SERVO_DRIVE_TURN_SPEED;
      }
      if ( my_gamepad.button_press_right() )
      {
        Serial.print( " and RIGHT" );
        servoSpeedRight += SERVO_DRIVE_TURN_SPEED;
      }
    }
    else if ( my_gamepad.button_press_down() )
    {
      Serial.print( "DOWN" );
      servoSpeedLeft = leftRevSpeed;
      servoSpeedRight = rightRevSpeed;
      if ( my_gamepad.button_press_left() )
      {
        Serial.print( " and LEFT" );
        servoSpeedLeft += SERVO_TURN_SPEED;
      }
      if ( my_gamepad.button_press_right() )
      {
        Serial.print( " and RIGHT" );
        servoSpeedRight -= SERVO_TURN_SPEED;
      }
    }
    else if ( my_gamepad.button_press_left() )
    {
      Serial.print( "LEFT" );
      servoSpeedLeft = CW_MIN_SPEED - SERVO_TURN_SPEED;
      servoSpeedRight = CW_MIN_SPEED - SERVO_TURN_SPEED;
    }
    else if ( my_gamepad.button_press_right() )
    {
      Serial.print( "RIGHT" );
      servoSpeedLeft = CCW_MIN_SPEED + SERVO_TURN_SPEED;
      servoSpeedRight = CCW_MIN_SPEED + SERVO_TURN_SPEED ;
    }

    if ( my_gamepad.button_press_start() )
    {
      Serial.print( "START" );
      SoundPlay(UP);
      _wheel_speed_trim_override += 5;
      my_gamepad.update_button_states();
    }
    if ( my_gamepad.button_press_select() )
    {
      Serial.print( "SELECT" );
      SoundPlay(DOWN);
      _wheel_speed_trim_override -= 5;
      my_gamepad.update_button_states();
    }

    if ( my_gamepad.button_press_b() )
    {
      Serial.print( "B" ); //(Left button)
      if ( SERVO_DRIVE_TURN_SPEED > 10 )
      {
        SERVO_DRIVE_TURN_SPEED -= 10;
        SoundPlay(DOWN);
      }
      else
      {
        SoundPlay(UHOH);
      }
      my_gamepad.update_button_states();
    }
    if ( my_gamepad.button_press_tb() )
    {
      Serial.print( "TB" ); //(Top button)
      switch( currentSpeed )
      {
      case SPEED_MIN:
        SoundPlay(UP);
        SERVO_TURN_SPEED = SERVO_TURN_SPEED_HIGH;
        currentSpeed = SPEED_MED;
        leftFwdSpeed = CCW_MED_SPEED;
        leftRevSpeed = CW_MED_SPEED;
        rightFwdSpeed = CW_MED_SPEED;
        rightRevSpeed = CCW_MED_SPEED;
        break;
      case SPEED_MED:
        SoundPlay(UP);
        currentSpeed = SPEED_MAX;
        leftFwdSpeed = CCW_MAX_SPEED;
        leftRevSpeed = CW_MAX_SPEED;
        rightFwdSpeed = CW_MAX_SPEED;
        rightRevSpeed = CCW_MAX_SPEED;
        break;
      default:
        SoundPlay(UHOH);
      }
      my_gamepad.update_button_states();
    }
    if ( my_gamepad.button_press_a() )
    {
      Serial.print( "A" ); //(Bottom button)
      switch( currentSpeed )
      {
      case SPEED_MED:
        SoundPlay(DOWN);
        currentSpeed = SPEED_MIN;
        SERVO_TURN_SPEED = SERVO_TURN_SPEED_LOW;
        leftFwdSpeed = CCW_MIN_SPEED + 20;
        leftRevSpeed = CW_MIN_SPEED - 20;
        rightFwdSpeed = CW_MIN_SPEED - 20;
        rightRevSpeed = CCW_MIN_SPEED + 20;
        break;
      case SPEED_MAX:
        SoundPlay(DOWN);
        currentSpeed = SPEED_MED;
        leftFwdSpeed = CCW_MED_SPEED;
        leftRevSpeed = CW_MED_SPEED;
        rightFwdSpeed = CW_MED_SPEED;
        rightRevSpeed = CCW_MED_SPEED;
        break;
      default:
        SoundPlay(UHOH);
      }
      my_gamepad.update_button_states();
    }
    if ( my_gamepad.button_press_ta() )
    {
      Serial.print( "TA" ); //(Right button)
      if ( SERVO_DRIVE_TURN_SPEED < 200 )
      {
        SERVO_DRIVE_TURN_SPEED += 10;
        SoundPlay(UP);
      }
      else
      {
        SoundPlay(UHOH);
      }
      my_gamepad.update_button_states();
    }

    Serial.print( " button" );

    Serial.print( " PWM L: " );
    Serial.print( servoSpeedLeft );
    Serial.print( " R: " );
    Serial.println( servoSpeedRight );
  }
  else if ( last_gamepad_command + gamepad_timeout < millis() )
  {
    servoSpeedLeft = SERVO_STOP;
    servoSpeedRight = SERVO_STOP;
  }

  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);
}


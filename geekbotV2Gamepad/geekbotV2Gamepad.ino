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
 *  Pressing directional button(s) on the gamepad starts the Geekbot rolling in that direction.
 *
 *  External Resources
 *
 ***********************************************************************************/

//Includes
#include <Servo.h>     //include servo library to control continous turn servos
#include "Sounds.h"
#include "Gamepad.h"
unsigned long last_gamepad_command = millis();

//pin constants
const int TRIM_KNOB_PIN = 0;
const int BUZZER_PIN = 12;
const int LED_RIGHT_PIN = 4;
const int LED_LEFT_PIN = 7;
const int LEFT_SERVO_PIN = 10;
const int RIGHT_SERVO_PIN = 11;

//Servo control
const int SERVO_STOP = 1500; //servo pulse in microseconds for stopped servo

const int CW_MIN_SPEED = 1400;
const int CW_MAX_SPEED = 1250;

const int CCW_MIN_SPEED = 1580;
const int CCW_MAX_SPEED = 1750;

int SERVO_DRIVE_TURN_SPEED = 70; //For turning while driving (controller selectable)
const int SERVO_TURN_SPEED = 50; //For in place turning

Servo servoLeft, servoRight;      //wheel servo objects
int servoSpeedLeft = SERVO_STOP;   //left servo speed.
int servoSpeedRight = SERVO_STOP;  //right servo speed.

int leftFwdSpeed = CCW_MAX_SPEED;
int leftRevSpeed = CW_MAX_SPEED;

int rightFwdSpeed = CW_MAX_SPEED;
int rightRevSpeed = CCW_MAX_SPEED;


//Trim testing
int _wheel_speed_trim = 0;
void updateDriveTrim()
{
  int knob_value = analogRead( TRIM_KNOB_PIN );
  _wheel_speed_trim = map( knob_value, 0, 1023, -100, 100 );
}

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
      servoSpeedLeft = leftFwdSpeed + _wheel_speed_trim;
      servoSpeedRight = rightFwdSpeed + _wheel_speed_trim;
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
    }
    if ( my_gamepad.button_press_select() )
    {
      Serial.print( "SELECT" );
    }

    if ( my_gamepad.button_press_b() )
    {
      Serial.print( "B" );
      if ( SERVO_DRIVE_TURN_SPEED < 10 )
      {
        SERVO_DRIVE_TURN_SPEED -= 10;
        SoundPlay(DOWN);
      }
      else
      {
        SoundPlay(UHOH);
      }

      delay(250);
      my_gamepad.update_button_states();
    }
    if ( my_gamepad.button_press_tb() )
    {
      Serial.print( "TB" );
    }
    if ( my_gamepad.button_press_a() )
    {
      Serial.print( "A" );
    }
    if ( my_gamepad.button_press_ta() )
    {
      Serial.print( "TA" );
      if ( SERVO_DRIVE_TURN_SPEED < 200 )
      {
        SERVO_DRIVE_TURN_SPEED += 10;
        SoundPlay(UP);
      }
      else
      {
        SoundPlay(UHOH);
      }

      delay(250);
      my_gamepad.update_button_states();
    }

    Serial.print( " button" );

    Serial.print( " PWM L: " );
    Serial.print( servoSpeedLeft );
    Serial.print( " R: " );
    Serial.println( servoSpeedRight );
  }
  else if ( last_gamepad_command + 150 < millis() )
  {
    servoSpeedLeft = SERVO_STOP;
    servoSpeedRight = SERVO_STOP;
  }

  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);
}


/*******************************************************************************
 *                         RobotGeek Geekbot V2 Gamepad
 *  __________________
 *   |              |
 *   |     ___      |
 *  _|____/___\ ____|_
 *   \_/  \___/   \_/
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
 *  Pressing directional button(s) (Up, Down, Left, Right) will drive the Geekbot.
 *  Pressing Select will trim the robot's drive to the left, Start will trim the drive right.
 *  Pressing B and TA (Left and Right) buttons will decrease or increase the amount of turn while driving.
 *  Pressing A and TB (Bottom and Top) buttons will decrease or increase the drive speed.
 *
 *******************************************************************************/

//Includes
#include <Servo.h>
#include <IRGamepad.h>
#include <PiezoEffects.h>

//Pin Constants
const int TRIM_KNOB_PIN = 0;
const int BUZZER_PIN = 12;
const int LED_RIGHT_PIN = 4;
const int LED_LEFT_PIN = 7;
const int LEFT_SERVO_PIN = 10;
const int RIGHT_SERVO_PIN = 11;
const int GAMEPAD_INPUT_PIN = 2;

//Speed constants for servo wheel speeds in microseconds
const int CCW_MAX_SPEED = 1750; //Left wheel forward speed
const int CCW_MED_SPEED = 1660;
const int CCW_MIN_SPEED = 1580;
const int CW_MAX_SPEED = 1250; //Right wheel forward speed
const int CW_MED_SPEED = 1300;
const int CW_MIN_SPEED = 1400;
const int SERVO_STOP = 1500; //Servo pulse in microseconds for stopped servo
const int SERVO_ROTATE_SPEED_HIGH = 50; //For in place rotation. Applied to CW and CCW_MIN_SPEEDs
const int SERVO_ROTATE_SPEED_LOW = 25; //For in place rotation. Applied to CW and CCW_MIN_SPEEDs
const int MAX_TRIM_ADJUSTMENT = 100; //Limit rotation knob trim effect in microseconds

//Gamepad
const unsigned long gamepadTimeout = 250; //Milliseconds to wait without control input for stopping robot (150 minimum)
const bool useModeB = false; //Set to true if your controller is switched to mode B
IR_Gamepad myGamepad(GAMEPAD_INPUT_PIN, useModeB); //IR Gamepad object
unsigned long gamepadCommandTimestamp = millis(); //Milliseconds since last command was received

//Servo control
enum SpeedSelections
{
  SPEED_MIN,
  SPEED_MED,
  SPEED_MAX
};
Servo servoLeft, servoRight; //Wheel servo objects
int servoRotateSpeed = SERVO_ROTATE_SPEED_HIGH; //For rotating in place (controller selectable) default HIGH
int servoDriveTurningSpeed = 70; //For turning while driving (controller selectable) default 70 (microseconds)
int servoSpeedLeft = SERVO_STOP; //Left servo speed to be sent to servo
int servoSpeedRight = SERVO_STOP; //Right servo speed to be sent to server
int leftFwdSpeed = CCW_MAX_SPEED; //Currently selected left forward speed
int leftRevSpeed = CW_MAX_SPEED; //Currently selected left reverse speed
int rightFwdSpeed = CW_MAX_SPEED; //Currently selected right forward speed
int rightRevSpeed = CCW_MAX_SPEED; //Currently selected right reverse speed
int currentSpeed = SPEED_MAX; //Currently selected speed from SpeedSelections enumeration

//Wheel speed trim control
int wheelSpeedTrim = 0; //Current wheel speed trim from rotation knob
int wheelSpeedTrimFromGamepad = 0; //Gamepad selectable wheel speed trim
void updateDriveTrim()
{
  int knob_value = analogRead( TRIM_KNOB_PIN );
  wheelSpeedTrim = map( knob_value, 0, 1023, -MAX_TRIM_ADJUSTMENT, MAX_TRIM_ADJUSTMENT );
}

//Piezo effects (sounds)
PiezoEffects mySounds( BUZZER_PIN ); //PiezoEffects object

void setup()
{
  pinMode(LED_LEFT_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_LEFT_PIN, HIGH);
  digitalWrite(LED_RIGHT_PIN, HIGH);

  Serial.begin(38400);

  servoLeft.attach(LEFT_SERVO_PIN);
  servoRight.attach(RIGHT_SERVO_PIN);
  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);

  myGamepad.enable();

  Serial.println("Geekbot V2 Start");
}

void loop()
{
  if ( myGamepad.update_button_states() )
  {
    updateDriveTrim();
    gamepadCommandTimestamp = millis();
    servoSpeedLeft = SERVO_STOP;
    servoSpeedRight = SERVO_STOP;

    if ( myGamepad.button_press_up() )
    {
      Serial.print( "UP" );
      servoSpeedLeft = leftFwdSpeed + wheelSpeedTrim + wheelSpeedTrimFromGamepad;
      servoSpeedRight = rightFwdSpeed + wheelSpeedTrim + wheelSpeedTrimFromGamepad;
      if ( myGamepad.button_press_left() )
      {
        Serial.print( " and LEFT" );
        servoSpeedLeft -= servoDriveTurningSpeed;
      }
      if ( myGamepad.button_press_right() )
      {
        Serial.print( " and RIGHT" );
        servoSpeedRight += servoDriveTurningSpeed;
      }
    }
    else if ( myGamepad.button_press_down() )
    {
      Serial.print( "DOWN" );
      servoSpeedLeft = leftRevSpeed;
      servoSpeedRight = rightRevSpeed;
      if ( myGamepad.button_press_left() )
      {
        Serial.print( " and LEFT" );
        servoSpeedLeft += servoRotateSpeed;
      }
      if ( myGamepad.button_press_right() )
      {
        Serial.print( " and RIGHT" );
        servoSpeedRight -= servoRotateSpeed;
      }
    }
    else if ( myGamepad.button_press_left() )
    {
      Serial.print( "LEFT" );
      servoSpeedLeft = CW_MIN_SPEED - servoRotateSpeed;
      servoSpeedRight = CW_MIN_SPEED - servoRotateSpeed;
    }
    else if ( myGamepad.button_press_right() )
    {
      Serial.print( "RIGHT" );
      servoSpeedLeft = CCW_MIN_SPEED + servoRotateSpeed;
      servoSpeedRight = CCW_MIN_SPEED + servoRotateSpeed;
    }

    if ( myGamepad.button_press_start() )
    {
      Serial.print( "START" );
      mySounds.play( soundUp );
      wheelSpeedTrimFromGamepad += 5;
      myGamepad.update_button_states();
    }
    if ( myGamepad.button_press_select() )
    {
      Serial.print( "SELECT" );
      mySounds.play( soundDown );
      wheelSpeedTrimFromGamepad -= 5;
      myGamepad.update_button_states();
    }

    if ( myGamepad.button_press_b() )
    {
      Serial.print( "B" ); //(Left button)
      if ( servoDriveTurningSpeed > 10 )
      {
        servoDriveTurningSpeed -= 10;
        mySounds.play( soundDown );
      }
      else
      {
        mySounds.play( soundUhoh );
      }
      myGamepad.update_button_states();
    }
    if ( myGamepad.button_press_tb() )
    {
      Serial.print( "TB" ); //(Top button)
      switch( currentSpeed )
      {
      case SPEED_MIN:
        mySounds.play( soundUp );
        servoRotateSpeed = SERVO_ROTATE_SPEED_HIGH;
        currentSpeed = SPEED_MED;
        leftFwdSpeed = CCW_MED_SPEED;
        leftRevSpeed = CW_MED_SPEED;
        rightFwdSpeed = CW_MED_SPEED;
        rightRevSpeed = CCW_MED_SPEED;
        break;
      case SPEED_MED:
        mySounds.play( soundUp );
        currentSpeed = SPEED_MAX;
        leftFwdSpeed = CCW_MAX_SPEED;
        leftRevSpeed = CW_MAX_SPEED;
        rightFwdSpeed = CW_MAX_SPEED;
        rightRevSpeed = CCW_MAX_SPEED;
        break;
      default:
        mySounds.play( soundUhoh );
      }
      myGamepad.update_button_states();
    }
    if ( myGamepad.button_press_a() )
    {
      Serial.print( "A" ); //(Bottom button)
      switch( currentSpeed )
      {
      case SPEED_MED:
        mySounds.play( soundDown );
        currentSpeed = SPEED_MIN;
        servoRotateSpeed = SERVO_ROTATE_SPEED_LOW;
        leftFwdSpeed = CCW_MIN_SPEED + 20;
        leftRevSpeed = CW_MIN_SPEED - 20;
        rightFwdSpeed = CW_MIN_SPEED - 20;
        rightRevSpeed = CCW_MIN_SPEED + 20;
        break;
      case SPEED_MAX:
        mySounds.play( soundDown );
        currentSpeed = SPEED_MED;
        leftFwdSpeed = CCW_MED_SPEED;
        leftRevSpeed = CW_MED_SPEED;
        rightFwdSpeed = CW_MED_SPEED;
        rightRevSpeed = CCW_MED_SPEED;
        break;
      default:
        mySounds.play( soundUhoh );
      }
      myGamepad.update_button_states();
    }
    if ( myGamepad.button_press_ta() )
    {
      Serial.print( "TA" ); //(Right button)
      if ( servoDriveTurningSpeed < 200 )
      {
        servoDriveTurningSpeed += 10;
        mySounds.play( soundUp );
      }
      else
      {
        mySounds.play( soundUhoh );
      }
      myGamepad.update_button_states();
    }

    Serial.print( " button" );

    Serial.print( " PWM L: " );
    Serial.print( servoSpeedLeft );
    Serial.print( " R: " );
    Serial.println( servoSpeedRight );
  }
  else if ( gamepadCommandTimestamp + gamepadTimeout < millis() )
  {
    servoSpeedLeft = SERVO_STOP;
    servoSpeedRight = SERVO_STOP;
  }

  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);
}

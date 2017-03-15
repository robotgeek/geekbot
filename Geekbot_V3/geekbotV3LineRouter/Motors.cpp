#include "Motors.h"

const int TRIM_KNOB_PIN = 0;

/* PID Controller */
double Setpoint, Input, Output;
PID turningPID(&Input, &Output, &Setpoint, 0.5, 0.02, 0.02, DIRECT); //last success: 0.5, 0.02, 0.02

//Servo control
Servo servoLeft, servoRight; //wheel servo objects
const int SERVO_STOP = 1500; //servo pulse in microseconds for stopped servo
const int CCW_MIN_SPEED = 1580; //Speed constants from RGS-4C No Load Test Data
const int CW_MIN_SPEED = 1400;

//Turning and forward speed constants
//NOTE: Maximum speed with RGS-4C servo is approximately 200 ( microseconds )
const int TRAVEL_SPEED = 100; //Default: 100. Experiment with higher values to move faster.
const int SERVO_DRIVE_TURN_SPEED = 70; //Default: 70. For turning while driving. Higher values will turn faster.
const int SERVO_TURN_SPEED = 25; //Default: 25. For in place turning. Applied to rightFwdSpeed and leftFwdSpeed

const int leftFwdSpeed = CCW_MIN_SPEED + TRAVEL_SPEED;
const int rightFwdSpeed = CW_MIN_SPEED - TRAVEL_SPEED;

int servoSpeedLeft = SERVO_STOP;   //left servo speed.
int servoSpeedRight = SERVO_STOP;  //right servo speed.

int _wheel_speed_trim = 0;

void updateDriveTrim()
{
  int knob_value = analogRead( TRIM_KNOB_PIN );
  _wheel_speed_trim = map( knob_value, 0, 1023, -50, 50 );
}

void motorsInit( int leftPin, int rightPin )
{
  servoLeft.attach(leftPin);
  servoRight.attach(rightPin);
  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);
#ifdef USE_PID
  Input = 0;
  Setpoint = 0;
  turningPID.SetMode(AUTOMATIC);
  turningPID.SetOutputLimits(-SERVO_DRIVE_TURN_SPEED, SERVO_DRIVE_TURN_SPEED);
#endif
}
void motorsSetSpeed()
{
  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);
}
void motorsUpdateForwardSpeed()
{
  updateDriveTrim();
  servoSpeedLeft = leftFwdSpeed + _wheel_speed_trim;
  servoSpeedRight = rightFwdSpeed + _wheel_speed_trim;
}
void motorsStop()
{
  servoSpeedLeft = SERVO_STOP;
  servoSpeedRight = SERVO_STOP;
  motorsSetSpeed();
}
void motorsForward()
{
  motorsUpdateForwardSpeed();
  motorsSetSpeed();
}
void motorsTurnLeft()
{
  motorsUpdateForwardSpeed();
#ifdef USE_PID
  servoSpeedLeft -= Output;
#else
  servoSpeedLeft -= SERVO_DRIVE_TURN_SPEED;
#endif
  motorsSetSpeed();
}
void motorsTurnRight()
{
  motorsUpdateForwardSpeed();
#ifdef USE_PID
  servoSpeedRight -= Output;
#else
  servoSpeedRight += SERVO_DRIVE_TURN_SPEED;
#endif

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

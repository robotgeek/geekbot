#include "Motors.h"

/* PID Controller */
double Setpoint, Input, Output;
PID turningPID(&Input, &Output, &Setpoint, 0.5, 0.02, 0.02, DIRECT); //last success: 0.5, 0.02, 0.02

//Turning and forward speed constants.
//NOTE: Maximum speed with RGS-4C servo is approximately 200 ( microseconds )
const int TRAVEL_SPEED = map(DRIVE_SPEED, 0, 100, SPEED_MIN, SPEED_MAX); //Default: 20. Experiment with higher values to move faster.
const int DRIVE_TURN_SPEED = map(TURN_SPEED, 0, 100, DRIVE_TURN_SPEED_MIN, DRIVE_TURN_SPEED_MAX); //Default: 70. For turning while driving. Higher values will turn faster.
const int ROTATION_SPEED = map(ROTATE_SPEED, 0, 100, ROTATE_SPEED_MIN, ROTATE_SPEED_MAX); //Default: 25. For in place turning. Applied to rightFwdSpeed and leftFwdSpeed

// servo control
Servo servoLeft, servoRight; //wheel servo objects
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
  turningPID.SetOutputLimits(-DRIVE_TURN_SPEED, DRIVE_TURN_SPEED);
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
  servoSpeedLeft = leftFwdSpeed + _wheel_speed_trim ;
  servoSpeedRight = rightFwdSpeed + _wheel_speed_trim;
}

void motorsUpdateForwardSpeed(int speed)
{
  updateDriveTrim();
  servoSpeedLeft = CCW_MIN_SPEED  + speed;
  servoSpeedRight = CW_MIN_SPEED  - speed;
}

void motorsUpdateReverseSpeed(int speed)
{
  updateDriveTrim();
  servoSpeedLeft = CW_MIN_SPEED - speed;
  servoSpeedRight = CCW_MIN_SPEED + speed;
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

void motorsForward(int norm_speed)
{
  int speed = map(norm_speed, 0, 100, SPEED_MIN, SPEED_MAX);
  motorsUpdateForwardSpeed(speed);
  motorsSetSpeed();
}

void motorsReverse(int norm_speed)
{
  int speed = map(norm_speed, 0, 100, SPEED_MIN, SPEED_MAX);
  motorsUpdateReverseSpeed(speed);
  motorsSetSpeed();
}

void motorsTurnLeft()
{
  motorsUpdateForwardSpeed();
#ifdef USE_PID
  servoSpeedLeft -= Output;
#else
  servoSpeedLeft -= DRIVE_TURN_SPEED;
#endif
  motorsSetSpeed();
}

void motorsTurnRight()
{
  motorsUpdateForwardSpeed();
#ifdef USE_PID
  servoSpeedRight -= Output;
#else
  servoSpeedRight += DRIVE_TURN_SPEED;
#endif

  motorsSetSpeed();
}

void motorsRotateLeft()
{
  servoSpeedLeft = CW_MIN_SPEED - ROTATION_SPEED;
  servoSpeedRight = CW_MIN_SPEED - ROTATION_SPEED;
  motorsSetSpeed();
}

void motorsRotateLeft(int norm_speed)
{
  int speed = map(norm_speed, 0, 100, ROTATE_SPEED_MIN, ROTATE_SPEED_MAX);
  servoSpeedLeft = CW_MIN_SPEED - speed;
  servoSpeedRight = CW_MIN_SPEED - speed;
  motorsSetSpeed();
}

void motorsRotateRight()
{
  servoSpeedLeft = CCW_MIN_SPEED + ROTATION_SPEED;
  servoSpeedRight = CCW_MIN_SPEED + ROTATION_SPEED;
  motorsSetSpeed();
}

void motorsRotateRight(int norm_speed)
{
  int speed = map(norm_speed, 0, 100, ROTATE_SPEED_MIN, ROTATE_SPEED_MAX);
  servoSpeedLeft = CCW_MIN_SPEED + speed;
  servoSpeedRight = CCW_MIN_SPEED + speed;
  motorsSetSpeed();
}

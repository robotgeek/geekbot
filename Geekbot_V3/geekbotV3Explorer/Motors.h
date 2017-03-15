#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include <Servo.h>     //include servo library to control continous turn servos
#include <PID_v1.h>
#include "Config.h"

#define USE_PID

//extern const int SERVO_DRIVE_TURN_SPEED;
extern PID turningPID;
extern double Input, Output;

void updateDriveTrim();

void motorsInit( int leftPin, int rightPin );
void motorsSetSpeed();
void motorsUpdateForwardSpeed();
void motorsUpdateForwardSpeed(int speed);
void motorsUpdateReverseSpeed(int speed);
void motorsStop();
void motorsForward();
void motorsForward(int norm_speed);
void motorsReverse(int norm_speed);
void motorsTurnLeft();
void motorsTurnRight();
void motorsRotateLeft();
void motorsRotateLeft(int norm_speed);
void motorsRotateRight();
void motorsRotateRight(int norm_speed);

#endif

#ifndef MOTORS_H
#define MOTORS_H

#include <Arduino.h>
#include <Servo.h>     //include servo library to control continous turn servos
#include <PID_v1.h>

#define USE_PID

extern const int SERVO_DRIVE_TURN_SPEED;
extern PID turningPID;
extern double Input, Output;

void updateDriveTrim();

void motorsInit( int leftPin, int rightPin );
void motorsSetSpeed();
void motorsUpdateForwardSpeed();
void motorsStop();
void motorsForward();
void motorsTurnLeft();
void motorsTurnRight();
void motorsRotateLeft();
void motorsRotateRight();

#endif

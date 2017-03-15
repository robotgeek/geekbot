#ifndef CONFIG_H
#define CONFIG_H

/***********************************************************************************
 *                         RobotGeek Geekbot Configuration
 *
 ***********************************************************************************/
#include <Arduino.h>

/***********************************************************************************
 *                         Arduino Pin Definitions
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
 *    LCD Stop Button - Digital Pin 8
 *
 *    Jumper for pins 9/10/11 should be set to 'VIN'
 *    Jumper for pins 3/5/6 should be set to '5V'
 *
 ***********************************************************************************/

const int BUZZER_PIN 	  = 12;	//Digital Pin 12
const int LED_RIGHT_PIN   =  4;	//Digital Pin 4
const int LED_LEFT_PIN    =  7;	//Digital Pin 7

const int LEFT_SERVO_PIN  = 10;	//Digital Pin 10
const int RIGHT_SERVO_PIN = 11;	//Digital Pin 11

const int TRIM_KNOB_PIN   =  0;	//Analog Pin 0

const uint8_t LCD_UP_PIN   = 3;	//Digital Pin 3
const uint8_t LCD_DOWN_PIN = 9;	//Digital Pin 9
const uint8_t LCD_PLAY_PIN = 1;	//Digital Pin 1
const uint8_t LCD_STOP_PIN = 8;	//Digital Pin 8

/***********************************************************************************
 *                         Robot Parameters
 *
 ***********************************************************************************/

// LineFollowing Color Designation. 0 = White line on dark colored floor, 1 = Black line on light colored floor.
const bool LINE_COLOR =  0;

//Robot Auto-Driving Speed Parameters
const unsigned int DRIVE_SPEED 		= 50;	//Forward/Reverse Driving Speed Value, 0-100
const unsigned int TURN_SPEED  		= 50;	//Turning while Driving Speed, 0-100
const unsigned int ROTATE_SPEED  	= 50;	//Turning in place (rotating) Speed, 0-100

/* Comment out this define if you would like to disable the return home feature */
#define AUTO_RETURN_HOME

/***********************************************************************************
 *                         Advanced Parameters
 *
 ***********************************************************************************/

//Servo control paramaters. Applied in Motors.cpp/h
const int SERVO_STOP    = 1500; //servo pulse in microseconds for stopped servo
const int CCW_MIN_SPEED = 1580; //Speed constants from RGS-4C No Load Test Data
const int CW_MIN_SPEED  = 1400;

const int SPEED_MIN = 10;
const int SPEED_MAX = 30;
const int DRIVE_TURN_SPEED_MIN = 50;
const int DRIVE_TURN_SPEED_MAX = 90;
const int ROTATE_SPEED_MIN = 10;
const int ROTATE_SPEED_MAX = 40;

//Intersection constants, Applied in Navigation.cpp/h
const int BLIND_DRIVE_TIME = 700; //milliseconds to drive forward at intersection
const int BLIND_TURN_TIME  = 250; //milliseconds to turn without checking sensor (time to move off line)


#endif

/***********************************************************************************
 *    _                     RobotGeek Geekbot Rover
 *  _|_|______________      
 *   |___         |
 *  _/ . \ _______|_
 *   \___/      \_/
 *
 *  The
 *
 *    
 *  Wiring
 *    
 *    Right Servo - Digital Pin 9
 *    Left Servo - Digital Pin 10
 *    Scanning Servo - Digital Pin 11
 *    IR Sensor - Analog Pin 0
 *    Jumper for pins 9/10/11 should be set to 'VIN'
 *    Jumper for pins 3/5/6 should be set to '5V'
 *
 *  Control Behavior:
 *
 *  External Resources
 *
 ***********************************************************************************/
//Includes


//servo speed
//95 deg - 125 deg full ccw
//85 deg - 55 deg full cw

//
//Try Full 115 ccw / 45 cw


#include <Servo.h>

#define CW_SPEED 45
#define CCW_SPEED 115

void setup()
{
  Servo servoLeft;
  Servo servoRight;

  servoLeft.attach(9); 
  servoRight.attach(10);

  servoLeft.write(CW_SPEED);
  servoRight.write(CCW_SPEED);
}

void loop()
{
  
}

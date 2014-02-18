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
  
#define OBJECT_IR_READING 500
  
Servo servoLeft;
Servo servoRight;

void setup()
{

  servoLeft.attach(9); 
  servoRight.attach(10);
  
  roverForward();
}

void loop()
{
  if(analogRead(0) >OBJECT_IR_READING)
  {
    roverStop();
    
  }
  else
  {
    roverForward();
    
  }
  
  
}


void roverForward()
{
  servoLeft.write(CCW_SPEED);
  servoRight.write(CW_SPEED);
}

void roverBackward()
{
  servoLeft.write(CW_SPEED);
  servoRight.write(CCW_SPEED);
}

void roverLeft()
{
  servoLeft.write(CW_SPEED);
  servoRight.write(CW_SPEED);
}

void roverRight()
{
  servoLeft.write(CCW_SPEED);
  servoRight.write(CCW_SPEED);
}
void roverStop()
{
  servoLeft.write(90);
  servoRight.write(90);
}

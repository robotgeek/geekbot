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
//Try Full 115 ccw / 65 cw


#include <Servo.h>

#define CW_SPEED 72 //increase if the rover is drifting left
#define CCW_SPEED 113 //decrease if the rover is drifting right
  
#define OBJECT_IR_READING 400
  
Servo servoLeft;
Servo servoRight;
Servo servoPan;


long previousMillis = 0;        // will store last time LED was updated
long interval = 2;           // interval at which to blink (milliseconds)

int sign = 5; //positive when going up, negative when going down
int panValue = 0;

void setup()
{

  servoLeft.attach(9); 
  servoRight.attach(10);
  servoPan.attach(11);
  
  roverForward();
}

void loop()
{
  
  
  
  
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis > interval) 
  {
    // save the last time you blinked the LED 
    previousMillis = currentMillis;   
    
    panValue = sign + panValue;
    
    if(panValue > 160)
    {
      sign = -2;
    }
    else if (panValue <20)
    {
      sign = 2;
    }
    
    servoPan.write(panValue);
  }
  
  
  if(analogRead(0) >OBJECT_IR_READING)
  {
   //roverHeading(panValue);
   
   if(panValue > 110)
   {
    
     roverRight(250); 
   }
   
   else if(panValue < 70)
   {
     
     roverLeft(250);
   }
   
   else
   {
     roverBackward(1000);
     roverLeft(500);
   }
    
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

void roverBackward(int time)
{
  servoLeft.write(CW_SPEED);
  servoRight.write(CCW_SPEED);
  delay(time);
}

void roverLeft(int time)
{
  servoLeft.write(CW_SPEED);
  servoRight.write(CW_SPEED);
  delay(time);
}

void roverRight(int time)
{
  servoLeft.write(CCW_SPEED);
  servoRight.write(CCW_SPEED);
  delay(time);
}
void roverStop()
{
  servoLeft.write(90);
  servoRight.write(90);
}

//heading is a value from 0 to 180. 90 is dead ahead, 0 is left, 180 is right
void roverHeading(int heading)
{
  int centeredHeading = heading - 90; //center the heading so -90 is left, 0 is dead ahdaed and 90 is right
  int turningServo;
  
  //if heading is positive, move t the right
  if(centeredHeading > 0)
  {
    turningServo = map(centeredHeading, 0, 90, 95, CCW_SPEED); 
    servoLeft.write(90);
    servoRight.write(CW_SPEED);
    
  }
  
  else if(centeredHeading<0)
  {
    turningServo = map(centeredHeading, -90, 0, CW_SPEED, 85) ;
    servoLeft.write(CCW_SPEED);
    servoRight.write(90);
    
  }
  
  else
  {
    roverForward() ;
  }
  
  
  
}


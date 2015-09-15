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
#define PAN_SERVO_PIN 12
#define WRIST_SERVO_PIN 6
#define GRIPPER_SERVO_PIN 9
#define LEFT_SERVO_PIN 10
#define RIGHT_SERVO_PIN 11


#define CW_MIN_SPEED 1400 
#define CW_MAX_SPEED 1000 

#define CCW_MIN_SPEED 1600 
#define CCW_MAX_SPEED 2000

#define SERVO_STOP 1500

#define DISTANCE_SENSOR_PIN 1
#define LEFT_FSR_PIN 6
#define RIGHT_FSR_PIN 7

#define LOW_SPEED 0
#define HIGH_SPEED 10
  
#define SERVO_DIRECTION_CW 1
#define SERVO_DIRECTION_CCW -1
#define SERVO_DIRECTION_STOP 0
 
#define BUZZER_PIN 2

#include <Servo.h>

#define CW_SPEED 72 //increase if the rover is drifting left
#define CCW_SPEED 113 //decrease if the rover is drifting right
  
#define OBJECT_IR_READING 150
  
Servo servoPan, servoWrist, servoGripper, servoLeft, servoRight;

long previousMillis = 0;        // will store last time pan servo, was updated
long interval = 2;           // interval at which to blink (milliseconds)

int sign = 5; //positive when going up, negative when going down
int panValue = 0;

int servoSpeedLeft;
int servoSpeedRight;
  
int roverSpeed = 1;  
void setup()
{

  servoPan.attach(PAN_SERVO_PIN);
  servoWrist.attach(WRIST_SERVO_PIN);
  servoGripper.attach(GRIPPER_SERVO_PIN);
  servoLeft.attach(LEFT_SERVO_PIN); 
  servoRight.attach(RIGHT_SERVO_PIN);
  
  servoPan.write(1500); //write 1500us, 90 degrees
  servoWrist.write(1500); //write 1500us, 90 degrees
  servoGripper.write(1500); //write 1500us, 90 degrees
  servoLeft.write(1500); //write 1500us, 90 degrees
  servoRight.write(1500); //write 1500us, 90 degrees
  
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(250);
  digitalWrite(BUZZER_PIN, LOW);
  roverForward(3);
}

void loop()
{
  
  /****Pan Servo Update****/
  unsigned long currentMillis = millis();  //store the current time
 
  //check if interval # of milliseconds has padded
  if(currentMillis - previousMillis > interval) 
  {
    previousMillis = currentMillis;   // save the last time you updated the servo
    
    panValue = sign + panValue; //increment panValue by sign
    //if panValue is more than 160, switch sign to be negative so that panValue will start going down
    if(panValue > 160)
    {
      sign = -2; 
    }
    //if panValue is less than 20, switch sign to be positive so that panValue will start going up
    else if (panValue <20)
    {
      sign = 2;
    }
    
    servoPan.write(panValue); //set servo value
  }
  
  //read analogs
  if(analogRead(0) >OBJECT_IR_READING)
  {
   //roverHeading(panValue);
   //object is to the left
   if(panValue > 110)
   {
     roverRotateRight(3); 
     delay(250);
     roverStop();
   }
   //object is to the right
   else if(panValue < 70)
   {
     
     roverRotateLeft(3); 
     delay(250);
     roverStop();
   }
   //object is dead ahead
   else
   {
     roverBackward(3);
     delay(250);
     roverStop();
     roverRotateLeft(3);
     delay(500);
     roverStop();
   }
    
  }
  //no objects, move forward
  else
  {
    roverForward(3);
    
  }
  
  
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
    roverForward(3) ;
  }
  
  
  
}




void setLeftServoSpeed(int servoSpeed, int servoDirection)
{
  //make sure the servoSpeed passed to the function is within the limits
  servoSpeed = min(servoSpeed, HIGH_SPEED);
  servoSpeed = max(servoSpeed, LOW_SPEED);
  
  //if servo speed is 0 or direction is 0, set the servo to stop
  if(servoSpeed == 0 || servoDirection == SERVO_DIRECTION_STOP)
  {
    servoSpeedLeft = SERVO_STOP;

  }
  //cw direction/speed
  else if(servoDirection == SERVO_DIRECTION_CW)
  {
    servoSpeedLeft = map(servoSpeed, 0, 10, CW_MIN_SPEED, CW_MAX_SPEED );
  }
  //ccw direction/speed
  else if(servoDirection == SERVO_DIRECTION_CCW)
  {
    servoSpeedLeft = map(servoSpeed, 0, 10, CCW_MIN_SPEED, CCW_MAX_SPEED );

  }
  //send results to servo  
  servoLeft.writeMicroseconds(servoSpeedLeft);
}

void setRightServoSpeed(int servoSpeed, int servoDirection)
{
  //make sure the servoSpeed passed to the function is within the limits
  servoSpeed = min(servoSpeed, HIGH_SPEED);
  servoSpeed = max(servoSpeed, LOW_SPEED);
  
  //if servo speed is 0 or direction is 0, set the servo to stop
  if(servoSpeed == 0 || servoDirection == SERVO_DIRECTION_STOP)
  {
    servoSpeedRight = SERVO_STOP;

  }
  //cw direction/speed
  else if(servoDirection == SERVO_DIRECTION_CW)
  {
    servoSpeedRight = map(servoSpeed, 0, 10, CW_MIN_SPEED, CW_MAX_SPEED );
  }
  //ccw direction/speed
  else if(servoDirection == SERVO_DIRECTION_CCW)
  {
    servoSpeedRight = map(servoSpeed, 0, 10, CCW_MIN_SPEED, CCW_MAX_SPEED );

  }
  //send results to servo  
  servoRight.writeMicroseconds(servoSpeedRight);
}



void roverForward(int tempRoverSpeed)
{
  setLeftServoSpeed(tempRoverSpeed, SERVO_DIRECTION_CCW);
  setRightServoSpeed(tempRoverSpeed, SERVO_DIRECTION_CW);

}

void roverBackward(int tempRoverSpeed)
{
  setLeftServoSpeed(tempRoverSpeed, SERVO_DIRECTION_CW);
  setRightServoSpeed(tempRoverSpeed, SERVO_DIRECTION_CCW);

}

void roverRotateLeft(int tempRoverSpeed)
{
  setLeftServoSpeed(tempRoverSpeed, SERVO_DIRECTION_CW);
  setRightServoSpeed(tempRoverSpeed, SERVO_DIRECTION_CW);
  
}

void roverRotateRight(int tempRoverSpeed)
{
  setLeftServoSpeed(tempRoverSpeed, SERVO_DIRECTION_CCW);
  setRightServoSpeed(tempRoverSpeed, SERVO_DIRECTION_CCW);

}
void roverStop()
{
  setLeftServoSpeed(0, SERVO_DIRECTION_STOP);
  setRightServoSpeed(0, SERVO_DIRECTION_STOP);
}



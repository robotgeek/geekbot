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
#define PAN_SERVO_PIN 5
#define WRIST_SERVO_PIN 6
#define GRIPPER_SERVO_PIN 9
#define LEFT_SERVO_PIN 10
#define RIGHT_SERVO_PIN 11

#define DISTANCE_SENSOR_PIN 1
#define LEFT_FSR_PIN 6
#define RIGHT_FSR_PIN 7


#include <Servo.h>

#define CW_MIN_SPEED 1400 //increase if the rover is drifting left
#define CW_MAX_SPEED 1000 //increase if the rover is drifting left

#define CCW_MIN_SPEED 1600 //decrease if the rover is drifting right
#define CCW_MAX_SPEED 2000 //decrease if the rover is drifting right

#define SERVO_STOP 1500

#define LOW_SPEED 0
#define HIGH_SPEED 10
  
#define SERVO_DIRECTION_CW 1
#define SERVO_DIRECTION_CCW -1
#define SERVO_DIRECTION_STOP 0
  
#define OBJECT_IR_READING 400
  
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
  servoWrist.attach(WRIST_SERVO_PIN, 1000, 2000);    //attach with limits
  servoGripper.attach(GRIPPER_SERVO_PIN, 750, 2400); //attach with limits, lower limit makes sure gripper doesn't burn out
  servoLeft.attach(LEFT_SERVO_PIN); 
  servoRight.attach(RIGHT_SERVO_PIN);
  
  servoPan.write(1500); //write 1500us, 90 degrees
  servoWrist.write(1500); //write 1500us, 90 degrees
  servoGripper.write(1500); //write 1500us, 90 degrees
  servoLeft.write(1500); //write 1500us, 90 degrees
  servoRight.write(1500); //write 1500us, 90 degrees
  
}

void loop()
{

  roverForward(roverSpeed);   //forward 1 second half speed
  delay(1000);
  roverStop();
  delay(1000);

  roverBackward(roverSpeed);  //reverse 1 second half speed
  delay(1000);
  roverStop();
  delay(1000);

  roverRotateLeft(roverSpeed);       //left 1 second half speed
  delay(1000);
  roverStop();
  delay(1000);
 
  roverRotateRight(roverSpeed);      //right 1 second half speed
  delay(1000);
  roverStop();
  delay(1000);

  roverSpeed = roverSpeed + 1;
  
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



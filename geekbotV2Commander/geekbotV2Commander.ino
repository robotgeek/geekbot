/***********************************************************************************
 *                         RobotGeek Geekbot V2 Rover
 *  __________________     
 *   |              |     
 *   |     ___      |
 *  _|____/___\ ____|_
 *   \_/  \___/   \_/
 *
 *  The
 *
 *    
 *  Wiring
 *    
 *    Left Servo - Digital Pin 10
 *    Right Servo - Digital Pin 11
 *    Buzzer - Digital Pin 2
 *    Right LED - Digital Pin 4
 *    Left LED - Digital Pin 7
 *
 *    Jumper for pins 9/10/11 should be set to 'VIN'
 *    Jumper for pins 3/5/6 should be set to '5V'
 *
 *  Control Behavior:
 *
 *  External Resources
 *
 ***********************************************************************************/

//Includes
#include <Servo.h>     //include servo library to control continous turn servos
#include <Commander.h> //include commander library to parse instructions from commander

//pin constants
const int BUZZER_PIN = 2;
const int LED_RIGHT_PIN = 4;
const int LED_LEFT_PIN = 7;
const int LEFT_SERVO_PIN = 10;
const int RIGHT_SERVO_PIN = 11;

//speed contants
const int CW_MIN_SPEED = 1400;      //servo pulse in microseconds for slowest clockwise speed
const int CW_MAX_SPEED = 1000;      //servo pulse in microseconds for fastest clockwise speed
const int CCW_MIN_SPEED = 1600;      //servo pulse in microseconds for slowest counter-clockwise speed
const int CCW_MAX_SPEED = 2000;      //servo pulse in microseconds for fastest counter-clockwise speed
const int SERVO_STOP = 1500;         //servo pulse in microseconds for stopped servo

const int FULL_SPEED_FORWARD = CCW_MAX_SPEED - CCW_MIN_SPEED;  //full speed forward, in this case 400
const int FULL_SPEED_REVERESE = CW_MAX_SPEED - CW_MIN_SPEED;    //full speed backward, in this case -400

//mis constants
const int DEADBAND_H = 10;          //deadband around center (0) for horizontal sticks
const int DEADBAND_V = 5;           //deadband around center (0) for vertical sticks
const bool ENABLE_TIMOUT = false;   //setting this to true will enable checking of the commander. This will play notes when connected/disconnected, and stop the servos when disconnected  
const int COMMANDER_TIMEOUT = 2500; //time in MS before timeout occures
const int reportInterval = 5000;    //interval for sending data back to BT / virtual commander

Servo servoLeft, servoRight;      //wheel servo objects
Commander command = Commander();  //commander object

float servoSpeedLeft = SERVO_STOP;   //left servo speed. Ranges from FULL_SPEED_REVERESE to FULL_SPEED_FORWARD with halfway in between (0) being stopped
float servoSpeedRight = SERVO_STOP;  //right servo speed. Ranges from FULL_SPEED_REVERESE to FULL_SPEED_FORWARD with halfway in between (0) being stopped

long lastCommanderTime = (-1 * COMMANDER_TIMEOUT) - 1;  //last time the commander 

bool commanderConnected = false;

int leftMod;
int rightMod;

unsigned long reportTime;

void setup() {
  // put your setup code here, to run once:

  pinMode(LED_LEFT_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_LEFT_PIN, HIGH);

  Serial.begin(38400);

  

  servoLeft.attach(LEFT_SERVO_PIN);
  servoRight.attach(RIGHT_SERVO_PIN);

  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);

  Serial.println("Geekbot V2 Start");


}

void loop() 
{
  // put your main code here, to run repeatedly:

  if (command.ReadMsgs() > 0)
  {
    lastCommanderTime = millis();
    digitalWrite(LED_LEFT_PIN, !digitalRead(LED_LEFT_PIN));


    if ((command.walkV) > DEADBAND_V || (command.walkV < -1 * DEADBAND_V) )
    {
      
      if ((command.walkH) > DEADBAND_H || (command.walkH < -1 * DEADBAND_H) )
      {
        rightMod= map( constrain(command.walkH, 0, 120) , 0, 127, 100, 0);
        leftMod  = map( constrain(command.walkH, -120, 0) , -127, 0, 0, 100);
      }
      else
      {
        leftMod = 100;
        rightMod = 100;
      }
      
      servoSpeedLeft = map(command.walkV, -127, 127, -400, 400);
      servoSpeedRight = map(command.walkV, -127, 127, 400, -400);
      
            
      setLeftServoSpeed(servoSpeedLeft * (float(leftMod/100.0)));
      setRightServoSpeed(servoSpeedRight * (float(rightMod/100.0)));
      
    }
    else
    {  
      if (command.walkH > DEADBAND_H  )
      {
      
        servoSpeedLeft = map(command.walkH, -127, 127, -400, 400);
        servoSpeedRight = map(command.walkH, -127, 127, -400, 400);
        
      }
      else if (command.walkH < -1 * DEADBAND_H)
      {
      
      
        servoSpeedLeft = map(command.walkH, -127, 127, -400, 400);
        servoSpeedRight = map(command.walkH, -127, 127, -400, 400);


        
      }
      
      else
      {
        servoSpeedLeft = 0;
        servoSpeedRight = 0;
      }
      
        setLeftServoSpeed(servoSpeedLeft);
        setRightServoSpeed(servoSpeedRight);
        

    }

  



    if ((command.lookH) > DEADBAND_H || (command.lookH < -1 * DEADBAND_H) )
    {

    }
    else
    {

    }
    if ((command.lookV) > DEADBAND_V || (command.lookV < -1 * DEADBAND_V) )
    {

    }
    else
    {

    }

    if (command.buttons & BUT_R1)
    {
      digitalWrite(BUZZER_PIN, HIGH);
    }
    else
    {
  
      digitalWrite(BUZZER_PIN, LOW);
    }
  



  }


  if (ENABLE_TIMOUT == true)
  {
    checkCommander();
  }
  
  if(millis() - reportTime > reportInterval)
  {
    Serial.print("Left Motor Speed:");
    Serial.print(servoSpeedLeft);
    Serial.print("Right Motor Speed:");
    Serial.print(servoSpeedRight);
    reportTime = millis();
  }



  
}

void checkCommander()
{

  if ((millis() - lastCommanderTime > COMMANDER_TIMEOUT) && commanderConnected == true)
  {
    
  
      commanderConnected = false;
      
      tone(BUZZER_PIN, 2000, 250);
      delay(250);
      tone(BUZZER_PIN, 1500, 250);
      delay(250);
      tone(BUZZER_PIN, 900, 250);
      delay(250);
      noTone(BUZZER_PIN);
      
      setLeftServoSpeed(0);
      setRightServoSpeed(0);
  }
  
  if( (millis() - lastCommanderTime < COMMANDER_TIMEOUT)  && (commanderConnected == false))
  {
    
      commanderConnected = true;
      tone(BUZZER_PIN, 900, 250);
      delay(250);
      tone(BUZZER_PIN, 1500, 250);
      delay(250);
      tone(BUZZER_PIN, 2000, 250);
      delay(250);
      noTone(BUZZER_PIN);
    
  }
  
}


void setLeftServoSpeed(int servoSpeed)
{
  if(servoSpeed > 0)
  {
    servoSpeed = CCW_MIN_SPEED + servoSpeed;    
  }
  else if (servoSpeed < 0)
  {
    servoSpeed = CW_MIN_SPEED + servoSpeed;    
  }
  else
  {
    servoSpeed = SERVO_STOP;
  }

  servoLeft.writeMicroseconds(servoSpeed);
  Serial.print("Left ");
  Serial.print(servoSpeed);
}


void setRightServoSpeed(int servoSpeed)
{
  //servoSpeed = -1 * servoSpeed;
  if(servoSpeed > 0)
  {
    servoSpeed = CCW_MIN_SPEED + servoSpeed;    
  }
  else if (servoSpeed < 0)
  {
    servoSpeed = CW_MIN_SPEED + servoSpeed;    
  }
  else
  {
    servoSpeed = SERVO_STOP;
  }

  servoRight.writeMicroseconds(servoSpeed);
  Serial.print(" Right ");
  Serial.println(servoSpeed);
}




void roverForward()
{
  //  servoLeft.write(CCW_SPEED);
  //  servoRight.write(CW_SPEED);
}

void roverBackward(int time)
{
  //  servoLeft.write(CW_SPEED);
  //  servoRight.write(CCW_SPEED);
  delay(time);
}

void roverLeft(int time)
{
  //  servoLeft.write(CW_SPEED);
  //  servoRight.write(CW_SPEED);
  delay(time);
}

void roverRight(int time)
{
  //  servoLeft.write(CCW_SPEED);
  //  servoRight.write(CCW_SPEED);
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
  //  int centeredHeading = heading - 90; //center the heading so -90 is left, 0 is dead ahdaed and 90 is right
  //  int turningServo;
  //
  //  //if heading is positive, move t the right
  //  if(centeredHeading > 0)
  //  {
  //    turningServo = map(centeredHeading, 0, 90, 95, CCW_SPEED);
  //    servoLeft.write(90);
  //    servoRight.write(CW_SPEED);
  //
  //  }
  //
  //  else if(centeredHeading<0)
  //  {
  //    turningServo = map(centeredHeading, -90, 0, CW_SPEED, 85) ;
  //    servoLeft.write(CCW_MAX_SPEED);
  //    servoRight.write(90);
  //
  //  }
  //
  //  else
  //  {
  //    roverForward() ;
  //  }
  //


}





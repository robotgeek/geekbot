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

#include "pitches.h"

// notes in the melody:
int melody[] = {
  NOTE_C2, NOTE_F3, NOTE_C3, NOTE_A2,
  NOTE_C3, NOTE_F3, NOTE_C3,
  NOTE_C3, NOTE_F3, NOTE_C3, NOTE_F3,
  NOTE_AS3, NOTE_G3, NOTE_F3, NOTE_E3, NOTE_D3, NOTE_CS3,
  NOTE_C3, NOTE_F3, NOTE_C3, NOTE_A2, // the same again
  NOTE_C3, NOTE_F3, NOTE_C3,
  NOTE_AS3, 0, NOTE_G3, NOTE_F3,
  NOTE_E3, NOTE_D3, NOTE_CS3, NOTE_C3};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4,    4,    4,    4,
  4,    4,          2,
  4,    4,    4,    4,
  3,   8, 8, 8, 8, 8,
  4,    4,    4,    4, // the same again
  4,    4,          2,
  4, 8, 8,    4,    4,
  4,    4,    4,    4,
  0};

  
//pin constants
const int BUZZER_PIN = 2;
const int LED_RIGHT_PIN = 4;
const int LED_LEFT_PIN = 7;
const int LEFT_SERVO_PIN = 10;
const int RIGHT_SERVO_PIN = 11;
const int CAMERA_SERVO_PIN = 9;
const int WRIST_SERVO_PIN = 5;
const int GRIPPER_SERVO_PIN = 6;

//speed contants
const int PULSE_MIN = 600;
const int PULSE_MAX = 2400;
const int CW_MIN_SPEED = 1400;      //servo pulse in microseconds for slowest clockwise speed
const int CW_MAX_SPEED = 1300;      //servo pulse in microseconds for fastest clockwise speed
const int CCW_MIN_SPEED = 1600;      //servo pulse in microseconds for slowest counter-clockwise speed
const int CCW_MAX_SPEED = 1700;      //servo pulse in microseconds for fastest counter-clockwise speed
const int SERVO_STOP = 1500;         //servo pulse in microseconds for stopped servo

const int FULL_SPEED_FORWARD = CCW_MAX_SPEED - CCW_MIN_SPEED;  //full speed forward, in this case 400
const int FULL_SPEED_REVERESE = CW_MAX_SPEED - CW_MIN_SPEED;    //full speed backward, in this case -400

//mis constants
const int DEADBAND_H = 60;          //deadband around center (0) for horizontal sticks
const int DEADBAND_V = 10;           //deadband around center (0) for vertical sticks
const bool ENABLE_TIMEOUT = true;   //setting this to true will enable checking of the commander. This will play notes when connected/disconnected, and stop the servos when disconnected  
const int COMMANDER_TIMEOUT = 2500; //time in MS before timeout occures
const int reportInterval = 5000;    //interval for sending data back to BT / virtual commander

const int JOINT_SPEED = 20;

Servo servoLeft, servoRight, servoWrist, servoCamera, servoGripper;      //wheel servo objects
Commander command = Commander();  //commander object

float servoSpeedLeft = SERVO_STOP;   //left servo speed. Ranges from FULL_SPEED_REVERESE to FULL_SPEED_FORWARD with halfway in between (0) being stopped
float servoSpeedRight = SERVO_STOP;  //right servo speed. Ranges from FULL_SPEED_REVERESE to FULL_SPEED_FORWARD with halfway in between (0) being stopped

long lastCommanderTime = (-1 * COMMANDER_TIMEOUT) - 1;  //last time the commander 

bool commanderConnected = false;

int leftMod;
int rightMod;

unsigned long reportTime;


int wristPos = 2400;
int gripperPos = 2400;
int cameraPos = SERVO_STOP;

int wristCommand = 1;
int wristUp = 1;
unsigned long lastGripChange;

int servoLeftTopSpeed = 80;
int servoRightTopSpeed = 100;

int servoLeftTopSpeedReverse = 100;
int servoRightTopSpeedReverse = 85;

      
float leftErr = 1;
float rightErr = 1;

void setup() {
  // put your setup code here, to run once:

  pinMode(LED_LEFT_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_LEFT_PIN, HIGH);

  Serial.begin(38400);

  

  servoLeft.attach(LEFT_SERVO_PIN);
  servoRight.attach(RIGHT_SERVO_PIN);
  servoWrist.attach(WRIST_SERVO_PIN);
  servoGripper.attach(GRIPPER_SERVO_PIN);
  servoCamera.attach(CAMERA_SERVO_PIN);


  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);
  servoCamera.writeMicroseconds(cameraPos);
  servoWrist.writeMicroseconds(wristPos);
  servoGripper.writeMicroseconds(gripperPos);

  Serial.println("Geekbot V2 Start Deluxe Royale with Cheese");

  digitalWrite(BUZZER_PIN, HIGH);
  delay(500);
  
  digitalWrite(BUZZER_PIN, LOW);

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
      
      servoSpeedLeft = map(command.walkV, -127, 127, -servoLeftTopSpeedReverse , servoLeftTopSpeed);
      servoSpeedRight = map(command.walkV, -127, 127, servoRightTopSpeedReverse , -servoRightTopSpeed );
      
            
      setLeftServoSpeed(servoSpeedLeft * (float(leftMod/100.0)));
      setRightServoSpeed(servoSpeedRight * (float(rightMod/100.0)));
      
    }
    else
    {  
      if (command.walkH > DEADBAND_H  )
      {
      
        servoSpeedLeft = map(command.walkH, -127, 127, -servoLeftTopSpeed, servoLeftTopSpeed);
        servoSpeedRight = map(command.walkH, -127, 127, -servoRightTopSpeed, servoRightTopSpeed);
        
      }
      else if (command.walkH < -1 * DEADBAND_H)
      {
      
      
        servoSpeedLeft = map(command.walkH, -127, 127, -servoLeftTopSpeed, servoLeftTopSpeed);
        servoSpeedRight = map(command.walkH, -127, 127, -servoRightTopSpeed, servoRightTopSpeed);


        
      }
      
      else
      {

            
        if (command.buttons & 0x10)
        {
            servoSpeedLeft = 5;
            servoSpeedRight = -5;
          
        } 
    
    
        else if (command.buttons & 0x20)
        {
            servoSpeedLeft = -5;
            servoSpeedRight = 5;
            
    
        } 

        else
        {
        servoSpeedLeft = 0;
        servoSpeedRight = 0;
          
        }



      }
      
        setLeftServoSpeed(servoSpeedLeft);
        setRightServoSpeed(servoSpeedRight);
        

    }

  



//    if ((command.lookH) > DEADBAND_H || (command.lookH < -1 * DEADBAND_H) )
//    {
//
//      
//    }
//    else
//    {
//
//    }




    if (command.lookH > DEADBAND_H  )
    {
    
      servoSpeedLeft = map(command.lookH, -127, 127, -5, 5);
      servoSpeedRight = map(command.lookH, -127, 127, -5, 5);
        setLeftServoSpeed(servoSpeedLeft);
        setRightServoSpeed(servoSpeedRight);
      
    }
    else if (command.lookH < -1 * DEADBAND_H)
    {
    
    
      servoSpeedLeft = map(command.lookH, -127, 127, -5, 5);
      servoSpeedRight = map(command.lookH, -127, 127, -5, 5);
        setLeftServoSpeed(servoSpeedLeft);
        setRightServoSpeed(servoSpeedRight);


      
    }
      
    
    if ((command.lookV) > DEADBAND_V || (command.lookV < -1 * DEADBAND_V) )
    {
//      int wristIncrement = map(command.lookV, -127, 127, -1 * JOINT_SPEED, JOINT_SPEED);
//      wristPos = wristPos + wristIncrement;

      int camIncrement = map(command.lookV, -127, 127, -1 * JOINT_SPEED, JOINT_SPEED);
      cameraPos = cameraPos + camIncrement;
    }
    else
    {

    }
//down
    if (command.buttons & 0x04)
    {
      
      gripperPos = 2400;
      servoGripper.writeMicroseconds(gripperPos);
      wristCommand = 1;
      delay(400);
     wristPos = 2400;
       servoWrist.writeMicroseconds(wristPos);

      //down
      //gripperPos = gripperPos - JOINT_SPEED;
    }
    if (command.buttons & 0x01)
    {
      
      wristPos = 2400;
      servoWrist.writeMicroseconds(wristPos);
      delay(400);
      gripperPos = 2400;
      servoGripper.writeMicroseconds(gripperPos);

      //up
     // gripperPos = gripperPos + JOINT_SPEED;
    }
    if (command.buttons & 0x02 || command.buttons & 0x08  )
    {
      
      gripperPos = 1500;
      servoGripper.writeMicroseconds(gripperPos);
      delay(400);
      wristPos = 1950; //2000
      servoWrist.writeMicroseconds(wristPos);

      
     // gripperPos = gripperPos + JOINT_SPEED;
    }

// if (command.ext & 0x20)
//    {
//      
//     wristPos = 1050;
//       servoWrist.writeMicroseconds(wristPos);
//       delay(1000);
//       
//      gripperPos = 2400;
//      servoGripper.writeMicroseconds(gripperPos);
//      wristCommand = 1;
//      delay(100);
//     wristPos = 1900;
//       servoWrist.writeMicroseconds(wristPos);
//      
//    }



    
    if (command.buttons & 0x40)
    {
        
      servoLeftTopSpeed = servoLeftTopSpeed - 10;
      servoRightTopSpeed = servoRightTopSpeed - 10;
      tone(BUZZER_PIN,servoLeftTopSpeed * 30);
      delay(100);
      noTone(BUZZER_PIN);
    } 


    if (command.buttons & 0x80)
    {
        
      servoLeftTopSpeed = servoLeftTopSpeed + 10;
      servoRightTopSpeed = servoRightTopSpeed + 10;
      tone(BUZZER_PIN,servoLeftTopSpeed * 30);
      delay(100);
      noTone(BUZZER_PIN);
    } 

  //select
    if (command.ext & 0x01)
    {
      //servoTopSpeed = 25;
      servoLeftTopSpeed = 13;
      servoRightTopSpeed = 29;
      
      servoLeftTopSpeedReverse = 29;
      servoRightTopSpeedReverse = 15;
      
        
      tone(BUZZER_PIN,servoLeftTopSpeed * 30);
      delay(100);
      noTone(BUZZER_PIN);
    }

//home
     if (command.ext & 0x02)
    {
        
      //servoTopSpeed = 100;
      servoLeftTopSpeed = 80;
      servoRightTopSpeed = 100;
      servoLeftTopSpeedReverse = 100;
      servoRightTopSpeedReverse = 85;
        
      tone(BUZZER_PIN,servoLeftTopSpeed * 30);
      delay(100);
      noTone(BUZZER_PIN);

    } 

  //start
    if (command.ext & 0x04)
    {
      //servoTopSpeed = 400;
      servoLeftTopSpeed = 360;
      servoRightTopSpeed = 400;
      servoLeftTopSpeedReverse = 350;
      servoRightTopSpeedReverse = 400;
        
      tone(BUZZER_PIN,servoLeftTopSpeed * 30);
      delay(100);
      noTone(BUZZER_PIN);
      
    }


    //x
    if (command.ext & 0x08)
    {
        
        setLeftServoSpeed(400);
        setRightServoSpeed(400);
        delay(550);
        setLeftServoSpeed(0);
        setRightServoSpeed(0);
      
    }
    //y
    if (command.ext & 0x10)
    {
      
        setLeftServoSpeed(-400);
        setRightServoSpeed(-400);
        delay(550);
        setLeftServoSpeed(0);
        setRightServoSpeed(0);

      
    }

    //a
    if (command.ext & 0x20)
    {
      
      
      gripperPos = 2400;
      servoGripper.writeMicroseconds(gripperPos);
      setLeftServoSpeed(200);
      setRightServoSpeed(-200);

      delay(300);
      
      setLeftServoSpeed(0);
      setRightServoSpeed(0);
      
      
      gripperPos = 1500;
      servoGripper.writeMicroseconds(gripperPos);
      
      delay(300);
      wristPos = 1950; //2000
      servoWrist.writeMicroseconds(wristPos);

      

      
    }

    //b
    if (command.ext & 0x40)
    {
      
      
      wristPos = 2400;
      servoWrist.writeMicroseconds(wristPos);
      delay(250);
      
      setLeftServoSpeed(400);
      setRightServoSpeed(400);
      delay(250);
      setLeftServoSpeed(0);
      setRightServoSpeed(0);
      
      wristPos = 1950; //2000
      servoWrist.writeMicroseconds(wristPos);
      
      delay(250);
      wristPos = 2400;
      servoWrist.writeMicroseconds(wristPos);
      delay(250);
      
      setLeftServoSpeed(-400);
      setRightServoSpeed(-400);
      delay(400);
      setLeftServoSpeed(0);
      setRightServoSpeed(0);
      
      
      wristPos = 1950; //2000
      servoWrist.writeMicroseconds(wristPos);
      
      delay(250);
      wristPos = 2400;
      servoWrist.writeMicroseconds(wristPos);
      delay(250);

      
      setLeftServoSpeed(400);
      setRightServoSpeed(400);
      delay(250);
      setLeftServoSpeed(0);
      setRightServoSpeed(0);
      
      
      wristPos = 1950; //2000
      servoWrist.writeMicroseconds(wristPos);
      
      delay(250);
      wristPos = 2400;
      servoWrist.writeMicroseconds(wristPos);
      delay(250);

      lastCommanderTime = millis();
      while(Serial.available())
      {
        Serial.read();
      }
        
    }





//
//    if (command.ext & 0x20)
//    {
//          for (int thisNote = 0; noteDurations[thisNote] != 0; thisNote++) {
//      
//          // to calculate the note duration, take one second 
//          // divided by the note type.
//          //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
//          int noteDuration = 2000/noteDurations[thisNote];
//          tone(BUZZER_PIN, melody[thisNote],noteDuration * 0.9);
//      
//          // to distinguish the notes, set a minimum time between them.
//          // the note's duration + 30% seems to work well:
//          //int pauseBetweenNotes = noteDuration * 1.30;
//          //delay(pauseBetweenNotes);
//        delay(noteDuration);
//        }
//    }


    servoRightTopSpeed = constrain(servoRightTopSpeed,5, 400);
    servoLeftTopSpeed = constrain(servoLeftTopSpeed,5, 400);
   




    
//    else
//    {
//  
//      digitalWrite(BUZZER_PIN, LOW);
//    }
  
  cameraPos = constrain(cameraPos, PULSE_MIN,PULSE_MAX );
  gripperPos = constrain(gripperPos, PULSE_MIN,PULSE_MAX );
  wristPos = constrain(wristPos, PULSE_MIN,PULSE_MAX );
  
  servoCamera.writeMicroseconds(cameraPos);
  servoWrist.writeMicroseconds(wristPos);
  servoGripper.writeMicroseconds(gripperPos);

  }


  if (ENABLE_TIMEOUT == true)
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
      delay(100);
      tone(BUZZER_PIN, 1500, 250);
      delay(500);
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
      delay(500);
      tone(BUZZER_PIN, 2000, 250);
      delay(100);
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





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
 *    Buzzer - Digital Pin 12
 *    IR Receiver - Digital Pin 2
 *    Right LED - Digital Pin 4
 *    Left LED - Digital Pin 7
 *
 *    Jumper for pins 9/10/11 should be set to 'VIN'
 *    Jumper for pins 3/5/6 should be set to '5V'
 *
 *  Control Behavior:
 *  Pressing a directional button on the remote starts the Geekbot rolling in that direction.
 *  Pressing Enter/Select, or any unassigned button causes the Geekbot to stop.
 *  Pressing buttons 1-9 selects the speed 
 *
 *  External Resources
 *
 ***********************************************************************************/

//Includes
#include <IRLib.h>
#include <Servo.h>     //include servo library to control continous turn servos
#include "remotes.h"

//pin constants
const int BUZZER_PIN = 8;
const int LED_RIGHT_PIN = 4;
const int LED_LEFT_PIN = 7;
const int LEFT_SERVO_PIN = 10;
const int RIGHT_SERVO_PIN = 11;
const int WRIST_SERVO_PIN = 5;
const int GRIPPER_SERVO_PIN = 6;

//speed control
int CW_MIN_SPEED = 1400;      //servo pulse in microseconds for slowest clockwise speed
int CW_MAX_SPEED = 1000;      //servo pulse in microseconds for fastest clockwise speed
int CCW_MIN_SPEED = 1600;      //servo pulse in microseconds for slowest counter-clockwise speed
int CCW_MAX_SPEED = 2000;      //servo pulse in microseconds for fastest counter-clockwise speed
const int SERVO_STOP = 1500;         //servo pulse in microseconds for stopped servo

//gripper control
int wristPos = 2400;
int gripperPos = 2400;

int wristCommand = 1;
int wristUp = 1;
unsigned long lastGripChange;


IRrecv My_Receiver(2);    // Receive Pin
IRdecode My_Decoder;       // decoder object

Servo servoLeft, servoRight, servoWrist, servoGripper;      // servo objects

unsigned long lastCommand; // last command sent from IR remote

float servoSpeedLeft = SERVO_STOP;   //left servo speed. Ranges from FULL_SPEED_REVERESE to FULL_SPEED_FORWARD with halfway in between (0) being stopped
float servoSpeedRight = SERVO_STOP;  //right servo speed. Ranges from FULL_SPEED_REVERESE to FULL_SPEED_FORWARD with halfway in between (0) being stopped


void setup() {
  // put your setup code here, to run once:

  My_Receiver.No_Output();        // Turn off any unused IR LED output circuit
  My_Receiver.enableIRIn();       // Start the receiver
  
  pinMode(LED_LEFT_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_LEFT_PIN, HIGH);
  digitalWrite(LED_RIGHT_PIN, HIGH);

  Serial.begin(38400);

  servoLeft.attach(LEFT_SERVO_PIN);
  servoRight.attach(RIGHT_SERVO_PIN);
  servoWrist.attach(WRIST_SERVO_PIN);
  servoGripper.attach(GRIPPER_SERVO_PIN);

  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);
  servoWrist.writeMicroseconds(wristPos);
  servoGripper.writeMicroseconds(gripperPos);


  Serial.println("Geekbot V2 Start");

}

void loop() 
{
  // put your main code here, to run repeatedly:

  if (My_Receiver.GetResults(&My_Decoder)) // read signal from decoder object
  {
     My_Decoder.decode();
     if(My_Decoder.decode_type == MY_PROTOCOL) // if decoder is using the set protocol
     {
      
        servoLeft.attach(LEFT_SERVO_PIN);
        servoRight.attach(RIGHT_SERVO_PIN);

       if(My_Decoder.value != REPEATING) // if the decoder value received is not the NEC Repeat code (0xFFFFFFFF)
       {
         lastCommand = My_Decoder.value; // variable lastCommand gets set to the decoder value
       }

       if(lastCommand == UP_BUTTON) // if the last command sent is the up arrow
       {
        servoSpeedLeft = CCW_MAX_SPEED;
        servoSpeedRight = CW_MAX_SPEED;
       }

       else if(lastCommand == DOWN_BUTTON) // if the last command sent is the down arrow
       {
        servoSpeedLeft = CW_MAX_SPEED;
        servoSpeedRight = CCW_MAX_SPEED;
       }

       else if(lastCommand == RIGHT_BUTTON) // if the last command sent is the right arrow
       {
        servoSpeedLeft = CCW_MAX_SPEED;
        servoSpeedRight = CCW_MAX_SPEED;
       }
       
       else if(lastCommand == LEFT_BUTTON) // if the last command sent is the left arrow
       {
        servoSpeedLeft = CW_MAX_SPEED;
        servoSpeedRight = CW_MAX_SPEED;
       }
       
       else if(lastCommand == SELECT_BUTTON) // if the last command sent is the enter/select button
       {
        //stop the servos
        servoSpeedLeft = SERVO_STOP;
        servoSpeedRight = SERVO_STOP;
       }

       else if(lastCommand == ONE_BUTTON) // if the last command sent is the 1 button
       {
        CW_MAX_SPEED = 1400;
        CCW_MAX_SPEED = 1600;
       }

       else if(lastCommand == TWO_BUTTON) // if the last command sent is the 1 button
       {
        CW_MAX_SPEED = 1350;
        CCW_MAX_SPEED = 1650;
       }

       else if(lastCommand == THREE_BUTTON) // if the last command sent is the 1 button
       {
        CW_MAX_SPEED = 1300;
        CCW_MAX_SPEED = 1700;
       }

       else if(lastCommand == FOUR_BUTTON) // if the last command sent is the 1 button
       {
        CW_MAX_SPEED = 1250;
        CCW_MAX_SPEED = 1750;
       }

       else if(lastCommand == FIVE_BUTTON) // if the last command sent is the 1 button
       {
        CW_MAX_SPEED = 1200;
        CCW_MAX_SPEED = 1800;
       }

       else if(lastCommand == SIX_BUTTON) // if the last command sent is the 1 button
       {
        CW_MAX_SPEED = 1150;
        CCW_MAX_SPEED = 1850;
       }

       else if(lastCommand == SEVEN_BUTTON) // if the last command sent is the 1 button
       {
        CW_MAX_SPEED = 1100;
        CCW_MAX_SPEED = 1900;
       }
       
       else if(lastCommand == EIGHT_BUTTON) // if the last command sent is the 1 button
       {
        CW_MAX_SPEED = 1050;
        CCW_MAX_SPEED = 1950;
       }

       else if(lastCommand == NINE_BUTTON) // if the last command sent is the 1 button
       {
        CW_MAX_SPEED = 1000;
        CCW_MAX_SPEED = 2000;
       }

       else if(lastCommand == ZERO_BUTTON) // if the last command sent is the 1 button
       {
        CW_MAX_SPEED = 1500;
        CCW_MAX_SPEED = 1500;
       }

       else if(lastCommand == SPECIAL_1_BUTTON)
       {
         wristPos = 2400;
         servoWrist.writeMicroseconds(wristPos);
         delay(400);
         gripperPos = 2400;
         servoGripper.writeMicroseconds(gripperPos);
         Serial.println(wristPos); // print last command to serial for debugging
         Serial.println(gripperPos); // print last command to serial for debugging
       }

       else if(lastCommand == SPECIAL_2_BUTTON)
       {
         gripperPos = 1500;
         servoGripper.writeMicroseconds(gripperPos);
         delay(400);
         wristPos = 1950; //2000
         servoWrist.writeMicroseconds(wristPos);
         Serial.println(wristPos); // print last command to serial for debugging
         Serial.println(gripperPos); // print last command to serial for debugging
       }
       
       else //if any undefined command comes through
       {
        //stop the servos
        servoSpeedLeft = SERVO_STOP;
        servoSpeedRight = SERVO_STOP;
       }
       
        servoLeft.writeMicroseconds(servoSpeedLeft);
        servoRight.writeMicroseconds(servoSpeedRight);
        Serial.print("Last Command:"); // print last command to serial for debugging
        Serial.print(lastCommand, HEX); // print last command to serial for debugging
        Serial.println(servoSpeedLeft); // print last command to serial for debugging
        Serial.println(servoSpeedRight); // print last command to serial for debugging
     } 
     
     My_Receiver.resume();
  }
}





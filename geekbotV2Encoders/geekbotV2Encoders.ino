/***********************************************************************************
 *                      RobotGeek Geekbot V2 Encoder Support
 *  __________________     
 *   |              |     
 *   |     ___      |
 *  _|____/___\ ____|_
 *   \_/  \___/   \_/
 *
 *  Wiring:
 *    
 *    Left Servo - Digital Pin 10
 *    Right Servo - Digital Pin 11
 *    Buzzer - Digital Pin 12
 *    Right LED - Digital Pin 4
 *    Left LED - Digital Pin 7
 *    Push Switch - Digital Pin 5 //TODO: Validate
 *
 *    Jumper for pins 9/10/11 should be set to 'VIN'
 *    Jumper for pins 3/5/6 should be set to '5V'
 *
 *  Control Behavior:
 *  
 *  External Resources:
 *
 ***********************************************************************************/

#include <Servo.h>     //include servo library to control continous turn servos
#include <SharpIR.h>

#define USB_DEBUG //Enables serial.print() statements

#define SWITCH_PIN 5
#define SPKR_PIN 12

#include "sounds.h"
#include "led_control.h"
#include "drive_control.h"

void waitForHumanInput()
{
  playSound(WHISTLE);

  while ( digitalRead( SWITCH_PIN ) == LOW )
  {
    flashLEDs( 2, 50 );
  }
}

void program_finished()
{
  while( true )
  {
#ifdef USB_DEBUG
    Serial.println( "Program complete" );
#endif
    delay(1000);
  }
}

void setup()
{
  servoLeft.attach(LEFT_SERVO_PIN);
  servoRight.attach(RIGHT_SERVO_PIN);

  servoSensor.attach( SENSOR_PAN_PIN );
  servoSensor.writeMicroseconds( IR_PAN_CENTER );

#ifdef USB_DEBUG
  Serial.begin(38400);
  delay(1000);
  Serial.println("Starting");
#endif

  pinMode( SPKR_PIN, OUTPUT );

  pinMode( LED_LEFT, OUTPUT );
  pinMode( LED_RIGHT, OUTPUT );

  pinMode( SWITCH_PIN, INPUT );

  last_timestamp = millis();
  last_distance_timestamp = millis();
}

void loop() 
{
  /******************************************************\
   * Create your own program by utilizing
   * the following drive functions:
   * 
   * driveForward( distance )
   * driveReverse( distance )
   * 
   * driveLeft( degrees )
   * driveRight( degrees )
   * 
   * wallFollowLeft( wall_distance, travel_distance )
   * wallFollowRight( wall_distance, travel_distance )
   * 
   * wallFollowLeftUntil( wall_distance, stop_distance )
   * wallFollowRightUntil( wall_distance, stop_distance )
   * 
  \******************************************************/

/*  BUZZER TESTS 
  playSound(UP); delay(500);
  playSound(DOWN); delay(500);
  playSound(WHISTLE); delay(500);
  playSound(LAUGH); delay(500);
  playSound(OHH); delay(500);
  playSound(UHOH); delay(500);
  playSound(BEEPS); delay(500);
*/

  /* Basic: Press button and robot will run-away, whistling for you to press it's button again
  playSound(BEEPS);
  delay(1000);
  while(1)
  {
    waitForHumanInput(); 
    driveForward( 1.0 );
    lookRight();
    driveRight( 180.0 );
    lookLeft();
  }
  */

  /* Intermediate: Follow wall on left until it ends, robot will wait for input, then return home */
  double meters_traveled = 0;
  int sampled_wall_distance = 0;

  lookLeft();
  
  waitForHumanInput(); //Pause program until button is pressed
  
  sampled_wall_distance = getCurrentDistance(); //Get current distance for wall following
  
  playSound(BEEPS);   
  meters_traveled = wallFollowLeftUntil( sampled_wall_distance, sampled_wall_distance*2 ); //Follow wall

  playSound(LAUGH);
  waitForHumanInput(); //Pause program until button is pressed
  delay(250); //Give person time to back away after pressing button

  lookRight();
  playSound(UP);
  driveRight(180.0); //Turn around

  sampled_wall_distance = getCurrentDistance(); //Get current distance for wall following
  
  playSound(BEEPS);
  wallFollowRight( sampled_wall_distance, meters_traveled ); //Follow wall until we've reached the origin

  driveLeft(180.0);
  playSound(LAUGH);
  
  program_finished();
  
  /* Advanced: Travel around square object, wait for input.. return to other side
  {
    float corner_forward_distance = 0.45; //distance to travel forward when cornering 90 degrees
    int sampled_wall_distance = 0;
    playSound(BEEPS);
    
    lookRight();
  
    waitForHumanInput(); //Pause program until button is pressed
  
    sampled_wall_distance = getCurrentDistance(); //Get current distance for wall following
    //long distance
    wallFollowRightUntil( sampled_wall_distance, sampled_wall_distance * 2 );
    driveForward( corner_forward_distance );
    driveRight( 90.0 );
    driveForward( 0.5 );
    
    //short distance
    sampled_wall_distance = getCurrentDistance(); //Get current distance for wall following
    wallFollowRightUntil( sampled_wall_distance, sampled_wall_distance * 2 );
    driveForward( corner_forward_distance );
    driveRight( 90.0 );
    driveForward( corner_forward_distance );
  
    //final stretch to pickup
    sampled_wall_distance = getCurrentDistance(); //Get current distance for wall following 
    wallFollowRight( sampled_wall_distance, 1.0 );
  
    //wait for pickup
    waitForHumanInput();
  
    lookLeft();
  
    driveLeft(180.0); //Turn around
  
    //long distance
    sampled_wall_distance = getCurrentDistance(); //Get current distance for wall following 
    wallFollowLeftUntil( sampled_wall_distance, irsensorValue * 2 );
    driveForward( corner_forward_distance );
    driveLeft( 90.0 );
    driveForward( corner_forward_distance );
  
    //short distance
    sampled_wall_distance = getCurrentDistance(); //Get current distance for wall following 
    wallFollowLeftUntil( sampled_wall_distance, irsensorValue * 2 );
    driveForward( corner_forward_distance );
    driveLeft( 90.0 );
    driveForward( corner_forward_distance );
  
    //Final stretch
    sampled_wall_distance = getCurrentDistance(); //Get current distance for wall following 
    wallFollowLeft( sampled_wall_distance, 2.0 );
  
    waitForHumanInput();
  }
  */
}

/***********************************************************************************
 *                      RobotGeek Geekbot V2 Encoder Support
 *  __________________     
 *   |              |     
 *   |     ___      |
 *  _|____/___\ ____|_
 *   \_/  \___/   \_/
 *
 *  Wiring: //TODO: Standardize
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
 *  
 *  External Resources:
 *
 ***********************************************************************************/

#define USB_DEBUG //Enables serial.print() statements
#define USE_BUZZER //Install a piezo buzzer on DIO-3 for sound feedback
#define USE_LEDS //Install a left and right LED as blinkers on DIO-12 and DIO-2 respectively
#define USE_SWITCHINPUT //Install a pushbutton switch on DIO-4 to allow for human interaction

#include <Servo.h>     //include servo library to control continous turn servos
#include <SharpIR.h>

#ifdef USE_SWITCHINPUT
#define SWITCH_PIN 4
#endif

#ifdef USE_LEDS
#define LED_LEFT 12
#define LED_RIGHT 2
unsigned long last_led_blink = millis();
unsigned long led_flash_delay = 100; //time in milliseconds for LED flashing
int led_blinks_left = 0;
int led_blinks_right = 0;
bool led_state_left = false;
bool led_state_right = false;
#endif

#ifdef USE_BUZZER
#define SPKR_PIN 3
#include "sounds.h"
#endif

#ifdef USE_ULTRASONIC
#define trigPin 7
#define echoPin 8
#endif

#include "led_control.h"
#include "drive_control.h"

#ifdef USE_SWITCHINPUT
void waitForHumanInput()
{
#ifdef USE_BUZZER
  playSound(WHISTLE);
#endif

  while ( digitalRead( SWITCH_PIN ) == LOW )
  {
    flashLEDs( 2, 50 );
  }
}
#endif

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
  
#ifdef USE_ULTRASONIC
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
#endif

#ifdef USB_DEBUG
  Serial.begin(38400);
  delay(1000);
  Serial.println("Starting");
#endif

#ifdef USE_BUZZER
  pinMode( SPKR_PIN, OUTPUT );
#endif

#ifdef USE_LEDS
  pinMode( LED_LEFT, OUTPUT );
  pinMode( LED_RIGHT, OUTPUT );
#endif

#ifdef USE_SWITCHINPUT
  pinMode( SWITCH_PIN, INPUT );
#endif

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

  /* Intermediate: something something something
  playSound(BEEPS);  
    wallFollowLeftUntil( 50, 75 ); //Follow wall
  playSound(LAUGH);
    driveLeft( 180.0 ); //Turn around
  playSound(UP);
    driveForward(0.25);
  playSound(BEEPS);
    wallFollowRightUntil( 50, 75 ); //Follow wall
  playSound(LAUGH);
    driveLeft( 180.0 ); //Turn around
  playSound(DOWN);
    driveForward(0.25);
  */
  
  /* Advanced: Travel around square object, wait for input.. return to other side */
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
  
}

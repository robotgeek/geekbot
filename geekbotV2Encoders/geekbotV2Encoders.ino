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
 *    Left LED - Digital Pin 7
 *    Right LED - Digital Pin 4
 *    Push Switch - Digital Pin 5
 *    Rotation Knob - Analog Pin 2
 *    Left IR Encoder - Analog Pin 3
 *    Right IR Encoder - Analog Pin 4
 *    SharpIR Module - Analog Pin 5
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

#include <Servo.h>     //include servo library to control continous turn servos
#include <SharpIR.h>

#include "sounds.h"
#include "led_control.h"
#include "drive_control.h"

#define SWITCH_PIN 5

void waitForButtonPress()
{
  playSound(WHISTLE);

  while ( digitalRead( SWITCH_PIN ) == LOW )
  {
    flashLEDs( 2, 50 );
  }
}

void waitForButtonRelease()
{
  playSound(WHISTLE);

  while ( digitalRead( SWITCH_PIN ) == HIGH )
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

  _last_timestamp = millis();
  _last_distance_timestamp = millis();

  irsensorValue = getCurrentDistance();
}

void corner_left_example()
{
  Drive( 0.45 );
  Rotate( -85 );
  Drive( 0.45 );
  lookLeft();
}
void loop() 
{
  /******************************************************\
   * Create your own program by utilizing
   * the following drive functions:
   * 
   * Drive( distance_in_meters )
   *    Positive distance_in_meters forward movement
   *    Negative distance_in_meters reverse movement
   *    
   * Drive( distance_in_meters, stop_trigger_range )
   *    Positive distance_in_meters = limit forward movement to distance
   *    Negative distance_in_meters = limit reverse movement to distance
   *    Zero distance_in_meters = unlimited forward movement
   *    
   *    Positive stop_trigger_range = Stop movement when IR range exceedes value
   *    Negative stop_trigger_range = Stop movement when IR range is under value
   *    
   * Drive( distance_in_meters, stop_trigger_range, look_direction )
   *    Positive distance_in_meters = limit forward movement to distance
   *    Negative distance_in_meters = limit reverse movement to distance
   *    Zero distance_in_meters = unlimited forward movement
   *    
   *    Positive stop_trigger_range = Stop movement when IR range exceedes value
   *    Negative stop_trigger_range = Stop movement when IR range is under value
   *    
   *    look_direction is the IR servo value in microseconds. Provided values:
   *        IR_PAN_CENTER
   *        IR_PAN_LEFT
   *        IR_PAN_RIGHT
   *    
   * Rotate( distance_in_degrees )
   *    Positive distance_in_degrees = Clockwise rotation
   *    Negative distance_in_degrees = Counter-clockwise rotation
   *    
   * WallFollow( which_wall, distance_in_meters, wall_range )
   *    which_wall can be either WALL_LEFT or WALL_RIGHT
   *    
   *    Positive distance_in_meters = limit forward movement to distance
   *    
   *    wall_range is the IR range to the wall in centimeters that the robot will attempt to follow
   *   
   * WallFollow( which_wall, distance_in_meters, wall_range, stop_trigger_range )
   *    which_wall can be either WALL_LEFT or WALL_RIGHT
   *    
   *    Positive distance_in_meters = limit forward movement to distance
   *    Zero distance_in_meters = unlimited forward movement
   *    
   *    wall_range is the IR range to the wall in centimeters that the robot will attempt to follow
   *      
   *    Positive stop_trigger_range = Stop movement when IR range exceedes value
   *    Negative stop_trigger_range = Stop movement when IR range is under value
   *
   * IRread()
   *    Returns integer value representing the IR range in centimeters
   *    
   * Orientate()
   *    Experimental: Measures front facing wall, estimates angle and rotates robot to be perpendicular to the wall
   *    
   * waitForButtonPress()
   * waitForButtonRelease()
   * 
   * playSound( SoundID )
   * 
  \******************************************************/
 
  playSound(UP);
  
  waitForButtonPress();

  /* Path from lab to kitchen */
  Drive( 2.0, -40, IR_PAN_CENTER ); //Exit room until wall is sampled less than 40cm away, looking forward
  Rotate( -90 ); //Turn left
  Drive( 3.0, -30 ); //Drive until wall is sampled less than 30cm away
  Rotate( 90 ); //Turn right
  lookLeft(); //Looking left for IRread in next command
  WallFollow( WALL_LEFT, 0.0, IRread(), IRread()+10 ); //Follow left wall at current IR distance until wall ends
  Drive( 0.0, -35, IR_PAN_CENTER ); //Drive forward until sofa is sampled less than 35cm away, looking forward
  Rotate( 90 ); //Turn right
  lookLeft(); //Looking left for IRread in next command
  WallFollow( WALL_LEFT, 0.0, IRread(), IRread()+10 ); //Follow left wall at current IR distance until wall ends
  corner_left_example(); //Round the corner
  WallFollow( WALL_LEFT, 0.0, IRread(), IRread()+10 ); //Follow left wall at current IR distance until wall ends
  corner_left_example(); //Round the corner
  WallFollow( WALL_LEFT, 0.0, IRread(), IRread()+10 ); //End of back side of sofa
  Drive( 1.05 ); //Blind drive 1.95 meters
  Rotate( 90 ); //Turn right
  Drive( 0.0, -40, IR_PAN_RIGHT ); //Drive until wall on right is sampled less than 40cm away
  WallFollow( WALL_RIGHT, 0.0, IRread(), IRread()+10 ); //Follow right wall at current IR distance until wall ends
}

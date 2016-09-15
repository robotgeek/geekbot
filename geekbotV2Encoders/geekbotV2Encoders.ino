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
 *    Rotation Knob - Analog Pin 0
 *    Left IR Encoder - Analog Pin 1
 *    Right IR Encoder - Analog Pin 2
 *    SharpIR Module - Analog Pin 3
 *    
 *    Jumper for pins 9/10/11 should be set to 'VIN'
 *    Jumper for pins 3/5/6 should be set to '5V'
 *
 *  Control Behavior:
 *  
 *  External Resources:
 *
 ***********************************************************************************/
#define LCD_DEBUG //Enabled output to I2C display
#define USB_DEBUG //Enables serial.print() statements

#include <Servo.h>     //include servo library to control continous turn servos
#include <SharpIR.h>

#ifdef LCD_DEBUG
#include <Wire.h>                 //Load for I2C
#include <LiquidCrystal_I2C.h>    //Load the LiquidCrystal I2C Library for the LCD Display
LiquidCrystal_I2C lcd(0x27, 16, 2);  //I2C 2 Line LCD Screen
#endif

#include "sounds.h"
#include "led_control.h"
#include "drive_control.h"

#define SWITCH_PIN 5

void waitForButtonPress()
{
#ifdef LCD_DEBUG
  lcd.clear();
  lcd.print( "Waiting for" );
  lcd.setCursor(0, 1);
  lcd.print( "button press!" );
#endif

  playSound(WHISTLE);

  while ( digitalRead( SWITCH_PIN ) == LOW )
  {
    flashLEDs( 2, 50 );
  }
}

void waitForButtonRelease()
{
#ifdef LCD_DEBUG
  lcd.clear();
  lcd.print( "Waiting for" );
  lcd.setCursor(0, 1);
  lcd.print( "button release!" );
#endif

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

  irsensorValue = getCurrentDistance();
  
#ifdef LCD_DEBUG
  // initlaize the lcd object - this sets up all the variables and IIC setup for the LCD object to work
  lcd.begin();
  lcd.backlight();
  // Print a message to the LCD.
  lcd.print("Geekbot starting");
  delay(1000);
#endif
}

void corner_left_example()
{
  Drive( 0.45 );
  Rotate( -85 );
  Drive( 0.45 );
  lookLeft();
}

void corner_right_example()
{
  Drive( 0.45 );
  Rotate( 85 );
  Drive( 0.45 );
  lookRight();
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
  
  while( 0 )
  {
    Serial.print( analogRead( LEFT_ENCODER_PIN ) );
    Serial.print( "\t" );
    Serial.println( analogRead( RIGHT_ENCODER_PIN ) );
    delay(50);
  }
  
  playSound(UP);

  waitForButtonPress();

  while(0) //Square test
  {
    Drive( 1.0 );
    delay(1000); //Time to read LCD
    Rotate( 90 );
    delay(1000); //Time to read LCD
  }

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

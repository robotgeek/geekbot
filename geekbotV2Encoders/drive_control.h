#define LEFT_SERVO_PIN 10
#define RIGHT_SERVO_PIN 11
#define LEFT_ENCODER_PIN 3
#define RIGHT_ENCODER_PIN 4

#include "panning_range_sensor.h"

Servo servoLeft, servoRight;      //wheel servo objects

//Continous servo speed control
const int CW_MIN_SPEED = 1380; //1400;    //servo pulse in microseconds for slowest clockwise speed
const int CCW_MIN_SPEED = 1600;           //servo pulse in microseconds for slowest counter-clockwise speed
const int SERVO_STOP = 1500;        //servo pulse in microseconds for stopped servo
int servoSpeedLeft = SERVO_STOP;  //left servo speed
int servoSpeedRight = SERVO_STOP; //right servo speed

const int ENCODER_VALUE_THRESHOLD = 512; //ADC input value for high signal
const int encoderCounts_per_revolution = 64; //Number of slices on wheel encoder
bool rightEncoderRising = true; //state of encoder counting
bool rightEncoderFalling = false;
bool leftEncoderRising = true;
bool leftEncoderFalling = false;

int rightEncoderCount = 0; //number of encoder ticks per command
int leftEncoderCount = 0;
double degreesTraveled = 0.0; //total degrees of rotation per command
double distanceTraveled = 0.0; //integrated velocity over time
unsigned long ticksTraveledLeft = 0; //total ticks traveled per command
unsigned long ticksTraveledRight = 0;

#define WheelRadius 0.05 //meters (50 millimeters)
const double meters_per_revolution = 2.0 * PI * WheelRadius; //Wheel circumference for distance traveled

#define WheelSpacing 0.195 //meters (195 millimeters)
const double wheel_to_wheel_circumference = 2.0 * PI * ( WheelSpacing / 2.0 ); //Circumference for in-place rotation
const double degrees_per_revolution = meters_per_revolution / wheel_to_wheel_circumference * 360;

const double meters_per_tick = meters_per_revolution / encoderCounts_per_revolution;
const double degrees_per_tick = degrees_per_revolution / encoderCounts_per_revolution;

double wheelSpeedRight = 0.0; //Current wheel speed in revolutions/second
double wheelSpeedLeft = 0.0;

int driveDirection = 1; //Current driving direction for wheel speed correction

unsigned long last_timestamp = millis();

void processEncoders()
{
  int rightEncoderValue = analogRead(RIGHT_ENCODER_PIN); // get encoder value
  int leftEncoderValue = analogRead(LEFT_ENCODER_PIN);

  //Catch falling edge ( black stripe ) for right and left wheel speed encoders
  if (rightEncoderFalling && rightEncoderValue < ENCODER_VALUE_THRESHOLD)
  {
    ++rightEncoderCount;
    rightEncoderRising = true;
    rightEncoderFalling = false;
  }
  if (leftEncoderFalling && leftEncoderValue < ENCODER_VALUE_THRESHOLD)
  {
    ++leftEncoderCount; 
    leftEncoderRising = true;
    leftEncoderFalling = false;
  }

  //Catch rising edge ( white stripe ) for right and left wheel speed encoders
  if (rightEncoderRising && rightEncoderValue > ENCODER_VALUE_THRESHOLD) 
  {
    ++rightEncoderCount;     
    rightEncoderRising = false;
    rightEncoderFalling = true;
  } 
  if (leftEncoderRising && leftEncoderValue > ENCODER_VALUE_THRESHOLD) 
  {
    ++leftEncoderCount;     
    leftEncoderRising = false;
    leftEncoderFalling = true;
  }
  
  if ( last_timestamp + 100ul < millis() )
  {
    last_timestamp = millis();

    wheelSpeedRight = (double)rightEncoderCount/(double)encoderCounts_per_revolution;
    wheelSpeedLeft = (double)leftEncoderCount/(double)encoderCounts_per_revolution;
    wheelSpeedRight *= 10.0; //Sampling at 2Hz but reporting speed at 1 Revolution/Second
    wheelSpeedLeft *= 10.0;

#ifdef USB_DEBUG
    Serial.print("Ticks L:");
    Serial.print(leftEncoderCount);
    Serial.print(" R:");
    Serial.print(rightEncoderCount);
    Serial.print(" RPS L:");
    Serial.print(wheelSpeedLeft);
    Serial.print(" R:");
    Serial.print(wheelSpeedRight);
#endif

    //Wheel speed compensation
    if ( driveDirection < 2 ) //Only compensate for FWD and REV ( +1, -1 )
    {
      if ( leftEncoderCount - rightEncoderCount > 1 )
      {
        servoSpeedLeft += driveDirection * 3;
        servoSpeedRight -= driveDirection * 3;
      }
      else if ( rightEncoderCount - leftEncoderCount > 1 )
      {
        servoSpeedLeft -= driveDirection * 3;
        servoSpeedRight += driveDirection * 3;
      }
    }
#ifdef USB_DEBUG
    Serial.print( " PWM L: " );
    Serial.print( servoSpeedLeft );

    Serial.print( " R: " );
    Serial.print( servoSpeedRight );

    Serial.print( " Range: " );
    Serial.println( irsensorValue );
#endif
    
    ticksTraveledLeft += leftEncoderCount;
    ticksTraveledRight += rightEncoderCount;
    
    distanceTraveled += (leftEncoderCount + rightEncoderCount)/2.0 * meters_per_tick;
    degreesTraveled += (leftEncoderCount + rightEncoderCount)/2.0 * degrees_per_tick;
    
    rightEncoderCount = 0;
    leftEncoderCount = 0;
  }

  servoLeft.writeMicroseconds( servoSpeedLeft );
  servoRight.writeMicroseconds( servoSpeedRight );
}

void driveStop()
{
  servoSpeedLeft = SERVO_STOP;
  servoSpeedRight = SERVO_STOP;
  servoLeft.writeMicroseconds( servoSpeedLeft );
  servoRight.writeMicroseconds( servoSpeedRight );
  
#ifdef USB_DEBUG
  Serial.print( "Distance Traveled: " );
  Serial.print( distanceTraveled );
  Serial.print( " Ticks Traveled: " );
  Serial.print( ticksTraveledLeft );
  Serial.print( " Degrees Traveled: " );
  Serial.println( degreesTraveled );
#endif

#ifdef USE_LEDS
  flashLEDs( 3, 250 );
#endif

  ticksTraveledLeft = 0;
  ticksTraveledRight = 0;
  distanceTraveled = 0.0;
  degreesTraveled = 0.0;
}

void driveForward( double meters )
{
  servoSpeedLeft = CCW_MIN_SPEED;
  servoSpeedRight = CW_MIN_SPEED;
  while ( distanceTraveled < meters )
  {
    driveDirection = 1;
    processEncoders();
  }
  driveStop();
}

void driveReverse( double meters )
{
  servoSpeedLeft = CW_MIN_SPEED;
  servoSpeedRight = CCW_MIN_SPEED;
  
  while ( distanceTraveled < meters )
  {
    driveDirection = -1;
    processEncoders();
  }
  driveStop();
}

void driveLeft( double p_degrees )
{
  servoSpeedLeft = CW_MIN_SPEED;
  servoSpeedRight = CW_MIN_SPEED;
  while ( degreesTraveled < p_degrees )
  {
    driveDirection = 2;
    processEncoders();
  }
  driveStop();
}

void driveRight( double p_degrees )
{
  servoSpeedLeft = CCW_MIN_SPEED;
  servoSpeedRight = CCW_MIN_SPEED;
  while ( degreesTraveled < p_degrees )
  { 
    driveDirection = 2;
    processEncoders();
  }
  driveStop();
}

void wallFollowLeft( int p_distance, double p_meters )
{
#ifdef USB_DEBUG
  Serial.println( "Sensor looking left" );
#endif

  servoSensor.writeMicroseconds( IR_PAN_LEFT );
  delay(IR_PAN_DELAY); //delay so we have time to look left before sampling sensor

  servoSpeedLeft = CCW_MIN_SPEED;
  servoSpeedRight = CW_MIN_SPEED;
  
  while ( distanceTraveled < p_meters )
  {
    processDistanceSensor();
    if ( irsensorValue > p_distance )
    {
      servoSpeedLeft = CCW_MIN_SPEED - DISTANCE_TURN_GAIN;
      servoSpeedRight = CW_MIN_SPEED;
    }
    else
    {
      servoSpeedRight = CW_MIN_SPEED + DISTANCE_TURN_GAIN;
      servoSpeedLeft = CCW_MIN_SPEED;
    }
    
    driveDirection = 1;
    processEncoders();
    setBlinksLeft( 2 );
    processLEDs();
  }
  driveStop();
}

double wallFollowLeftUntil( int p_distance, int p_stopRange )
{
#ifdef USB_DEBUG
  Serial.println( "Sensor looking left" );
#endif

  servoSensor.writeMicroseconds( IR_PAN_LEFT );
  delay(IR_PAN_DELAY); //delay so we have time to look left before sampling sensor

  servoSpeedLeft = CCW_MIN_SPEED;
  servoSpeedRight = CW_MIN_SPEED;
  processDistanceSensor();
  while ( irsensorValue < p_stopRange )
  {
    processDistanceSensor();
    if ( irsensorValue > p_distance )
    {
      servoSpeedLeft = CCW_MIN_SPEED - DISTANCE_TURN_GAIN;
      servoSpeedRight = CW_MIN_SPEED;
    }
    else
    {
      servoSpeedRight = CW_MIN_SPEED + DISTANCE_TURN_GAIN;
      servoSpeedLeft = CCW_MIN_SPEED;
    }
    
    driveDirection = 1;
    processEncoders();
    setBlinksLeft( 2 );
    processLEDs();
  }
  double total_distance = distanceTraveled;
  driveStop();
  return total_distance;
}

void wallFollowRight( int p_distance, double p_meters )
{
#ifdef USB_DEBUG
  Serial.println( "Sensor looking right" );
#endif

  servoSensor.writeMicroseconds( IR_PAN_RIGHT );
  delay(IR_PAN_DELAY); //delay so we have time to look left before sampling sensor

  servoSpeedLeft = CCW_MIN_SPEED;
  servoSpeedRight = CW_MIN_SPEED;
  
  while ( distanceTraveled < p_meters )
  {
    processDistanceSensor();
    if ( irsensorValue > p_distance )
    {
      servoSpeedLeft = CCW_MIN_SPEED;
      servoSpeedRight = CW_MIN_SPEED + DISTANCE_TURN_GAIN;
    }
    else
    {
      servoSpeedRight = CW_MIN_SPEED;
      servoSpeedLeft = CCW_MIN_SPEED - DISTANCE_TURN_GAIN;
    }
    
    driveDirection = 1;
    processEncoders();
    setBlinksRight( 2 );
    processLEDs();
  }
  driveStop();
}

void wallFollowRightUntil( int p_distance, int p_stopRange )
{
#ifdef USB_DEBUG
  Serial.println( "Sensor looking right" );
#endif

  servoSensor.writeMicroseconds( IR_PAN_RIGHT );
  delay(IR_PAN_DELAY); //delay so we have time to look left before sampling sensor

  servoSpeedLeft = CCW_MIN_SPEED;
  servoSpeedRight = CW_MIN_SPEED;
  processDistanceSensor();
  while ( irsensorValue < p_stopRange )
  {
    processDistanceSensor();
    if ( irsensorValue > p_distance )
    {
      servoSpeedLeft = CCW_MIN_SPEED;
      servoSpeedRight = CW_MIN_SPEED + DISTANCE_TURN_GAIN;
    }
    else
    {
      servoSpeedRight = CW_MIN_SPEED;
      servoSpeedLeft = CCW_MIN_SPEED - DISTANCE_TURN_GAIN;
    }
    
    driveDirection = 1;
    processEncoders();
    setBlinksRight( 2 );
    processLEDs();
  }
  driveStop();
}

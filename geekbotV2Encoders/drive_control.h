#ifndef DRIVE_CONTROL_H
#define DRIVE_CONTROL_H

#include "panning_range_sensor.h"

#define LEFT_SERVO_PIN 10
#define RIGHT_SERVO_PIN 11
#define LEFT_ENCODER_PIN 1
#define RIGHT_ENCODER_PIN 2

#define ROTATION_KNOB_PIN 0 //For wheel speed trim adjustment

#define WheelRadius 0.05 //meters (50 millimeters)
#define WheelSpacing 0.195 //meters (195 millimeters)

Servo servoLeft, servoRight;      //wheel servo objects

//Constants
const int CW_MIN_SPEED = 1400;    //servo pulse in microseconds for slowest clockwise speed
const int CCW_MIN_SPEED = 1600;   //servo pulse in microseconds for slowest counter-clockwise speed
const int SERVO_STOP = 1500;        //servo pulse in microseconds for stopped servo
const int ENCODER_VALUE_THRESHOLD = 250; //ADC input value for high/low signal change
const int encoderCounts_per_revolution = 64; //Number of slices on wheel encoder

const double meters_per_revolution = 2.0 * PI * WheelRadius; //Wheel circumference for distance traveled

const double wheel_to_wheel_circumference = 2.0 * PI * ( WheelSpacing / 2.0 ); //Circumference for in-place rotation
const double degrees_per_revolution = meters_per_revolution / wheel_to_wheel_circumference * 360;

const double meters_per_tick = meters_per_revolution / encoderCounts_per_revolution;
const double degrees_per_tick = degrees_per_revolution / encoderCounts_per_revolution;

int _wheel_speed_trim = 0; //Current trim for servo pulse in microseconds

int _servoSpeedLeft = SERVO_STOP;  //left servo speed
int _servoSpeedRight = SERVO_STOP; //right servo speed

bool _rightEncoderRising = true;    //state of encoder counting
bool _rightEncoderFalling = false;  //state of encoder counting
bool _leftEncoderRising = true;     //state of encoder counting
bool _leftEncoderFalling = false;   //state of encoder counting

int _rightEncoderCount = 0; //number of encoder ticks per interval during command
int _leftEncoderCount = 0; //number of encoder ticks per interval during command
double _degreesTraveled = 0.0; //total degrees of rotation per command
double _distanceTraveled = 0.0; //integrated velocity over time
unsigned long _ticksTraveledLeft = 0; //total ticks per interval during command
unsigned long _ticksTraveledRight = 0; //total ticks per interval during command
unsigned long _totalTicksRight = 0; //total ticks traveled per command
unsigned long _totalTicksLeft = 0; //total ticks traveled per command
int _driveDirection = 1; //Current driving direction for wheel speed correction

unsigned long _last_speed_correction_timestamp = millis();
unsigned long _last_travel_timestamp = millis();
unsigned long _last_encoder_poll_timestamp = millis();

void updateDriveTrim()
{
  int knob_value = analogRead( ROTATION_KNOB_PIN );
  _wheel_speed_trim = map( knob_value, 0, 1023, -20, 20 );
}

void processEncoders()
{
  unsigned long millisnow = millis();

  //Checking encoder state every 5 milliseconds
  if ( _last_encoder_poll_timestamp + 5ul < millisnow )
  {
    _last_encoder_poll_timestamp = millis();
    
    int rightEncoderValue = analogRead(RIGHT_ENCODER_PIN); // get encoder value
    int leftEncoderValue = analogRead(LEFT_ENCODER_PIN);
  
    //Catch falling edge ( white stripe ) for right and left wheel encoders
    if (_rightEncoderFalling && rightEncoderValue < ENCODER_VALUE_THRESHOLD)
    {
      ++_rightEncoderCount;
      ++_ticksTraveledRight;
      _rightEncoderRising = true;
      _rightEncoderFalling = false;
    }
    if (_leftEncoderFalling && leftEncoderValue < ENCODER_VALUE_THRESHOLD)
    {
      ++_leftEncoderCount;
      ++_ticksTraveledLeft;
      _leftEncoderRising = true;
      _leftEncoderFalling = false;
    }
  
    //Catch rising edge ( black stripe ) for right and left wheel encoders
    if (_rightEncoderRising && rightEncoderValue > ENCODER_VALUE_THRESHOLD) 
    {
      ++_rightEncoderCount;
      ++_ticksTraveledRight;
      _rightEncoderRising = false;
      _rightEncoderFalling = true;
    } 
    if (_leftEncoderRising && leftEncoderValue > ENCODER_VALUE_THRESHOLD) 
    {
      ++_leftEncoderCount;
      ++_ticksTraveledLeft;
      _leftEncoderRising = false;
      _leftEncoderFalling = true;
    }
  }

  //Integrate distance travel and degrees traveled every 20 milliseconds
  if ( _last_travel_timestamp + 20ul < millisnow )
  {
    _last_travel_timestamp = millisnow;

    _distanceTraveled += (_ticksTraveledLeft + _ticksTraveledRight)/2.0 * meters_per_tick;
    _degreesTraveled += (_ticksTraveledLeft + _ticksTraveledRight)/2.0 * degrees_per_tick;

    _totalTicksLeft += _ticksTraveledLeft;
    _totalTicksRight += _ticksTraveledRight;
    
    _ticksTraveledLeft = 0;
    _ticksTraveledRight = 0;
  }

  //Compensate wheel speeed based on encoder feedback every 500 milliseconds
  if ( _last_speed_correction_timestamp + 500ul < millisnow )
  {
    _last_speed_correction_timestamp = millisnow;

#ifdef USB_DEBUG
    Serial.print("Encoder Count L:");
    Serial.print(_leftEncoderCount);
    Serial.print(" R:");
    Serial.print(_rightEncoderCount);
#endif

#ifdef LCD_DEBUG
    lcd.setCursor(0, 0);
    lcd.print( "L:" );
    lcd.print( _leftEncoderCount );
    lcd.print( " " );
    lcd.setCursor(7, 0);
    lcd.print( "R:" );
    lcd.print( _rightEncoderCount );
    lcd.print( " " );
    lcd.setCursor(13, 0);
    lcd.print( irsensorValue );
    lcd.print( " " );
#endif

    //Update wheel speeds based on rotation knob input
    updateDriveTrim();
    if ( _driveDirection == 1 )
    {
      _servoSpeedLeft = CCW_MIN_SPEED + _wheel_speed_trim * _driveDirection;
      _servoSpeedRight = CW_MIN_SPEED + _wheel_speed_trim * _driveDirection;
    }
    else if ( _driveDirection == -1 )
    {
      _servoSpeedLeft = CW_MIN_SPEED + _wheel_speed_trim * _driveDirection;
      _servoSpeedRight = CCW_MIN_SPEED + _wheel_speed_trim * _driveDirection;
    }
      
#ifdef USB_DEBUG
    Serial.print( " PWM L: " );
    Serial.print( _servoSpeedLeft );

    Serial.print( " R: " );
    Serial.print( _servoSpeedRight );

    Serial.print( " IR Range: " );
    Serial.print( irsensorValue );

    Serial.print( " Distance: " );
    Serial.print( _distanceTraveled );
    Serial.print( " Degrees: " );
    Serial.println( _degreesTraveled );
#endif

#ifdef LCD_DEBUG
    lcd.setCursor(0, 1);
    lcd.print( "Dst:" );
    lcd.print( _distanceTraveled );
    lcd.print( "Deg:" );
    lcd.print( _degreesTraveled );
#endif

    _rightEncoderCount = 0;
    _leftEncoderCount = 0;
  }

  servoLeft.writeMicroseconds( _servoSpeedLeft );
  servoRight.writeMicroseconds( _servoSpeedRight );
}

double driveStop()
{
  double total_distance_traveled = 0.0;
  
  _servoSpeedLeft = SERVO_STOP;
  _servoSpeedRight = SERVO_STOP;
  servoLeft.writeMicroseconds( _servoSpeedLeft );
  servoRight.writeMicroseconds( _servoSpeedRight );
  
#ifdef USB_DEBUG
  Serial.print( "Distance Traveled: " );
  Serial.print( _distanceTraveled );
  Serial.print( " Ticks Traveled: " );
  Serial.print( _ticksTraveledLeft );
  Serial.print( " Degrees Traveled: " );
  Serial.println( _degreesTraveled );
#endif

#ifdef LCD_DEBUG
  lcd.clear();
  lcd.print( "L:" );
  lcd.print( _totalTicksLeft );
  lcd.print( " " );
  lcd.setCursor(7, 0);
  lcd.print( "R:" );
  lcd.print( _totalTicksRight );
  lcd.print( " " );
  lcd.setCursor(13, 0);
  lcd.print( irsensorValue );
  lcd.print( " " );
  lcd.setCursor(0, 1);
  lcd.print( "Dst:" );
  lcd.print( _distanceTraveled );
  lcd.print( "Deg:" );
  lcd.print( _degreesTraveled );
  delay( 1000 );
#endif

  flashLEDs( 3, 250 );

  total_distance_traveled = _distanceTraveled;

  _rightEncoderCount = 0;
  _leftEncoderCount = 0;
  _ticksTraveledLeft = 0;
  _ticksTraveledRight = 0;
  _totalTicksLeft = 0;
  _totalTicksRight = 0;
  _distanceTraveled = 0.0;
  _degreesTraveled = 0.0;

  return total_distance_traveled;
}

void driveStart( int leftSpeed, int rightSpeed )
{
#ifdef LCD_DEBUG
  lcd.clear();
#endif

  int rampSpeedLeft = (SERVO_STOP - leftSpeed) / 2;
  int rampSpeedRight = (SERVO_STOP - rightSpeed) / 2;
  
  servoLeft.writeMicroseconds( SERVO_STOP + rampSpeedLeft );
  servoRight.writeMicroseconds( SERVO_STOP + rampSpeedRight );
}

double driveForward( double meters )
{
  updateDriveTrim();
  _servoSpeedLeft = CCW_MIN_SPEED + _wheel_speed_trim;
  _servoSpeedRight = CW_MIN_SPEED + _wheel_speed_trim;
  _driveDirection = 1;
  driveStart( _servoSpeedLeft, _servoSpeedRight );
  while ( _distanceTraveled < meters )
  {
    processEncoders();
    processLEDs();
  }
  return driveStop();
}

double driveReverse( double meters )
{
  updateDriveTrim();
  _servoSpeedLeft = CW_MIN_SPEED + _wheel_speed_trim;
  _servoSpeedRight = CCW_MIN_SPEED + _wheel_speed_trim;
  _driveDirection = -1;
  driveStart( _servoSpeedLeft, _servoSpeedRight );
  while ( _distanceTraveled < meters )
  {
    processEncoders();
  }
  return driveStop();
}

void driveLeft( double p_degrees )
{
  updateDriveTrim();
  _servoSpeedLeft = CW_MIN_SPEED + _wheel_speed_trim;
  _servoSpeedRight = CW_MIN_SPEED + _wheel_speed_trim;
  _driveDirection = 0;
  driveStart( _servoSpeedLeft, _servoSpeedRight );
  while ( _degreesTraveled < p_degrees )
  {
    processEncoders();
  }
  driveStop();
}

void driveRight( double p_degrees )
{
  updateDriveTrim();
  _servoSpeedLeft = CCW_MIN_SPEED + _wheel_speed_trim;
  _servoSpeedRight = CCW_MIN_SPEED + _wheel_speed_trim;
  _driveDirection = 0;
  driveStart( _servoSpeedLeft, _servoSpeedRight );
  while ( _degreesTraveled < p_degrees )
  { 
    processEncoders();
  }
  driveStop();
}

double drive_helper_with_trigger( double meter_limit, int stopRange )
{
  processDistanceSensor();
  if ( meter_limit == 0.0 ) //If there is no limit we drive until desired IR sensor reading is obtained
  {
    if ( stopRange > 0 ) //Positive stop range means we stop driving when the current irsensorValue is greater than the stopRange
    {
      while ( irsensorValue < stopRange ) //While the current irsensorValue is less than the stopRange
      {
        processDistanceSensor();
        processEncoders();
      }
    }
    else //Negative stop range means we stop driving when the current irsensorValue is less than absolute value of the stopRange
    {
      while ( irsensorValue > abs(stopRange) )
      {
        processDistanceSensor();
        processEncoders();
      }
    }
  }
  else //Use distance limit. We drive until desired IR sensor reading is obtained or meter_limit is reach
  {
    if ( stopRange > 0 ) //Positive stop range means we stop driving when the current irsensorValue is greater than the stopRange
    {
      while ( _distanceTraveled < meter_limit && irsensorValue < stopRange )
      {
        processDistanceSensor();
        processEncoders();
      }
    }
    else //Negative stop range means we stop driving when the absolute value of the stopRange goes under the current irsensorValue
    {
      while ( _distanceTraveled < meter_limit && irsensorValue > abs(stopRange) )
      {
        processDistanceSensor();
        processEncoders();
      }
    }
  }
  return driveStop();
}

double Drive( double distance_in_meters, int stop_trigger_ir_range = 0, int look_direction = 0 )
{
  if ( look_direction != 0 )
  {
    lookCustom( look_direction );
  }
  if ( stop_trigger_ir_range != 0 )
  {
    if ( distance_in_meters >= 0.0 )
    {
      updateDriveTrim();
      _servoSpeedLeft = CCW_MIN_SPEED + _wheel_speed_trim;
      _servoSpeedRight = CW_MIN_SPEED + _wheel_speed_trim;
      _driveDirection = 1;
      driveStart( _servoSpeedLeft, _servoSpeedRight );
      return drive_helper_with_trigger( distance_in_meters, stop_trigger_ir_range );
    }
    else
    {
      updateDriveTrim();
      _servoSpeedLeft = CW_MIN_SPEED + _wheel_speed_trim;
      _servoSpeedRight = CCW_MIN_SPEED + _wheel_speed_trim;
      _driveDirection = -1;
      driveStart( _servoSpeedLeft, _servoSpeedRight );
      return drive_helper_with_trigger( distance_in_meters, stop_trigger_ir_range );
    }
  }
  else if ( distance_in_meters > 0.0 )
  {
    return driveForward( distance_in_meters );
  }
  else
  {
    return driveReverse( fabs( distance_in_meters ) );
  }
}

void Rotate( double num_degrees )
{
  if ( num_degrees > 0.0 )
  {
    driveRight( num_degrees );
  }
  else
  {
    driveLeft( fabs(num_degrees) );
  }
}


bool _wall_follow_left_loop( double distance_in_meters, int wall_range )
{
  processDistanceSensor();
  if ( irsensorValue - wall_range >= 1 ) //If the wall is too far slow the left wheel
  {
    _servoSpeedLeft = CCW_MIN_SPEED - DISTANCE_TURN_GAIN + _wheel_speed_trim;
    _servoSpeedRight = CW_MIN_SPEED + _wheel_speed_trim;
  }
  else if ( wall_range - irsensorValue >= 1 ) //If the wall is too close slow down the right wheel.
  {
    _servoSpeedLeft = CCW_MIN_SPEED + _wheel_speed_trim;
    _servoSpeedRight = CW_MIN_SPEED + DISTANCE_TURN_GAIN + _wheel_speed_trim;
  }
  else //Drive straight
  {
    _servoSpeedLeft = CCW_MIN_SPEED + _wheel_speed_trim;
    _servoSpeedRight = CW_MIN_SPEED + _wheel_speed_trim;
  }

  processEncoders();
  setBlinksLeft( 2 );
  processLEDs();

  if ( distance_in_meters != 0.0 && _distanceTraveled > distance_in_meters )
  {
    return false;
  }
  return true;
}

bool _wall_follow_right_loop( double distance_in_meters, int wall_range )
{
  processDistanceSensor();
  if ( irsensorValue - wall_range >= 1 ) //If the wall is too far slow the right wheel
  {
    _servoSpeedLeft = CCW_MIN_SPEED + _wheel_speed_trim;
    _servoSpeedRight = CW_MIN_SPEED + DISTANCE_TURN_GAIN + _wheel_speed_trim;
  }
  else if ( wall_range - irsensorValue >= 1 ) //If the wall is too close slow down the left wheel.
  {
    _servoSpeedLeft = CCW_MIN_SPEED - DISTANCE_TURN_GAIN + _wheel_speed_trim;
    _servoSpeedRight = CW_MIN_SPEED + _wheel_speed_trim;
  }
  else //Drive straight
  {
    _servoSpeedLeft = CCW_MIN_SPEED + _wheel_speed_trim;
    _servoSpeedRight = CW_MIN_SPEED + _wheel_speed_trim;
  }

  processEncoders();
  setBlinksRight( 2 );
  processLEDs();

  if ( distance_in_meters != 0.0 && _distanceTraveled > distance_in_meters )
  {
    return false;
  }
  return true;
}

double wall_follow_left( double distance_in_meters, int wall_distance )
{
  updateDriveTrim();
  _servoSpeedLeft = CCW_MIN_SPEED + _wheel_speed_trim;
  _servoSpeedRight = CW_MIN_SPEED + _wheel_speed_trim;
  _driveDirection = 1;

  lookLeft();

  while ( _distanceTraveled < distance_in_meters )
  {
    _wall_follow_left_loop( distance_in_meters, wall_distance );
  }

  return driveStop();
}

double wall_follow_right( double distance_in_meters, int wall_distance )
{
  updateDriveTrim();
  _servoSpeedLeft = CCW_MIN_SPEED + _wheel_speed_trim;
  _servoSpeedRight = CW_MIN_SPEED + _wheel_speed_trim;
  _driveDirection = 1;

  lookRight();
  driveStart( _servoSpeedLeft, _servoSpeedRight );
  while ( _distanceTraveled < distance_in_meters )
  {
    _wall_follow_right_loop( distance_in_meters, wall_distance );
  }
  
  return driveStop();
}

double wall_follow_left_trigger_helper( double distance_in_meters, int wall_range, int stop_trigger_ir_range )
{
  updateDriveTrim();
  _servoSpeedLeft = CCW_MIN_SPEED + _wheel_speed_trim;
  _servoSpeedRight = CW_MIN_SPEED + _wheel_speed_trim;
  _driveDirection = 1;

  lookLeft();
  processDistanceSensor();
  driveStart( _servoSpeedLeft, _servoSpeedRight );
  
  if ( stop_trigger_ir_range > 0 )
  {
    while ( irsensorValue < stop_trigger_ir_range )
    {
      if ( !_wall_follow_left_loop( distance_in_meters, wall_range ) ) break;
    }
  }
  else
  {
    while ( irsensorValue > abs(stop_trigger_ir_range) )
    {
      if ( !_wall_follow_left_loop( distance_in_meters, wall_range ) ) break;
    }
  }

  return driveStop();
}
double wall_follow_right_trigger_helper( double distance_in_meters, int wall_range, int stop_trigger_ir_range )
{
  updateDriveTrim();
  _servoSpeedLeft = CCW_MIN_SPEED + _wheel_speed_trim;
  _servoSpeedRight = CW_MIN_SPEED + _wheel_speed_trim;
  _driveDirection = 1;

  lookRight();
  processDistanceSensor();
  driveStart( _servoSpeedLeft, _servoSpeedRight );

  if ( stop_trigger_ir_range > 0 )
  {
    while ( irsensorValue < stop_trigger_ir_range )
    {
      if ( !_wall_follow_right_loop( distance_in_meters, wall_range ) ) break;
    }
  }
  else
  {
    while ( irsensorValue > fabs(stop_trigger_ir_range) )
    {
      if ( !_wall_follow_right_loop( distance_in_meters, wall_range ) ) break;
    }
  }
  
  return driveStop();
}

#define WALL_LEFT 0
#define WALL_RIGHT 1
double WallFollow( int which_wall, double distance_in_meters, int wall_range, int stop_trigger_ir_range = 0 )
{
  if ( stop_trigger_ir_range == 0 ) //If there is no stop trigger follow the wall until distance_in_meters is reached
  {
    if ( which_wall == WALL_LEFT ) return wall_follow_left( distance_in_meters, wall_range );
    else                           return wall_follow_right( distance_in_meters, wall_range );
  }
  else //There is a stop trigger, follow wall until trigger and use distance_in_meters as a travel limit
  {
    if ( which_wall == WALL_LEFT ) return wall_follow_left_trigger_helper( distance_in_meters, wall_range, stop_trigger_ir_range );
    else                           return wall_follow_right_trigger_helper( distance_in_meters, wall_range, stop_trigger_ir_range );
  }
  return 0.0;
}

int IRread()
{
  return getCurrentDistance();
}

void Orientate()
{
  int distance_fwd, distance_left, distance_right;

  lookCustom( IR_PAN_CENTER );
  distance_fwd = IRread();
  lookCustom( IR_PAN_LEFT_45 );
  distance_left = IRread();
  lookCustom( IR_PAN_RIGHT_45 );
  distance_right = IRread();


#ifdef USB_DEBUG
  Serial.print( "Orientate IR Range L: " );
  Serial.print( distance_left );
  Serial.print( " C: " );
  Serial.print( distance_fwd );
  Serial.print( " R: " );
  Serial.println( distance_right );
#endif

  double wall_length, wall_angle;

  if ( distance_left > distance_right )
  {
#ifdef USB_DEBUG
    Serial.println( "Left is further, solving for right." );
#endif
    //https://www.mathsisfun.com/algebra/trig-solving-sas-triangles.html
    wall_length = (distance_fwd * distance_fwd) + (distance_right * distance_right) - (2 * distance_fwd * distance_right) * cos( 45 );
    wall_length = sqrt( wall_length );
    
    //https://www.mathsisfun.com/algebra/trig-solving-sss-triangles.html
    //Solving for C
    wall_angle = (wall_length * wall_length) + (distance_fwd * distance_fwd) - ( distance_right * distance_right );
    wall_angle /= 2 * wall_length * distance_fwd; 
    wall_angle *= 57.2958; //Rad to Deg

    if ( fabs( wall_angle ) > 22.0 ) Rotate( wall_angle );
  }
  else
  {
#ifdef USB_DEBUG
    Serial.println( "Right is further, solving for left." );
#endif
    //https://www.mathsisfun.com/algebra/trig-solving-sas-triangles.html
    wall_length = (distance_fwd * distance_fwd) + (distance_left * distance_left) - (2 * distance_fwd * distance_left) * cos( 45 );
    wall_length = sqrt( wall_length );
    
    //https://www.mathsisfun.com/algebra/trig-solving-sss-triangles.html
    //Solving for B
    wall_angle = (distance_fwd * distance_fwd) + (wall_length * wall_length) - ( distance_left * distance_left );
    wall_angle /= 2 * distance_fwd * wall_length; 
    wall_angle *= 57.2958; //Rad to Deg

    if ( fabs( wall_angle ) > 22.0 ) Rotate( -wall_angle );
  }

#ifdef USB_DEBUG
  Serial.print( "Wall length: " );
  Serial.print( wall_length );
  Serial.print( " Wall angle: " );
  Serial.println( wall_angle );
#endif

}

#endif //--DRIVE_CONTROL_H

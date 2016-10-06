//TODO: If not using a schmitt trigger before calibrating average the high and low values to determine
//      optimal left and right encoder thresholds. Enchances calibration. Ooh la la

#define LEFT_SERVO_PIN 10
#define RIGHT_SERVO_PIN 11
#define LEFT_ENCODER_PIN 1
#define RIGHT_ENCODER_PIN 2

#define CORRECTION_DELAY 5000
#define SATISFIED_DELAY 15000

#include <Servo.h>

#include <Wire.h>                 //Load for I2C
#include <LiquidCrystal_I2C.h>    //Load the LiquidCrystal I2C Library for the LCD Display
LiquidCrystal_I2C lcd(0x27, 16, 2);  //I2C LCD Screen

Servo servo_left, servo_right;

bool testingCW = true;

bool cal_left_cw = false;
bool cal_left_ccw = false;
bool cal_right_cw = false;
bool cal_right_ccw = false;

//Starting PWMs for Left and Right servos in both CW and CCW directions.
int CW_left = 1400;
int CW_right = 1400;
int CCW_left = 1580;
int CCW_right = 1580;

unsigned long last_integration = millis();
unsigned long last_speed_adjustment_left = millis();
unsigned long last_speed_adjustment_right = millis();

const int ENCODER_VALUE_THRESHOLD = 768; //ADC input value for high/low signal change
const int encoderCounts_per_revolution = 64; //Number of slices on wheel encoder
bool _rightEncoderRising = true;    //state of encoder counting
bool _rightEncoderFalling = false;  //state of encoder counting
bool _leftEncoderRising = true;     //state of encoder counting
bool _leftEncoderFalling = false;   //state of encoder counting
int _rightEncoderCount = 0; //number of encoder ticks per interval during command
int _leftEncoderCount = 0; //number of encoder ticks per interval during command

double desired_rate = 20.0; //Ticks per second
double rate_left = 0.0;
double rate_right = 0.0;


#include <EEPROM.h>
#define CONFIG_VERSION "RG1"
#define CONFIG_START 32
struct StoreStruct {
  // This is for mere detection if they are your settings
  char version[4];
  // The variables of your settings
  int left_cw, left_ccw, right_cw, right_ccw;
} CalibrationResults = {
  CONFIG_VERSION,
  // The default values
  1500, 1500, 1500, 1500
};
void saveCalibration()
{
  for (unsigned int t=0; t<sizeof(CalibrationResults); t++)
  { // writes to EEPROM
    EEPROM.write(CONFIG_START + t, *((char*)&CalibrationResults + t));
    // and verifies the data
    if (EEPROM.read(CONFIG_START + t) != *((char*)&CalibrationResults + t))
    {
      // error writing to EEPROM
      lcd.clear();
      lcd.print( "Error saving." );
      while(1);
    }
  }

  delay( 3000 );
  lcd.clear();
  lcd.print( "Calibration");
  lcd.setCursor(0, 1);
  lcd.print( "Completed!!" );
  while(1);
}


void setup()
{
  servo_left.attach(LEFT_SERVO_PIN);
  servo_right.attach(RIGHT_SERVO_PIN);

  Serial.begin( 38400 );

  lcd.begin();
  lcd.backlight();

}

void loop()
{
  check_encoders(); //Always update current encoder state
  
  if ( testingCW == true )
  {
    if ( !cal_left_cw ) servo_left.writeMicroseconds( CW_left );
    if ( !cal_right_cw ) servo_right.writeMicroseconds( CW_right );

    if ( last_integration + 1000 < millis() )
    {
      last_integration = millis();
      
      if ( !cal_left_cw ) rate_left = ( 0.8 * rate_left ) + ( 0.2 * (_leftEncoderCount) );
      if ( !cal_right_cw ) rate_right = ( 0.8 * rate_right ) + ( 0.2 * (_rightEncoderCount ) );
      _leftEncoderCount = 0;
      _rightEncoderCount = 0;

      lcd.clear();
      lcd.print( "L:" );
      lcd.print( rate_left );
      lcd.print( " " );
      lcd.setCursor(8, 0);
      lcd.print( "R:" );
      lcd.print( rate_right );
      lcd.setCursor(0, 1);
      lcd.print( CW_left );
      lcd.setCursor(8, 1);
      lcd.print( CW_right );

      lcd.setCursor(5, 1);
      if ( !cal_left_cw ) lcd.print( (millis() - last_speed_adjustment_left) / 1000 );

      lcd.setCursor(13, 1);
      if ( !cal_right_cw ) lcd.print( (millis() - last_speed_adjustment_right) / 1000 );
//LEFT
      if ( last_speed_adjustment_left + CORRECTION_DELAY < millis() && round(rate_left) != desired_rate && !cal_left_cw )
      {
        last_speed_adjustment_left = millis();
        if ( rate_left < desired_rate )
        {
          --CW_left;
        }
        else
        {
          ++CW_left;
        }
        servo_left.writeMicroseconds( CW_left );
      }
      if ( last_speed_adjustment_left + SATISFIED_DELAY < millis() && round(rate_left) == desired_rate )
      {
        cal_left_cw = true;
        servo_left.writeMicroseconds( 1500 ); //Stop servo
        rate_left = 0.0;
      }
//RIGHT
      if ( last_speed_adjustment_right + CORRECTION_DELAY < millis() && round(rate_right) != desired_rate && !cal_right_cw )
      {
        last_speed_adjustment_right = millis();
        if ( rate_right < desired_rate )
        {
          --CW_right;
        }
        else
        {
          ++CW_right;
        }
        servo_right.writeMicroseconds( CW_right );
      }
      if ( last_speed_adjustment_right + SATISFIED_DELAY < millis() && round(rate_right) == desired_rate )
      {
        cal_right_cw = true;
        servo_right.writeMicroseconds( 1500 ); //Stop servo
        rate_right = 0.0;
      }
//DONE
      if ( cal_left_cw == true && cal_right_cw == true )
      {
        testingCW = false;
      }
    }
  }
  else if ( cal_left_ccw == false || cal_right_ccw == false )
  {
    if ( !cal_left_ccw ) servo_left.writeMicroseconds( CCW_left );
    if ( !cal_right_ccw ) servo_right.writeMicroseconds( CCW_right );

    if ( last_integration + 1000 < millis() )
    {
      last_integration = millis();
      
      if ( !cal_left_ccw ) rate_left = ( 0.9 * rate_left ) + ( 0.1 * (_leftEncoderCount) );
      if ( !cal_right_ccw ) rate_right = ( 0.9 * rate_right ) + ( 0.1 * (_rightEncoderCount ) );
      _leftEncoderCount = 0;
      _rightEncoderCount = 0;

      lcd.clear();
      lcd.print( "L:" );
      lcd.print( rate_left );
      lcd.print( " " );
      lcd.setCursor(8, 0);
      lcd.print( "R:" );
      lcd.print( rate_right );
      lcd.setCursor(0, 1);
      lcd.print( CCW_left );
      lcd.setCursor(8, 1);
      lcd.print( CCW_right );

      lcd.setCursor(5, 1);
      if ( !cal_left_ccw ) lcd.print( (millis() - last_speed_adjustment_left) / 1000 );

      lcd.setCursor(13, 1);
      if ( !cal_right_ccw ) lcd.print( (millis() - last_speed_adjustment_right) / 1000 );
//LEFT
      if ( last_speed_adjustment_left + CORRECTION_DELAY < millis() && round(rate_left) != desired_rate && !cal_left_ccw )
      {
        last_speed_adjustment_left = millis();
        if ( rate_left > desired_rate )
        {
          --CCW_left;
        }
        else
        {
          ++CCW_left;
        }
        servo_left.writeMicroseconds( CCW_left );
      }
      if ( last_speed_adjustment_left + SATISFIED_DELAY < millis() && round(rate_left) == desired_rate )
      {
        cal_left_ccw = true;
        servo_left.writeMicroseconds( 1500 ); //Stop servo
      }
//RIGHT
      if ( last_speed_adjustment_right + CORRECTION_DELAY < millis() && round(rate_right) != desired_rate && !cal_right_ccw )
      {
        last_speed_adjustment_right = millis();
        if ( rate_right > desired_rate )
        {
          --CCW_right;
        }
        else
        {
          ++CCW_right;
        }
        servo_right.writeMicroseconds( CCW_right );
      }
      if ( last_speed_adjustment_right + SATISFIED_DELAY < millis() && round(rate_right) == desired_rate )
      {
        cal_right_ccw = true;
        servo_right.writeMicroseconds( 1500 ); //Stop servo
      }
//DONE
      if ( cal_left_ccw == true && cal_right_ccw == true )
      {
        testingCW = false;
      }
    }
  }
  else
  {
    lcd.clear();
    lcd.print( "L:" );
    lcd.print( CW_left );
    lcd.print( " " );
    lcd.setCursor(8, 0);
    lcd.print( "R:" );
    lcd.print( CW_right );
    lcd.setCursor(0, 1);
    lcd.print( CCW_left );
    lcd.setCursor(8, 1);
    lcd.print( CCW_right );

    CalibrationResults.left_cw = CW_left;
    CalibrationResults.left_ccw = CCW_left;
    CalibrationResults.right_cw = CW_right;
    CalibrationResults.right_ccw = CCW_right;

    saveCalibration();
    
    while( 1 );
  }
  
}

void check_encoders()
{
  int rightEncoderValue = analogRead(RIGHT_ENCODER_PIN); // get encoder value
  int leftEncoderValue = analogRead(LEFT_ENCODER_PIN);

  //Catch falling edge ( white stripe ) for right and left wheel encoders
  if (_rightEncoderFalling && rightEncoderValue < ENCODER_VALUE_THRESHOLD)
  {
    ++_rightEncoderCount;
    _rightEncoderRising = true;
    _rightEncoderFalling = false;
  }
  if (_leftEncoderFalling && leftEncoderValue < ENCODER_VALUE_THRESHOLD)
  {
    ++_leftEncoderCount;
    _leftEncoderRising = true;
    _leftEncoderFalling = false;
  }

  //Catch rising edge ( black stripe ) for right and left wheel encoders
  if (_rightEncoderRising && rightEncoderValue > ENCODER_VALUE_THRESHOLD) 
  {
    ++_rightEncoderCount;
    _rightEncoderRising = false;
    _rightEncoderFalling = true;
  } 
  if (_leftEncoderRising && leftEncoderValue > ENCODER_VALUE_THRESHOLD) 
  {
    ++_leftEncoderCount;
    _leftEncoderRising = false;
    _leftEncoderFalling = true;
  }
}


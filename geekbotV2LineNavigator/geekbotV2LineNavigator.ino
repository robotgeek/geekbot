/***********************************************************************************
 *                         RobotGeek Geekbot V2 Line Navigator
 *  __________________
 *   |              |
 *   |     ___      |
 *  _|____/___\ ____|_
 *   \_/  \___/   \_/
 *
 *  Wiring
 *
 *    Rotation Knob - Analog Pin 0
 *    Left Servo - Digital Pin 10
 *    Right Servo - Digital Pin 11
 *    Buzzer - Digital Pin 12
 *    IR Receiver - Digital Pin 2
 *    Right LED - Digital Pin 4
 *    Left LED - Digital Pin 7
 *    Line Sensor Array - I2C
 *    LCD (4 line) - I2C
 *    LCD Up Button - Digital Pin 3
 *    LCD Down Button - Digital Pin 9
 *    LCD Play Button - Digital Pin 1
 *
 *    Jumper for pins 9/10/11 should be set to 'VIN'
 *    Jumper for pins 3/5/6 should be set to '5V'
 *
 *  Control Behavior:
 *
 *    Put it on a line and use the LCD panel to navigate between destinations
 *
 *  External Resources
 *
 ***********************************************************************************/

template< typename T, size_t N > size_t ArraySize (T (&) [N]) { return N; }

enum NavCmds
{
  NAV_FWD = 1,
  NAV_UTURN,
  NAV_RIGHT,
  NAV_LEFT,
  NAV_STOP
} navigationCommand = NAV_STOP;

/* You must enter your list of destinations */
String destinationList[] =
{
  "Robot Lab",
  "Garage",
  "Kitchen"
};

/* You must define a route from each destination to another */
uint8_t routeLabGarage[] = {  NAV_LEFT, NAV_FWD, NAV_UTURN, NAV_STOP  };
uint8_t routeLabKitchen[] = {  NAV_LEFT, NAV_RIGHT, NAV_UTURN, NAV_STOP  };

uint8_t routeGarageLab[] = { NAV_FWD, NAV_RIGHT, NAV_UTURN, NAV_STOP };
uint8_t routeGarageKitchen[] = { NAV_LEFT, NAV_UTURN, NAV_STOP  };

uint8_t routeKitchenLab[] = { NAV_LEFT, NAV_RIGHT, NAV_UTURN, NAV_STOP };
uint8_t routeKitchenGarage[] = { NAV_RIGHT, NAV_UTURN, NAV_STOP };

/* You must define a map for each location and it's destinations */
uint8_t * navigationMap[][3] =
{
  //Location: Robot Lab. Routes in same order as destinationList
  {0, routeLabGarage, routeLabKitchen}, //First entry is 0 because there is no route from Robot Lab to Robot Lab

  //Location: Garage. Routes in same order as destinationList
  {routeGarageLab, 0, routeGarageKitchen}, //Second entry is 0 because there is no route from Garage to Garage

  //Location: Kitchen. Routes in same order as destinationList
  {routeKitchenLab, routeKitchenGarage, 0 }  //Third entry is 0 because there is no route from Kitchen to Kitchen
};

int currentNavigationLocation = -1; //Below 0 is unknown, otherwise will be index of destinationList[]
int currentNavigationDestination = -1; //Below 0 is unknown, otherwise will be index of destinationList[]
int currentNavigationIntersection = -1; //Below 0 is unknown, otherwise will be index of route in destinationList[]

/* LCD and controls */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 20, 4);
const uint8_t LCD_UP_PIN = 3;
const uint8_t LCD_DOWN_PIN = 9;
const uint8_t LCD_PLAY_PIN = 1;



#include <sensorbar.h>
// Uncomment one of the four lines to match your SX1509's address
//  pin selects. SX1509 breakout defaults to [0:0] (0x3E).
const uint8_t SX1509_ADDRESS = 0x3E;  // SX1509 I2C address (00)
//const byte SX1509_ADDRESS = 0x3F;  // SX1509 I2C address (01)
//const byte SX1509_ADDRESS = 0x70;  // SX1509 I2C address (10)
//const byte SX1509_ADDRESS = 0x71;  // SX1509 I2C address (11)
SensorBar mySensorBar(SX1509_ADDRESS);
#define IDLE_STATE 0
#define READ_LINE 1
#define GO_FORWARD 2
#define GO_LEFT 3
#define GO_RIGHT 4
#define GO_INTERSECTION 5

uint8_t lineFollowingState = IDLE_STATE; //State of line following

//Intersection constants
const int BLIND_DRIVE_TIME = 700; //milliseconds to drive forward at intersection
const int BLIND_TURN_TIME = 250; //milliseconds to turn without checking sensor (time to move off line)

//Includes
#include <Servo.h>     //include servo library to control continous turn servos
#include "Sounds.h"

//pin constants
const int TRIM_KNOB_PIN = 0;
const int BUZZER_PIN = 12;
const int LED_RIGHT_PIN = 4;
const int LED_LEFT_PIN = 7;
const int LEFT_SERVO_PIN = 10;
const int RIGHT_SERVO_PIN = 11;

//Servo control
const int SERVO_STOP = 1500; //servo pulse in microseconds for stopped servo

//Speed constants from RGS-4C No Load Test Data
const int CCW_MIN_SPEED = 1580;
const int CW_MIN_SPEED = 1400;
const int CCW_MAX_SPEED = 1750;
const int CW_MAX_SPEED = 1250;
//Experimental speed constant
const int CCW_MED_SPEED = 1660;
const int CW_MED_SPEED = 1300;
//Turning speed constants
const int SERVO_TURN_SPEED_HIGH = 50;
const int SERVO_TURN_SPEED_LOW = 25;
int SERVO_DRIVE_TURN_SPEED = 70; //For turning while driving
int SERVO_TURN_SPEED = SERVO_TURN_SPEED_LOW; //For in place turning. Applied to CW and CCW_MIN_SPEEDs

Servo servoLeft, servoRight;      //wheel servo objects

int servoSpeedLeft = SERVO_STOP;   //left servo speed.
int servoSpeedRight = SERVO_STOP;  //right servo speed.

//Wheel speeds from "gear" selection
int leftFwdSpeed = CCW_MIN_SPEED;
int leftRevSpeed = CW_MIN_SPEED;
int rightFwdSpeed = CW_MIN_SPEED;
int rightRevSpeed = CCW_MIN_SPEED;

enum SpeedSelections
{
  SPEED_MIN,
  SPEED_MED,
  SPEED_MAX
};
int currentSpeed = SPEED_MIN;

//Trim testing
int _wheel_speed_trim = 0;
void updateDriveTrim()
{
  int knob_value = analogRead( TRIM_KNOB_PIN );
  _wheel_speed_trim = map( knob_value, 0, 1023, -50, 50 );
}

void setup()
{
  //lcd buttons
  pinMode(LCD_UP_PIN, INPUT_PULLUP);
  pinMode(LCD_DOWN_PIN, INPUT_PULLUP);
  pinMode(LCD_PLAY_PIN, INPUT_PULLUP);

  // initialize the LCD
  lcd.begin();

  // Turn on the blacklight and print a message.
  lcd.backlight();
  lcd.print("Geekbot Navigator");

  //Default: the IR will only be turned on during reads.
  mySensorBar.setBarStrobe();
  //Other option: Command to run all the time
  //mySensorBar.clearBarStrobe();

  //Default: dark on light
  mySensorBar.clearInvertBits();
  //Other option: light line on dark
  //mySensorBar.setInvertBits();

  //Don't forget to call .begin() to get the bar ready.  This configures HW.
  uint8_t returnStatus = mySensorBar.begin();
  if(returnStatus)
  {
    Serial.println("sx1509 IC communication OK");
  }
  else
  {
    Serial.println("sx1509 IC communication FAILED!");
  }

  // put your setup code here, to run once:
  pinMode(LED_LEFT_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(LED_LEFT_PIN, HIGH);
  digitalWrite(LED_RIGHT_PIN, HIGH);

  Serial.begin(38400);

  servoLeft.attach(LEFT_SERVO_PIN);
  servoRight.attach(RIGHT_SERVO_PIN);
  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);

  SoundEnable();

  Serial.println("Geekbot V2 Start");
}

void intersectionForward()
{
  motorsForward();
  while ( mySensorBar.getDensity() > 5 )
  {
    delay(100);
  }
}
void intersectionRight()
{
  motorsForward();
  delay(BLIND_DRIVE_TIME); //time to move forward onto intersection
  motorsRotateRight();
  delay(BLIND_TURN_TIME); //time to get off the line
  while( true )
  {
    if( mySensorBar.getDensity() < 3 )
    {
      uint8_t lastBarRawValue = mySensorBar.getRaw();

      if ( lastBarRawValue == 0x08 || lastBarRawValue == 0x18 || lastBarRawValue == 0x10 )
      {
        break; //Stop rotating when line is detected in middle.
      }

      //Detecting wide tape
      if ( lastBarRawValue == 0x1C || lastBarRawValue == 0x38 )
      {
        break; //Stop rotating when line is detected in middle.
      }
    }
  }
}
void intersectionLeft()
{
  motorsForward();
  delay(BLIND_DRIVE_TIME); //time to move forward onto intersection
  motorsRotateLeft();
  delay(BLIND_TURN_TIME); //time to get off the line
  while( true )
  {
    if( mySensorBar.getDensity() < 3 )
    {
      uint8_t lastBarRawValue = mySensorBar.getRaw();

      if ( lastBarRawValue == 0x08 || lastBarRawValue == 0x18 || lastBarRawValue == 0x10 )
      {
        break; //Stop rotating when line is detected in middle.
      }

      //Detecting wide tape
      if ( lastBarRawValue == 0x1C || lastBarRawValue == 0x38 )
      {
        break; //Stop rotating when line is detected in middle.
      }
    }
  }
}
void intersectionUTurn()
{
  motorsRotateLeft();
  delay(BLIND_TURN_TIME); //time to get off the line
  while( true )
  {
    if( mySensorBar.getDensity() < 3 )
    {
      uint8_t lastBarRawValue = mySensorBar.getRaw();

      if ( lastBarRawValue == 0x08 || lastBarRawValue == 0x18 || lastBarRawValue == 0x10 )
      {
        break; //Stop rotating when line is detected in middle.
      }

      //Detecting wide tape
      if ( lastBarRawValue == 0x1C || lastBarRawValue == 0x38 )
      {
        break; //Stop rotating when line is detected in middle.
      }
    }
  }
  intersectionDetected(); //Treating the discovered line as an intersection so program will take next step
}

void intersectionDetected()
{
  motorsStop();
  uint8_t * testIntersectionValue = navigationMap[ currentNavigationLocation ][ currentNavigationDestination ];
  uint8_t currentIntersectionCommand = testIntersectionValue[ ++currentNavigationIntersection ];
  Serial.print( "Intersection command: " );
  Serial.println( currentIntersectionCommand );
  switch( currentIntersectionCommand )
  {
  case NAV_FWD:
    SoundPlay(WHISTLE);
    intersectionForward();
    break;
  case NAV_UTURN:
    SoundPlay(WHISTLE);
    intersectionUTurn();
    break;
  case NAV_LEFT:
    SoundPlay(WHISTLE);
    intersectionLeft();
    break;
  case NAV_RIGHT:
    SoundPlay(WHISTLE);
    intersectionRight();
    break;
  case NAV_STOP:
    SoundPlay(LAUGH);
  default:
    SoundPlay(BEEPS);
    motorsStop();
    currentNavigationLocation = currentNavigationDestination;
    currentNavigationDestination = -1;
    currentNavigationIntersection = -1;
  }
}

void motorsSetSpeed()
{
  servoLeft.writeMicroseconds(servoSpeedLeft);
  servoRight.writeMicroseconds(servoSpeedRight);
}

void motorsStop()
{
  servoSpeedLeft = SERVO_STOP;
  servoSpeedRight = SERVO_STOP;
  motorsSetSpeed();
}
void motorsForward()
{
  currentSpeed = SPEED_MIN;
  SERVO_TURN_SPEED = SERVO_TURN_SPEED_LOW;
  leftFwdSpeed = CCW_MIN_SPEED + 20;
  leftRevSpeed = CW_MIN_SPEED - 20;
  rightFwdSpeed = CW_MIN_SPEED - 20;
  rightRevSpeed = CCW_MIN_SPEED + 20;

  updateDriveTrim();
  servoSpeedLeft = leftFwdSpeed + _wheel_speed_trim;
  servoSpeedRight = rightFwdSpeed + _wheel_speed_trim;

  motorsSetSpeed();
}
void motorsTurnLeft()
{
  motorsForward();
  servoSpeedLeft -= SERVO_DRIVE_TURN_SPEED;
  motorsSetSpeed();
}
void motorsTurnRight()
{
  motorsForward();
  servoSpeedRight += SERVO_DRIVE_TURN_SPEED;
  motorsSetSpeed();
}
void motorsRotateLeft()
{
  servoSpeedLeft = CW_MIN_SPEED - SERVO_TURN_SPEED;
  servoSpeedRight = CW_MIN_SPEED - SERVO_TURN_SPEED;
  motorsSetSpeed();
}
void motorsRotateRight()
{
  servoSpeedLeft = CCW_MIN_SPEED + SERVO_TURN_SPEED;
  servoSpeedRight = CCW_MIN_SPEED + SERVO_TURN_SPEED;
  motorsSetSpeed();
}


uint8_t lcdCurrentSelection = 0;

void lcdSelectLocation()
{
  uint8_t currentPrintItem = lcdCurrentSelection;

  lcd.setCursor(0, 1);
  lcd.print('>');

  for ( int i = 0; i < 3; ++i )
  {
    lcd.print( destinationList[currentPrintItem] );
    lcd.setCursor(0, 2 + i);
    if ( ++currentPrintItem == ArraySize( destinationList ) )
    {
      currentPrintItem = 0;
    }
  }
}

void lcdSelectDestination()
{
  uint8_t currentPrintItem = lcdCurrentSelection;

  lcd.setCursor(0, 1);
  lcd.print('>');

  for ( int i = 0; i < 3; ++i )
  {
    lcd.print( destinationList[currentPrintItem] );
    lcd.setCursor(0, 2 + i);
    if ( ++currentPrintItem == ArraySize( destinationList ) )
    {
      currentPrintItem = 0;
    }
  }
}

void loop()
{
  if ( currentNavigationLocation < 0 )
  {
    lcd.clear();
    lcd.print( "Select Location:" );
    lcdSelectLocation();

    while ( digitalRead(LCD_UP_PIN) == HIGH || digitalRead(LCD_DOWN_PIN) == HIGH || digitalRead(LCD_PLAY_PIN) == HIGH )
    {
      if ( digitalRead(LCD_UP_PIN) == LOW )
      {
        SoundPlay(UP);
        if ( --lcdCurrentSelection < 0 ) lcdCurrentSelection = ArraySize( destinationList ) - 1;
        delay(250); //debounce time
        break;
      }
      if ( digitalRead(LCD_DOWN_PIN) == LOW )
      {
        SoundPlay(DOWN);
        if ( ++lcdCurrentSelection == ArraySize( destinationList ) ) lcdCurrentSelection = 0;
        delay(250); //debounce time
        break;
      }
      if ( digitalRead(LCD_PLAY_PIN) == LOW )
      {
        SoundPlay(UHOH);
        currentNavigationLocation = lcdCurrentSelection;
        Serial.print( "Current Location: " );
        Serial.print( currentNavigationLocation );
        Serial.print( ": " );
        Serial.println( destinationList[currentNavigationLocation] );
        delay(250); //debounce time
        break;
      }
    }
    return;
  }
  //Only process line following if navigation destination is known
  if ( currentNavigationDestination < 0 )
  {
    lcd.clear();
    lcd.print( "Select Destination:" );
    lcdSelectDestination();

    while ( digitalRead(LCD_UP_PIN) == HIGH || digitalRead(LCD_DOWN_PIN) == HIGH || digitalRead(LCD_PLAY_PIN) == HIGH )
    {
      if ( digitalRead(LCD_UP_PIN) == LOW )
      {
        SoundPlay(UP);
        if ( --lcdCurrentSelection < 0 ) lcdCurrentSelection = ArraySize( destinationList ) - 1;
        delay(250); //debounce time
        break;
      }
      if ( digitalRead(LCD_DOWN_PIN) == LOW )
      {
        SoundPlay(DOWN);
        if ( ++lcdCurrentSelection == ArraySize( destinationList ) ) lcdCurrentSelection = 0;
        delay(250); //debounce time
        break;
      }
      if ( digitalRead(LCD_PLAY_PIN) == LOW )
      {
        SoundPlay(UHOH);
        currentNavigationDestination = lcdCurrentSelection;
        delay(250); //debounce time

        Serial.print( "Current Destination: " );
        Serial.print( currentNavigationDestination );
        Serial.print( ": " );
        Serial.println( destinationList[currentNavigationDestination] );

        lcd.clear();
        lcd.print( "Hold on to your butt" );
        lcd.setCursor(0, 1); lcd.print( destinationList[currentNavigationLocation] );
        lcd.setCursor(0, 2); lcd.print( "to" );
        lcd.setCursor(0, 3); lcd.print( destinationList[currentNavigationDestination] );
        delay(1000);
        break;
      }
    }
    return;
  }

  uint8_t nextState = lineFollowingState;
  switch (lineFollowingState)
  {
  case IDLE_STATE:
    motorsStop();       // Stops both motors
    nextState = READ_LINE;
    break;
  case GO_INTERSECTION:
    intersectionDetected(); //This function will handle all drive and sensor commands until intersection is complete
    nextState = IDLE_STATE;
    break;
  case READ_LINE:
    if( mySensorBar.getDensity() < 7 )
    {
      nextState = GO_FORWARD;
      if( mySensorBar.getPosition() < -50 )
      {
        nextState = GO_LEFT;
      }
      if( mySensorBar.getPosition() > 50 )
      {
        nextState = GO_RIGHT;
      }
    }
    else //all 8 on means we found an intersection
    {
      nextState = GO_INTERSECTION;
    }
    break;
  case GO_FORWARD:
    motorsForward();
    nextState = READ_LINE;
    break;
  case GO_LEFT:
    motorsTurnLeft();
    nextState = READ_LINE;
    break;
  case GO_RIGHT:
    motorsTurnRight();
    nextState = READ_LINE;
    break;
  default:
    motorsStop();       // Stops both motors
    while(1)
    {
      SoundPlay(WHISTLE);
      delay(1000);
    }
    break;
  }
  lineFollowingState = nextState;
}


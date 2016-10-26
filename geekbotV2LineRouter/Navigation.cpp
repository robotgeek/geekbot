#include "Navigation.h"
#include "myMap.h"

/* LCD and buttons */
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x3F, 20, 4);
const uint8_t LCD_UP_PIN = 3;
const uint8_t LCD_DOWN_PIN = 9;
const uint8_t LCD_PLAY_PIN = 1;
const uint8_t LCD_STOP_PIN = 8;

#ifdef AUTO_RETURN_HOME
unsigned long navigationReturnHomeTimeout = 60; //seconds until timeout so robot will return to home location
int currentNavigationHome = -1;
#endif

int currentNavigationLocation = -1; //Below 0 is unknown, otherwise will be index of destinationList[]
int currentNavigationDestination = -1; //Below 0 is unknown, otherwise will be index of destinationList[]
int currentNavigationIntersection = -1; //Below 0 is unknown, otherwise will be index of route in destinationList[]

//Intersection constants
const int BLIND_DRIVE_TIME = 700; //milliseconds to drive forward at intersection
const int BLIND_TURN_TIME = 250; //milliseconds to turn without checking sensor (time to move off line)

int navigationCommand = NAV_STOP;

PiezoEffects * navigationSounds;

const int commandListSize = 5;
String commandList[commandListSize] =
{
  "Go Straight",
  "Turn Left",
  "Turn Right",
  "End Route",
  "End Route & Rotate"
};

/* I2C LCD Control */
int lcdCurrentSelection = 0;
void lcdSelectCommand()
{
  uint8_t currentPrintItem = lcdCurrentSelection;

  lcd.setCursor(0, 2);
  lcd.print('>');

  for ( int i = 0; i < 2; ++i )
  {
    lcd.print( commandList[currentPrintItem] );
    lcd.setCursor(0, 3 + i);
    if ( ++currentPrintItem == commandListSize )
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

  for ( int i = 0; i < 2; ++i )
  {
    lcd.print( destinationList[currentPrintItem] );
    lcd.setCursor(0, 2 + i);
    if ( ++currentPrintItem == MAP_LOCATION_COUNT )
    {
      currentPrintItem = 0;
    }
  }
}

void navigationInit(PiezoEffects * mySounds)
{
  navigationSounds = mySounds;

  //lcd buttons
  pinMode(LCD_UP_PIN, INPUT_PULLUP);
  pinMode(LCD_DOWN_PIN, INPUT_PULLUP);
  pinMode(LCD_PLAY_PIN, INPUT_PULLUP);
  pinMode(LCD_STOP_PIN, INPUT_PULLUP);

  // initialize the LCD
  lcd.begin();
  lcd.backlight();
  lcd.print("Geekbot Navigator");
}

/* Check for stop button. Returns true if pressed */
bool navigationCheckEStop()
{
  if ( digitalRead(LCD_STOP_PIN) == LOW )
  {
    return true;
  }
  return false;
}

bool isMapReady = false;
int currentIntersection = 0;
bool navigationCheckMap()
{
  while ( !isMapReady )
  {
    lcd.clear();
    lcd.print( "Let's build a route!" );
    lcd.setCursor(0, 1);
    lcd.print( "Intersection " );
    lcd.print( currentIntersection + 1 );
    lcd.print( "?" );
    lcdSelectCommand();

    while ( digitalRead(LCD_UP_PIN) == HIGH ||
            digitalRead(LCD_DOWN_PIN) == HIGH ||
            digitalRead(LCD_PLAY_PIN) == HIGH ||
            digitalRead(LCD_STOP_PIN) == HIGH
          )
    {
      if ( digitalRead(LCD_UP_PIN) == LOW )
      {
        navigationSounds->play( soundOneBeep );
        if ( --lcdCurrentSelection < 0 ) lcdCurrentSelection = commandListSize - 1;
        delay(250); //debounce time
        break;
      }
      if ( digitalRead(LCD_DOWN_PIN) == LOW )
      {
        navigationSounds->play( soundOneBeep );
        if ( ++lcdCurrentSelection == commandListSize ) lcdCurrentSelection = 0;
        delay(250); //debounce time
        break;
      }
      if ( digitalRead(LCD_PLAY_PIN) == LOW )
      {
        navigationSounds->play( soundOneBeep );

        //Check if we are selecting "End Route" or "End Route & Rotate"
        if ( lcdCurrentSelection > 2 )
        {
          routeHomeGoal[ currentIntersection ] = lcdCurrentSelection; //Save selection
          routeGoalHome[ currentIntersection ] = lcdCurrentSelection; //Store route finish

          //Reverse routes from HomeGoal to GoalHome
          for ( int i = 0, j = currentIntersection - 1; i < currentIntersection; ++i, --j )
          {
            switch( routeHomeGoal[i] )
            {
            case NAV_LEFT:
              routeGoalHome[j] = NAV_RIGHT;
              break;
            case NAV_RIGHT:
              routeGoalHome[j] = NAV_LEFT;
              break;
            case NAV_FWD:
              routeGoalHome[j] = routeHomeGoal[i];
              break;
            }
          }

          currentNavigationLocation = 0; //Set our current location as "Home"
          currentNavigationHome = 0; //Save navigation's optional homing point as "Home"
          lcdCurrentSelection = 1; //Set LCD selection to "Goal"
          isMapReady = true;
        }
        else //Storing intersection.
        {
          //TODO: This does not account reaching intersection limit
          routeHomeGoal[ currentIntersection ] = lcdCurrentSelection; //Save selection
          ++currentIntersection; //Increment to next intersection
        }

        delay(250); //debounce time
        break;
      }
      if ( navigationCheckEStop() && currentIntersection != 0 )
      {
        navigationSounds->play( soundButtonPushed );
        --currentIntersection; //Decrement to previous intersection
        break;
      }
    }
    return false;
  }
  return true;
}

bool navigationCheckDestination()
{
  //Only process line following if navigation destination is known
  if ( currentNavigationDestination < 0 )
  {
    lcd.clear();
    lcd.print( "Select destination:" );
    lcdSelectDestination();

#ifdef AUTO_RETURN_HOME
    unsigned long navigationDestinationPromptTime = millis();
#endif
    while ( digitalRead(LCD_UP_PIN) == HIGH ||
            digitalRead(LCD_DOWN_PIN) == HIGH ||
            digitalRead(LCD_PLAY_PIN) == HIGH ||
            digitalRead(LCD_STOP_PIN) == HIGH
          )
    {
#ifdef AUTO_RETURN_HOME
      if ( currentNavigationLocation != currentNavigationHome && navigationDestinationPromptTime + navigationReturnHomeTimeout * 1000ul < millis() )
      {
        //Timeout has occured.. we are going to return home
        navigationSounds->play( soundUhoh );
        currentNavigationDestination = currentNavigationHome;
        lcd.clear();
        lcd.print( "Returning home!" );
        lcd.setCursor(0, 1);
        lcd.print( navigationReturnHomeTimeout );
        lcd.print( " second timeout." );
        lcd.setCursor(0, 2); lcd.print( "Next stop:" );
        lcd.setCursor(0, 3); lcd.print( destinationList[currentNavigationHome] );
        break;
      }
#endif
      if ( digitalRead(LCD_UP_PIN) == LOW )
      {
        navigationSounds->play( soundUp );
        if ( --lcdCurrentSelection < 0 ) lcdCurrentSelection = MAP_LOCATION_COUNT - 1;
        break;
      }
      if ( digitalRead(LCD_DOWN_PIN) == LOW )
      {
        navigationSounds->play( soundDown );
        if ( ++lcdCurrentSelection == MAP_LOCATION_COUNT ) lcdCurrentSelection = 0;
        break;
      }
      if ( digitalRead(LCD_PLAY_PIN) == LOW )
      {
        if ( lcdCurrentSelection != currentNavigationLocation )
        {
          navigationSounds->play( soundUhoh );
          currentNavigationDestination = lcdCurrentSelection;

          lcd.clear();
          lcd.print( "Traveling from:" );
          lcd.setCursor(0, 1); lcd.print( destinationList[currentNavigationLocation] );
          lcd.setCursor(0, 2); lcd.print( "to:" );
          lcd.setCursor(0, 3); lcd.print( destinationList[currentNavigationDestination] );
          delay(1000);
          break;
        }
        else
        {
          navigationSounds->play( soundOhh );
        }
      }
      if ( navigationCheckEStop() )
      {
        navigationSounds->play( soundButtonPushed );
        isMapReady = false;
        return false;
      }
    }
    return false;
  }
  return true;
}

void navigationCancel()
{
  motorsStop(); //Make sure motors are stopped
  isMapReady = false; //Returning user to route building/finishing
  currentNavigationLocation = -1; //Reset starting location
  currentNavigationDestination = -1; //Reset travel destination
  currentNavigationIntersection = -1; //Reset navigation intersection counter
  navigationSounds->play( soundSad );
}

void intersectionForward()
{
  motorsForward();
  while ( mySensorBar.getDensity() > 5 )
  {
    if ( navigationCheckEStop() )
    {
      navigationCancel();
      return;
    }
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
    if ( navigationCheckEStop() )
    {
      navigationCancel();
      return;
    }
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
    if ( navigationCheckEStop() )
    {
      navigationCancel();
      return;
    }
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
    if ( navigationCheckEStop() )
    {
      navigationCancel();
      return;
    }
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

void intersectionDetected()
{
  motorsStop();
  uint8_t * testIntersectionValue = navigationMap[ currentNavigationLocation ][ currentNavigationDestination ];
  uint8_t currentIntersectionCommand = testIntersectionValue[ ++currentNavigationIntersection ];
  //Serial.print( "Intersection command: " );
  //Serial.println( currentIntersectionCommand );
  switch( currentIntersectionCommand )
  {
  case NAV_FWD:
    navigationSounds->play( soundWhistle );
    intersectionForward();
    break;
  case NAV_LEFT:
    navigationSounds->play( soundWhistle );
    intersectionLeft();
    break;
  case NAV_RIGHT:
    navigationSounds->play( soundWhistle );
    intersectionRight();
    break;
  case NAV_STOP:
    motorsStop();
    navigationSounds->play( soundBeeps );
    currentNavigationLocation = currentNavigationDestination;
    currentNavigationDestination = -1;
    currentNavigationIntersection = -1;
    break;
  case NAV_STOP_ROTATE:
    navigationSounds->play( soundWhistle );
    intersectionUTurn();
    motorsStop();
    navigationSounds->play( soundBeeps );
    currentNavigationLocation = currentNavigationDestination;
    currentNavigationDestination = -1;
    currentNavigationIntersection = -1;
    break;
  case NAV_IDLE:
    break;
  }
}

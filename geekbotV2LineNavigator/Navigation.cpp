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

/* I2C LCD Control */
int lcdCurrentSelection = 0;
void lcdSelectLocation()
{
  uint8_t currentPrintItem = lcdCurrentSelection;

  lcd.setCursor(0, 1);
  lcd.print('>');

  for ( int i = 0; i < 3; ++i )
  {
    lcd.print( destinationList[currentPrintItem] );
    lcd.setCursor(0, 2 + i);
    if ( ++currentPrintItem == MAP_LOCATION_COUNT )
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

bool navigationCheckLocation()
{
  while ( currentNavigationLocation < 0 )
  {
    lcd.clear();
    lcd.print( "Where am I starting?" );
    lcdSelectLocation();

    while ( digitalRead(LCD_UP_PIN) == HIGH || digitalRead(LCD_DOWN_PIN) == HIGH || digitalRead(LCD_PLAY_PIN) == HIGH )
    {
      if ( digitalRead(LCD_UP_PIN) == LOW )
      {
        navigationSounds->play( soundUp );
        if ( --lcdCurrentSelection < 0 ) lcdCurrentSelection = MAP_LOCATION_COUNT - 1;
        delay(250); //debounce time
        break;
      }
      if ( digitalRead(LCD_DOWN_PIN) == LOW )
      {
        navigationSounds->play( soundDown );
        if ( ++lcdCurrentSelection == MAP_LOCATION_COUNT ) lcdCurrentSelection = 0;
        delay(250); //debounce time
        break;
      }
      if ( digitalRead(LCD_PLAY_PIN) == LOW )
      {
        navigationSounds->play( soundUhoh );
        currentNavigationLocation = lcdCurrentSelection;
        currentNavigationHome = lcdCurrentSelection;

        /*
        Serial.print( "Current Location: " );
        Serial.print( currentNavigationLocation );
        Serial.print( ": " );
        Serial.println( destinationList[currentNavigationLocation] );
        */

        delay(250); //debounce time
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
    lcd.print( "Where am I going?" );
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

          /*
          Serial.print( "Current Destination: " );
          Serial.print( currentNavigationDestination );
          Serial.print( ": " );
          Serial.println( destinationList[currentNavigationDestination] );
          */

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
        currentNavigationLocation = -1; //Returning to previous question
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
  currentNavigationLocation = -1; //Reset starting location
  currentNavigationDestination = -1; //Reset travel destination
  currentNavigationIntersection = 0; //Reset navigation intersection counter
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
  intersectionDetected(); //Treating the discovered line as an intersection so program will take next step
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
  case NAV_UTURN:
    navigationSounds->play( soundWhistle );
    intersectionUTurn();
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
    navigationSounds->play( soundLaugh );
  //no break, default desired
  default:
    motorsStop();
    navigationSounds->play( soundBeeps );
    currentNavigationLocation = currentNavigationDestination;
    currentNavigationDestination = -1;
    currentNavigationIntersection = -1;
  }
}
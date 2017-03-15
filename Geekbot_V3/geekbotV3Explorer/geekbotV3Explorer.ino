/***********************************************************************************
 *                         RobotGeek Geekbot V2 Explorer
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
 *    LCD Stop Button - Digital Pin 8
 *
 *    Jumper for pins 9/10/11 should be set to 'VIN'
 *    Jumper for pins 3/5/6 should be set to '5V'
 *
 *  Control Behavior:
 *
 *    Operate robot using myGeekBot object.
 *
 *  External Resources
 *
 ***********************************************************************************/

#include <PiezoEffects.h>

/* GeekBot API */
#include "GeekBot.h"
/* Robot Config Parameters */
#include "Config.h"
#include "LineFollower.h"
#include "Navigation.h"

GeekBot myGeekBot; // Create GeekBot object, named myGeekBot
int playButtonState = HIGH;

void setup()
{
  Serial.begin(38400);
  Serial.println("Geekbot Explorer Starting...");
  pinMode(LCD_PLAY_PIN, INPUT);
  myGeekBot.init();
  delay(1000);
  myGeekBot.sound(soundUp);
}

void loop()
{
//  myGeekBot.timedRotate(1000, 50); // time in mS, rotation speed CCW -100 to CW +100 (Clockwise Turn)
//  myGeekBot.timedRotate(1000, -50); // time in mS, rotation speed CCW -100 to CW +100 (Counter Clockwise Turn)
//  myGeekBot.timedDrive(2000, 50); //time in mS, travel speed -100 to 100 (Drive Forward)
//  myGeekBot.timedDrive(2000, -50); //time in mS, travel speed -100 to 100 (Drive Backward)
  playButtonState = digitalRead(LCD_PLAY_PIN);
  if (playButtonState == LOW)
  {
  customFunction();
  }
}

void customFunction()
{
  myGeekBot.timedRotate(2000, 50); // time in mS, rotation speed CCW -100 to CW +100
  delay(2000); //pause delay in mS
  myGeekBot.timedDrive(3000, 50); //time in mS, travel speed -100 to 100
  delay(3000); //pause delay in mS
  myGeekBot.recoverLine(1); //Follow Line until Intersection, state = NAV_CMD for next action.
  myGeekBot.sound(soundWhistle);
  myGeekBot.lineFollow(NAV_LEFT); //Follow Line until Intersection, state = NAV_CMD for next action.
  myGeekBot.lineFollow(NAV_FWD); //Follow Line until Intersection, state = NAV_CMD for next action.
  myGeekBot.lineFollow(NAV_RIGHT); //Follow Line until Intersection, state = NAV_CMD for next action.
  myGeekBot.lineFollow(NAV_UTURN); //Follow Line until Intersection, state = NAV_CMD for next action.
  myGeekBot.lineFollow(NAV_STOP); //Follow Line until Intersection, state = NAV_CMD for next action.
}

/*
   NavCmds
   NAV_FWD,
   NAV_UTURN,
   NAV_RIGHT,
   NAV_LEFT,
   NAV_STOP
 */

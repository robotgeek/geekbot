#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <Arduino.h>
#include <PiezoEffects.h>
#include "Motors.h"
#include "LineSensorArray.h"


//extern const uint8_t LCD_STOP_PIN;
extern int currentNavigationHome;
extern int currentNavigationLocation;
extern int currentNavigationDestination;

enum NavCmds
{
	NAV_FWD = 1,
	NAV_UTURN,
	NAV_RIGHT,
	NAV_LEFT,
  BLIND_FWD,
  BLIND_REV,
  BLIND_CW,
  BLIND_CCW,
	NAV_STOP
};

void navigationInit(PiezoEffects * mySounds);
bool navigationCheckLocation();
bool navigationCheckDestination();
void navigationReturnHome();
void navigationCancel();
void intersectionForward();
void intersectionRight();
void intersectionLeft();
void intersectionUTurn();
void intersectionDetected();
void intersectionDetected(int userIntersectionCommand);

#endif

#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <Arduino.h>
#include <PiezoEffects.h>
#include "Motors.h"
#include "LineSensorArray.h"

/* Comment this define if you would like to disable the return home feature */
#define AUTO_RETURN_HOME

enum NavCmds
{
	NAV_FWD = 1,
	NAV_UTURN,
	NAV_RIGHT,
	NAV_LEFT,
	NAV_STOP
};

void navigationInit(PiezoEffects * mySounds);
bool navigationCheckLocation();
bool navigationCheckDestination();
void intersectionForward();
void intersectionRight();
void intersectionLeft();
void intersectionUTurn();
void intersectionDetected();

#endif
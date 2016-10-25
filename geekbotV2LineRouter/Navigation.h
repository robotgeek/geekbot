#ifndef NAVIGATION_H
#define NAVIGATION_H

#include <Arduino.h>
#include <PiezoEffects.h>
#include "Motors.h"
#include "LineSensorArray.h"

/* Comment this define if you would like to disable the return home feature */
#define AUTO_RETURN_HOME

extern const uint8_t LCD_STOP_PIN;
enum NavCmds
{
	NAV_FWD,
	NAV_LEFT,
	NAV_RIGHT,
	NAV_STOP,
	NAV_STOP_ROTATE,
	NAV_IDLE
};

void navigationInit(PiezoEffects * mySounds);
bool navigationCheckMap();
bool navigationCheckDestination();
void navigationCancel();
void intersectionForward();
void intersectionRight();
void intersectionLeft();
void intersectionUTurn();
void intersectionDetected();

#endif
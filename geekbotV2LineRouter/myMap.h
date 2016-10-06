#ifndef MYMAP_H
#define MYMAP_H

#include <Arduino.h>

#define MAP_LOCATION_COUNT 2

extern String destinationList[];
extern uint8_t * navigationMap[][MAP_LOCATION_COUNT];

extern uint8_t routeHomeGoal[16];
extern uint8_t routeGoalHome[16];

#endif

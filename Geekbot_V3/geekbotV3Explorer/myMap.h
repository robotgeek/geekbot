#ifndef MYMAP_H
#define MYMAP_H

#include <Arduino.h>

#define MAP_LOCATION_COUNT 4

extern String destinationList[];
extern uint8_t * navigationMap[][MAP_LOCATION_COUNT];

#endif

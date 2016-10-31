#include "myMap.h"
#include "Navigation.h"

/* Don't forget to specify MAP_LOCATION_COUNT in myMap.h */

/* Enter your list of destinations */
String destinationList[MAP_LOCATION_COUNT] =
{
	"Home",
	"Goal"
};

/* Define a route from each destination to another */
uint8_t routeHomeGoal[16] = { NAV_IDLE };
uint8_t routeGoalHome[16] = { NAV_IDLE };

/* Use this route when the location and destination match in the navigationMap below */
uint8_t routeNoRoute[] = { NAV_IDLE };

/* Define your navigationMap for each location and it's destinations */
uint8_t * navigationMap[][MAP_LOCATION_COUNT] =
{
	/* Enter routes for each location in same order as destinationList above */
	{routeNoRoute, routeHomeGoal}, //Location: Home
	{routeGoalHome, routeNoRoute}, //Location: Goal
};

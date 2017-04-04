#include "myMap.h"
#include "Navigation.h"

/* Don't forget to specify MAP_LOCATION_COUNT in myMap.h */

/* Enter your list of destinations */
String destinationList[MAP_LOCATION_COUNT] =
{
	"Rick",
	"Kevin",
	"Wade",
	"Matt"
};

/* Define a route from each destination to another */
uint8_t routeRickKevin[] = {  NAV_FWD, NAV_UTURN, NAV_STOP  };
uint8_t routeRickWade[] = {  NAV_RIGHT, NAV_LEFT, NAV_UTURN, NAV_STOP  };
uint8_t routeRickMatt[] = {  NAV_RIGHT, NAV_FWD, NAV_UTURN, NAV_STOP  };

uint8_t routeKevinRick[] = { NAV_FWD, NAV_UTURN, NAV_STOP };
uint8_t routeKevinWade[] = { NAV_LEFT, NAV_LEFT, NAV_UTURN, NAV_STOP  };
uint8_t routeKevinMatt[] = { NAV_LEFT, NAV_FWD, NAV_UTURN, NAV_STOP  };

uint8_t routeWadeRick[] = { NAV_RIGHT, NAV_LEFT, NAV_UTURN, NAV_STOP };
uint8_t routeWadeKevin[] = { NAV_RIGHT, NAV_RIGHT, NAV_UTURN, NAV_STOP };
uint8_t routeWadeMatt[] = { NAV_LEFT, NAV_UTURN, NAV_STOP };

uint8_t routeMattRick[] = { NAV_FWD, NAV_LEFT, NAV_UTURN, NAV_STOP };
uint8_t routeMattKevin[] = { NAV_FWD, NAV_RIGHT, NAV_UTURN, NAV_STOP };
uint8_t routeMattWade[] = { NAV_RIGHT, NAV_UTURN, NAV_STOP };

/* Use this route when the location and destination match in the navigationMap below */
const uint8_t routeNoRoute[] = { NAV_STOP };

/* Define your navigationMap for each location and it's destinations */
uint8_t * navigationMap[][MAP_LOCATION_COUNT] =
{
	/* Enter routes for each location in same order as destinationList above */
	{routeNoRoute,   routeRickKevin, routeRickWade,  routeRickMatt}, //Location: Rick's Office
	{routeKevinRick, routeNoRoute,   routeKevinWade, routeKevinMatt}, //Location: Kevin's Desk
	{routeWadeRick,  routeWadeKevin, routeNoRoute,   routeWadeMatt}, //Location: Wade's Desk
	{routeMattRick,  routeMattKevin, routeMattWade,  routeNoRoute} //Location: Matt's Office
};

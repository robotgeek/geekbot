#include "myMap.h"
#include "Navigation.h"

/* Don't forget to specify MAP_LOCATION_COUNT in myMap.h */

/* Enter your list of destinations */
String destinationList[MAP_LOCATION_COUNT] =
{
	"Robot Lab",
	"Garage",
	"Kitchen",
	"Dogs Bed"
};

/* Define a route from each destination to another */
uint8_t routeLabGarage[] = {  NAV_LEFT, NAV_FWD, NAV_UTURN, NAV_STOP  };
uint8_t routeLabKitchen[] = {  NAV_LEFT, NAV_RIGHT, NAV_FWD, NAV_UTURN, NAV_STOP  };
uint8_t routeLabDogs[] = {  NAV_LEFT, NAV_RIGHT, NAV_LEFT, NAV_UTURN, NAV_STOP  };

uint8_t routeGarageLab[] = { NAV_FWD, NAV_RIGHT, NAV_UTURN, NAV_STOP };
uint8_t routeGarageKitchen[] = { NAV_LEFT, NAV_FWD, NAV_UTURN, NAV_STOP  };
uint8_t routeGarageDogs[] = { NAV_LEFT, NAV_LEFT, NAV_UTURN, NAV_STOP  };

uint8_t routeKitchenLab[] = { NAV_FWD, NAV_LEFT, NAV_RIGHT, NAV_UTURN, NAV_STOP };
uint8_t routeKitchenGarage[] = { NAV_FWD, NAV_RIGHT, NAV_UTURN, NAV_STOP };
uint8_t routeKitchenDogs[] = { NAV_RIGHT, NAV_UTURN, NAV_STOP };

uint8_t routeDogsLab[] = { NAV_RIGHT, NAV_LEFT, NAV_RIGHT, NAV_UTURN, NAV_STOP };
uint8_t routeDogsGarage[] = { NAV_RIGHT, NAV_RIGHT, NAV_UTURN, NAV_STOP };
uint8_t routeDogsKitchen[] = { NAV_LEFT, NAV_UTURN, NAV_STOP };

/* Use this route when the location and destination match in the navigationMap below */
const uint8_t routeNoRoute[] = { NAV_STOP };

/* Define your navigationMap for each location and it's destinations */
uint8_t * navigationMap[][MAP_LOCATION_COUNT] =
{
	/* Enter routes for each location in same order as destinationList above */
	{routeNoRoute, routeLabGarage, routeLabKitchen, routeLabDogs}, //Location: Robot Lab
	{routeGarageLab, routeNoRoute, routeGarageKitchen, routeGarageDogs}, //Location: Garage
	{routeKitchenLab, routeKitchenGarage, routeNoRoute, routeKitchenDogs}, //Location: Kitchen
	{routeDogsLab, routeDogsGarage, routeDogsKitchen, routeNoRoute} //Location: Dogs Bed
};

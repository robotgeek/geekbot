#include "LineFollower.h"
#include "Motors.h"
#include "Navigation.h" //For calling intersectionDetected()

/* Line Following States */
#define IDLE_STATE 0
#define READ_LINE 1
#define GO_FORWARD 2
#define GO_LEFT 3
#define GO_RIGHT 4
#define GO_INTERSECTION 5

uint8_t lineFollowingState = IDLE_STATE; //State of line following

PiezoEffects * lineFollowerSounds;

void lineFollowerInit( PiezoEffects * mySounds )
{
	lineFollowerSounds = mySounds;
}

void lineFollowerUpdate()
{
	uint8_t nextState = lineFollowingState;
	switch (lineFollowingState)
	{
	case IDLE_STATE:
		motorsStop();       // Stops both motors
		nextState = READ_LINE;
		break;
	case GO_INTERSECTION:
		intersectionDetected(); //This function will handle all drive and sensor commands until intersection is complete
		nextState = IDLE_STATE;
		break;
	case READ_LINE:
		if( mySensorBar.getDensity() < 7 )
		{
#ifdef USE_PID
			Input = mySensorBar.getPosition();
			turningPID.Compute();
#endif
			nextState = GO_FORWARD;
			if( mySensorBar.getPosition() < -50 )
			{
				nextState = GO_LEFT;
			}
			if( mySensorBar.getPosition() > 50 )
			{
				nextState = GO_RIGHT;
			}
		}
		else //all 8 on means we found an intersection
		{
			nextState = GO_INTERSECTION;
		}
		break;
	case GO_FORWARD:
		motorsForward();
		nextState = READ_LINE;
		break;
	case GO_LEFT:
		motorsTurnLeft();
		nextState = READ_LINE;
		break;
	case GO_RIGHT:
		motorsTurnRight();
		nextState = READ_LINE;
		break;
	default:
		motorsStop();       // Stops both motors
		while(1)
		{
			lineFollowerSounds->play( soundWhistle );
			delay(1000);
		}
		break;
	}
	lineFollowingState = nextState;
}
#ifndef LINEFOLLOWER_H
#define LINEFOLLOWER_H

#include <PiezoEffects.h>

/* Line Following States */
#define IDLE_STATE 0
#define READ_LINE 1
#define GO_FORWARD 2
#define GO_LEFT 3
#define GO_RIGHT 4
#define GO_INTERSECTION 5

void lineFollowerInit( PiezoEffects * mySounds );
bool lineFollowerUpdate();
bool lineFollowCommand();
void recoverLineLeft();
void recoverLineRight();

#endif

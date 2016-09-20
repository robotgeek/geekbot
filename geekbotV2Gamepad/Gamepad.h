#ifndef RGGAMEPAD_H
#define RGGAMEPAD_H

#include <IRGamepad.h>

#define GAMEPAD_INPUT_PIN 2
bool useModeB = false; //Set to true if your controller is switched to mode B

IR_Gamepad my_gamepad(GAMEPAD_INPUT_PIN, useModeB);

void GamepadEnable()
{
	my_gamepad.enable();
}
#endif

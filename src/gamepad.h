#pragma once

#include <vector>
#include <windows.h>

#define EXTRACTBIT(var, index) ((var >> index) & 1)

class Gamepad {
public:
	virtual int Poll();
	virtual int IsPressed(char *btnName);
	virtual float GetLX();
	virtual float GetLY();
	virtual float GetRX();
	virtual float GetRY();
	virtual float GetLT();
	virtual float GetRT();
};

class GamepadGroup {
public:
	Gamepad *activeGamepad = nullptr;
	unsigned int activeGamepadID = -1;
public:
	virtual Gamepad* GetActiveGamepad();
	virtual int CheckDevices();
	virtual int Poll();
};
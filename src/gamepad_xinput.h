#pragma once

#include "gamepad.h"

#include <vector>
#include <windows.h>
#include <Xinput.h>

//#pragma comment( lib, "XINPUT.lib" )
#pragma comment( lib, "XINPUT9_1_0.LIB" ) // for vs2012 sdk

class XInputGamepad : public Gamepad {
public:
	DWORD prevPacketNumber = 0;
	DWORD controllerID = 0;
	XINPUT_STATE state;
	bool isMoving = false;
	
	//BOOL isConnected = 0;
private:
	UINT skipCounter = 0;
	DWORD prevJoyButtonState = 0;

public:
	XInputGamepad(unsigned int gamepadId);
	int Poll();
	int IsPressed(char *btnName);
	float GetLX();
	float GetLY();
	float GetRX();
	float GetRY();
	float GetLT();
	float GetRT();
};

class XInputGamepadGroup: public GamepadGroup {
public:
	std::vector<XInputGamepad> gamepads;
	XInputGamepad* activeGamepad = nullptr;
	unsigned int activeGamepadID = -1;
public:
	XInputGamepadGroup();
	Gamepad* GetActiveGamepad();
	int Poll();

};
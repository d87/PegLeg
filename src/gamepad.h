#pragma once

#include <vector>
#include <windows.h>
#include <Xinput.h>

//#pragma comment( lib, "XINPUT.lib" )
#pragma comment( lib, "XINPUT9_1_0.LIB" ) // for vs2012 sdk

class Gamepad {
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
	Gamepad(unsigned int gamepadId);
	int Poll();
	int IsPressed(char *btnName);
};

class GamepadGroup {
public:
	std::vector<Gamepad> gamepads;
	Gamepad *activeGamepad;
	unsigned int activeGamepadID;
public:
	GamepadGroup();
	int Poll();
};
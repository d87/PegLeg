#pragma once

#include "gamepad.h"

#include <vector>
#include <array>
#include <windows.h>

#include <dinput.h>
#include <dinputd.h>
#pragma comment( lib, "dinput8.lib")
#pragma comment( lib, "dxguid.lib")

class DirectInputGamepad : public Gamepad {
public:
	unsigned long deviceInstanceID;
	DWORD prevPacketNumber = 0;
	DWORD controllerID = 0;
	bool isMoving = false;

	LPDIRECTINPUTDEVICE8 pDIDevice;
	//BOOL isConnected = 0;

private:
	DWORD prevJoyButtonState = 0;
	DIJOYSTATE2 js;
	DWORD btnState = 0;

	std::array<int, 6> axisState{ 0,0,0,0,0,0 };
	std::array<int, 6> prevAxisState {0,0,0,0,0,0};

public:
	DirectInputGamepad(unsigned long DIID);
	int Poll();
	int IsPressed(char* btnName);
	float GetLX();
	float GetLY();
	float GetRX();
	float GetRY();
	float GetLT();
	float GetRT();
};

class DirectInputGamepadGroup: public GamepadGroup {
public:
	LPDIRECTINPUT8 pDirectInput;
	std::vector<DirectInputGamepad> gamepads;
	unsigned int activeGamepadID = -1;
	DirectInputGamepad* activeGamepad = nullptr;

private:
	UINT activeRetryCounter = 0;

public:
	DirectInputGamepadGroup();
	Gamepad* GetActiveGamepad();
	int CheckDevices();
	int Poll();
};
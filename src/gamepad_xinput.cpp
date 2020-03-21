#include "pegleg.h"
#include "gamepad_xinput.h"

#include <unordered_map>
#include <vector>

//#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
//#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
//#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30
#define THUMB_DEADZONE 3200

const vector<string> GamepadButtonNames = {
	"UP",
	"DOWN",
	"LEFT",
	"RIGHT",
	"START",
	"BACK",
	"LS",
	"RS",
	"L1",
	"R1",
	"L2",
	"R2",
	"A",
	"B",
	"X",
	"Y",
};

const int NumMaxButtons = 16;

enum XInputButton {
	UP,
	DOWN,
	LEFT,
	RIGHT,
	START,
	BACK,
	LS,
	RS,
	L1,
	R1,
	L2,
	R2,
	A,
	B,
	X,
	Y,
};

const unordered_map<string, XInputButton> ButtonStrToID = {
	{ "UP", XInputButton::UP },
	{ "DOWN", XInputButton::DOWN },
	{ "LEFT", XInputButton::LEFT },
	{ "RIGHT", XInputButton::RIGHT },
	{ "START", XInputButton::START },
	{ "BACK", XInputButton::BACK },
	{ "LS", XInputButton::LS },
	{ "RS", XInputButton::RS },
	{ "L1", XInputButton::L1 },
	{ "R1", XInputButton::R1 },
	{ "L2", XInputButton::L2 },
	{ "R2", XInputButton::R2 },
	{ "A", XInputButton::A },
	{ "B", XInputButton::B },
	{ "X", XInputButton::X },
	{ "Y", XInputButton::Y },
	{ "CROSS", XInputButton::A },
	{ "CIRCLE", XInputButton::B },
	{ "SQUARE", XInputButton::X },
	{ "TRIANGLE", XInputButton::Y },
};

float XInputGamepad::GetLX() {
	return state.Gamepad.sThumbLX * 100 / 0xFFFF;
}
float XInputGamepad::GetLY() {
	return state.Gamepad.sThumbLY * 100 / 0xFFFF;
}

float XInputGamepad::GetRX() {
	return state.Gamepad.sThumbRX * 100 / 0xFFFF;
}
float XInputGamepad::GetRY() {
	return state.Gamepad.sThumbRY * 100 / 0xFFFF;
}

float XInputGamepad::GetLT() {
	return state.Gamepad.bLeftTrigger * 100 / 0xFF;
}
float XInputGamepad::GetRT() {
	return state.Gamepad.bRightTrigger * 100 / 0xFF;
}

Gamepad* XInputGamepadGroup::GetActiveGamepad() {
	return this->activeGamepad;
}

XInputGamepadGroup::XInputGamepadGroup() {
	activeGamepad = NULL;
	activeGamepadID = -1;
	for (int i = 0; i < XUSER_MAX_COUNT; i++) {
		gamepads.push_back(XInputGamepad(i));
	}
}

int XInputGamepadGroup::Poll() {
	BOOL newPackets = false;

	for (XInputGamepad& pad : gamepads){
		int R = pad.Poll();
		if (R == -1 && activeGamepad == &pad) {
			activeGamepad = NULL;
			activeGamepadID = -1;
		}
		if (R == 1) {
			activeGamepad = &pad;
			activeGamepadID = pad.controllerID;
			newPackets = true;
		}
	}

	if (newPackets) {
		FireEvent(L, PegLegEvent::JOYUPDATE, 0, 0, 0);
	}
			
	return 1;
}



XInputGamepad::XInputGamepad(unsigned int gamepadId) {
	controllerID = gamepadId;
	ZeroMemory(&state, sizeof(XINPUT_STATE));
}

int XInputGamepad::Poll() {
	if (skipCounter > 0) {
		skipCounter--;
		return -1;
	}


	DWORD changedJoyButtons = 0;

	DWORD error = XInputGetState(controllerID, &state);
	if (error == ERROR_SUCCESS) {
		if (state.dwPacketNumber != prevPacketNumber) {
			prevPacketNumber = state.dwPacketNumber;

			changedJoyButtons = prevJoyButtonState ^ state.Gamepad.wButtons;
			prevJoyButtonState = state.Gamepad.wButtons;
			if (changedJoyButtons) {
				for (int btn = 0; btn < NumMaxButtons; btn++) {
					if (EXTRACTBIT(changedJoyButtons, btn)) {
						if (EXTRACTBIT(prevJoyButtonState, btn)) { // it's inverted
							FireEvent(L, PegLegEvent::JOYBUTTONDOWN, (char *)GamepadButtonNames[btn].c_str(), btn + 1, 0);
						}
						else {
							FireEvent(L, PegLegEvent::JOYBUTTONUP, (char *)GamepadButtonNames[btn].c_str(), btn + 1, 0);
						}
					}
				}
				return 1;
			}


			float LX = state.Gamepad.sThumbLX;
			float LY = state.Gamepad.sThumbLY;
			//determine how far the controller is pushed
			float LSmagnitude = sqrt(LX*LX + LY*LY);

			if (LSmagnitude > THUMB_DEADZONE) {
				isMoving = true;
				return isMoving;
			}

			float RX = state.Gamepad.sThumbRX;
			float RY = state.Gamepad.sThumbRY;
			//determine how far the controller is pushed
			float RSmagnitude = sqrt(RX*RX + RY*RY);

			if (RSmagnitude > THUMB_DEADZONE) {
				isMoving = true;
				return isMoving;
			}

			float LT = state.Gamepad.bLeftTrigger;
			if (LT > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
				isMoving = true;
				return isMoving;
			}

			float RT = state.Gamepad.bRightTrigger;
			if (RT > XINPUT_GAMEPAD_TRIGGER_THRESHOLD) {
				isMoving = true;
				return isMoving;
			}

			isMoving = false;
			return 1;
		}

			
		return isMoving;
	}
	else {
		if (error == ERROR_DEVICE_NOT_CONNECTED) {
			skipCounter = 60;
			return -1;
			//for (int i = 0; i < XUSER_MAX_COUNT; i++) {
			//DWORD error = XInputGetState(i, &state);
			//if (error == ERROR_SUCCESS)
			//ControllerID = i;
			//}
		}
		//state.Gamepad. = NULL;
	}
	return -2;
}

int XInputGamepad::IsPressed(char *btnName) {
	auto search = ButtonStrToID.find(btnName);
	if (search != ButtonStrToID.end()) {
		const int buttonID = search->second;
		return ((state.Gamepad.wButtons >> buttonID) & 1);
	}
	return 0;
}
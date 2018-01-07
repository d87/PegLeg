#include "pegleg.h"
#include "gamepad.h"

//#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE  7849
//#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
//#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD    30
#define THUMB_DEADZONE 3200

#define EXTRACTBIT(var, index) ((var >> index) & 1)


char *GamepadButtonNames[17] = {
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
	"UB1",
	"UB2",
	"A",
	"B",
	"X",
	"Y",
	(char)NULL
};


GamepadGroup::GamepadGroup() {
	activeGamepad = NULL;
	activeGamepadID = -1;
	for (int i = 0; i < XUSER_MAX_COUNT; i++) {
		gamepads.push_back(Gamepad(i));
	}
}

int GamepadGroup::Poll() {
	BOOL newPackets = false;

	for ( Gamepad& pad : gamepads){
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
		for (int i = 0; events[JOYUPDATE][i] && i < MAX_EVENTS; i++)
			FireEvent(L, events[JOYUPDATE][i], 0, 0, 0);
	}
			
	return 1;
}



Gamepad::Gamepad(unsigned int gamepadId) {
	controllerID = gamepadId;
	ZeroMemory(&state, sizeof(XINPUT_STATE));
}

int Gamepad::Poll() {
	DWORD changedJoyButtons = 0;

	DWORD error = XInputGetState(controllerID, &state);
	if (error == ERROR_SUCCESS) {
		//if (state.dwPacketNumber != prevPacketNumber) {
			prevPacketNumber = state.dwPacketNumber;

			changedJoyButtons = prevJoyButtonState ^ state.Gamepad.wButtons;
			prevJoyButtonState = state.Gamepad.wButtons;
			if (changedJoyButtons) {
				for (int btn = 0; btn<20; btn++) {
					if (EXTRACTBIT(changedJoyButtons, btn)) {
						if (EXTRACTBIT(prevJoyButtonState, btn)) { // it's inverted
							for (int i = 0; events[JOYBUTTONDOWN][i] && i < MAX_EVENTS; i++)
								FireEvent(L, events[JOYBUTTONDOWN][i], GamepadButtonNames[btn], btn + 1, 0);
						}
						else {
							for (int i = 0; events[JOYBUTTONUP][i] && i < MAX_EVENTS; i++)
								FireEvent(L, events[JOYBUTTONUP][i], GamepadButtonNames[btn], btn + 1, 0);
						}
					}
				}
				return 1;
			}

			float LX = state.Gamepad.sThumbLX;
			float LY = state.Gamepad.sThumbLY;
			//determine how far the controller is pushed
			float LSmagnitude = sqrt(LX*LX + LY*LY);

			if (LSmagnitude > THUMB_DEADZONE)
				return 1;

			float RX = state.Gamepad.sThumbRX;
			float RY = state.Gamepad.sThumbRY;
			//determine how far the controller is pushed
			float RSmagnitude = sqrt(RX*RX + RY*RY);

			if (RSmagnitude > THUMB_DEADZONE)
				return 1;

			float LT = state.Gamepad.bLeftTrigger;
			if (LT > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
				return 1;

			float RT = state.Gamepad.bRightTrigger;
			if (RT > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
				return 1;
			
		//}
			return 0;
	}
	else {
		if (error == ERROR_DEVICE_NOT_CONNECTED) {
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

int Gamepad::IsPressed(char *btnName) {
	for (int i = 0; GamepadButtonNames[i]; i++)
		if (!strcmp(_strupr((char*)btnName), GamepadButtonNames[i])) {
			int buttonID = i;
			return ((state.Gamepad.wButtons >> buttonID) & 1);
		}
	return 0;
}
#include "pegleg.h"
#include "gamepad_directinput.h"


#include <unordered_map>
#include <vector>

#define THUMB_DEADZONE 3200

#define DUALSHOCK4_V1_HWID 0x05C4054C // 054C:05C4
#define DUALSHOCK4_V2_HWID 0x09CC054C // 054C:09CC

const DWORD highestBit = 1 << 31;

const std::vector<std::string> GamepadButtonNames = {
	"UP",
	"RIGHT",
	"DOWN",
	"LEFT",
	"X", //Rect
	"A", //Cross
	"B", //Circle
	"Y", //Triangle
	"L1",
	"R1",
	"L2", //X rot
	"R2", //Y rot
	"BACK", //Select, Share
	"START", //Options
	"LS",
	"RS",
	"PS",
	"TOUCHPAD",
};

const int NumMaxButtons = 16;

enum DirectInputButton {
	UP,
	RIGHT,
	DOWN,
	LEFT,
	X, //Rect
	A, //Cross
	B, //Circle
	Y, //Triangle
	L1,
	R1,
	L2, //X rot
	R2, //Y rot
	BACK, //Select, Share
	START, //Options
	LS,
	RS,
	PS,
	TOUCHPAD,
};

const std::unordered_map<std::string, DirectInputButton> ButtonStrToID = {
	{ "UP", DirectInputButton::UP },
	{ "DOWN", DirectInputButton::DOWN },
	{ "LEFT", DirectInputButton::LEFT },
	{ "RIGHT", DirectInputButton::RIGHT },
	{ "START", DirectInputButton::START },
	{ "BACK", DirectInputButton::BACK },
	{ "LS", DirectInputButton::LS },
	{ "RS", DirectInputButton::RS },
	{ "L1", DirectInputButton::L1 },
	{ "R1", DirectInputButton::R1 },
	{ "L2", DirectInputButton::L2 },
	{ "R2", DirectInputButton::R2 },
	{ "A", DirectInputButton::A },
	{ "B", DirectInputButton::B },
	{ "X", DirectInputButton::X },
	{ "Y", DirectInputButton::Y },
	{ "CROSS", DirectInputButton::A },
	{ "CIRCLE", DirectInputButton::B },
	{ "SQUARE", DirectInputButton::X },
	{ "TRIANGLE", DirectInputButton::Y },
};

float DirectInputGamepad::GetLX() {
	return (js.lX * 100 / 0xFFFF) - 50;
}
float DirectInputGamepad::GetLY() {
	return 100 - (js.lY * 100 / 0xFFFF) - 50;
}

float DirectInputGamepad::GetRX() {
	return (js.lZ * 100 / 0xFFFF) - 50;
}
float DirectInputGamepad::GetRY() {
	return 100 - (js.lRz * 100 / 0xFFFF) - 50 ;
}

float DirectInputGamepad::GetLT() {
	return js.lRx * 100 / 0xFFFF;
}
float DirectInputGamepad::GetRT() {
	return js.lRy * 100 / 0xFFFF;
}


BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext)
{
	HRESULT hr;

	if (pdidInstance->guidProduct.Data1 == DUALSHOCK4_V1_HWID || pdidInstance->guidProduct.Data1 == DUALSHOCK4_V2_HWID) {

		printf("Found DualShock 4 %u\n", pdidInstance->guidProduct.Data1);

		auto gamepadGroup = static_cast<DirectInputGamepadGroup*>(pContext);

		DirectInputGamepad* pad = nullptr;

		for (auto it = gamepadGroup->gamepads.begin(); it != gamepadGroup->gamepads.end(); it++) {
			if (it->deviceInstanceID == pdidInstance->guidInstance.Data1) {
				pad = &(*it);
				return DIENUM_CONTINUE;
				//break;
			}
		}

		if (pad == nullptr) {
			gamepadGroup->gamepads.push_back(DirectInputGamepad(pdidInstance->guidInstance.Data1));
			printf("Created new gamepad\n");
			pad = &gamepadGroup->gamepads.back();
		}

		//auto currentGamepad = gamepadGroup->gamepads[gamepadGroup->enumIndex];
		//gamepadGroup->enumIndex++;

	
		// Obtain an interface to the enumerated joystick.
		hr = gamepadGroup->pDirectInput->CreateDevice(pdidInstance->guidInstance, &pad->pDIDevice, NULL);


		// Switching buffered mode on
		/*
		DIPROPDWORD  dipdw;
		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = 3; // buffer size

		if (FAILED(hr = pad->pDIDevice->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
			return DIENUM_STOP;
		*/

		if (FAILED(hr = pad->pDIDevice->SetDataFormat(&c_dfDIJoystick2))) // predefined global variable
			return DIENUM_STOP;

		if (FAILED(hr = pad->pDIDevice->SetCooperativeLevel(NULL, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND)))
			return DIENUM_STOP;

		if (FAILED(hr))
			return DIENUM_CONTINUE;
	}
	

	//return DIENUM_STOP;
	return DIENUM_CONTINUE;
}

int DirectInputGamepadGroup::CheckDevices() {
	HRESULT hr;
	hr = pDirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumJoysticksCallback, this, DIEDFL_ATTACHEDONLY);

	return hr;
}


DirectInputGamepadGroup::DirectInputGamepadGroup() {
	HRESULT hr;
	if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&pDirectInput, NULL)))
		return;

	CheckDevices();

	return;
}

int DirectInputGamepadGroup::Poll() {
	/*if (skipCounter > 0) {
		skipCounter--;
	} else {
		CheckDevices();
		skipCounter = 60;
	}*/

	if (gamepads.size() == 0) return -1;

	BOOL newPackets = false;

	// A single gamepad can be connected as two separate devices, both of them working at the same time.
	// So to combat this focus on a single device until it's not responding for several ticks, then repick
	if (activeGamepad) {
		int R = activeGamepad->Poll();
		if (R == -1) {
			activeRetryCounter++;
		}
		else {
			newPackets = true;
			activeRetryCounter = 0;
		}
	}
	else{
		activeRetryCounter = 10;
	}

	if (activeRetryCounter > 5) {
		int i = 0;
		int padToRemove = -1;

		for (DirectInputGamepad& pad : gamepads) {
			int R = pad.Poll();
			if (R == -1 && activeGamepad == &pad) {
				activeGamepad = nullptr;
				activeGamepadID = -1;
				padToRemove = i;
				break;
			}
			if (R == 1) {
				activeGamepad = &pad;
				activeGamepadID = i;
				newPackets = true;
			}
			i++;
		}

		if (padToRemove > -1) {
			gamepads.erase(gamepads.begin() + i);
		}
	}

	if (newPackets) {
		FireEvent(L, PegLegEvent::JOYUPDATE, 0, 0, 0);
	}

	return 1;
}

Gamepad* DirectInputGamepadGroup::GetActiveGamepad() {
	return this->activeGamepad;
}

DirectInputGamepad::DirectInputGamepad(unsigned long instanceID) {
	deviceInstanceID = instanceID;
	ZeroMemory(&js, sizeof(DIJOYSTATE2));
}

int DirectInputGamepad::Poll() {
	HRESULT hr;

	DWORD changedJoyButtons = 0;

	hr = pDIDevice->Poll();

	if (FAILED(hr = pDIDevice->Acquire())) {
		return -1;
	}
	else {

		/*DIDEVICEOBJECTDATA devData[3];
		DWORD dwItems = 3; // On entry, the number of elements in the array pointed to by the rgdod parameter. On exit, the number of elements actually obtained.
		hr = pDIDevice->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), &devData[0], &dwItems, NULL);
		//DIDEVICEOBJECTDATA* latestPacket = &devData[dwItems-1];
		if (dwItems == 0) { // || latestPacket->dwSequence == prevPacketNumber) {
			//prevPacketNumber = latestPacket->dwSequence;
			return 0;
		}*/

		hr = pDIDevice->GetDeviceState(sizeof(DIJOYSTATE2), &js);

		hr = pDIDevice->Unacquire();

		btnState = 0;
		for (int i = 0; i < 32; i++) {
			btnState = btnState >> 1;
			if (js.rgbButtons[i] == 128) {
				// rgbButtons - Array of buttons.
				// The high - order bit of the byte is set if the corresponding button is down, and clear if the button is up or does not exist.
				btnState += highestBit;
				// printf("%i pressed\n", i);
			}
		}
		// printf("btn state: %u \n", btnState);

		int POVState = js.rgdwPOV[0];
		int bitPOVState = 0; // LEFT|DOWN|RIGHT|UP
		switch (POVState) {
			case -1:   bitPOVState = 0b0000; break;
			case 0:    bitPOVState = 0b0001; break;
			case 4500: bitPOVState = 0b0011; break;
			case 9000: bitPOVState = 0b0010; break;
			case 13500:bitPOVState = 0b0110; break;
			case 18000:bitPOVState = 0b0100; break;
			case 22500:bitPOVState = 0b1100; break;
			case 27000:bitPOVState = 0b1000; break;
			case 31500:bitPOVState = 0b1001; break;
		}
		//printf("%i\n", js.rgdwPOV[0]);

		// Merging POV state with button state
		// This will crop the last 4 of the 32 possible buttons, but that's an unlikely scenario when it matters
		btnState = (btnState << 4) + bitPOVState;

		// printf("btn state with POV: %u \n", btnState);


		changedJoyButtons = prevJoyButtonState ^ btnState;
		prevJoyButtonState = btnState;

		if (changedJoyButtons) {
			for (int btn = 0; btn < 32; btn++) {
				if (EXTRACTBIT(changedJoyButtons, btn)) {
					if (EXTRACTBIT(prevJoyButtonState, btn)) { //it's inverted
						FireEvent(L, PegLegEvent::JOYBUTTONDOWN, (char*)GamepadButtonNames[btn].c_str(), btn + 1, 0);
					}
					else {
						FireEvent(L, PegLegEvent::JOYBUTTONUP, (char*)GamepadButtonNames[btn].c_str(), btn + 1, 0);
					}
				}
			}
			return 1;
		}

		const int mid = 0xFFFF * 0.5;

		int LX = js.lX - mid;
		if (abs(LX) < THUMB_DEADZONE) LX = 0;
		axisState[0] = LX;

		int LY = mid - js.lY;
		if (abs(LY) < THUMB_DEADZONE) LY = 0;
		axisState[1] = LY;

		int RX = js.lZ - mid;
		if (abs(RX) < THUMB_DEADZONE) RX = 0;
		axisState[2] = RX;

		int RY = mid - js.lRz;
		if (abs(RY) < THUMB_DEADZONE) RY = 0;
		axisState[3] = RY;

		axisState[4] = js.lRx;
		axisState[5] = js.lRy;

		bool isMoving = false;
		for (int i = 0; i < axisState.size(); i++) {
			if (axisState[i] != 0) {
				isMoving = true;
				break;
			}
		}

		return isMoving;

		//bool axisStateChanged = axisState != prevAxisState;
		//prevAxisState = axisState;
		//return (axisStateChanged||changedJoyButtons) ? 1 : 0;



	}
	return -2;
}

int DirectInputGamepad::IsPressed(char* btnName) {
	auto search = ButtonStrToID.find(btnName);
	if (search != ButtonStrToID.end()) {
		const int buttonID = search->second;
		return ((btnState >> buttonID) & 1);
	}
	return 0;
}



/*
	HRESULT hr;	// Poll the device to read the current state
	if( FAILED( hr = pDevice->Poll() ) )    {
		// DInput is telling us that the input stream has been
		// interrupted. We aren't tracking any state between polls, so
		// we don't have any special reset that needs to be done. We
		// just re-acquire and try again.
		hr = pDevice->Acquire();
		while( hr == DIERR_INPUTLOST )
		{            hr = pDevice->Acquire();
		}		// If we encounter a fatal error, return failure.
		if( (hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED) )
		{
			return E_FAIL;
		}
		// If another application has control of this device, return success.
		// We'll just have to wait our turn to use the joystick.
		if( hr == DIERR_OTHERAPPHASPRIO )
		{
			return E_FAIL;
		}
		// hr may be DIERR_OTHERAPPHASPRIO or other errors.  This
		// may occur when the app is minimized or in the process of
		// switching, so just try again later
		return E_FAIL;
	}
	return DI_OK;
}	// Poll

*/
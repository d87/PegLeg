#include "gamepad.h"

int Gamepad::Poll() {
	return 0;
}
int Gamepad::IsPressed(char* btnName) {
	return 0;
}

float Gamepad::GetLX() { return 0.0; }
float Gamepad::GetLY() { return 0.0; }
float Gamepad::GetRX() { return 0.0; }
float Gamepad::GetRY() { return 0.0; }
float Gamepad::GetLT() { return 0.0; }
float Gamepad::GetRT() { return 0.0; }

Gamepad* GamepadGroup::GetActiveGamepad() {
	return nullptr;
}

int GamepadGroup::Poll() {
	return 0;
}

int GamepadGroup::CheckDevices() {
	return 1;
}
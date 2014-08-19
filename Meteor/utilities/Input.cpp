#include "Input.h"

#include <windows.h>
#include <XInput.h>
#pragma comment(lib, "XInput.lib")

#include <math.h>

namespace Input
{
	enum KeyMapping { L_TRIGGER, R_TRIGGER, A_DOWN, A_LEFT, A_RIGHT, A_UP, NUM_MAPPINGS };

	InputDevice devices[5];
	int numDevices = 1;
	int playerDevices[MAX_PLAYERS];

	WORD keyBindings[NUM_BUTTONS + NUM_MAPPINGS];

	int mousePosition[2];
	float mouseDelta[2];
	float mouseSensitivity = 16.0f;
	bool isMouseRelative = true;

	void PollKeyboardAndMouse(bool buttonsPressed[]);
	void PollXInputGamepad(int index, bool buttonsPressed[]);
	void PollJoystickGamepad(bool buttonsPressed[]);
}

void Input::Initialize()
{
	for(int i = 0; i < MAX_PLAYERS; ++i)
		playerDevices[i] = 0;

	keyBindings[A] = VK_SPACE;
	keyBindings[B] = 'X';
	keyBindings[X] = 'E';
	keyBindings[Y] = 'R';
	keyBindings[START] = VK_ESCAPE;
	keyBindings[SELECT] = VK_RETURN;
	keyBindings[L_SHOULDER] = VK_OEM_MINUS;
	keyBindings[R_SHOULDER] = VK_OEM_PLUS;
	keyBindings[D_RIGHT] = VK_END;
	keyBindings[D_UP] = VK_PRIOR;
	keyBindings[D_LEFT] = VK_HOME;
	keyBindings[D_DOWN] = VK_NEXT;

	keyBindings[NUM_BUTTONS + L_TRIGGER] = VK_OEM_4;
	keyBindings[NUM_BUTTONS + R_TRIGGER] = VK_OEM_6;
	keyBindings[NUM_BUTTONS + A_RIGHT] = 'D';
	keyBindings[NUM_BUTTONS + A_UP] = 'W';
	keyBindings[NUM_BUTTONS + A_LEFT] = 'A';
	keyBindings[NUM_BUTTONS + A_DOWN] = 'S';

	mousePosition[0] = mousePosition[1] = 0;
}

void Input::Terminate()
{
}

void Input::GetMousePosition(int point[2])
{
	point[0] = mousePosition[0];
	point[1] = mousePosition[1];
}

void Input::SetMouseDelta(int delta[2])
{
	mouseDelta[0] = delta[0];
	mouseDelta[1] = delta[1];
}

InputDevice* Input::GetDevice(PlayerSlot slot)
{
	if(slot >= numDevices) return nullptr;
	return &devices[playerDevices[slot]];
}

void Input::DetectDevices()
{
	numDevices = 1;

	// detect XInput gamepads
	for(DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		XINPUT_CAPABILITIES capabilities;
		ZeroMemory(&capabilities, sizeof capabilities);

		DWORD result = XInputGetCapabilities(i, 0, &capabilities);
		if(result != ERROR_SUCCESS) continue;

		InputDevice& pad = devices[numDevices++];
		pad.type = GAMEPAD_XINPUT;
	}
}

void Input::Poll()
{
	// input device handling
	for(int i = 0; i < numDevices; i++)
	{
		InputDevice& pad = devices[i];

		// set device to defaults before polling
		pad.leftAnalog[0] = pad.rightAnalog[0] = 0.0f;
		pad.leftAnalog[1] = pad.rightAnalog[1] = 0.0f;
		pad.leftTrigger = pad.rightTrigger = 0.0f;

		bool buttonsPressed[NUM_BUTTONS] = { false };

		// do polling
		switch(pad.type)
		{
			case KEYBOARD_AND_MOUSE:
				PollKeyboardAndMouse(buttonsPressed); break;
			case GAMEPAD_XINPUT:
				PollXInputGamepad(i, buttonsPressed); break;
			case GAMEPAD_JOYSTICK:
				PollJoystickGamepad(buttonsPressed); break;
		}

		pad.UpdateButtons(buttonsPressed);
	}

	// mouse position handling
	POINT mouseScreenPosition;
	if(GetCursorPos(&mouseScreenPosition))
	{
		mousePosition[0] = mouseScreenPosition.x;
		mousePosition[1] = mouseScreenPosition.y;
	}
}

void Input::PollKeyboardAndMouse(bool buttonsPressed[])
{
	// poll keyboard and mouse buttons
	for(int i = 0; i < NUM_BUTTONS; ++i)
		buttonsPressed[i] = GetKeyState(keyBindings[i]) & 0x8000;

	bool mappingsPressed[NUM_MAPPINGS];
	for(int i = 0; i < NUM_MAPPINGS; ++i)
		mappingsPressed[i] = GetKeyState(keyBindings[NUM_BUTTONS + i]) & 8000;

	InputDevice& device = devices[0];
	if(mappingsPressed[L_TRIGGER])	device.leftTrigger = 1.0f;
	if(mappingsPressed[R_TRIGGER])	device.rightTrigger = 1.0f;

	float dx = 0.0f, dy = 0.0f;
	if(mappingsPressed[A_LEFT])		dx -= 1.0f;
	if(mappingsPressed[A_RIGHT])	dx += 1.0f;
	if(mappingsPressed[A_UP])		dy += 1.0f;
	if(mappingsPressed[A_DOWN])		dy -= 1.0f;

	float magnitude = sqrt(dx * dx + dy * dy);
	if(magnitude > 0.0f)
	{
		device.leftAnalog[0] = dx / magnitude;
		device.leftAnalog[1] = dy / magnitude;
	}

	device.rightAnalog[0] = mouseDelta[0] / mouseSensitivity;
	device.rightAnalog[1] = mouseDelta[1] / mouseSensitivity;

	mouseDelta[0] = mouseDelta[1] = 0.0f;
}

void Input::PollXInputGamepad(int index, bool buttonsPressed[])
{
	XINPUT_STATE state;
	ZeroMemory(&state, sizeof state);

	DWORD result = XInputGetState(index - 1, &state);
	if(result != ERROR_SUCCESS) return;

	InputDevice& pad = devices[index];
	
	//left thumbstick handling
	float LX = state.Gamepad.sThumbLX;
	float LY = state.Gamepad.sThumbLY;

	float magnitudeL = sqrt(LX * LX + LY * LY);

	float normalizedLX = LX / magnitudeL;
	float normalizedLY = LY / magnitudeL;

	float normalizedMagnitudeL = 0;

	if(magnitudeL > XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE)
	{
		if(magnitudeL > 32767) magnitudeL = 32767;
		magnitudeL -= XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE;
		normalizedMagnitudeL = magnitudeL / (32767 - XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
	}
	else
	{
		magnitudeL = 0.0;
		normalizedMagnitudeL = 0.0;
	}

	pad.leftAnalog[0] = normalizedMagnitudeL * normalizedLX;
	pad.leftAnalog[1] = normalizedMagnitudeL * normalizedLY;

	//right thumbstick handling
	float RX = state.Gamepad.sThumbRX;
	float RY = state.Gamepad.sThumbRY;

	float magnitudeR = sqrt(RX * RX + RY * RY);

	float normalizedRX = RX / magnitudeR;
	float normalizedRY = RY / magnitudeR;

	float normalizedMagnitudeR = 0;

	if (magnitudeR > XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE)
	{
		if (magnitudeR > 32767) magnitudeR = 32767;
		magnitudeR -= XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE;
		normalizedMagnitudeR = magnitudeR / (32767 - XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
	}
	else
	{
		magnitudeR = 0.0;
		normalizedMagnitudeR = 0.0;
	}

	pad.rightAnalog[0] = normalizedMagnitudeR * normalizedRX;
	pad.rightAnalog[1] = normalizedMagnitudeR * normalizedRY;

	//trigger handling
	float triggerMagnitudeL = state.Gamepad.bLeftTrigger;
	float normalizedTriggerMagnitudeL = 0.0f;
	if(triggerMagnitudeL > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
	{
		if(triggerMagnitudeL > 255) triggerMagnitudeL = 255;
		triggerMagnitudeL -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		normalizedTriggerMagnitudeL = triggerMagnitudeL / (255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
	}
	pad.leftTrigger = normalizedTriggerMagnitudeL;

	float triggerMagnitudeR = state.Gamepad.bRightTrigger;
	float normalizedTriggerMagnitudeR = 0.0f;
	if(triggerMagnitudeR > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
	{
		if(triggerMagnitudeR > 255) triggerMagnitudeR = 255;
		triggerMagnitudeR -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		normalizedTriggerMagnitudeR = triggerMagnitudeR / (255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
	}
	pad.rightTrigger = normalizedTriggerMagnitudeR;

	//buttons handling
	buttonsPressed[A] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
	buttonsPressed[B] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
	buttonsPressed[X] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
	buttonsPressed[Y] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
	buttonsPressed[START] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
	buttonsPressed[SELECT] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
	buttonsPressed[L_SHOULDER] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
	buttonsPressed[R_SHOULDER] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
	buttonsPressed[D_UP] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
	buttonsPressed[D_LEFT] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
	buttonsPressed[D_DOWN] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
	buttonsPressed[D_RIGHT] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
}

void Input::PollJoystickGamepad(bool buttonsPressed[])
{

}

bool InputDevice::GetButtonDown(Input::Button button) const
{
	return buttons[button] == PRESSED 
		|| buttons[button] == ONCE;
}

bool InputDevice::GetButtonReleased(Input::Button button) const
{
	return buttons[button] == RELEASED;
}

bool InputDevice::GetButtonTapped(Input::Button button) const
{
	return buttons[button] == ONCE;
}

void InputDevice::UpdateButtons(bool pressed[])
{
	for(int i = 0; i < Input::NUM_BUTTONS; ++i)
	{
		if(pressed[i])
			buttons[i] = (buttons[i] == RELEASED) ? ONCE : PRESSED;
		else
			buttons[i] = RELEASED;
	}
}

#include "Input.h"

#include "Logging.h"

#if defined(_WIN32)
#include <XInput.h>
#pragma comment(lib, "XInput.lib")
#include <windows.h>

#elif defined(X11)
#define XK_LATIN1
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include <X11/Xlib.h>
#endif

#include <math.h>

namespace Input
{
	enum KeyMapping { L_TRIGGER, R_TRIGGER, A_DOWN, A_LEFT, A_RIGHT, A_UP, NUM_MAPPINGS };

	InputDevice devices[5];
	int numDevices = 1;
	int playerDevices[MAX_PLAYERS];

	unsigned short keyBindings[NUM_BUTTONS + NUM_MAPPINGS];

	int mousePosition[2];
	float mouseDelta[2];
	float mouseSensitivity = 16.0f;
	bool isMouseRelative = true;

#if defined(X11)
	Display* display;
#endif

	void PollKeyboardAndMouse(bool buttonsPressed[]);
	void PollXInputGamepad(int index, bool buttonsPressed[]);
	void PollJoystickGamepad(bool buttonsPressed[]);

	unsigned short GetScanCode(int virtualKey);
	void PollKeyboard(char* keyboardState);
}

unsigned short Input::GetScanCode(int virtualKey)
{
#if defined(_WIN32)
	return MapVirtualKey(virtualKey, MAPVK_VK_TO_VSC);
#elif defined(X11)
	return XKeysymToKeycode(display, virtualKey);
#endif
}

void Input::Initialize()
{
	for(int i = 0; i < MAX_PLAYERS; ++i)
		playerDevices[i] = 0;

	mousePosition[0] = mousePosition[1] = 0;

#if defined(_WIN32)
	keyBindings[A] = GetScanCode(VK_SPACE);
	keyBindings[B] = GetScanCode('X');
	keyBindings[X] = GetScanCode('E');
	keyBindings[Y] = GetScanCode('R');
	keyBindings[START] = GetScanCode(VK_ESCAPE);
	keyBindings[SELECT] = GetScanCode(VK_RETURN);
	keyBindings[L_SHOULDER] = GetScanCode(VK_OEM_MINUS);
	keyBindings[R_SHOULDER] = GetScanCode(VK_OEM_PLUS);
	keyBindings[D_RIGHT] = GetScanCode(VK_END);
	keyBindings[D_UP] = GetScanCode(VK_PRIOR);
	keyBindings[D_LEFT] = GetScanCode(VK_HOME);
	keyBindings[D_DOWN] = GetScanCode(VK_NEXT);

	keyBindings[NUM_BUTTONS + L_TRIGGER] = GetScanCode(VK_OEM_4);
	keyBindings[NUM_BUTTONS + R_TRIGGER] = GetScanCode(VK_OEM_6);
	keyBindings[NUM_BUTTONS + A_RIGHT] = GetScanCode('D');
	keyBindings[NUM_BUTTONS + A_UP] = GetScanCode('W');
	keyBindings[NUM_BUTTONS + A_LEFT] = GetScanCode('A');
	keyBindings[NUM_BUTTONS + A_DOWN] = GetScanCode('S');

#elif defined(X11)
	display = XOpenDisplay(NULL);

	keyBindings[A] = GetScanCode(XK_space);
	keyBindings[B] = GetScanCode('X');
	keyBindings[X] = GetScanCode('E');
	keyBindings[Y] = GetScanCode('R');
	keyBindings[START] = GetScanCode(XK_Escape);
	keyBindings[SELECT] = GetScanCode(XK_Return);
	keyBindings[L_SHOULDER] = GetScanCode(XK_minus);
	keyBindings[R_SHOULDER] = GetScanCode(XK_plus);
	keyBindings[D_RIGHT] = GetScanCode(XK_End);
	keyBindings[D_UP] = GetScanCode(XK_Prior);
	keyBindings[D_LEFT] = GetScanCode(XK_Home);
	keyBindings[D_DOWN] = GetScanCode(XK_Next);

	keyBindings[NUM_BUTTONS + L_TRIGGER] = GetScanCode(XK_bracketleft);
	keyBindings[NUM_BUTTONS + R_TRIGGER] = GetScanCode(XK_bracketright);
	keyBindings[NUM_BUTTONS + A_RIGHT] = GetScanCode('D');
	keyBindings[NUM_BUTTONS + A_UP] = GetScanCode('W');
	keyBindings[NUM_BUTTONS + A_LEFT] = GetScanCode('A');
	keyBindings[NUM_BUTTONS + A_DOWN] = GetScanCode('S');
#endif
}

void Input::Terminate()
{
#if defined(X11)
	XCloseDisplay(display);
#endif
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

#if defined(_WIN32)
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
#endif
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
#if defined(WIN32)
	POINT mouseScreenPosition;
	if(GetCursorPos(&mouseScreenPosition))
	{
		mousePosition[0] = mouseScreenPosition.x;
		mousePosition[1] = mouseScreenPosition.y;
	}
#endif
}

void Input::PollKeyboard(char* keyboardState)
{
#if defined(WIN32)
	GetKeyboardState((PBYTE) keyboardState);
#elif defined(X11)
	XQueryKeymap(display, keyboardState);
#endif
}

#if defined(WIN32)
#define GET_KEY_STATE(code, keyboard) keyboard[(code)] & 0x8000;
#elif defined(X11)
#define GET_KEY_STATE(code, keyboard) keyboard[(code) >> 3] >> ((code) & 0x07) & 0x01;
#endif

void Input::PollKeyboardAndMouse(bool buttonsPressed[])
{
	// poll keyboard and mouse buttons
	char keys[256];
	PollKeyboard(keys);

	for(int i = 0; i < NUM_BUTTONS; ++i)
		buttonsPressed[i] = GET_KEY_STATE(keyBindings[i], keys);

	bool mappingsPressed[NUM_MAPPINGS];
	for(int i = 0; i < NUM_MAPPINGS; ++i)
		mappingsPressed[i] = GET_KEY_STATE(keyBindings[NUM_BUTTONS + i], keys);

	// update device and buttons with polled values
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
#if defined(_WIN32)

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

#endif
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

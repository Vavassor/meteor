#if defined(_WIN32)

#include "WindowsInput.h"

#include "InternalGlobals.h"

#include "utilities/Logging.h"
#include "utilities/DeviceGUID.h"

#include "xinput.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "dbt.h"
#include "windows.h"

#include <math.h>

namespace Input
{
	HWND window;
	HDEVNOTIFY deviceNotification;

	float mouseDelta[2];
	float mouseSensitivity = 16.0f;

	LRESULT OnDeviceChange(WPARAM eventType, LPARAM eventData);
	void OnInput(HRAWINPUT input);

	LRESULT CALLBACK WindowProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
}

void Input::InitializeWindow()
{
	// make invisible window for receiving raw input messages
	HINSTANCE instance = GetModuleHandle(NULL);

	WNDCLASSEX classEx = {};
	classEx.cbSize = sizeof classEx;
	classEx.lpfnWndProc = WindowProc;
	classEx.hInstance = instance;
	classEx.lpszClassName = L"MeteorInputWindowClass";

	if(RegisterClassEx(&classEx) == 0)
	{
		LOG_ISSUE("RegisterClassEx failed!");
		return;
	}

	// create the actual window
	window = CreateWindowEx(WS_EX_APPWINDOW, classEx.lpszClassName, NULL, NULL,
		0, 0, 0, 0, NULL, NULL, instance, NULL);
	if(window == NULL)
	{
		LOG_ISSUE("CreateWindowEx failed!");
		return;
	}

	// set up notification for detecting gamepads
	DEV_BROADCAST_DEVICEINTERFACE filter = {};
	filter.dbcc_size = sizeof filter;
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	filter.dbcc_classguid = GUID_DEVINTERFACE_HID;

	deviceNotification = RegisterDeviceNotification(window, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
	if(deviceNotification == NULL)
	{
		LOG_INFO("Registering device notification for detecting gamepads failed!");
	}

	// set up Raw Input for mouse and keyboard controls
	{
		RAWINPUTDEVICE devices[2];

		devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		devices[0].dwFlags = RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE;
		devices[0].hwndTarget = window;

		devices[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		devices[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		devices[1].dwFlags = RIDEV_NOLEGACY | RIDEV_NOHOTKEYS;
		devices[1].hwndTarget = window;

		BOOL registered = RegisterRawInputDevices(devices, ARRAYSIZE(devices),
			sizeof(RAWINPUTDEVICE));
		if(registered == FALSE)
		{
			LOG_INFO("Registering keyboard and/or mouse as Raw Input devices failed!");
		}
	}
}

void Input::TerminateWindow()
{
	// unregister from raw input mouse and keyboard events
	{
		RAWINPUTDEVICE devices[2];

		devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		devices[0].dwFlags = RIDEV_REMOVE;
		devices[0].hwndTarget = NULL;

		devices[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		devices[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		devices[1].dwFlags = RIDEV_REMOVE;
		devices[1].hwndTarget = NULL;

		RegisterRawInputDevices(devices, ARRAYSIZE(devices), sizeof(RAWINPUTDEVICE));
	}

	UnregisterDeviceNotification(deviceNotification);

	DestroyWindow(window);
}

int Input::DetectDevices(ControllerType types[])
{
	int numControllers = 1;
	types[0] = KEYBOARD_AND_MOUSE;

	// detect XInput gamepads
	for(DWORD i = 0; i < XUSER_MAX_COUNT; i++)
	{
		XINPUT_CAPABILITIES capabilities;
		ZeroMemory(&capabilities, sizeof capabilities);

		DWORD result = XInputGetCapabilities(i, 0, &capabilities);
		if(result != ERROR_SUCCESS) continue;

		types[numControllers++] = GAMEPAD_XINPUT;
	}

	return numControllers;
}

void Input::CaptureMouse(bool enable)
{
	if(enable)
	{
		// capture mouse input so that stray clicks don't make the program lose focus
		RAWINPUTDEVICE devices[1];
		devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		devices[0].dwFlags = RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE;
		devices[0].hwndTarget = window;
		RegisterRawInputDevices(devices, ARRAYSIZE(devices), sizeof(RAWINPUTDEVICE));

		// hide cursor
		ShowCursor(FALSE);
		SetCursor(NULL);
	}
	else
	{
		// unregister so mouse stops being captured with RIDEV_CAPTUREMOUSE
		RAWINPUTDEVICE devices[1];
		devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		devices[0].dwFlags = RIDEV_REMOVE;
		devices[0].hwndTarget = NULL;
		RegisterRawInputDevices(devices, ARRAYSIZE(devices), sizeof(RAWINPUTDEVICE));

		// re-register mouse for raw input without the capture
		devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		devices[0].dwFlags = 0;
		devices[0].hwndTarget = window;
		RegisterRawInputDevices(devices, ARRAYSIZE(devices), sizeof(RAWINPUTDEVICE));

		// show cursor
		//SetCursor(oldCursor);
		ShowCursor(TRUE);
	}
}

void Input::MessageLoop()
{
	MSG msg = {};
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if(msg.message == WM_QUIT) break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

void Input::PollMouse(Controller* controller)
{
	controller->rightAnalog[0] = mouseDelta[0] / mouseSensitivity;
	controller->rightAnalog[1] = mouseDelta[1] / mouseSensitivity;

	mouseDelta[0] = mouseDelta[1] = 0.0f;
}

void Input::PollXInputDevice(int deviceNum, Controller* pad)
{
	XINPUT_STATE state;
	ZeroMemory(&state, sizeof state);

	DWORD result = XInputGetState(deviceNum, &state);
	if(result != ERROR_SUCCESS) return;

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

	pad->leftAnalog[0] = normalizedMagnitudeL * normalizedLX;
	pad->leftAnalog[1] = normalizedMagnitudeL * normalizedLY;

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

	pad->rightAnalog[0] = normalizedMagnitudeR * normalizedRX;
	pad->rightAnalog[1] = normalizedMagnitudeR * normalizedRY;

	//trigger handling
	float triggerMagnitudeL = state.Gamepad.bLeftTrigger;
	float normalizedTriggerMagnitudeL = 0.0f;
	if(triggerMagnitudeL > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
	{
		if(triggerMagnitudeL > 255) triggerMagnitudeL = 255;
		triggerMagnitudeL -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		normalizedTriggerMagnitudeL = triggerMagnitudeL / (255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
	}
	pad->leftTrigger = normalizedTriggerMagnitudeL;

	float triggerMagnitudeR = state.Gamepad.bRightTrigger;
	float normalizedTriggerMagnitudeR = 0.0f;
	if(triggerMagnitudeR > XINPUT_GAMEPAD_TRIGGER_THRESHOLD)
	{
		if(triggerMagnitudeR > 255) triggerMagnitudeR = 255;
		triggerMagnitudeR -= XINPUT_GAMEPAD_TRIGGER_THRESHOLD;
		normalizedTriggerMagnitudeR = triggerMagnitudeR / (255 - XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
	}
	pad->rightTrigger = normalizedTriggerMagnitudeR;

	//buttons handling
	pad->buttons[A] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) != 0;
	pad->buttons[B] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) != 0;
	pad->buttons[X] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) != 0;
	pad->buttons[Y] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) != 0;
	pad->buttons[START] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) != 0;
	pad->buttons[SELECT] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) != 0;
	pad->buttons[L_SHOULDER] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0;
	pad->buttons[R_SHOULDER] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0;
	pad->buttons[D_UP] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) != 0;
	pad->buttons[D_LEFT] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) != 0;
	pad->buttons[D_DOWN] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) != 0;
	pad->buttons[D_RIGHT] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) != 0;
}

LRESULT Input::OnDeviceChange(WPARAM eventType, LPARAM eventData)
{
	if (eventType != DBT_DEVICEARRIVAL &&
		eventType != DBT_DEVICEREMOVECOMPLETE)
		return TRUE;

	PDEV_BROADCAST_HDR deviceBroadcast = (PDEV_BROADCAST_HDR) eventData;
	if(deviceBroadcast->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
		return TRUE;

	PDEV_BROADCAST_DEVICEINTERFACE deviceInterface =
		(PDEV_BROADCAST_DEVICEINTERFACE) deviceBroadcast;
	switch(eventType)
	{
		case DBT_DEVICEARRIVAL:
		case DBT_DEVICEREMOVECOMPLETE:
			DetectControllers();
			break;
	}
	return TRUE;
}

void Input::OnInput(HRAWINPUT input)
{
	RAWINPUT raw;
	UINT dataSize = sizeof raw;

	GetRawInputData(input, RID_INPUT, &raw, &dataSize, sizeof(RAWINPUTHEADER));

	if(raw.header.dwType == RIM_TYPEMOUSE)
	{
		if(raw.data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
		{
			mouseDelta[0] = raw.data.mouse.lLastX;
			mouseDelta[1] = raw.data.mouse.lLastY;
		}

		bool buttons[2];
		USHORT buttonFlags = raw.data.mouse.usButtonFlags;
		buttons[0] = buttonFlags & (~RI_MOUSE_LEFT_BUTTON_UP | RI_MOUSE_LEFT_BUTTON_DOWN);
		buttons[1] = buttonFlags & (~RI_MOUSE_RIGHT_BUTTON_UP | RI_MOUSE_RIGHT_BUTTON_DOWN);
	}
	else if(raw.header.dwType == RIM_TYPEKEYBOARD)
	{
		switch(raw.data.keyboard.Message)
		{
			case WM_KEYUP:
			case WM_KEYDOWN:
				break;
		}
	}
}

LRESULT CALLBACK Input::WindowProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uiMsg)
	{
		case WM_DEVICECHANGE:
			return Input::OnDeviceChange(wParam, lParam);

		// WM_INPUT requires DefWindowProc to be called after processing for cleanup
		case WM_INPUT:
			Input::OnInput((HRAWINPUT)lParam);
			break;
	}
	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

#endif

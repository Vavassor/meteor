#include "Input.h"

#if defined(__linux__)
#include "LinuxInput.h"
#endif

#include "InternalGlobals.h"

#include "utilities/Logging.h"
#include "utilities/Macros.h"

#if defined(_WIN32)
#include "DeviceGUID.h"

#include <XInput.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Dbt.h>
#include <Windows.h>
#endif

#if defined(X11)
#include "utilities/XLibUtils.h"

#define XK_LATIN1
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include <X11/Xlib.h>
#endif

#include <math.h>

namespace Input
{
	enum KeyMapping { L_TRIGGER, R_TRIGGER, A_DOWN, A_LEFT, A_RIGHT, A_UP, NUM_MAPPINGS };

	Controller controllers[5];
	int num_controllers = 1;

	int playerControllers[MAX_PLAYERS];

	unsigned short keyBindings[NUM_BUTTONS + NUM_MAPPINGS];

	int mousePosition[2];
	float mouseDelta[2];
	float mouseSensitivity = 16.0f;
	bool isMouseRelative = true;

#if defined(_WIN32)
	HWND hWnd;
	HDEVNOTIFY deviceNotification;
#endif

#if defined(X11)
	Display* display;
	Window window;
	Cursor invisibleCursor;
#endif

	void PollKeyboardAndMouse();
	void PollXInputGamepad(int index);
	void PollEvDevGamepad(int index);

	unsigned short GetScanCode(int virtualKey);
	void PollKeyboard(char* keyboardState);

#if defined(_WIN32)
	LRESULT OnDeviceChange(WPARAM eventType, LPARAM eventData);
	void OnInput(HRAWINPUT input);

	LRESULT CALLBACK WindowProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);
#endif
}

unsigned short Input::GetScanCode(int virtualKey)
{
#if defined(X11)
	return XKeysymToKeycode(display, virtualKey);
#endif
	return 0;
}

void Input::Initialize()
{
	for(int i = 0; i < MAX_PLAYERS; ++i)
		playerControllers[i] = 0;

	mousePosition[0] = mousePosition[1] = 0;

#if defined(_WIN32)
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
	hWnd = CreateWindowEx(WS_EX_APPWINDOW, classEx.lpszClassName, NULL, NULL,
		0, 0, 0, 0, NULL, NULL, instance, NULL);
	if(hWnd == NULL)
	{
		LOG_ISSUE("CreateWindowEx failed!");
		return;
	}

	// set up notification for detecting gamepads
	DEV_BROADCAST_DEVICEINTERFACE filter = {};
	filter.dbcc_size = sizeof filter;
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	filter.dbcc_classguid = GUID_DEVINTERFACE_HID;

	deviceNotification = RegisterDeviceNotification(hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
	if(deviceNotification == NULL)
	{
		Log::Add(Log::INFO, "Registering device notification for detecting gamepads failed!");
	}

	// set up Raw Input for mouse and keyboard controls
	{
		RAWINPUTDEVICE devices[2];

		devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
		devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
		devices[0].dwFlags = RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE;
		devices[0].hwndTarget = hWnd;

		devices[1].usUsagePage = HID_USAGE_PAGE_GENERIC;
		devices[1].usUsage = HID_USAGE_GENERIC_KEYBOARD;
		devices[1].dwFlags = RIDEV_NOLEGACY | RIDEV_NOHOTKEYS;
		devices[1].hwndTarget = hWnd;

		BOOL registered = RegisterRawInputDevices(devices, ARRAY_LENGTH(devices), sizeof(RAWINPUTDEVICE));
		if(registered == FALSE)
		{
			Log::Add(Log::INFO, "Registering keyboard and/or mouse as Raw Input devices failed!");
		}
	}

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

#elif defined(__linux__)
	RegisterMonitor();
#endif

#if defined(X11)
	display = XOpenDisplay(NULL);

	window = DefaultRootWindow(display);

	// create transparent cursor for when mouse is in relative-movement mode
	{
		static char noData[] = { 0 };
		XColor black = { 0 };
		Pixmap blankBitmap = XCreateBitmapFromData(display, window, noData, 1, 1);

		invisibleCursor = XCreatePixmapCursor(display, blankBitmap, blankBitmap,
			&black, &black, 0, 0);
		XFreePixmap(display, blankBitmap);
	}

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
#if defined(_WIN32)
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

		RegisterRawInputDevices(devices, ARRAY_LENGTH(devices), sizeof(RAWINPUTDEVICE));
	}

	UnregisterDeviceNotification(deviceNotification);

	DestroyWindow(hWnd);

#elif defined(__linux__)
	UnregisterMonitor();
#endif

#if defined(X11)
	XFreeCursor(display, invisibleCursor);

	XCloseDisplay(display);
#endif
}

void Input::GetMousePosition(int point[2])
{
	point[0] = mousePosition[0];
	point[1] = mousePosition[1];
}

void Input::SetMouseMode(bool relative)
{
	if(relative != isMouseRelative)
	{
#if defined(_WIN32)
		if(relative)
		{
			// capture mouse input so that stray clicks don't make the program lose focus
			RAWINPUTDEVICE devices[1];
			devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
			devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
			devices[0].dwFlags = RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE;
			devices[0].hwndTarget = hWnd;
			RegisterRawInputDevices(devices, ARRAY_LENGTH(devices),
				sizeof(RAWINPUTDEVICE));

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
			RegisterRawInputDevices(devices, ARRAY_LENGTH(devices),
				sizeof(RAWINPUTDEVICE));

			// re-register mouse for raw input without the capture
			devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
			devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
			devices[0].dwFlags = 0;
			devices[0].hwndTarget = hWnd;
			RegisterRawInputDevices(devices, ARRAY_LENGTH(devices),
				sizeof(RAWINPUTDEVICE));

			// show cursor
			//SetCursor(oldCursor);
			ShowCursor(TRUE);
		}
#endif
	}
	isMouseRelative = relative;
}

Input::Controller* Input::GetController(PlayerSlot slot)
{
	if(slot >= num_controllers) return nullptr;
	return &controllers[playerControllers[slot]];
}

#if defined(_WIN32)
void Input::DetectDevices()
{
	numControllers = 1;

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
#endif

void Input::Poll()
{
	// poll input window messages
#if defined(_WIN32)
	MSG msg = {};
	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if(msg.message == WM_QUIT) break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

#elif defined(__linux__)
	CheckMonitor();
#endif

	// input device handling
	for(int i = 0; i < num_controllers; ++i)
	{
		Controller& pad = controllers[i];

		switch(pad.type)
		{
			case KEYBOARD_AND_MOUSE:
				PollKeyboardAndMouse(); break;
			case GAMEPAD_XINPUT:
				PollXInputGamepad(i); break;
			case GAMEPAD_EVDEV:
				PollEvDevGamepad(i); break;
		}

		pad.Update();
	}

	// mouse position handling
#if defined(WIN32)
	POINT mouseScreenPosition;
	if(GetCursorPos(&mouseScreenPosition))
	{
		mousePosition[0] = mouseScreenPosition.x;
		mousePosition[1] = mouseScreenPosition.y;
	}
#elif defined(X11)
	{
		Window root, child;
		int rootCoords[2];
		int windowCoords[2];
		unsigned int mask;

		XQueryPointer(display, window, &root, &child,
			&rootCoords[0], &rootCoords[1], &windowCoords[0], &windowCoords[1], &mask);

		mousePosition[0] = windowCoords[0];
		mousePosition[1] = windowCoords[1];
	}
#endif
}

void Input::PollKeyboard(char* keyboardState)
{
#if defined(_WIN32)
	GetKeyboardState((PBYTE) keyboardState);
#elif defined(X11)
	XQueryKeymap(display, keyboardState);
#endif
}

#if defined(_WIN32)
#define GET_KEY_STATE(code, keyboard) keyboard[(code)] & 0x80;
#elif defined(X11)
#define GET_KEY_STATE(code, keyboard) keyboard[(code) >> 3] >> ((code) & 0x07) & 0x01;
#endif

void Input::PollKeyboardAndMouse()
{
	// set controller to defaults before polling
	Controller& controller = controllers[0];

	for(int i = 0; i < NUM_BUTTONS; ++i)
		controller.buttons[i] = false;

	controller.leftAnalog[0] = controller.rightAnalog[0] = 0.0f;
	controller.leftAnalog[1] = controller.rightAnalog[1] = 0.0f;
	controller.leftTrigger = controller.rightTrigger = 0.0f;

	// poll keyboard and mouse buttons
	char keys[256];
	PollKeyboard(keys);

	bool buttonsPressed[NUM_BUTTONS];
	for(int i = 0; i < NUM_BUTTONS; ++i)
		buttonsPressed[i] = GET_KEY_STATE(keyBindings[i], keys);

	bool mappingsPressed[NUM_MAPPINGS];
	for(int i = 0; i < NUM_MAPPINGS; ++i)
		mappingsPressed[i] = GET_KEY_STATE(keyBindings[NUM_BUTTONS + i], keys);

	// update controller and buttons with polled values
	for(int i = 0; i < NUM_BUTTONS; ++i)
		controller.buttons[i] = buttonsPressed[i];

	if(mappingsPressed[L_TRIGGER])	controller.leftTrigger = 1.0f;
	if(mappingsPressed[R_TRIGGER])	controller.rightTrigger = 1.0f;

	float dx = 0.0f, dy = 0.0f;
	if(mappingsPressed[A_LEFT])		dx -= 1.0f;
	if(mappingsPressed[A_RIGHT])	dx += 1.0f;
	if(mappingsPressed[A_UP])		dy += 1.0f;
	if(mappingsPressed[A_DOWN])		dy -= 1.0f;

	float magnitude = sqrt(dx * dx + dy * dy);
	if(magnitude > 0.0f)
	{
		controller.leftAnalog[0] = dx / magnitude;
		controller.leftAnalog[1] = dy / magnitude;
	}

	controller.rightAnalog[0] = mouseDelta[0] / mouseSensitivity;
	controller.rightAnalog[1] = mouseDelta[1] / mouseSensitivity;

	mouseDelta[0] = mouseDelta[1] = 0.0f;
}

void Input::PollXInputGamepad(int index)
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

void Input::PollEvDevGamepad(int index)
{
#if defined(__linux__)
	PollDevice(index - 1, &controllers[index]);
#endif
}

#if defined(_WIN32)

LRESULT Input::OnDeviceChange(WPARAM eventType, LPARAM eventData)
{
	if (eventType != DBT_DEVICEARRIVAL &&
		eventType != DBT_DEVICEREMOVECOMPLETE)
		return TRUE;

	PDEV_BROADCAST_HDR deviceBroadcast = (PDEV_BROADCAST_HDR) eventData;
	if(deviceBroadcast->dbch_devicetype != DBT_DEVTYP_DEVICEINTERFACE)
		return TRUE;

	PDEV_BROADCAST_DEVICEINTERFACE deviceInterface = (PDEV_BROADCAST_DEVICEINTERFACE) deviceBroadcast;
	switch(eventType)
	{
		case DBT_DEVICEARRIVAL:
		case DBT_DEVICEREMOVECOMPLETE:
			DetectDevices();
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

#endif // defined(_WIN32)

bool Input::Controller::GetButtonDown(Button button) const
{
	return buttonStates[button] == PRESSED
		|| buttonStates[button] == ONCE;
}

bool Input::Controller::GetButtonReleased(Button button) const
{
	return buttonStates[button] == RELEASED;
}

bool Input::Controller::GetButtonTapped(Button button) const
{
	return buttonStates[button] == ONCE;
}

void Input::Controller::Update()
{
	for(int i = 0; i < NUM_BUTTONS; ++i)
	{
		if(buttons[i])
			buttonStates[i] = (buttonStates[i] == RELEASED) ? ONCE : PRESSED;
		else
			buttonStates[i] = RELEASED;
	}
}

#include "Input.h"

#if defined(_WIN32)
#include "WindowsInput.h"
#elif defined(__linux__)
#include "LinuxInput.h"
#endif

#include "InternalGlobals.h"

#include "utilities/Logging.h"
#include "utilities/Macros.h"

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"
#endif

#if defined(X11)
#include "utilities/XLibUtils.h"

#define XK_LATIN1
#define XK_MISCELLANY
#include <X11/keysymdef.h>
#include <X11/Xlib.h>
#endif

#include <math.h>

#define MAX_CONTROLLERS 5

namespace Input
{
	enum KeyMapping { L_TRIGGER, R_TRIGGER, A_DOWN, A_LEFT, A_RIGHT, A_UP, NUM_MAPPINGS };

	Controller controllers[MAX_CONTROLLERS];
	int numControllers = 1;

	int playerControllers[MAX_PLAYERS];

	unsigned short keyBindings[NUM_BUTTONS + NUM_MAPPINGS];

	int mousePosition[2];
	bool isMouseRelative = true;

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
	InitializeWindow();

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

	DetectControllers();
}

void Input::Terminate()
{
#if defined(_WIN32)
	TerminateWindow();
#elif defined(__linux__)
	UnregisterMonitor();
#endif

#if defined(X11)
	XFreeCursor(display, invisibleCursor);

	XCloseDisplay(display);
#endif
}

void Input::DetectControllers()
{
	ControllerType types[MAX_CONTROLLERS];
	numControllers = DetectDevices(types);
	for(int i = 0; i < numControllers; ++i)
		controllers[i].type = types[i];
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
		CaptureMouse(relative);
#endif
	}
	isMouseRelative = relative;
}

Input::Controller* Input::GetController(PlayerSlot slot)
{
	if(slot >= numControllers) return nullptr;
	return &controllers[playerControllers[slot]];
}

void Input::Poll()
{
	// poll input window messages
#if defined(_WIN32)
	MessageLoop();

#elif defined(__linux__)
	CheckMonitor();
#endif

	// input device handling
	for(int i = 0; i < numControllers; ++i)
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

	if(mappingsPressed[L_TRIGGER]) controller.leftTrigger = 1.0f;
	if(mappingsPressed[R_TRIGGER]) controller.rightTrigger = 1.0f;

	float dx = 0.0f, dy = 0.0f;
	if(mappingsPressed[A_LEFT])  dx -= 1.0f;
	if(mappingsPressed[A_RIGHT]) dx += 1.0f;
	if(mappingsPressed[A_UP])    dy += 1.0f;
	if(mappingsPressed[A_DOWN])  dy -= 1.0f;

	float magnitude = sqrt(dx * dx + dy * dy);
	if(magnitude > 0.0f)
	{
		controller.leftAnalog[0] = dx / magnitude;
		controller.leftAnalog[1] = dy / magnitude;
	}

	PollMouse(&controller);
}

void Input::PollXInputGamepad(int index)
{
#if defined(_WIN32)
	PollXInputDevice(index - 1, &controllers[index]);
#endif
}

void Input::PollEvDevGamepad(int index)
{
#if defined(__linux__)
	PollDevice(index - 1, &controllers[index]);
#endif
}

bool Input::Controller::GetButtonDown(Button button) const
{
	return buttonStates[button] == PRESSED || buttonStates[button] == ONCE;
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

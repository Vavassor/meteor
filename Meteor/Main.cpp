#include "Main.h"

#include <stddef.h>
#include <wchar.h>

#include "resource.h"

// open and save file dialogs
#include <string>
#include <stdio.h>
#include <Commdlg.h>

// input devices
#include <Dbt.h>
#include "DeviceGUID.h"

// engine
#if defined(GRAPHICS_OPENGL)
#include "gl/GLRenderer.h"
#endif

#if defined(GRAPHICS_DIRECTX)
#include "dx/DXRenderer.h"
#endif

#include "GlobalInfo.h"
#include "Input.h"
#include "Sound.h"
#include "Game.h"

// utilities
#include "Textblock.h"
#include "UnicodeUtils.h"
#include "Logging.h"
#include "Macros.h"
#include "Benaphore.h"
#include "Timer.h"
#include "ThreadMessages.h"

const char* module_directory;
bool enable_capture_render_statistics;
float texture_anisotropy;

namespace
{
	static const int MAX_THREADS = 1;
	DWORD threadIDs[MAX_THREADS];
    HANDLE threads[MAX_THREADS];

	bool enableDebugging;
	ViewportData viewport;
}

Window::Window():
	paused(false),
	isFullscreen(false),
	isBorderless(false),
	enableVSync(true),
	isAltDown(false),

	hWnd(NULL),
	windowName(L"METEOR"),
	width(1280),
	height(720),
	samples(0),

	oldCursor(NULL),
	isCursorHidden(false),
	showFPS(false),
	mouseModeRelative(true),
	lastTickTime(0.0),

	deviceName(nullptr),
	renderMode(RENDER_GL),

	deviceNotification(NULL)
{
	wchar_t wcsFilePath[MAX_PATH];
	GetModuleFileName(GetModuleHandle(NULL), wcsFilePath, MAX_PATH);
	*(wcsrchr(wcsFilePath, '\\') + 1) = 0;

	char* filePath = new char[MAX_PATH];
	wcs_to_utf8(filePath, wcsFilePath, MAX_PATH);
	module_directory = filePath;

#if defined(_DEBUG)
	enableDebugging = true;
#else
	enableDebugging = false;
#endif

	enable_capture_render_statistics = false;
	texture_anisotropy = 16.0f;
}

Window::~Window()
{
	delete[] module_directory;
}

bool Window::Create(HINSTANCE hInstance)
{
	// reset log file so initialization errors can be recorded
	Log::Clear_File();

	// initialize defaults
	bool createForwardCompatibleContext = false;

	// load configuration file values
	{
		Textblock block;
		Textblock::LoadFromFile("main.conf", &block);

		if(block.HasAttribute("screenWidth"))
			width = block.GetAttributeAsInt("screenWidth");
		if(block.HasAttribute("screenHeight"))
			height = block.GetAttributeAsInt("screenHeight");
		if(block.HasAttribute("renderer"))
		{
			String& renderName = block.GetAttributeAsStrings("renderer")[0];
			if(renderName == "DIRECTX")	renderMode = RENDER_DX;
			if(renderName == "OPENGL")	renderMode = RENDER_GL;
		}
		if(block.HasAttribute("isFullscreen"))
			isFullscreen = block.GetAttributeAsBool("isFullscreen");
		if(block.HasAttribute("verticalSynchronization"))
			enableVSync = block.GetAttributeAsBool("verticalSynchronization");
		if(block.HasAttribute("textureAnisotropy"))
			texture_anisotropy = block.GetAttributeAsFloat("textureAnisotropy");
		if(block.HasChild("OpenGL"))
		{
			Textblock* glConf = block.GetChildByName("OpenGL");
			if(glConf->HasAttribute("createForwardCompatibleContext"))
				createForwardCompatibleContext = glConf->GetAttributeAsBool("createForwardCompatibleContext");
		}
		if(block.HasAttribute("enableDebugging"))
			enableDebugging = block.GetAttributeAsBool("enableDebugging");
	}

	// setup window class
	WNDCLASSEX classEx = {};
	classEx.cbSize = sizeof classEx;
	classEx.style = CS_HREDRAW | CS_VREDRAW;
	classEx.lpfnWndProc = WindowProc;
	classEx.hInstance = hInstance;
	classEx.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	classEx.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0);
	classEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	classEx.lpszClassName = L"HellitorWindowClass";
	classEx.cbWndExtra = sizeof(Window*);

	if(RegisterClassEx(&classEx) == 0)
	{
		Log::Add(Log::ERR, "%s", "RegisterClassEx failed!");
		return false;
	}

	// create the actual window
	DWORD style = WS_OVERLAPPEDWINDOW;
	hWnd = CreateWindowEx(WS_EX_APPWINDOW, classEx.lpszClassName, windowName, style, 
		0, 0, width, height, NULL, NULL, hInstance, NULL);
	if(hWnd == NULL)
	{
		Log::Add(Log::ERR, "%s", "CreateWindowEx failed!");
		return false;
	}
	SetWindowLongPtr(hWnd, 0, (LONG)this);

	// initialize graphics contexts
	HDC hDC = NULL;
	if((hDC = GetDC(hWnd)) == NULL)
	{
		Log::Add(Log::ERR, "%s", "GetDC failed!");
		return false;
	}

	#if defined(GRAPHICS_OPENGL)
	if(renderMode == RENDER_GL)
	{
		if(!GLRenderer::Initialize(hDC, createForwardCompatibleContext))
			return false;
		GLRenderer::SetVSync(enableVSync);
	}
	#endif

	#if defined(GRAPHICS_DIRECTX)
	if(renderMode == RENDER_DX)
	{
		if(!DXRenderer::Initialize(hWnd, isFullscreen, enableDebugging))
			return false;
		DXRenderer::SetVSync(enableVSync);
	}
	#endif

	// set up notification for detecting gamepads
	DEV_BROADCAST_DEVICEINTERFACE filter;
	ZeroMemory(&filter, sizeof filter);
	filter.dbcc_size = sizeof filter;
	filter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
	filter.dbcc_classguid = GUID_DEVINTERFACE_HID;

	deviceNotification = RegisterDeviceNotification(hWnd, &filter, DEVICE_NOTIFY_WINDOW_HANDLE);
	if(deviceNotification == NULL)
	{
		Log::Add(Log::INFO, "%s", "Registering device notification for detecting gamepads failed!");
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
			Log::Add(Log::INFO, "%s", "Registering keyboard and/or mouse as Raw Input devices failed!");
		}
	}

	// initialize everything else
	oldCursor = LoadCursor(hInstance, IDC_ARROW);
	ShowCursor(FALSE);
	SetCursor(NULL);

	DISPLAY_DEVICE dd = {};
	dd.cb = sizeof dd;
	BOOL gotGPUName = EnumDisplayDevices(NULL, 0, &dd, 0);
	if(gotGPUName)
	{
		char* gpuName = new char[sizeof dd.DeviceString];
		wcs_to_utf8(gpuName, dd.DeviceString, sizeof dd.DeviceString);
		deviceName = gpuName;
	}

	Sound::Initialize();

	lastTickTime = Timer::GetTime();

	return true;
}

void Window::Show(bool maximized)
{
	int desktopWidth = GetSystemMetrics(SM_CXSCREEN);
	int desktopHeight = GetSystemMetrics(SM_CYSCREEN);

	RECT wRect, cRect;
	GetWindowRect(hWnd, &wRect);
	GetClientRect(hWnd, &cRect);

	wRect.right += width - cRect.right;
	wRect.bottom += height - cRect.bottom;

	wRect.right -= wRect.left;
	wRect.bottom -= wRect.top;

	wRect.left = desktopWidth / 2 - wRect.right / 2;
	wRect.top = desktopHeight / 2 - wRect.bottom / 2;

	MoveWindow(hWnd, wRect.left, wRect.top, wRect.right, wRect.bottom, FALSE);
	ShowWindow(hWnd, (maximized) ? SW_MAXIMIZE : SW_SHOW);
}

void Window::ToggleFullscreen()
{
	if(isFullscreen)
	{
		//--- Exit Fullscreen ---//

		// set display settings back to default
		ChangeDisplaySettings(NULL, 0);

		// change window style back to bordered and load
		// window position/dimensions from before the program 
		// entered fullscreen
		SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(hWnd, &placement);
		SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE
			| SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		isBorderless = false;
	}
	else
	{
		//--- Enter Fullscreen ---//

		// save position/dimensions of window prior to entering full screen mode
		ZeroMemory(&placement, sizeof placement);
		placement.length = sizeof placement;
		GetWindowPlacement(hWnd, &placement);

		// get current monitor's screen size
		MONITORINFO mi = {};
		mi.cbSize = sizeof mi;
		GetMonitorInfo(MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &mi);

		// change window style and position/dimensions
		SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);
		SetWindowPos(hWnd, HWND_TOPMOST, 0, 0,
			mi.rcMonitor.right - mi.rcMonitor.left,
			mi.rcMonitor.bottom - mi.rcMonitor.top,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW);

		// set display settings to desired screen size and resolution
		DEVMODE newSettings = {};
		newSettings.dmSize = sizeof newSettings;
		newSettings.dmBitsPerPel = 32;
		newSettings.dmPelsWidth = mi.rcMonitor.right - mi.rcMonitor.left;
		newSettings.dmPelsHeight = mi.rcMonitor.bottom - mi.rcMonitor.top;
		newSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
		if(ChangeDisplaySettings(&newSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			Log::Add(Log::ERR, "%s", "Display Mode failed! Could not enter fullscreen mode");
		}

		isBorderless = true;
	}

	isFullscreen = !isFullscreen;
}

void Window::ToggleBorderlessMode()
{
	if(isFullscreen) return;

	LONG style = (isBorderless) ? WS_OVERLAPPEDWINDOW : WS_POPUP;
	SetWindowLongPtr(hWnd, GWL_STYLE, style);

	SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER 
		| SWP_FRAMECHANGED | SWP_SHOWWINDOW);

	isBorderless = !isBorderless;
}

void Window::SetMouseMode(bool relative)
{
	if(relative != mouseModeRelative)
	{
		if(relative)
		{
			// capture mouse input so that stray clicks don't make the program lose focus
			RAWINPUTDEVICE devices[1];
			devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
			devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
			devices[0].dwFlags = RIDEV_NOLEGACY | RIDEV_CAPTUREMOUSE;
			devices[0].hwndTarget = hWnd;
			RegisterRawInputDevices(devices, ARRAY_LENGTH(devices), sizeof(RAWINPUTDEVICE));

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
			RegisterRawInputDevices(devices, ARRAY_LENGTH(devices), sizeof(RAWINPUTDEVICE));

			// re-register mouse for raw input without the capture
			devices[0].usUsagePage = HID_USAGE_PAGE_GENERIC;
			devices[0].usUsage = HID_USAGE_GENERIC_MOUSE;
			devices[0].dwFlags = 0;
			devices[0].hwndTarget = hWnd;
			RegisterRawInputDevices(devices, ARRAY_LENGTH(devices), sizeof(RAWINPUTDEVICE));

			// show cursor
			SetCursor(oldCursor);
			ShowCursor(TRUE);
		}
	}
	mouseModeRelative = relative;
}

void Window::ThreadMessageLoop()
{
	Message message;
	while(Game::PumpMessage(message))
	{
		switch(message.type)
		{

		}
	}
}

void Window::Update()
{
	// poll message queue
	ThreadMessageLoop();

	// update frame-rate counters
	static DWORD lastFPSTime = GetTickCount();
	static int fps = 0;
	static int fpsSample = 0;

	DWORD time = GetTickCount();

	if(time - lastFPSTime > 1000)
	{
		wchar_t out[64];
		wsprintf(out, L"%s - %i FPS", windowName, fpsSample);
		SetWindowText(hWnd, out);

		bool printToConsole = enableDebugging;
		Log::Write(printToConsole);

		fpsSample = fps;
		lastFPSTime = time;
		fps = 0;
	}
	else
	{
		fps++;
	}
	Log::Inc_Time();

	// Update render data
	{
		CameraData cameraData;
		Game::GetCameraData(&cameraData);
		#if defined(GRAPHICS_OPENGL)
		if(renderMode == RENDER_GL)
		{
			GLRenderer::SetCameraState(cameraData);
		}
		#endif
	}

	// Render
	#if defined(GRAPHICS_OPENGL)
	if(renderMode == RENDER_GL)
	{
		GLRenderer::Render();
	}
	#endif

	#if defined(GRAPHICS_DIRECTX)
	if(renderMode == RENDER_DX)
	{
		DXRenderer::Render();
	}
	#endif

	// Update Systems
	Sound::Update();
}

LRESULT Window::OnGainedFocus()
{
	paused = false;

	SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	return 0;
}

LRESULT Window::OnLostFocus()
{
	paused = true;

	SetCursor(oldCursor);
	SetWindowPos(hWnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

	return 0;
}

LRESULT Window::OnSize(int dimX, int dimY)
{
	width = dimX;
	height = dimY;

	#if defined(GRAPHICS_OPENGL)
	if(renderMode == RENDER_GL)
	{
		GLRenderer::Resize(dimX, dimY);
	}
	#endif

	#if defined(GRAPHICS_DIRECTX)
	if(renderMode == RENDER_DX)
	{
		DXRenderer::Resize(dimX, dimY);
	}
	#endif

	viewport.width = dimX;
	viewport.height = dimY;
	Game::GiveMessage(MESSAGE_RESIZE, &viewport, sizeof viewport);

	return 0;
}

LRESULT Window::OnDeviceChange(WPARAM eventType, LPARAM eventData)
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
		{
			Input::DetectDevices();
			break;
		}
	}
	return TRUE;
}

void Window::OnInput(HRAWINPUT input)
{
	RAWINPUT raw;
	UINT dataSize = sizeof raw;

	GetRawInputData(input, RID_INPUT, &raw, &dataSize, sizeof(RAWINPUTHEADER));

	if(raw.header.dwType == RIM_TYPEMOUSE)
	{
		if(raw.data.mouse.usFlags == MOUSE_MOVE_RELATIVE)
		{
			MouseData mouse = {};
			mouse.delta[0] = raw.data.mouse.lLastX;
			mouse.delta[1] = raw.data.mouse.lLastY;
			Game::GiveMessage(MESSAGE_MOUSE, &mouse, sizeof mouse);
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
			case WM_KEYDOWN:	KeyDown(raw.data.keyboard.VKey);	break;
			case WM_KEYUP:		KeyUp(raw.data.keyboard.VKey);		break;
		}
	}
}

void Window::KeyDown(USHORT key)
{
	switch(key)
	{
		case VK_MENU:	isAltDown = true;					break;
		case VK_ESCAPE:	SetMouseMode(!mouseModeRelative);	break;
		case VK_F2:		ToggleBorderlessMode();				break;
		case VK_F11:	ToggleFullscreen();					break;
		case VK_RETURN:	if(isAltDown) ToggleFullscreen();	break;
	}
}

void Window::KeyUp(USHORT key)
{
	switch(key)
	{
		case VK_MENU:	isAltDown = false;	break;
		case VK_F4:
		{
			if(isAltDown)
				PostMessage(hWnd, WM_DESTROY, 0, 0);
			break;
		}
	}
}

void Window::MessageLoop()
{
	MSG msg = {};
	bool quit = false;
	while(!quit)
	{
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT) quit = true;
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		
		Update();

		const double oneFrameLimit = 1000.0 / 60.0;
		double timeLeft = Timer::GetTime() - lastTickTime;
		if(timeLeft >= oneFrameLimit || enableVSync)
		{
			Game::Signal();
			lastTickTime = Timer::GetTime();
		}
	}
}

void Window::Destroy()
{
	OnLostFocus();

	Sound::Terminate();

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

	delete[] deviceName;

	#if defined(GRAPHICS_OPENGL)
	if(renderMode == RENDER_GL)
	{
		GLRenderer::Terminate();
	}
	#endif

	#if defined(GRAPHICS_DIRECTX)
	if(renderMode == RENDER_DX)
	{
		DXRenderer::Terminate();
	}
	#endif
}

// ----------------------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK Window::WindowProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	Window* window = (Window*) GetWindowLongPtr(hWnd, 0);
	if(window == NULL)
		return DefWindowProc(hWnd, uiMsg, wParam, lParam);

	switch(uiMsg)
	{
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

		case WM_ACTIVATE:
			if(LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE) 
				return window->OnGainedFocus();
			else
				return window->OnLostFocus();

		case WM_SIZE:
			return window->OnSize(LOWORD(lParam), HIWORD(lParam));

		case WM_DEVICECHANGE:
			return window->OnDeviceChange(wParam, lParam);
			
		// WM_INPUT requires DefWindowProc to be called after processing for cleanup
		case WM_INPUT:
			window->OnInput((HRAWINPUT)lParam);
			break;
	}
	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

#define EXCEPTION_CASE(code)		\
	case code:						\
	exceptionString = L"" #code;	\
	break;

LONG WINAPI UnhandledException(LPEXCEPTION_POINTERS exceptionInfo)
{
    const wchar_t* exceptionString = NULL;
	switch(exceptionInfo->ExceptionRecord->ExceptionCode)
	{
		EXCEPTION_CASE(EXCEPTION_ACCESS_VIOLATION);
		EXCEPTION_CASE(EXCEPTION_DATATYPE_MISALIGNMENT);
		EXCEPTION_CASE(EXCEPTION_BREAKPOINT);
		EXCEPTION_CASE(EXCEPTION_SINGLE_STEP);
		EXCEPTION_CASE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED);
		EXCEPTION_CASE(EXCEPTION_FLT_DENORMAL_OPERAND);

		case 0xE06D7363:
			exceptionString = L"C++ exception (using throw)";
			break;

		default:
			exceptionString = L"Unknown exception";
			break;
	}

	wchar_t message[255];
	DWORD codeBase = (DWORD)GetModuleHandle(NULL);
	swprintf_s<255>(
		message,
		L"An exception occurred which wasn't handled!\nCode: %s (0x%08X)\nOffset: 0x%08X\nCodebase: 0x%08X",
		exceptionString,
		exceptionInfo->ExceptionRecord->ExceptionCode,
		(DWORD)exceptionInfo->ExceptionRecord->ExceptionAddress - codeBase,
		codeBase);

	MessageBox(0, message, L"Error!", MB_OK);
	LONG filterMode = (enableDebugging) ? EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER;
    return filterMode;
}

#if defined(_MSC_VER)
__declspec(noreturn) __declspec(nothrow) void termination_handler()
{
	// Abnormal program termination (terminate() function was called)
	try
	{

	}
	catch(...)
	{
		// ignore!
	}
	_exit(EXIT_FAILURE);
}
#endif

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR sCmdLine, int iShow)
{
	SetUnhandledExceptionFilter(UnhandledException);

#if defined(_MSC_VER)
	set_terminate(termination_handler);
#endif

	// create threads
	LPTHREAD_START_ROUTINE threadRoutines[MAX_THREADS] =
	{
		Game::Main,
	};
	for(int i = 0; i < MAX_THREADS; i++)
		threads[i] = CreateThread(NULL, 0, threadRoutines[i], NULL, 0, &threadIDs[i]);

	// create window and begin render loop
	Window window;
	if(window.Create(hInstance))
	{
		if(window.isFullscreen)
			window.ToggleFullscreen();
		window.Show();
		window.MessageLoop();
	}
	else
	{
		MessageBoxA(NULL, Log::GetText(), "Error", MB_OK | MB_ICONERROR);
	}

	// window shutdown
	window.Destroy();

	// thread shutdown
	void (*threadExitRoutines[MAX_THREADS])() =
	{
		Game::Quit,
	};
	for(int i = 0; i < MAX_THREADS; i++)
		threadExitRoutines[i]();

	WaitForMultipleObjects(MAX_THREADS, threads, TRUE, INFINITE);
	for(int i = 0; i < MAX_THREADS; i++)
		CloseHandle(threads[i]);

	return 0;
}

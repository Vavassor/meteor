#include "WindowsWindow.h"

#include "resource.h"

#include "../utilities/Textblock.h"
#include "../utilities/Unicode.h"
#include "../utilities/Logging.h"
#include "../utilities/Macros.h"
#include "../utilities/Timer.h"
#include "../utilities/concurrent/Benaphore.h"
#include "../utilities/input/Input.h"

// engine
#include "../Sound.h"
#include "../Game.h"
#include "../ThreadMessages.h"

#include "../gl/GLRenderer.h"
#include "wgl_extensions.h"

#include <GL/wglext.h>

#if defined(_MSC_VER)
#include "../dx/DXRenderer.h"
#endif

// Windows
#include <direct.h>

// general
#include <cstdio>
#include <stddef.h>
#include <wchar.h>

namespace
{
	const char* module_directory;
	const char* working_directory;
	bool enable_capture_render_statistics;
	float texture_anisotropy;

	bool enable_debugging;
	ViewportData viewport;

	HGLRC context = NULL;
}

WindowsWindow::WindowsWindow():
	paused(false),
	fullscreen(false),
	borderless(false),
	vertical_synchronization(true),

	window(NULL),
	window_name("METEOR"),
	width(1280),
	height(720),

	device(NULL),
	device_name(nullptr),
	render_mode(RENDER_GL),

	old_cursor(NULL),
	last_tick_time(0.0),
	alt_pressed(false)
{
	// get module directory
	{
		wchar_t wide_path[MAX_PATH];
		if(GetModuleFileNameW(NULL, wide_path, ARRAY_COUNT(wide_path)))
		{
			wchar_t* slash = 1 + wcsrchr(wide_path, '\\');
			if(slash) *slash = L'\0';
		}

		char* file_path = new char[MAX_PATH];
		wcs_to_utf8(file_path, wide_path, MAX_PATH);
		module_directory = file_path;
	}
	
	// get current working directory
	{
		char* utf8_path = new char[MAX_PATH];
		working_directory = _getcwd(utf8_path, MAX_PATH);
	}
	
#if defined(_DEBUG)
	enable_debugging = true;
#else
	enable_debugging = false;
#endif

	enable_capture_render_statistics = false;
	texture_anisotropy = 16.0f;
}

WindowsWindow::~WindowsWindow()
{
	delete[] module_directory;
	delete[] working_directory;
}

bool WindowsWindow::Create(HINSTANCE instance)
{
	// initialize defaults
	bool create_forward_compatible_context = false;

	// load configuration file values
	{
		Textblock block;
		Textblock::Load_From_File("main.conf", &block);

		block.Get_Attribute_As_Int("window_width", &width);
		block.Get_Attribute_As_Int("window_height", &height);
		if(block.Has_Attribute("renderer"))
		{
			String* values;
			block.Get_Attribute_As_Strings("renderer", &values);

			String& renderName = values[0];
			if(renderName.Equals("DIRECTX")) render_mode = RENDER_DX;
			if(renderName.Equals("OPENGL"))  render_mode = RENDER_GL;
		}
		block.Get_Attribute_As_Bool("is_fullscreen", &fullscreen);
		block.Get_Attribute_As_Bool("vertical_synchronization", &vertical_synchronization);

		block.Get_Attribute_As_Float("texture_anisotropy", &texture_anisotropy);
		if(block.Has_Child("OpenGL"))
		{
			Textblock* glConf = block.Get_Child_By_Name("OpenGL");
			glConf->Get_Attribute_As_Bool("create_forward_compatible_context",
				&create_forward_compatible_context);
		}
		block.Get_Attribute_As_Bool("enable_debugging", &enable_debugging);
	}

	// setup window class
	WNDCLASSEXA classEx = {};
	classEx.cbSize = sizeof classEx;
	classEx.style = CS_HREDRAW | CS_VREDRAW;
	classEx.lpfnWndProc = WindowProc;
	classEx.hInstance = instance;
	classEx.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
	classEx.hIconSm = (HICON)LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1), IMAGE_ICON, 16, 16, 0);
	classEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	classEx.lpszClassName = "MeteorWindowClass";
	classEx.cbWndExtra = sizeof(WindowsWindow*);

	if(RegisterClassExA(&classEx) == 0)
	{
		LOG_ISSUE("RegisterClassEx failed!");
		return false;
	}

	// create the actual window
	window = CreateWindowExA(WS_EX_APPWINDOW, classEx.lpszClassName, window_name, WS_OVERLAPPEDWINDOW,
		0, 0, width, height, NULL, NULL, instance, NULL);
	if(window == NULL)
	{
		LOG_ISSUE("CreateWindowEx failed!");
		return false;
	}
	SetWindowLongPtr(window, 0, (LONG)this);

	// initialize graphics contexts
	device = NULL;
	if((device = GetDC(window)) == NULL)
	{
		LOG_ISSUE("GetDC failed!");
		return false;
	}

	// do opengl graphics initialization
	if(render_mode == RENDER_GL)
	{
		// setup pixel format
		PIXELFORMATDESCRIPTOR pfd = {};
		pfd.nSize = sizeof pfd;
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;
		pfd.cDepthBits = 24;
		pfd.iLayerType = PFD_MAIN_PLANE;

		int PixelFormat;
		if((PixelFormat = ChoosePixelFormat(device, &pfd)) == 0)
		{
			LOG_ISSUE("ChoosePixelFormat failed!");
			return false;
		}

		static int MSAAPixelFormat = 0;
		if(SetPixelFormat(device, (MSAAPixelFormat == 0)? PixelFormat : MSAAPixelFormat, &pfd) == FALSE)
		{
			LOG_ISSUE("SetPixelFormat failed!");
			return false;
		}

		// create context
		if((context = wglCreateContext(device)) == NULL)
		{
			LOG_ISSUE("wglCreateContext failed!");
			return false;
		}

		if(wglMakeCurrent(device, context) == FALSE)
		{
			LOG_ISSUE("wglMakeCurrent failed!");
			return false;
		}

		// get procedures from OpenGL .dll
		{
			int loaded = ogl_LoadFunctions();
			if(loaded == ogl_LOAD_FAILED)
			{
				int num_failed = loaded - ogl_LOAD_SUCCEEDED;
				LOG_ISSUE("ogl_LoadFunctions failed! %i procedures failed to load", num_failed);
				return false;
			}
		}

		// get wgl procedures
		{
			int loaded = wgl_LoadFunctions(device);
			if(loaded == wgl_LOAD_FAILED)
			{
				int num_failed = loaded - wgl_LOAD_SUCCEEDED;
				LOG_ISSUE("wgl_LoadFuntions failed: %i procedures didn't load", num_failed);
				return false;
			}
		}

		// get OpenGL version info
		int major = ogl_GetMajorVersion();
		int minor = ogl_GetMinorVersion();
		int glVersion = major * 10 + minor;

		// create forward compatible context if desired/possible
		if(create_forward_compatible_context && wgl_ext_ARB_create_context != wgl_LOAD_FAILED)
		{
			wglDeleteContext(context);

			int attribute_list[] =
			{
				WGL_CONTEXT_MAJOR_VERSION_ARB, major,
				WGL_CONTEXT_MINOR_VERSION_ARB, minor,
				WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
				0
			};

			if((context = wglCreateContextAttribsARB(device, 0, attribute_list)) == NULL)
			{
				LOG_ISSUE("wglCreateContextAttribsARB failed!");
				return false;
			}

			if(wglMakeCurrent(device, context) == FALSE)
			{
				LOG_ISSUE("wglMakeCurrent failed!");
				return false;
			}

			wgl_context_forward_compatible = true;
		}
		else
		{
			LOG_INFO("WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB is not set");
			wgl_context_forward_compatible = false;
		}

		{
			if(!GLRenderer::Initialize())
				return false;

			// set vertical synchronization
			if(wgl_ext_EXT_swap_control != wgl_LOAD_FAILED)
			{
				wglSwapIntervalEXT(vertical_synchronization ? 1 : 0);
			}
		}
	}

	// go ahead and initialize DirectX in the renderer, since DirectX is windows-only anyways
	#if defined(_MSC_VER)
	if(render_mode == RENDER_DX)
	{
		if(!DXRenderer::Initialize(window, fullscreen, enable_debugging))
			return false;
		DXRenderer::SetVSync(vertical_synchronization);
	}
	#endif

	// initialize everything else
	old_cursor = LoadCursor(instance, IDC_ARROW);
	ShowCursor(FALSE);
	SetCursor(NULL);

	DISPLAY_DEVICEW dd = {};
	dd.cb = sizeof dd;
	BOOL gotGPUName = EnumDisplayDevicesW(NULL, 0, &dd, 0);
	if(gotGPUName)
	{
		char* gpuName = new char[sizeof dd.DeviceString];
		wcs_to_utf8(gpuName, dd.DeviceString, sizeof dd.DeviceString);
		device_name = gpuName;
	}

	Sound::Initialize();
	Game::Start();

	last_tick_time = Timer::GetTime();

	return true;
}

void WindowsWindow::Show(bool maximized)
{
	int desktopWidth = GetSystemMetrics(SM_CXSCREEN);
	int desktopHeight = GetSystemMetrics(SM_CYSCREEN);

	RECT wRect, cRect;
	GetWindowRect(window, &wRect);
	GetClientRect(window, &cRect);

	wRect.right += width - cRect.right;
	wRect.bottom += height - cRect.bottom;

	wRect.right -= wRect.left;
	wRect.bottom -= wRect.top;

	wRect.left = desktopWidth / 2 - wRect.right / 2;
	wRect.top = desktopHeight / 2 - wRect.bottom / 2;

	MoveWindow(window, wRect.left, wRect.top, wRect.right, wRect.bottom, FALSE);
	ShowWindow(window, (maximized)? SW_MAXIMIZE : SW_SHOW);
}

void WindowsWindow::ToggleFullscreen()
{
	if(fullscreen)
	{
		//--- Exit Fullscreen ---//

		// set display settings back to default
		ChangeDisplaySettings(NULL, 0);

		// change window style back to bordered and load
		// window position/dimensions from before the program
		// entered fullscreen
		SetWindowLongPtr(window, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(window, &placement);
		SetWindowPos(window, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE
			| SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);

		borderless = false;
	}
	else
	{
		//--- Enter Fullscreen ---//

		// save position/dimensions of window prior to entering full screen mode
		CLEAR_STRUCT(placement);
		placement.length = sizeof placement;
		GetWindowPlacement(window, &placement);

		// get current monitor's screen size
		MONITORINFO mi = {};
		mi.cbSize = sizeof mi;
		GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &mi);

		// change window style and position/dimensions
		SetWindowLongPtr(window, GWL_STYLE, WS_POPUP);
		SetWindowPos(window, HWND_TOPMOST, 0, 0,
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
			LOG_ISSUE("Display Mode change failed! Could not enter fullscreen mode");
		}

		borderless = true;
	}

	fullscreen = !fullscreen;
}

void WindowsWindow::ToggleBorderlessMode()
{
	if(fullscreen) return;

	LONG style = (borderless)? WS_OVERLAPPEDWINDOW : WS_POPUP;
	SetWindowLongPtr(window, GWL_STYLE, style);

	SetWindowPos(window, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER 
		| SWP_FRAMECHANGED | SWP_SHOWWINDOW);

	borderless = !borderless;
}

void WindowsWindow::Update()
{
	// update frame-rate counters
	static DWORD last_fps_time = GetTickCount64();
	static int fps = 0;
	static int fps_sample = 0;

	DWORD time = GetTickCount64();

	if(time - last_fps_time > 1000)
	{
		char out[64];
		wsprintf(out, "%s - %i FPS", window_name, fps_sample);
		SetWindowTextA(window, out);

		bool print_to_console = enable_debugging;
		Log::Output(print_to_console);

		fps_sample = fps;
		last_fps_time = time;
		fps = 0;
	}
	else
	{
		fps++;
	}
	Log::Inc_Time();

	// Update Systems
	double delta_time = Timer::GetTime() - last_tick_time;
	last_tick_time = Timer::GetTime();

	Sound::Update();
	Game::Update(delta_time);

	// Update render data
	{
		CameraData cameraData = Game::Get_Camera_Data();
		if(render_mode == RENDER_GL)
		{
			GLRenderer::SetCameraState(cameraData);
		}
	}

	// Render
	if(render_mode == RENDER_GL)
	{
		GLRenderer::Render();
		SwapBuffers(device);
	}

	#if defined(_MSC_VER)
	if(render_mode == RENDER_DX)
	{
		DXRenderer::Render();
	}
	#endif
}

LRESULT WindowsWindow::OnGainedFocus()
{
	paused = false;

	SetWindowPos(window, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

	return 0;
}

LRESULT WindowsWindow::OnLostFocus()
{
	paused = true;

	SetCursor(old_cursor);
	SetWindowPos(window, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

	return 0;
}

LRESULT WindowsWindow::OnSize(int dimX, int dimY)
{
	width = dimX;
	height = dimY;

	if(render_mode == RENDER_GL)
	{
		GLRenderer::Resize(dimX, dimY);
	}

	#if defined(_MSC_VER)
	if(render_mode == RENDER_DX)
	{
		DXRenderer::Resize(dimX, dimY);
	}
	#endif

	viewport.width = dimX;
	viewport.height = dimY;

	return 0;
}

LRESULT WindowsWindow::KeyDown(USHORT key)
{
	switch(key)
	{
		case VK_MENU:   alt_pressed = true;                 break;
		case VK_F2:     ToggleBorderlessMode();             break;
		case VK_F11:    ToggleFullscreen();                 break;
		case VK_RETURN: if(alt_pressed) ToggleFullscreen(); break;
	}

	return 0;
}

LRESULT WindowsWindow::KeyUp(USHORT key)
{
	switch(key)
	{
		case VK_MENU: alt_pressed = false; break;
		case VK_F4:
		{
			if(alt_pressed)
				PostMessage(window, WM_DESTROY, 0, 0);
			break;
		}
	}

	return 0;
}

int WindowsWindow::MessageLoop()
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
	}

	return msg.wParam;
}

void WindowsWindow::Destroy()
{
	OnLostFocus();

	Sound::Terminate();
	Game::Quit();

	delete[] device_name;

	if(render_mode == RENDER_GL)
	{
		GLRenderer::Terminate();
		wglDeleteContext(context);
	}

	#if defined(GRAPHICS_DIRECTX)
	if(render_mode == RENDER_DX)
	{
		DXRenderer::Terminate();
	}
	#endif
}

// ----------------------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK WindowsWindow::WindowProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	WindowsWindow* window = (WindowsWindow*) GetWindowLongPtr(hWnd, 0);
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

		case WM_KEYDOWN:
			return window->KeyDown(wParam);

		case WM_KEYUP:
			return window->KeyUp(wParam);
	}
	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

#define W_(code)	L ## code

#define EXCEPTION_CASE(code)		\
	case code:						\
	exceptionString = W_(#code);	\
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
	swprintf(
		message,
		255,
		L"An exception occurred which wasn't handled!\nCode: %s (0x%08X)\nOffset: 0x%08X\nCodebase: 0x%08X",
		exceptionString,
		exceptionInfo->ExceptionRecord->ExceptionCode,
		(DWORD)exceptionInfo->ExceptionRecord->ExceptionAddress - codeBase,
		codeBase);

	MessageBoxW(0, message, L"Error!", MB_OK);
	LONG filterMode = (enable_debugging)? EXCEPTION_CONTINUE_SEARCH : EXCEPTION_EXECUTE_HANDLER;
    return filterMode;
}

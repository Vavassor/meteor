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
#include "../PlatformDefines.h"

#if defined(GRAPHICS_OPENGL)
#include "../gl/GLRenderer.h"
#include "wgl_extensions.h"

#include <GL/wglext.h>
#endif

#if defined(GRAPHICS_DIRECTX)
#include "../dx/DXRenderer.h"
#endif

// Windows
#include <direct.h>

// general
#include <cstdio>

namespace
{
	char* working_directory;
	bool enable_capture_render_statistics;
	float texture_anisotropy;

	bool enable_debugging;

	HGLRC context = NULL;
}

WindowsWindow::WindowsWindow():
	paused(false),
	fullscreen(false),
	vertical_synchronization(true),

	window(NULL),
	window_name("METEOR"),
	width(1280),
	height(720),

	device(NULL),
	device_name(nullptr),
	render_mode(RENDER_GL),

	last_tick_time(0.0),
	alt_pressed(false)
{
	// get current working directory
	{
		wchar_t wide_path[MAX_PATH];
		_wgetcwd(wide_path, MAX_PATH);
		utf16_to_utf8(reinterpret_cast<char16_t*>(wide_path), &working_directory);
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
	#if defined(GRAPHICS_OPENGL)
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
	#endif

	// go ahead and initialize DirectX in the renderer, since DirectX is windows-only anyways
	#if defined(GRAPHICS_DIRECTX)
	if(render_mode == RENDER_DX)
	{
		if(!DXRenderer::Initialize(window, fullscreen, enable_debugging))
			return false;
		DXRenderer::SetVSync(vertical_synchronization);
	}
	#endif

	// fetch display device name
	{
		DISPLAY_DEVICEW display = {};
		display.cb = sizeof display;
		if(EnumDisplayDevicesW(NULL, 0, &display, 0))
		{
			utf16_to_utf8(reinterpret_cast<char16_t*>(display.DeviceString), &device_name);
		}
	}

	// initialize systems and timer
	Sound::Initialize();
	Game::Start();

	Timer::Initialize();
	last_tick_time = Timer::Get_Time();

	return true;
}

void WindowsWindow::Show(int show_mode)
{
	MONITORINFO monitor = {};
	monitor.cbSize = sizeof monitor;
	GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &monitor);

	int desktop_width = monitor.rcMonitor.right - monitor.rcMonitor.left;
	int desktop_height = monitor.rcMonitor.bottom - monitor.rcMonitor.top;

	RECT window_rect, client_rect;
	GetWindowRect(window, &window_rect);
	GetClientRect(window, &client_rect);

	int border_width = window_rect.right + (width - client_rect.right);
	int border_height = window_rect.bottom + (height - client_rect.bottom);

	border_width -= window_rect.left;
	border_height -= window_rect.top;

	int x = desktop_width / 2 - border_width / 2;
	int y = desktop_height / 2 - border_height / 2;

	MoveWindow(window, x, y, border_width, border_height, FALSE);
	ShowWindow(window, show_mode);
}

void WindowsWindow::ToggleFullscreen()
{
	if(fullscreen)
	{
		//--- Exit Fullscreen ---//

		// load window position/dimensions/styles from before the program
		// entered fullscreen
		SetWindowLongPtr(window, GWL_STYLE, saved_info.style);
		SetWindowLongPtr(window, GWL_EXSTYLE, saved_info.ex_style);

		SetWindowPos(window, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE
			| SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
		SetWindowPlacement(window, &saved_info.placement);
	}
	else
	{
		//--- Enter Fullscreen ---//

		// save window position/dimensions and styles prior to entering full screen mode
		CLEAR_STRUCT(saved_info);
		saved_info.placement.length = sizeof saved_info.placement;
		GetWindowPlacement(window, &saved_info.placement);

		saved_info.style = GetWindowLongPtr(window, GWL_STYLE);
		saved_info.ex_style = GetWindowLongPtr(window, GWL_EXSTYLE);

		// change window style to remove all borders, frames, and bars
		SetWindowLongPtr(window, GWL_STYLE, saved_info.style & ~(WS_CAPTION | WS_THICKFRAME));
		SetWindowLongPtr(window, GWL_EXSTYLE, saved_info.ex_style & 
			~(WS_EX_DLGMODALFRAME | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

		// get current monitor's screen size
		MONITORINFO monitor = {};
		monitor.cbSize = sizeof monitor;
		GetMonitorInfo(MonitorFromWindow(window, MONITOR_DEFAULTTONEAREST), &monitor);

		int screen_x = monitor.rcMonitor.left;
		int screen_y = monitor.rcMonitor.top;
		int screen_width = monitor.rcMonitor.right - monitor.rcMonitor.left;
		int screen_height = monitor.rcMonitor.bottom - monitor.rcMonitor.top;

		// change window position/dimensions and flag the style change so windows
		// can take care of it
		SetWindowPos(window, HWND_TOPMOST, screen_x, screen_y, screen_width, screen_height,
			SWP_FRAMECHANGED | SWP_SHOWWINDOW);
	}

	fullscreen = !fullscreen;
}

void WindowsWindow::Update()
{
	Log::Inc_Time();

	// Update time counter
	double time = Timer::Get_Time();
	double delta_time = time - last_tick_time;
	last_tick_time = time;

	// Update Systems
	Sound::Update();
	Game::Update(delta_time);

	// Render
	#if defined(GRAPHICS_OPENGL)
	if(render_mode == RENDER_GL)
	{
		GLRenderer::Render();
		SwapBuffers(device);
	}
	#endif

	#if defined(GRAPHICS_DIRECTX)
	if(render_mode == RENDER_DX)
	{
		DXRenderer::Render();
	}
	#endif
}

LRESULT WindowsWindow::OnGainedFocus()
{
	paused = false;
	return 0;
}

LRESULT WindowsWindow::OnLostFocus()
{
	paused = true;
	return 0;
}

LRESULT WindowsWindow::OnSize(int dimX, int dimY)
{
	width = dimX;
	height = dimY;

	#if defined(GRAPHICS_OPENGL)
	if(render_mode == RENDER_GL)
	{
		GLRenderer::Resize(dimX, dimY);
	}
	#endif

	#if defined(GRAPHICS_DIRECTX)
	if(render_mode == RENDER_DX)
	{
		DXRenderer::Resize(dimX, dimY);
	}
	#endif

	return 0;
}

LRESULT WindowsWindow::OnKeyDown(USHORT key)
{
	switch(key)
	{
		case VK_MENU:   alt_pressed = true;                 break;
		case VK_F11:    ToggleFullscreen();                 break;
		case VK_RETURN: if(alt_pressed) ToggleFullscreen(); break;
	}

	return 0;
}

LRESULT WindowsWindow::OnKeyUp(USHORT key)
{
	switch(key)
	{
		case VK_MENU: alt_pressed = false; break;
		case VK_F4:
		{
			if(alt_pressed) DestroyWindow(window); break;
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

	#if defined(GRAPHICS_OPENGL)
	if(render_mode == RENDER_GL)
	{
		GLRenderer::Terminate();
		wglDeleteContext(context);
	}
	#endif

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
			return window->OnKeyDown(wParam);

		case WM_KEYUP:
			return window->OnKeyUp(wParam);
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

#include "utilities/Logging.h"

#include "PlatformDefines.h"
#include "Game.h"

#if defined(OS_WINDOWS)
#include "windows/WindowsWindow.h"

#if defined(_MSC_VER)
#include <stdlib.h>
#include <eh.h>
#endif

#elif defined(OS_LINUX)
#include "x11/X11Window.h"

#include <pthread.h>
#include <errno.h>
#include <string.h>
#endif

#if defined(_DEBUG)
#define PRINT_TO_CONSOLE true
#else
#define PRINT_TO_CONSOLE false
#endif

namespace
{
	static const int MAX_THREADS = 1;
}

#if defined(OS_WINDOWS)

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

int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR sCmdLine,
	_In_ int iShow)
{
	SetUnhandledExceptionFilter(UnhandledException);

#if defined(_MSC_VER)
	set_terminate(termination_handler);
#endif

	// reset log file so initialization errors can be recorded
	Log::Clear_File();

	// create window and begin render loop
	int main_return = 0;

	WindowsWindow window;
	if(window.Create(hInstance))
	{
		window.Show(iShow);
		if(window.fullscreen)
			window.ToggleFullscreen();
		main_return = window.MessageLoop();
	}
	else
	{
		MessageBoxA(NULL, Log::Get_Text(), "Error", MB_OK | MB_ICONERROR);
	}

	// window shutdown
	window.Destroy();

	// flush remaining log messages
	Log::Output(PRINT_TO_CONSOLE);

	return main_return;
}

#endif // defined(OS_WINDOWS)

#if defined(OS_LINUX)

int main(int argc, const char* argv[])
{
	// reset log file so initialization errors can be recorded
	Log::Clear_File();

	// startup window and begin render loop
	X11Window window;
	if(window.Create())
	{
		window.MessageLoop();
	}

	// window shutdown
	window.Destroy();

	// flush remaining log messages
	Log::Output(PRINT_TO_CONSOLE);

	return 0;
}

#endif // defined(OS_LINUX)

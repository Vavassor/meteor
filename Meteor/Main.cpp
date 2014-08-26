#if defined(_WIN32)

#include "windows/WindowsWindow.h"

#include "utilities/Logging.h"

#include "Game.h"

#if defined(_MSC_VER)
#include <stdlib.h>
#include <eh.h>
#endif

namespace
{
	static const int MAX_THREADS = 1;
	DWORD threadIDs[MAX_THREADS];
    HANDLE threads[MAX_THREADS];
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
	WindowsWindow window;
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

#endif // defined(_WIN32)

#if defined(X11)

#include "x11/X11Window.h"

int main(int argc, const char* argv[])
{
	X11Window window;
	if(window.Create())
	{
		window.MessageLoop();
	}

	window.Destroy();

	return 0;
}

#endif // defined(X11)

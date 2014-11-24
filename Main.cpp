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

#if defined(OS_WINDOWS)
	DWORD threadIDs[MAX_THREADS];
    HANDLE threads[MAX_THREADS];

#elif defined(OS_LINUX)
    pthread_t threads[MAX_THREADS];
#endif
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

typedef LPTHREAD_START_ROUTINE ThreadStartRoutine;
typedef void (*ThreadQuitRoutine)(void);

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

	// create threads
	ThreadStartRoutine start_routines[MAX_THREADS] =
	{
		Game::Main,
	};
	for(int i = 0; i < MAX_THREADS; ++i)
	{
		HANDLE thread = CreateThread(NULL, 0, start_routines[i], NULL, 0, &threadIDs[i]);
		if(thread == NULL)
		{
			MessageBoxA(NULL, "thread failed to spawn", "Error", MB_OK | MB_ICONERROR);
			return 0;
		}
		threads[i] = thread;
	}

	// create window and begin render loop
	int main_return = 0;

	WindowsWindow window;
	if(window.Create(hInstance))
	{
		if(window.fullscreen)
			window.ToggleFullscreen();
		window.Show();
		main_return = window.MessageLoop();
	}
	else
	{
		MessageBoxA(NULL, Log::Get_Text(), "Error", MB_OK | MB_ICONERROR);
	}

	// window shutdown
	window.Destroy();

	// thread shutdown
	ThreadQuitRoutine quit_routines[MAX_THREADS] =
	{
		Game::Quit,
	};
	for(int i = 0; i < MAX_THREADS; i++)
		quit_routines[i]();

	WaitForMultipleObjects(MAX_THREADS, threads, TRUE, INFINITE);
	for(int i = 0; i < MAX_THREADS; i++)
		CloseHandle(threads[i]);

	// flush remaining log messages
	Log::Output(PRINT_TO_CONSOLE);

	return main_return;
}

#endif // defined(OS_WINDOWS)

#if defined(OS_LINUX)

typedef void* (*Thread_Start_Routine)(void*);

int main(int argc, const char* argv[])
{
	// reset log file so initialization errors can be recorded
	Log::Clear_File();

	// thread startup
	Thread_Start_Routine startRoutines[MAX_THREADS] =
	{
		Game::Main,
	};
	for(int i = 0; i < MAX_THREADS; ++i)
	{
		int result = pthread_create(&threads[i], NULL, startRoutines[i], NULL);
		if(result)
		{
			LOG_ISSUE("thread could not start - return code: %s", strerror(result));
		}
	}

	// startup window and begin render loop
	X11Window window;
	if(window.Create())
	{
		window.MessageLoop();
	}

	// window shutdown
	window.Destroy();

	// thread shutdown
	void (*exitRoutines[MAX_THREADS])() =
	{
		Game::Quit,
	};
	for(int i = 0; i < MAX_THREADS; ++i)
		exitRoutines[i]();

	for(int i = 0; i < MAX_THREADS; ++i)
		pthread_join(threads[i], NULL);

	// flush remaining log messages
	Log::Output(PRINT_TO_CONSOLE);

	return 0;
}

#endif // defined(OS_LINUX)

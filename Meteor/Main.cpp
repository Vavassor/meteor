#include "utilities/Logging.h"

#include "Game.h"

#if defined(_WIN32)

#include "windows/WindowsWindow.h"

#if defined(_MSC_VER)
#include <stdlib.h>
#include <eh.h>
#endif

#elif defined(X11)

#include "x11/X11Window.h"

#include <pthread.h>
#include <errno.h>
#include <string.h>

#endif

namespace
{
	static const int MAX_THREADS = 1;

#if defined(_WIN32)
	DWORD threadIDs[MAX_THREADS];
    HANDLE threads[MAX_THREADS];

#elif defined(__unix__)
    pthread_t threads[MAX_THREADS];
#endif
}

#if defined(_WIN32)

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

typedef void* (*Thread_Start_Routine)(void*);

int main(int argc, const char* argv[])
{
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
			Log::Add(Log::ISSUE, "thread could not start - return code: %s", strerror(result));
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

	return 0;
}

#endif // defined(X11)

#ifndef WINDOWS_WINDOW_H
#define WINDOWS_WINDOW_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"

class WindowsWindow
{
public:
	bool paused, fullscreen, borderless, vertical_synchronization;

	WindowsWindow();
	~WindowsWindow();

	bool Create(HINSTANCE instance);
	void Show(bool maximized = false);
	int MessageLoop();
	void Destroy();

	void ToggleFullscreen();
	void ToggleBorderlessMode();
	LRESULT OnSize(int width, int height);
	LRESULT OnGainedFocus();
	LRESULT OnLostFocus();
	LRESULT KeyDown(USHORT key);
	LRESULT KeyUp(USHORT key);
	void Update();

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:
	enum RenderMode
	{
		RENDER_NONE,
		RENDER_GL,
		RENDER_DX,
	};

	HWND window;
	const char* window_name;
	int width, height;

	HDC device;
	const char* device_name;
	RenderMode render_mode;

	WINDOWPLACEMENT placement;
	HCURSOR old_cursor;
	double last_tick_time;
	bool alt_pressed;
};

// ----------------------------------------------------------------------------------------------------------------------------

LONG WINAPI UnhandledException(LPEXCEPTION_POINTERS exceptionInfo);
int WINAPI wWinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR sCmdLine,
	_In_ int iShow);

#endif

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
	void MessageLoop();
	void Destroy();

	void ToggleFullscreen();
	void ToggleBorderlessMode();
	void SetMouseMode(bool relative);
	LRESULT OnSize(int width, int height);
	LRESULT OnGainedFocus();
	LRESULT OnLostFocus();
	void KeyDown(USHORT key);
	void KeyUp(USHORT key);
	void ThreadMessageLoop();
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
	const wchar_t* window_name;
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
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR sCmdLine, int iShow);

#endif

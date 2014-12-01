#ifndef WINDOWS_WINDOW_H
#define WINDOWS_WINDOW_H

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "windows.h"

class WindowsWindow
{
public:
	bool paused, fullscreen, vertical_synchronization;

	WindowsWindow();
	~WindowsWindow();

	bool Create(HINSTANCE instance);
	void Show(int show_mode);
	int MessageLoop();
	void Destroy();

	void ToggleFullscreen();
	LRESULT OnSize(int width, int height);
	LRESULT OnGainedFocus();
	LRESULT OnLostFocus();
	LRESULT OnKeyDown(USHORT key);
	LRESULT OnKeyUp(USHORT key);
	void Update();

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam);

private:
	HWND window;
	const char* window_name;
	int width, height;

	HDC device;
	char* device_name;

	enum
	{
		RENDER_NONE,
		RENDER_GL,
		RENDER_DX,
	} render_mode;

	struct
	{
		LONG style;
		LONG ex_style;
		WINDOWPLACEMENT placement;
	} saved_info;

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

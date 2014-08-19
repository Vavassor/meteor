#ifndef MAIN_H
#define MAIN_H

#include <windows.h>

class Window
{
public:
	bool paused, isFullscreen, isBorderless, enableVSync, isAltDown;

	Window();
	~Window();

	bool Create(HINSTANCE hInstance);
	void Show(bool maximized = false);
	void MessageLoop();
	void Destroy();

	void ToggleFullscreen();
	void ToggleBorderlessMode();
	void SetMouseMode(bool relative);
	LRESULT OnSize(int width, int height);
	LRESULT OnGainedFocus();
	LRESULT OnLostFocus();
	LRESULT OnDeviceChange(WPARAM eventType, LPARAM eventData);
	void OnInput(HRAWINPUT input);
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

	HWND hWnd;
	const wchar_t* windowName;
	int width, height, samples;
	HCURSOR oldCursor;
	bool isCursorHidden, showFPS, mouseModeRelative;
	double lastTickTime;
	const char* deviceName;
	RenderMode renderMode;

	HDEVNOTIFY deviceNotification;

	WINDOWPLACEMENT placement;
};

// ----------------------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
LONG WINAPI UnhandledException(LPEXCEPTION_POINTERS exceptionInfo);
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR sCmdLine, int iShow);

#endif

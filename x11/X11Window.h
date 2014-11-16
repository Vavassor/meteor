#ifndef X11_WINDOW_H
#define X11_WINDOW_H

#include <X11/X.h>
#include <X11/Xlib.h>

class X11Window
{
public:
	Display* display;
	Window root;
	Window window;

	const char* name;
	int width, height;

	X11Window();
	~X11Window();
	bool Create();
	void Destroy();
	void MessageLoop();

private:
	enum RenderMode
	{
		RENDER_NONE,
		RENDER_GL,
	};

	RenderMode renderMode;
	bool enableVSync;
	bool fullscreen, borderless;

	bool enableDebugging;

	double lastTickTime;

	void ToggleBorderlessMode();
	void ToggleFullscreen();
	void Update();
	bool TranslateMessage(const XEvent& event);
	void OnSize(int dimX, int dimY);
	void OnKeyPress(KeySym keyCode, unsigned int modifierMask);
	void OnGainedFocus(int mode, int detail);
	void OnLostFocus(int mode, int detail);
};

// ------------------------------------------------------------------------------------------------

int XErrorFilter(Display* display, XErrorEvent* event);

#endif

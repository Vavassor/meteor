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
	void ToggleBorderlessMode();
	void ToggleFullscreen();
	void MessageLoop();
	void Update();
	bool TranslateMessage(const XEvent& event);
	void OnSize(int dimX, int dimY);
	void OnKeyPress(unsigned long keyCode, unsigned int modifierMask);

private:
	enum RenderMode
	{
		RENDER_NONE,
		RENDER_GL,
	};

	RenderMode renderMode;
	bool enableVSync;
	bool fullscreen, borderless;

	double lastTickTime;
};

#endif

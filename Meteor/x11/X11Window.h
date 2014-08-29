#ifndef X11_WINDOW_H
#define X11_WINDOW_H

#include <X11/X.h>
#include <X11/Xlib.h>

class X11Window
{
public:
	Display*	display;
	Window		root;
	Window		window;

	int width, height;

	X11Window();
	~X11Window();
	bool Create();
	void Destroy();
	void MessageLoop();
	bool TranslateMessage(const XEvent& event);
	void Update();
	void OnSize(int dimX, int dimY);

private:
	enum RenderMode
	{
		RENDER_NONE,
		RENDER_GL,
	};

	RenderMode renderMode;
	bool enableVSync;

	double lastTickTime;
};

#endif

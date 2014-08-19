#ifndef X11_WINDOW_H
#define X11_WINDOW_H

#include <X11/X.h>
#include <X11/Xlib.h>

#include <GL/gl.h>
#include <GL/glx.h>

class X11Window
{
public:
	Display*	display;
	Window		root;
	Window		window;
	GLXContext	glContext;

	bool Create();
	void Destroy();
	void MessageLoop();
};

#endif

#include "X11Window.h"

#include "Logging.h"

namespace
{
	Atom wmDeleteMessage;
}

bool X11Window::Create()
{
	display = XOpenDisplay(NULL);

	if(display == NULL)
	{
		Log::Add(Log::ERR, "%s", "\n\tcannot connect to X server\n");
		return false;
	}

	root = DefaultRootWindow(display);

	GLint visualAttributes[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	XVisualInfo* visual = glXChooseVisual(display, 0, visualAttributes);

	if(visual == NULL)
	{
		Log::Add(Log::ERR, "%s", "\n\tno appropriate visual found\n");
		return false;
	}
	else
	{
		Log::Add(Log::INFO, "%s%p%s", "\n\tvisual ", (void*) visual->visualid, "selected\n");
	}

	Colormap colorMap = XCreateColormap(display, root, visual->visual, AllocNone);

	XSetWindowAttributes windowAttributes = {};
	windowAttributes.colormap = colorMap;
	windowAttributes.event_mask =  ExposureMask | KeyPressMask;

 	window = XCreateWindow(display, root, 0, 0, 600, 600, 0,
 			visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &windowAttributes);

	XMapWindow(display, window);
	XStoreName(display, window, "METEOR");

	wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &wmDeleteMessage, 1);

	glContext = glXCreateContext(display, visual, NULL, GL_TRUE);
	glXMakeCurrent(display, window, glContext);

	return true;
}

void X11Window::MessageLoop()
{
	bool running = true;
	while(running)
	{
		XEvent event = {};
		XNextEvent(display, &event);

		switch(event.type)
		{
			case Expose:
			{
				glClearColor(1.0, 1.0, 1.0, 1.0);
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

				glBegin(GL_QUADS);
				{
					glColor3f(1., 0., 0.); glVertex3f(-.75, -.75, 0.);
					glColor3f(0., 1., 0.); glVertex3f( .75, -.75, 0.);
					glColor3f(0., 0., 1.); glVertex3f( .75,  .75, 0.);
					glColor3f(1., 1., 0.); glVertex3f(-.75,  .75, 0.);
				}
				glEnd();

				glXSwapBuffers(display, window);
				break;
			}
			case ClientMessage:
			{
				if((Atom)event.xclient.data.l[0] == wmDeleteMessage)
					running = false;
				break;
			}
		}
	}
}

void X11Window::Destroy()
{
	glXMakeCurrent(display, None, NULL);
	glXDestroyContext(display, glContext);

	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

int main(int argc, const char* argv[])
{
	X11Window window;
	if(window.Create())
	{
		window.MessageLoop();
	}

	window.Destroy();

	return 0;
}

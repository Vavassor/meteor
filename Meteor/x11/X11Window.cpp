#include "X11Window.h"

#include "utilities/Logging.h"
#include "utilities/Timer.h"
#include "utilities/Macros.h"

#include "GlobalInfo.h"
#include "ThreadMessages.h"
#include "Game.h"

#include "XPM.h"
#include "icon.xpm"

#if defined(GRAPHICS_OPENGL)
#include "gl/GLRenderer.h"

#include <GL/glxew.h>
#include <GL/glx.h>
#endif

#include <unistd.h>

#include <cstring>
#include <cstdio>

const char* module_directory;
float texture_anisotropy;

namespace
{
	Atom wmDeleteMessage;

	struct MotifHints {
		unsigned long flags;
		unsigned long functions;
		unsigned long decorations;
		long inputMode;
		unsigned long status;
	};

	Atom motifWMHints;
	Atom wmState;
	Atom wmStateFullscreen;

	ViewportData viewport;

#if defined(GRAPHICS_OPENGL)
	GLXContext	glContext;
#endif
}

X11Window::X11Window():
	display(nullptr),
	root(None),
	window(None),

	name("METEOR"),
	width(1280),
	height(720),

	renderMode(RENDER_GL),
	enableVSync(true),
	fullscreen(false),
	borderless(false),

	lastTickTime(0.0)
{
	char buffer[1024];
	ssize_t size = readlink("/proc/self/exe", buffer, sizeof buffer);
	if(size != -1)
	{
		char* pathEnd = strrchr(buffer, '/') + 1;
		size_t length = pathEnd - buffer;

		char* path = new char[length + 1];
		memcpy(path, buffer, length);
		path[length] = '\0';

		module_directory = path;
	}
}

X11Window::~X11Window()
{
	delete[] module_directory;
}

bool X11Window::Create()
{
	// reset log file so initialization errors can be recorded
	Log::Clear_File();

	display = XOpenDisplay(NULL);

	if(display == NULL)
	{
		Log::Add(Log::ISSUE, "Cannot connect to X server");
		return false;
	}

	root = DefaultRootWindow(display);

#if defined(GRAPHICS_OPENGL)
	GLint visualAttributes[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	XVisualInfo* visual = glXChooseVisual(display, 0, visualAttributes);

	if(visual == NULL)
	{
		Log::Add(Log::ISSUE, "No appropriate visual found");
		return false;
	}
	else
	{
		Log::Add(Log::INFO, "visual %u selected", (void*) visual->visualid);
	}
#endif

	Colormap colorMap = XCreateColormap(display, root, visual->visual, AllocNone);

	XSetWindowAttributes windowAttributes = {};
	windowAttributes.colormap = colorMap;
	windowAttributes.event_mask =  KeyPressMask | StructureNotifyMask;
 	window = XCreateWindow(display, root, 0, 0, width, height, 0,
 		visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &windowAttributes);

	XMapWindow(display, window);

	// give application name
	{
		XStoreName(display, window, name);

		Atom wmName = XInternAtom(display, "_NET_WM_NAME", False);
		Atom nameEncoding = XInternAtom(display, "UTF8_STRING", False);
		XChangeProperty(display, window, wmName, nameEncoding,
			8, PropModeReplace, (const unsigned char*) name, strlen(name));
	}

	wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &wmDeleteMessage, 1);

	// load application icon
	{
		unsigned long* buffer = x_pixmap::parse_image(icon_16x16_xpm, ARRAY_LENGTH(icon_16x16_xpm));

		Atom wmIcon = XInternAtom(display, "_NET_WM_ICON", False);
		Atom cardinal = XInternAtom(display, "CARDINAL", False);

		XChangeProperty(display, window, wmIcon, cardinal, 32,
			PropModeReplace, (const unsigned char*) buffer, 2 + 16 * 16);
	}

#if defined(GRAPHICS_OPENGL)
	glContext = glXCreateContext(display, visual, NULL, GL_TRUE);
	glXMakeCurrent(display, window, glContext);

	/*	glewExperimental needed for GL3.2+ forward-compatible context to prevent
		glewInit's call to glGetString(GL_EXTENSIONS) which can cause
		the error GL_INVALID_ENUM */
	glewExperimental = GL_TRUE;
	if(glewInit() != GLEW_OK)
	{
		Log::Add(Log::ISSUE, "glewInit failed!");
		return false;
	}

	if(!GLRenderer::Initialize())
		return false;

	// set vertical synchronization
	if(GLX_EXT_swap_control)
	{
		glXSwapIntervalEXT(display, glXGetCurrentDrawable(), enableVSync ? 1 : 0);
	}
#endif

	motifWMHints = XInternAtom(display, "_MOTIF_WM_HINTS", true);
	wmState = XInternAtom(display, "_NET_WM_STATE", False);
	wmStateFullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", True);

	lastTickTime = Timer::GetTime();

	return true;
}

void X11Window::Destroy()
{
#if defined(GRAPHICS_OPENGL)
	GLRenderer::Terminate();

	glXMakeCurrent(display, None, NULL);
	glXDestroyContext(display, glContext);
#endif

	XDestroyWindow(display, window);
	XCloseDisplay(display);
}

void X11Window::ToggleBorderlessMode()
{
	if(fullscreen) return;

	MotifHints hints = {};
	hints.flags = 2;
	hints.decorations = (borderless) ? 1 : 0;
	XChangeProperty(display, window, motifWMHints, motifWMHints, 32,
		PropModeReplace, (unsigned char*) &hints, 5);
	XMapWindow(display, window);

	borderless = !borderless;
}

void X11Window::ToggleFullscreen()
{
	XEvent event = {};
	event.xclient.type = ClientMessage;
	event.xclient.window = window;
	event.xclient.message_type = wmState;
	event.xclient.format = 32;
	event.xclient.data.l[0] = 2; // _NET_WM_STATE_TOGGLE
	event.xclient.data.l[1] = wmStateFullscreen;
	event.xclient.data.l[2] = 0;
	event.xclient.data.l[3] = 1;
	event.xclient.data.l[4] = 0;

	XSendEvent(display, DefaultRootWindow(display), False,
		SubstructureRedirectMask | SubstructureNotifyMask, &event);

	fullscreen = !fullscreen;
}

void X11Window::MessageLoop()
{
	bool running = true;
	while(running)
	{
		XEvent event = {};
		while(XPending(display) > 0)
		{
			XNextEvent(display, &event);
			if(!TranslateMessage(event))
				running = false;
		}

		Update();

		const double oneFrameLimit = 1000.0 / 60.0;
		double timeLeft = Timer::GetTime() - lastTickTime;
		if(timeLeft >= oneFrameLimit || enableVSync)
		{
			Game::Signal();
			lastTickTime = Timer::GetTime();
		}
	}
}

void X11Window::Update()
{
	Log::Inc_Time();

	// Update render data
	{
		CameraData cameraData;
		Game::GetCameraData(&cameraData);
		#if defined(GRAPHICS_OPENGL)
		if(renderMode == RENDER_GL)
		{
			GLRenderer::SetCameraState(cameraData);
		}
		#endif
	}

	#if defined(GRAPHICS_OPENGL)
	if(renderMode == RENDER_GL)
	{
		GLRenderer::Render();

		glXSwapBuffers(display, window);
	}
	#endif
}

bool X11Window::TranslateMessage(const XEvent& event)
{
	switch(event.type)
	{
		case ClientMessage:
		{
			if((Atom)event.xclient.data.l[0] == wmDeleteMessage)
				return false;
			break;
		}
		case ConfigureNotify:
		{
			XConfigureEvent configure = event.xconfigure;
			OnSize(configure.width, configure.height);
			break;
		}
		case KeyPress:
		{
			XKeyEvent k = event.xkey;
			OnKeyPress(XLookupKeysym(&k, 0), k.state);
			break;
		}
	}
	return true;
}

void X11Window::OnSize(int dimX, int dimY)
{
	width = dimX;
	height = dimY;

	#if defined(GRAPHICS_OPENGL)
	if(renderMode == RENDER_GL)
	{
		GLRenderer::Resize(dimX, dimY);
	}
	#endif

	viewport.width = dimX;
	viewport.height = dimY;
	Game::GiveMessage(MESSAGE_RESIZE, &viewport, sizeof viewport);
}

void X11Window::OnKeyPress(unsigned long keyCode, unsigned int modifierMask)
{
	switch(keyCode)
	{
		case XK_F2:	ToggleBorderlessMode(); break;
		case XK_F11:	ToggleFullscreen(); break;
	}
}

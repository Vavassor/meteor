#include "X11Window.h"

#include "utilities/Logging.h"
#include "utilities/Timer.h"

#include "GlobalInfo.h"
#include "ThreadMessages.h"
#include "Game.h"

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

	ViewportData viewport;

#if defined(GRAPHICS_OPENGL)
	GLXContext	glContext;
#endif
}

X11Window::X11Window():
	display(nullptr),
	root(None),
	window(None),

	width(1280),
	height(720),

	renderMode(RENDER_GL),
	enableVSync(true),

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
	XStoreName(display, window, "METEOR");

	wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(display, window, &wmDeleteMessage, 1);

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

	lastTickTime = Timer::GetTime();

	return true;
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
	}
	return true;
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

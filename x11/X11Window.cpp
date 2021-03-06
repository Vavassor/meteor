#include "X11Window.h"

#include "../utilities/Logging.h"
#include "../utilities/Timer.h"
#include "../utilities/Textblock.h"
#include "../utilities/Macros.h"

#include "../ThreadMessages.h"
#include "../Game.h"

#include "XPM.h"
#include "icon.xpm"

#if defined(GRAPHICS_OPENGL)
#include "../gl/GLRenderer.h"
#include "glx_extensions.h"

#include <GL/glx.h>
#endif

#include <unistd.h>

#include <cstring>
#include <cstdio>

namespace
{
	struct MotifHints
	{
		unsigned long flags;
		unsigned long functions;
		unsigned long decorations;
		long inputMode;
		unsigned long status;
	};

	Atom wmDeleteMessage;

	Atom motifWMHints;
	Atom wmState;
	Atom wmStateFullscreen;

	ViewportData viewport;

	const char* module_directory;
	const char* working_directory;
	float texture_anisotropy;

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
	// get module directory
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

	// get working directory
	{
		char* path = new char[1024];
		working_directory = getcwd(path, 1024);
	}

#if defined(_DEBUG)
	enableDebugging = true;
#else
	enableDebugging = false;
#endif
}

X11Window::~X11Window()
{
	delete[] module_directory;
	delete[] working_directory;
}

bool X11Window::Create()
{
	// handler for XLib errors
	XSetErrorHandler(XErrorFilter);

	bool createForwardCompatibleContext = false;

	// load configuration file values
	{
		Textblock block;
		Textblock::Load_From_File("main.conf", &block);

		block.Get_Attribute_As_Int("window_width", &width);
		block.Get_Attribute_As_Int("window_height", &height);

		if(block.Has_Attribute("renderer"))
		{
			String* values;
			block.Get_Attribute_As_Strings("renderer", &values);
			String& renderName = values[0];
			if(renderName == "OPENGL") renderMode = RENDER_GL;
		}

		block.Get_Attribute_As_Bool("is_fullscreen", &fullscreen);
		block.Get_Attribute_As_Bool("vertical_synchronization", &enableVSync);
		block.Get_Attribute_As_Float("texture_anisotropy", &texture_anisotropy);

		if(block.Has_Child("OpenGL"))
		{
			Textblock* glConf = block.Get_Child_By_Name("OpenGL");
			glConf->Get_Attribute_As_Bool("create_forward_compatible_context",
				&createForwardCompatibleContext);
		}

		block.Get_Attribute_As_Bool("enable_debugging", &enableDebugging);
	}

	display = XOpenDisplay(NULL);
	if(display == NULL)
	{
		LOG_ISSUE("Cannot connect to X server");
		return false;
	}

	root = DefaultRootWindow(display);

#if defined(GRAPHICS_OPENGL)
	GLint visualAttributes[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	XVisualInfo* visual = glXChooseVisual(display, 0, visualAttributes);
	if(visual == NULL)
	{
		LOG_ISSUE("No appropriate visual found for graphics context");
		return false;
	}
#endif

	Colormap colorMap = XCreateColormap(display, root, visual->visual, AllocNone);

	XSetWindowAttributes windowAttributes = {};
	windowAttributes.colormap = colorMap;
	windowAttributes.event_mask = KeyPressMask | KeyReleaseMask | StructureNotifyMask
		| FocusChangeMask;
 	window = XCreateWindow(display, root, 0, 0, width, height, 0,
 		visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &windowAttributes);

 	wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
 	XSetWMProtocols(display, window, &wmDeleteMessage, 1);

	// give application name
	{
		XStoreName(display, window, name);

		Atom wmName = XInternAtom(display, "_NET_WM_NAME", False);
		Atom nameEncoding = XInternAtom(display, "UTF8_STRING", False);
		XChangeProperty(display, window, wmName, nameEncoding,
			8, PropModeReplace, (const unsigned char*) name, strlen(name));
	}

	// load application icon
	{
		unsigned long* buffer = x_pixmap::parse_image(icon_16x16_xpm,
			ARRAY_COUNT(icon_16x16_xpm));

		Atom wmIcon = XInternAtom(display, "_NET_WM_ICON", False);
		Atom cardinal = XInternAtom(display, "CARDINAL", False);

		XChangeProperty(display, window, wmIcon, cardinal, 32,
			PropModeReplace, (const unsigned char*) buffer, 2 + 16 * 16);
	}

	XMapWindow(display, window);

#if defined(GRAPHICS_OPENGL)
	glContext = glXCreateContext(display, visual, NULL, True);
	if(glContext == nullptr)
	{
		LOG_ISSUE("could not create graphics context");
		return false;
	}

	Bool madeCurrent = glXMakeCurrent(display, window, glContext);
	if(!madeCurrent)
	{
		LOG_ISSUE("could not attach graphics context to window");
		return false;
	}

	// get procedures from OpenGL .dll
	{
		int loaded = ogl_LoadFunctions();
		if(loaded == ogl_LOAD_FAILED)
		{
			int num_failed = loaded - ogl_LOAD_SUCCEEDED;
			LOG_ISSUE("ogl_LoadFunctions failed! %i procedures failed to load", num_failed);
			return false;
		}
	}

	// get glx procedures
	{
		int loaded = glx_LoadFunctions(display, 0);
		if(loaded == glx_LOAD_FAILED)
		{
			int num_failed = loaded - glx_LOAD_SUCCEEDED;
			LOG_ISSUE("glx_LoadFuntions failed: %i procedures didn't load", num_failed);
			return false;
		}
	}

	if(!GLRenderer::Initialize())
		return false;

	// set vertical synchronization
	if(glx_ext_EXT_swap_control)
	{
		glXSwapIntervalEXT(display, glXGetCurrentDrawable(), enableVSync ? 1 : 0);
	}
#endif

	// go ahead and fetch Atoms for any later usage
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

	XCloseDisplay(display);
}

void X11Window::ToggleBorderlessMode()
{
	if(fullscreen) return;

	borderless = !borderless;

	MotifHints hints = {};
	hints.flags = 2;
	hints.decorations = (borderless) ? 0 : 1;
	XChangeProperty(display, window, motifWMHints, motifWMHints, 32,
		PropModeReplace, (unsigned char*) &hints, 5);
	XMapWindow(display, window);
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
	// update time counters
	static unsigned long lastFPSTime = Timer::GetMilliseconds();
	static int fps = 0;

	unsigned long time = Timer::GetMilliseconds();

	if(time - lastFPSTime > 1000)
	{
		bool printToConsole = enableDebugging;
		Log::Output(printToConsole);

		lastFPSTime = time;
		fps = 0;
	}
	else
	{
		++fps;
	}

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
			XClientMessageEvent client = event.xclient;
			if(client.data.l[0] == wmDeleteMessage)
				return false;
			break;
		}
		case ConfigureNotify:
		{
			XConfigureEvent configuration = event.xconfigure;
			OnSize(configuration.width, configuration.height);
			break;
		}
		case KeyPress:
		{
			XKeyEvent k = event.xkey;
			OnKeyPress(XLookupKeysym(&k, 0), k.state);
			break;
		}
		case FocusIn:
		case FocusOut:
		{
			XFocusChangeEvent focus = event.xfocus;
			switch(focus.type)
			{
				case FocusIn:	OnGainedFocus(focus.mode, focus.detail); break;
				case FocusOut:	OnLostFocus(focus.mode, focus.detail); break;
			}
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

void X11Window::OnGainedFocus(int mode, int detail)
{

}

void X11Window::OnLostFocus(int mode, int detail)
{

}

int XErrorFilter(Display* display, XErrorEvent* event)
{
	char errorText[128];
	XGetErrorText(display, event->error_code, errorText, sizeof errorText);

	LOG_ISSUE("XLib error: %s - Request Code = %hhu", errorText, event->request_code);

	return 0;
}

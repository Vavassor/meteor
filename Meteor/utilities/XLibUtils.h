#ifndef XLIB_UTILS_H
#define XLIB_UTILS_H

#include "String.h"

#include <X11/Xlib.h>

typedef int (*X_Error_Handler)(Display*, XErrorEvent*);
typedef int (*X_IO_Error_Handler)(Display*);

String xlib_error_text(Display* display, int code)
{
	char out[128];
	XGetErrorText(display, code, out, sizeof out);
	return String(out);
}

#endif

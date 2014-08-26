#ifndef GL_UTILS_H
#define GL_UTILS_H

#if defined(_MSC_VER)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>

#include <stddef.h>

const char* glerr_text(GLenum errorCode);
const char* gl_fbstatus_text(GLenum errorCode);
size_t size_of_pixel_type(GLenum type);
size_t size_of_pixel_format(GLenum format);

#endif

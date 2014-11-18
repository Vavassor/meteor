#ifndef GL_INFO
#define GL_INFO

#include "gl_core_3_3.h"

/* Windows SDK version of gl.h and other gl headers requires Windows.h
 * to be included first because it uses windows types and macros.
 * Only compilers using the Windows SDK version of OpenGL such as
 * Visual Studio have this dependency.
 * MinGW on Windows uses alternate versions of the gl headers.
 */
#if defined(_MSC_VER)
#include <Windows.h>
#endif

#include <GL/gl.h>
#include <GL/glext.h>

extern bool wgl_context_forward_compatible;
extern int gl_version, gl_max_texture_size;
extern float gl_max_texture_max_anisotropy_ext;
extern int glsl_version;
extern int gl_max_combined_texture_image_units;

#endif

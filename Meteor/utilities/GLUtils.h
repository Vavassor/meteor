#ifndef GL_UTILS_H
#define GL_UTILS_H

#include <stddef.h>

#include <gl/glew.h>

const char* glerr_text(GLenum errorCode);
const char* gl_fbstatus_text(GLenum errorCode);
size_t size_of_pixel_type(GLenum type);
size_t size_of_pixel_format(GLenum format);

#endif

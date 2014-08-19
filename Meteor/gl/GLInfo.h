#ifndef GL_INFO
#define GL_INFO

#define GLEW_STATIC
#include <gl/glew.h>

#include <gl/gl.h>
#include <gl/glext.h>

extern bool wgl_context_forward_compatible;
extern int gl_version, gl_max_texture_size;
extern float gl_max_texture_max_anisotropy_ext;
extern int glsl_version;
extern int gl_max_combined_texture_image_units;

#endif

#include "GLUtils.h"

const char* glerr_text(GLenum errorCode)
{
	switch(errorCode)
	{
		case GL_NO_ERROR:						return "NO_ERROR";
		case GL_INVALID_OPERATION:				return "INVALID_OPERATION";
		case GL_INVALID_ENUM:					return "INVALID_ENUM";
		case GL_INVALID_VALUE:					return "INVALID_VALUE";
		case GL_OUT_OF_MEMORY:					return "OUT_OF_MEMORY";
		case GL_INVALID_FRAMEBUFFER_OPERATION:	return "INVALID_FRAMEBUFFER_OPERATION";
		case GL_STACK_UNDERFLOW:				return "STACK_UNDERFLOW";
		case GL_STACK_OVERFLOW:					return "STACK_OVERFLOW";
	}
	return "";
}

const char* gl_fbstatus_text(GLenum errorCode)
{
	switch(errorCode)
	{
		case GL_FRAMEBUFFER_COMPLETE:						return "FRAMEBUFFER_COMPLETE";
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			return "FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	return "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:			return "FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:			return "FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
		case GL_FRAMEBUFFER_UNSUPPORTED:					return "FRAMEBUFFER_UNSUPPORTED";
	}
	return "UNKNOWN ERROR";
}

size_t size_of_pixel_type(GLenum type)
{
	switch(type)
	{
		case GL_UNSIGNED_BYTE: 
		case GL_BYTE:
		case GL_UNSIGNED_BYTE_3_3_2:
		case GL_UNSIGNED_BYTE_2_3_3_REV:
			return 1;

		case GL_UNSIGNED_SHORT:
		case GL_SHORT:
		case GL_UNSIGNED_SHORT_5_6_5:
		case GL_UNSIGNED_SHORT_5_6_5_REV:
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_4_4_4_4_REV:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_SHORT_1_5_5_5_REV:
			return 2;

		case GL_UNSIGNED_INT:
		case GL_INT:
		case GL_FLOAT:
		case GL_UNSIGNED_INT_8_8_8_8:
		case GL_UNSIGNED_INT_8_8_8_8_REV:
		case GL_UNSIGNED_INT_10_10_10_2:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			return 4;
	}
	return 0;
}

size_t size_of_pixel_format(GLenum format)
{
	switch(format)
	{
		case GL_DEPTH_COMPONENT:
		case GL_STENCIL_INDEX:
		case GL_RED: 
		case GL_GREEN:
		case GL_BLUE:
		case GL_RED_INTEGER:
		case GL_GREEN_INTEGER: 
		case GL_BLUE_INTEGER:
			return 1;

		case GL_DEPTH_STENCIL:
		case GL_RG: 
		case GL_RG_INTEGER:
		case GL_HALF_FLOAT:
			return 2;

		case GL_RGB:
		case GL_BGR:
		case GL_RGB_INTEGER:
		case GL_BGR_INTEGER:
			return 3;

		case GL_RGBA:
		case GL_BGRA:
		case GL_RGBA_INTEGER: 
		case GL_BGRA_INTEGER:
			return 4;
	}
	return 0;
}

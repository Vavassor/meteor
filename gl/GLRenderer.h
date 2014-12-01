#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include "GLInfo.h"

namespace GLRenderer
{
	bool Initialize();
	void Terminate();

	void Resize(int dimX, int dimY);
	void Render();
}

#endif

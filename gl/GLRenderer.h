#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include "GLInfo.h"

#include "../CameraData.h"

namespace GLRenderer
{
	bool Initialize();
	void Terminate();

	void Resize(int dimX, int dimY);
	void SetCameraState(const CameraData& camera);
	void Render();
}

#endif

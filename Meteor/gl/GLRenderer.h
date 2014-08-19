#ifndef GL_RENDERER_H
#define GL_RENDERER_H

#include "GLInfo.h"

#include <Windows.h>

#include <gl/wglew.h>
#include <gl/wglext.h>

#include "../CameraData.h"

namespace GLRenderer
{
	bool Initialize(HDC hDC, bool createForwardCompatibleContext);
	void Terminate();

	void SetVSync(bool enable);
	void Resize(int dimX, int dimY);
	void SetCameraState(const CameraData& camera);
	void Render();
}

#endif

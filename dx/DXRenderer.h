#ifndef DX_RENDERER_H
#define DX_RENDERER_H

#include "DXInfo.h"

#include "../utilities/GLMath.h"

namespace DXRenderer
{
	bool Initialize(HWND hWnd, bool isFullscreen, bool enableDebugging = false);
	void Terminate();

	void Resize(int dimX, int dimY);
	void SetVSync(bool enable);
	void SetCameraState(mat4x4 view, mat4x4 projection, bool isOrthographic);
	void Render();
}

#endif

#ifndef GL_GUI_H
#define GL_GUI_H

#include <string>

#include "GLMath.h"
#include "GLTexture.h"

namespace GUI
{
	struct Text
	{
		std::wstring text;
		int screenX, screenY, fontHandle;
		int layer;
		vec4 color;
	};

	struct Label
	{
		std::wstring text;
		vec3 position;
		quaternion orientation;
		vec4 color;
		int fontHandle;
		bool isBillboarded;
	};

	struct Panel
	{
		GLTexture texture;
		vec4 screenCoords;
		vec4 color;
		int layer;
	};

	void Initialize();
	void Terminate();
	void Resize(int dimX, int dimY);
	void RenderWorldGL(mat4x4 view, mat4x4 projection, mat4x4 cameraOrientation);
	void RenderScreenGL();
}

#endif
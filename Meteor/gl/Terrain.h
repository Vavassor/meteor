#ifndef TERRAIN_H
#define TERRAIN_H

#include "GLInfo.h"

namespace Terrain
{
	void Initialize();
	void Terminate();
	void Render();

	class Chunk
	{
	public:
		GLuint vertexArray;
		GLuint buffers[2];
		unsigned short numIndices;

		Chunk();
		void Create();
		void Destroy();
		void Render() const;
	};
}

#endif

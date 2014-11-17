#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#include "GLInfo.h"

#include "../utilities/String.h"

class GLTexture
{
public:
	GLuint textureID;
	int width, height;

	static void Initialize();
	static void Terminate();

	GLTexture();
	operator GLuint() const;

	bool Load(const String& fileName);
	void Unload();

private:
	struct TextureRecord
	{
		String fileName;
		GLTexture* record;
		int numOwners;
	};

	static const int MAX_NUM_TEXTURES = 50;
	static TextureRecord loadedTextures[MAX_NUM_TEXTURES];

	void SetDefaults();
	bool LoadImageData(const String& fileName);
	bool BufferData(GLubyte* data, int numComponents, const String& errorText);
};

#endif

#ifndef GL_FONT_H
#define GL_FONT_H

#include "GLTexture.h"

struct TexCoord
{
	float leftX, topY, width, height, row;
};

class GLFont
{
private:
	static const int NUM_CHAR_MAPPINGS = 73;

public:
	GLTexture texture;
	TexCoord* texCoords;
	int fontSpacing;

	static void Initialize();
	static void Terminate();

	GLFont();
	~GLFont();

	void LoadBitmapFont(const String& fontFile);
	void Unload();
	int GetCharMapping(char character) const;

private:
	static const char defaultCharacterMap[NUM_CHAR_MAPPINGS + 1];

	char* characterMap;
	int numCharMappings;
};

#endif

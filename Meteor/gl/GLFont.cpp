#include "GLFont.h"

#include "utilities/Logging.h"

#include "GlobalInfo.h"

#include <cstdlib>
#include <fstream>
#include <sstream>

using namespace std;

const char GLFont::defaultCharacterMap[NUM_CHAR_MAPPINGS + 1] = " 0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz\'\".,_-:/\\+";

void GLFont::Initialize()
{
}

void GLFont::Terminate()
{
}

GLFont::GLFont()
{
	fontSpacing = 2;
	characterMap = nullptr;
	numCharMappings = 0;
	texCoords = nullptr;
}

GLFont::~GLFont()
{
	if(texCoords != nullptr)
		delete[] texCoords;
	if(characterMap != defaultCharacterMap)
		delete[] characterMap;
}

void GLFont::LoadBitmapFont(const String& fontFile)
{
	String layoutFilePath = module_directory + fontFile + ".fnt";

	ifstream file(layoutFilePath.Data(), ios::in);
	if(!file.is_open())
	{
		Log::Add(Log::ISSUE, "Font Error: bitmap font file: %s failed to load", fontFile.Data());
		return;
	}

	string fontSubpath = string(fontFile.Data());
	fontSubpath = fontSubpath.substr(0, fontSubpath.find_last_of('/') + 1);
	
	int fontHeight = 0;
	string imageFile;

	int glyphIndex = 0;
	string line;
	while(getline(file, line))
	{
		istringstream lineStream(line);
		string next;
		getline(lineStream, next, ' ');
	
		if(next == "char")
		{
			getline(lineStream, next, ' ');
			next = next.substr(next.find_first_of('=') + 1);
			wchar_t glyphMap = atol(next.c_str());
			characterMap[glyphIndex] = glyphMap;

			TexCoord& coord = texCoords[glyphIndex++];

			lineStream >> next;
			next = next.substr(next.find_first_of('=') + 1);
			coord.leftX = atol(next.c_str());

			lineStream >> next;
			next = next.substr(next.find_first_of('=') + 1);
			coord.topY = texture.height - atol(next.c_str());

			lineStream >> next;
			next = next.substr(next.find_first_of('=') + 1);
			coord.width = atol(next.c_str());

			lineStream >> next;
			next = next.substr(next.find_first_of('=') + 1);
			coord.height = atol(next.c_str());

			lineStream >> next;
			lineStream >> next;
			next = next.substr(next.find_first_of('=') + 1);
			int offset = atol(next.c_str());
			coord.row = coord.topY + offset;
		}
		else if(next == "info")
		{
			getline(lineStream, next, ' ');
			getline(lineStream, next, ' ');
			next = next.substr(next.find_first_of('=') + 1);
			fontHeight = atol(next.c_str());
		}
		else if(next == "page")
		{
			getline(lineStream, next, ' ');
			getline(lineStream, next, ' ');
			size_t beginName = next.find_first_of('"') + 1;
			imageFile = next.substr(beginName, next.find_last_of('"') - beginName);

			string imagePath = fontSubpath + imageFile;
			bool bitmapLoaded = texture.Load(imagePath.c_str());
			if(!bitmapLoaded)
			{
				Log::Add(Log::ISSUE, "Font Error: bitmap font texture: %s failed to load",
					imageFile.c_str());
				break;
			}
		}
		else if(next == "chars")
		{
			getline(lineStream, next, ' ');
			next = next.substr(next.find_first_of('=') + 1);
			long numChars = atol(next.c_str());
			texCoords = new TexCoord[numChars];
			characterMap = new char[numChars];
			numCharMappings = numChars;
		}
	}
	file.close();
}

void GLFont::Unload()
{
	texture.Unload();
}

int GLFont::GetCharMapping(char character) const
{
	for(int i = 0; i < numCharMappings; i++)
	{
		if(characterMap[i] == character) return i;
	}
	return 0;
}

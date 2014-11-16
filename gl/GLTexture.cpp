#include "GLTexture.h"

#include "utilities/Logging.h"
#include "utilities/Maths.h"
#include "utilities/stb_image.h"

GLTexture::TextureRecord GLTexture::loadedTextures[MAX_NUM_TEXTURES];

void GLTexture::Initialize()
{
	for(int i = 0; i < MAX_NUM_TEXTURES; i++)
	{
		loadedTextures[i].record = nullptr;
		loadedTextures[i].numOwners = 0;
	}
}

void GLTexture::Terminate()
{
}

GLTexture::GLTexture():
	textureID(0),
	width(0),
	height(0)
{}

void GLTexture::SetDefaults()
{
	textureID = 0;
	width = height = 0;
}

GLTexture::operator GLuint() const
{
	return textureID;
}

bool GLTexture::Load(const String& fileName)
{
	unsigned id = 0;
	for(int i = 0; i < MAX_NUM_TEXTURES; i++)
	{
		GLTexture* tex = loadedTextures[i].record;
		if(tex == nullptr) continue;

		if(tex->textureID != 0 && loadedTextures[i].fileName == fileName)
		{
			id = tex->textureID;
			width = tex->width;
			height = tex->height;
			loadedTextures[i].numOwners++;
			break;
		}
	}

	if(id == 0)
	{
		bool loadSuccess = LoadImageData(fileName);
		if(loadSuccess)
		{
			for(int i = 0; i < MAX_NUM_TEXTURES; i++)
			{
				GLTexture* tex = loadedTextures[i].record;
				if(tex != nullptr) continue;

				loadedTextures[i].fileName = fileName;
				loadedTextures[i].record = this;
				loadedTextures[i].numOwners = 1;
				break;
			}
		}
		return loadSuccess;
	}

	textureID = id;
	return true;
}

bool GLTexture::LoadImageData(const String& fileName)
{
	String errorText("Error loading file ");
	errorText.Append(fileName);
	errorText.Append("! -> ");

	FILE* file = fopen(fileName.Data(), "rb");
	if(file == nullptr)
	{
		LOG_ISSUE("%s : file could not be opened", errorText.Data());
		fclose(file);
		return false;
	}

	int numComponents;
	unsigned char* data = stbi_load_from_file(file, &width, &height, &numComponents, 0);
	if(data == nullptr)
	{
		LOG_ISSUE("%s STB IMAGE ERROR: %s", errorText.Data(), stbi_failure_reason());
		fclose(file);
		return false;
	}

	fclose(file);

	if(!GLEW_ARB_texture_non_power_of_two)
	{
		/*
		width = get_next_power_of_two(width);
		height = get_next_power_of_two(height);
		*/
	}

	int maxTextureSize = gl_max_texture_size;
	if(width > maxTextureSize) width = maxTextureSize;
	if(height > maxTextureSize) height = maxTextureSize;

	bool loaded = BufferData(data, numComponents, errorText);

	stbi_image_free(data);

	return loaded;
}

bool GLTexture::BufferData(GLubyte* data, int numComponents, const String& errorText)
{
	if(data == NULL)
	{
		Log::Add(Log::ISSUE, "%s could not buffer: image Data is empty", errorText.Data());
		return false;
	}

	GLenum Format = 0;

	if(numComponents == 4) Format = GL_RGBA;
	if(numComponents == 3) Format = GL_RGB;

	if(Format == 0)
	{
		Log::Add(Log::ISSUE, "%s could not buffer: image Format is 0", errorText.Data());
		return false;
	}

	glGenTextures(1, &textureID);

	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	if(GLEW_EXT_texture_filter_anisotropic)
	{
		float anisotropy = 16.0f;
		if(anisotropy > gl_max_texture_max_anisotropy_ext)
			anisotropy = gl_max_texture_max_anisotropy_ext;
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropy);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, Format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}

void GLTexture::Unload()
{
	for(int i = 0; i < MAX_NUM_TEXTURES; i++)
	{
		GLTexture* tex = loadedTextures[i].record;
		if(tex == nullptr) continue;
		if(tex->textureID != textureID) continue;

		if(loadedTextures[i].numOwners <= 1)
		{
			glDeleteTextures(1, &textureID);

			loadedTextures[i].numOwners = 0;
			loadedTextures[i].record = nullptr;
			loadedTextures[i].fileName.Clear();
		}
		else loadedTextures[i].numOwners--;
		break;
	}

	SetDefaults();
}

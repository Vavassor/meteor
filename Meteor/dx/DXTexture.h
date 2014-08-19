#ifndef DX_TEXTURE_H
#define DX_TEXTURE_H

#include "DXInfo.h"
#include "BString.h"

class DXTexture
{
public:
	ID3D11Texture2D* texture;
	ID3D11ShaderResourceView* textureView;
	ID3D11SamplerState* sampler;

	int width, height;

	static void Initialize();
	static void Terminate();

	DXTexture();
	bool Load(const String& fileName);
	void Unload();

private:
	struct TextureRecord
	{
		String fileName;
		DXTexture* record;
		int numOwners;
	};

	static const int MAX_NUM_TEXTURES = 50;
	static TextureRecord loadedTextures[MAX_NUM_TEXTURES];

	void SetDefaults();
	bool LoadTextureData(const String& fileName);
	bool LoadDataToDX(BYTE* data, int bpp, int pitch, const String& errorText);
};

#endif

#ifndef DX_SHADER_H
#define DX_SHADER_H

#include "DXInfo.h"
#include "ShaderConstant.h"
#include "DXTexture.h"

#include "../utilities/GLMath.h"
#include "../utilities/String.h"

class DXShader
{
public:
	ID3D11VertexShader* vertexShader;
	ID3D11PixelShader* pixelShader;
	ID3D11InputLayout* inputLayout;
	ID3D11Buffer* vertCB;
	ID3D11Buffer* pixelCB;

	int layoutType;

	DXShader();
	bool Load(const String& vertexFileName, const String& pixelFileName);
	void Unload();
	void Bind() const;
	void SetTexture(unsigned slot, const DXTexture& texture) const;
	void SetConstantMatrix(const String& name, int shaderType, const mat4x4& matrix);
	void SetConstantVec4(const String& name, int shaderType, const vec4& vec);
	void UpdateConstants();

private:
	static const int MAX_VERTEX_CONSTANTS = 3;
	static const int MAX_PIXEL_CONSTANTS = 1;

	static float vertBuffer[];
	static float pixelBuffer[];

	ShaderConstant vertexConstants[MAX_VERTEX_CONSTANTS];
	int numVertexConstants;
	ShaderConstant pixelConstants[MAX_PIXEL_CONSTANTS];
	int numPixelConstants;

	void SetDefaults();
	ShaderConstant* GetConstant(const String& name, int shaderType) const;
};

#endif

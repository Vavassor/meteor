#include "GLGUI.h"

#include "GLFont.h"
#include "GLPrimitives.h"
#include "GLShader.h"
#include "GLTexture.h"

#include "../utilities/Macros.h"

using namespace std;

namespace GUI
{
	static const int MAX_FONTS = 8;
	static const int MAX_LAYERS = 8;

	GLFont fonts[MAX_FONTS];

	mat4x4 screenProjection;

	GLShader fontShader;
	GLShader panelShader;

	void RenderStringGL(const wstring& text, int fontHandle);
}

void GUI::Initialize()
{
	screenProjection = MAT_I;

	GLFont::Initialize();

	fonts[0].LoadBitmapFont("fonts/GatsbyFLF");

	panelShader.Load("default_vertex.vert", "default_frag.frag");
	panelShader.Bind();
	panelShader.AddAttribute("position");
	panelShader.AddAttribute("texcoord");
	panelShader.SetUniformInt("texture", 0);

	fontShader.Load("default_vertex.vert", "default_frag.frag");
	fontShader.Bind();
	fontShader.AddAttribute("position");
	fontShader.AddAttribute("texcoord");
	fontShader.SetUniformInt("texture", 0);
}

void GUI::Terminate()
{
	panelShader.Unload();
	fontShader.Unload();

	for(int i = 0; i < MAX_FONTS; i++)
		fonts[i].Unload();

	GLFont::Terminate();
}

void GUI::Resize(int dimX, int dimY)
{
	screenProjection = orthogonal_projection_matrix(0, dimX, 0, dimY, -1.0f, 1.0f);
}

void GUI::RenderStringGL(const wstring& text, int fontHandle)
{
	int stringLength = text.size();
	vec4* verts = new vec4[stringLength * 4];
	vec2* coords = new vec2[stringLength * 4];

	float stringX = 0;
	float stringY = 0;
	const float depth = 1.0f;

	GLFont& font = fonts[fontHandle];
	for(int i = 0; i < stringLength; i++)
	{
		if(text[i] == L' ')
			stringX += font.fontSpacing * 4;

		int coordInd = font.GetCharMapping(text[i]);
		float leftTex = font.texCoords[coordInd].leftX / static_cast<float>(font.texture.width);
		float topTex = font.texCoords[coordInd].topY / static_cast<float>(font.texture.height);
		float rightTex = (font.texCoords[coordInd].leftX + font.texCoords[coordInd].width) / static_cast<float>(font.texture.width);
		float bottomTex = (font.texCoords[coordInd].topY - font.texCoords[coordInd].height) / static_cast<float>(font.texture.height);

		stringY = font.texCoords[coordInd].topY - font.texCoords[coordInd].height - font.texCoords[coordInd].row;

		coords[i*4+0] = vec2(leftTex, topTex);
		verts[i*4+0] = vec4(stringX, stringY + font.texCoords[coordInd].height, depth, 1.0f);

		coords[i*4+1] = vec2(leftTex, bottomTex);
		verts[i*4+1] = vec4(stringX, stringY, depth, 1.0f);

		coords[i*4+2] = vec2(rightTex, bottomTex);
		verts[i*4+2] = vec4(stringX + font.texCoords[coordInd].width, stringY, depth, 1.0f);

		coords[i*4+3] = vec2(rightTex, topTex);
		verts[i*4+3] = vec4(stringX + font.texCoords[coordInd].width, stringY + font.texCoords[coordInd].height, depth, 1.0f);

		stringX += font.texCoords[coordInd].width + font.fontSpacing;
	}
	GLPrimitives::AddQuads(verts, coords, stringLength * 4);
	GLPrimitives::Draw();

	delete[] verts;
	delete[] coords;
}

void GUI::RenderWorldGL(mat4x4 view, mat4x4 projection, mat4x4 cameraOrientation)
{
	/** LABEL PASS **/
	fontShader.Bind();
	/*for(int i = 0; i < MAX_LABELS; i++)
	{
		quaternion orientation = labels[i].orientation;
		mat4x4 rotMatrix = quat_to_matrix(orientation);
		if(labels[i].isBillboarded)
			rotMatrix = rotMatrix * cameraOrientation;

		vec3 position = labels[i].position;
		mat4x4 transMatrix = TranslationMatrix(position.x, position.y, position.z);
		mat4x4 modelMatrix = transMatrix * rotMatrix;

		fontShader.SetConstantMatrix("model", modelMatrix);
		fontShader.SetConstantVec4("color", labels[i].color);

		int font = labels[i].fontHandle;
		fontShader.SetTexture(0, fonts[font].texture);

		RenderStringGL(labels[i].text, font);
	}*/
}

void GUI::RenderScreenGL()
{
	/** TEXT PASS **/
	fontShader.Bind();
	/*for(int i = 0; i < MAX_TEXTS; i++)
	{
		mat4x4 modelMatrix = TranslationMatrix(texts[i].screenX, texts[i].screenY, 0.0f);
		fontShader.SetConstantMatrix("model", modelMatrix);

		int font = texts[i].fontHandle;
		fontShader.SetTexture(0, fonts[font].texture);
		RenderStringGL(texts[i].text, font);
	}*/

	/** PANELS PASS **/
	panelShader.Bind();
	/*for(int i = 0; i < MAX_LAYERS; i++)
	{
		for(int j = 0; j < MAX_PANELS; j++)
		{
			if(panels[j].layer != i)
				continue;
			
			panelShader.SetConstantVec4("color", panels[j].color);
			panelShader.SetTexture(0, panels[j].glTexture);

			vec4 coords = panels[j].screenCoords;
			const vec4 vertices[] = 
			{
				vec4(coords.x, coords.y + coords.w, 0.0f, 1.0f),
				vec4(coords.x, coords.y, 0.0f, 1.0f),
				vec4(coords.x + coords.z, coords.y, 0.0f, 1.0f),
				vec4(coords.x + coords.z, coords.y + coords.w, 0.0f, 1.0f),
			};
			const vec2 texcoords[] =
			{
				vec2(0.0f, 1.0f),
				vec2(0.0f, 0.0f),
				vec2(1.0f, 0.0f),
				vec2(1.0f, 1.0f),
			};
			GLPrimitives::DrawQuads(vertices, texcoords, ARRAY_LENGTH(vertices));
		}
	}*/
}

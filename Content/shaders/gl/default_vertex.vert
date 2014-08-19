#version 150
uniform layout(std140);

uniform ObjectBlock
{
	mat4 model_view_projection;
};

in vec4 position;
in vec2 textureCoordinate;

out vec2 texCoord;

void main(void)
{
	texCoord = textureCoordinate;
	gl_Position = model_view_projection * position;
}
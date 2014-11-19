#version 150
uniform layout(std140);

uniform MaterialBlock
{
	vec4 color;
};

uniform sampler2D texture;

in vec2 texCoord;
out vec4 outputColor;

void main()
{
    outputColor = color * texture2D(texture, texCoord);
}
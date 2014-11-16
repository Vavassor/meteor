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
	vec4 fragColor = color * texture2D(texture, texCoord);
	if(fragColor.a < 0.4) discard;
    outputColor = fragColor;
}
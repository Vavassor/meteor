#version 130
uniform sampler2D texture;

varying vec4 vertexColor;
varying vec2 texcoord;

void main()
{
	vec4 fragColor = texture2D(texture, texcoord) * vertexColor;
	if(fragColor.a < 0.4)
		discard;
	gl_FragColor = fragColor;
}
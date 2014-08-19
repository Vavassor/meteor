#version 130
uniform sampler2D texture;

varying vec4 vertexColor;
varying vec2 texcoord;

void main()
{
    gl_FragColor = texture2D(texture, texcoord) * vertexColor;
}
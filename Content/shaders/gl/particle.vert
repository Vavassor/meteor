#version 130
uniform mat4x4 model;
uniform mat4x4 view;
uniform mat4x4 projection;

attribute vec4 aPosition;
attribute vec4 aColor;
attribute vec2 aTexcoord;

varying vec4 vertexColor;
varying vec2 texcoord;

void main(void)
{
	vertexColor = aColor;
	texcoord = aTexcoord.xy;
	gl_Position = projection * view * model * aPosition;
}